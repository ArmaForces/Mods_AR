/**
* Server-side GameSystem that manages civilian population spawning and
* waypoint assignment based on registered AFM_CivPopEntity points of interest.
*
* Uses event-driven waypoint completion for behavior cycling:
*   1) LOCAL_PATROL - walk within the current PoI area
*   2) TRAVEL - get in vehicle, drive to a connected PoI, get out, patrol there
*
* Configuration is read from AFM_CivPopComponent attached to the game mode.
*/

// Movement behavior types
enum AFM_ECivPopBehavior
{
	IDLE,
	LOCAL_PATROL,
	TRAVEL
}

// Tracks per-group civilian state
class AFM_CivPopGroupData
{
	EntityID m_GroupId;
	AFM_CivPopEntity m_CurrentPoI;
	AFM_ECivPopBehavior m_eBehavior;
	ref AFM_CivPopWaypointHandler m_WaypointHandler;

	void AFM_CivPopGroupData(EntityID groupId, AFM_CivPopEntity poi)
	{
		m_GroupId = groupId;
		m_CurrentPoI = poi;
		m_eBehavior = AFM_ECivPopBehavior.IDLE;
	}
}

// Captures group ID so the waypoint-completion callback can identify which group finished
class AFM_CivPopWaypointHandler
{
	protected EntityID m_GroupId;
	protected AFM_CivPopSystem m_System;

	void AFM_CivPopWaypointHandler(EntityID groupId, AFM_CivPopSystem system)
	{
		m_GroupId = groupId;
		m_System = system;
	}

	void OnWaypointCompleted(AIWaypoint waypoint)
	{
		if (m_System)
			m_System.HandleWaypointCompleted(m_GroupId, waypoint);
	}

	EntityID GetGroupId()
	{
		return m_GroupId;
	}
}

class AFM_CivPopSystem : GameSystem
{
	// Update frequency for cleanup / idle checks
	protected const float UPDATE_INTERVAL = 5.0;
	protected float m_fUpdateTimer;
	protected bool m_bIsActive;

	// Registered Points of Interest
	protected ref array<AFM_CivPopEntity> m_aPoIs = {};

	// Per-group tracking
	protected ref map<EntityID, ref AFM_CivPopGroupData> m_mGroupData = new map<EntityID, ref AFM_CivPopGroupData>();

	// Configuration reference
	protected AFM_CivPopComponent m_pConfig;

	// Tuning
	protected const float LOCAL_PATROL_CHANCE = 0.3;
	protected const float MIN_WAIT_TIME = 2.0;
	protected const float MAX_WAIT_TIME = 15.0;

	// Random generator
	protected static ref RandomGenerator s_RandomGenerator = new RandomGenerator();

	//------------------------------------------------------------------------------------------------
	// System metadata - server-only, runs on FixedFrame
	//------------------------------------------------------------------------------------------------
	override static void InitInfo(WorldSystemInfo outInfo)
	{
		outInfo
			.SetAbstract(false)
			.SetUnique(true)
			.SetLocation(ESystemLocation.Server)
			.AddPoint(ESystemPoint.FixedFrame);
	}

	//------------------------------------------------------------------------------------------------
	static AFM_CivPopSystem GetInstance()
	{
		World world = GetGame().GetWorld();
		if (!world)
			return null;

		return AFM_CivPopSystem.Cast(world.FindSystem(AFM_CivPopSystem));
	}

	//------------------------------------------------------------------------------------------------
	override event bool ShouldBePaused()
	{
		return true;
	}

	//------------------------------------------------------------------------------------------------
	// PoI registration (called by AFM_CivPopEntity)
	//------------------------------------------------------------------------------------------------

	void RegisterPoI(notnull AFM_CivPopEntity poi)
	{
		if (m_aPoIs.Contains(poi))
			return;

		m_aPoIs.Insert(poi);
		PrintFormat("[AFM_CivPop] PoI registered: %1 (type %2, radius %3 m). Total: %4",
			poi, typename.EnumToString(AFM_ECivPopPoIType, poi.GetPoIType()),
			poi.GetLocationRadius(), m_aPoIs.Count());
	}

	//------------------------------------------------------------------------------------------------
	void UnregisterPoI(notnull AFM_CivPopEntity poi)
	{
		m_aPoIs.RemoveItem(poi);
	}

	//------------------------------------------------------------------------------------------------
	// Public API
	//------------------------------------------------------------------------------------------------

	//! Starts the system. Call after the game mode / configuration is ready.
	void StartSystem()
	{
		IEntity gameMode = GetGame().GetGameMode();
		if (gameMode)
			m_pConfig = AFM_CivPopComponent.Cast(gameMode.FindComponent(AFM_CivPopComponent));

		if (!m_pConfig)
		{
			Print("[AFM_CivPop] No AFM_CivPopComponent on GameMode - system will not start.", LogLevel.WARNING);
			return;
		}

		m_bIsActive = true;
		Enable(true);

		PrintFormat("[AFM_CivPop] System started. Max civilians: %1, tick: %2 s",
			m_pConfig.GetMaxCivilianCount(), m_pConfig.GetSpawnTickInterval());
	}

	//------------------------------------------------------------------------------------------------
	//! Stops the system and cleans up all tracked groups.
	void StopSystem()
	{
		m_bIsActive = false;
		Enable(false);

		foreach (EntityID groupId, AFM_CivPopGroupData data : m_mGroupData)
		{
			UnsubscribeFromGroup(groupId);
		}

		m_mGroupData.Clear();
		Print("[AFM_CivPop] System stopped.");
	}

	//------------------------------------------------------------------------------------------------
	bool IsSystemActive()
	{
		return m_bIsActive;
	}

	//------------------------------------------------------------------------------------------------
	int GetManagedGroupCount()
	{
		return m_mGroupData.Count();
	}

	//------------------------------------------------------------------------------------------------
	array<AFM_CivPopEntity> GetPoIs()
	{
		return m_aPoIs;
	}

	//------------------------------------------------------------------------------------------------
	// Main update loop
	//------------------------------------------------------------------------------------------------
	override event protected void OnUpdatePoint(WorldUpdatePointArgs args)
	{
		if (!m_bIsActive)
			return;

		m_fUpdateTimer += args.GetTimeSliceSeconds();

		if (m_fUpdateTimer < UPDATE_INTERVAL)
			return;

		m_fUpdateTimer = 0;

		CleanupDeadGroups();
		SpawnCivilians();
		CheckIdleGroups();
	}

	//------------------------------------------------------------------------------------------------
	// Spawning
	//------------------------------------------------------------------------------------------------

	//! Returns the max number of civilian groups a PoI type can have.
	//! Non-settlement PoIs return 0 — they are destinations only, not spawn sources.
	protected int GetPopulationCapForPoIType(AFM_ECivPopPoIType type)
	{
		switch (type)
		{
			case AFM_ECivPopPoIType.LARGE_CITY: return 8;
			case AFM_ECivPopPoIType.TOWN:       return 3;
		}

		// CAFE, MARKET, GAS_STATION, CHURCH, PARK — visit-only, no spawning
		return 0;
	}

	//------------------------------------------------------------------------------------------------
	//! Counts how many tracked groups currently belong to the given PoI.
	protected int CountGroupsAtPoI(AFM_CivPopEntity poi)
	{
		int count;
		foreach (EntityID id, AFM_CivPopGroupData data : m_mGroupData)
		{
			if (data.m_CurrentPoI == poi)
				count++;
		}
		return count;
	}

	//------------------------------------------------------------------------------------------------
	//! Spawn new groups at PoIs close to any player, up to the configured limit.
	//! Only settlement-type PoIs (TOWN, LARGE_CITY) produce civilians.
	//! Each PoI has its own population cap based on type.
	protected void SpawnCivilians()
	{
		if (m_aPoIs.IsEmpty())
			return;

		foreach (AFM_CivPopEntity poi : m_aPoIs)
		{
			// Global cap
			if (m_mGroupData.Count() >= m_pConfig.GetMaxCivilianCount())
				break;

			// Only settlements spawn civilians
			int poiCap = GetPopulationCapForPoIType(poi.GetPoIType());
			if (poiCap <= 0)
				continue;

			// Per-PoI cap
			int currentAtPoI = CountGroupsAtPoI(poi);
			if (currentAtPoI >= poiCap)
				continue;

			SpawnGroupAtPoI(poi);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Spawns a civilian group at the PoI and registers it for movement management.
	protected void SpawnGroupAtPoI(notnull AFM_CivPopEntity poi)
	{
		array<ResourceName> groupPrefabs = m_pConfig.GetCivilianGroupPrefabs();
		if (!groupPrefabs || groupPrefabs.IsEmpty())
		{
			Print("[AFM_CivPop] No civilian group prefabs configured.", LogLevel.WARNING);
			return;
		}

		ResourceName prefab = groupPrefabs.GetRandomElement();
		if (prefab.IsEmpty())
			return;

		vector spawnPos = GetRandomPositionInRadius(poi.GetPoIPosition(), poi.GetLocationRadius());
		IEntity group = SpawnEntityPrefab(prefab, spawnPos);
		if (!group)
		{
			Print("[AFM_CivPop] Failed to spawn civilian group.", LogLevel.ERROR);
			return;
		}

		RegisterGroup(group, poi);

		PrintFormat("[AFM_CivPop] Spawned group at PoI %1 (%2). Active: %3",
			poi, typename.EnumToString(AFM_ECivPopPoIType, poi.GetPoIType()),
			m_mGroupData.Count(), level: LogLevel.DEBUG);
	}

	//------------------------------------------------------------------------------------------------
	// Group registration
	//------------------------------------------------------------------------------------------------

	//! Registers a spawned group and subscribes to its waypoint events.
	void RegisterGroup(notnull IEntity groupEntity, notnull AFM_CivPopEntity poi)
	{
		SCR_AIGroup aiGroup = SCR_AIGroup.Cast(groupEntity);
		if (!aiGroup)
		{
			Print("[AFM_CivPop] Entity is not SCR_AIGroup - cannot register.", LogLevel.ERROR);
			return;
		}

		EntityID groupId = groupEntity.GetID();

		AFM_CivPopGroupData data = new AFM_CivPopGroupData(groupId, poi);
		AFM_CivPopWaypointHandler handler = new AFM_CivPopWaypointHandler(groupId, this);
		data.m_WaypointHandler = handler;

		m_mGroupData.Set(groupId, data);

		aiGroup.GetOnWaypointCompleted().Insert(handler.OnWaypointCompleted);

		AssignNewBehavior(groupId);
	}

	//------------------------------------------------------------------------------------------------
	protected void UnsubscribeFromGroup(EntityID groupId)
	{
		AFM_CivPopGroupData data = m_mGroupData.Get(groupId);
		if (!data || !data.m_WaypointHandler)
			return;

		IEntity ent = GetGame().GetWorld().FindEntityByID(groupId);
		if (!ent)
			return;

		SCR_AIGroup aiGroup = SCR_AIGroup.Cast(ent);
		if (aiGroup)
			aiGroup.GetOnWaypointCompleted().Remove(data.m_WaypointHandler.OnWaypointCompleted);
	}

	//------------------------------------------------------------------------------------------------
	// Waypoint completion handler (called via AFM_CivPopWaypointHandler)
	//------------------------------------------------------------------------------------------------

	void HandleWaypointCompleted(EntityID groupId, AIWaypoint waypoint)
	{
		AFM_CivPopGroupData data = m_mGroupData.Get(groupId);
		if (!data)
			return;

		IEntity ent = GetGame().GetWorld().FindEntityByID(groupId);
		if (!ent)
		{
			m_mGroupData.Remove(groupId);
			return;
		}

		SCR_AIGroup aiGroup = SCR_AIGroup.Cast(ent);
		if (!aiGroup)
			return;

		array<AIWaypoint> remaining = {};
		aiGroup.GetWaypoints(remaining);

		if (remaining.IsEmpty())
			AssignNewBehavior(groupId);
	}

	//------------------------------------------------------------------------------------------------
	// Behavior assignment
	//------------------------------------------------------------------------------------------------

	protected void AssignNewBehavior(EntityID groupId)
	{
		AFM_CivPopGroupData data = m_mGroupData.Get(groupId);
		if (!data)
			return;

		IEntity ent = GetGame().GetWorld().FindEntityByID(groupId);
		if (!ent)
		{
			m_mGroupData.Remove(groupId);
			return;
		}

		SCR_AIGroup aiGroup = SCR_AIGroup.Cast(ent);
		if (!aiGroup)
			return;

		ClearWaypoints(aiGroup);
		float roll = s_RandomGenerator.RandFloat01();

		if (roll < LOCAL_PATROL_CHANCE)
		{
			AssignLocalPatrol(aiGroup, data);
		}
		else
		{
			AssignTravelBehavior(aiGroup, data);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Walk to a random point inside the current PoI, then wait.
	protected void AssignLocalPatrol(SCR_AIGroup aiGroup, AFM_CivPopGroupData data)
	{
		data.m_eBehavior = AFM_ECivPopBehavior.LOCAL_PATROL;
		AFM_CivPopEntity poi = data.m_CurrentPoI;

		vector targetPos = GetRandomPositionInRadius(poi.GetPoIPosition(), poi.GetLocationRadius());

		AIWaypoint loiterWp = SpawnWaypoint(m_pConfig.GetWaypointLoiterPrefab(), targetPos);
		if (loiterWp)
		{
			loiterWp.SetCompletionRadius(poi.GetLocationRadius());
			aiGroup.AddWaypoint(loiterWp);
		}

		float waitTime = s_RandomGenerator.RandFloatXY(MIN_WAIT_TIME, MAX_WAIT_TIME);
		SCR_TimedWaypoint waitWp = SCR_TimedWaypoint.Cast(SpawnWaypoint(m_pConfig.GetWaypointWaitPrefab(), targetPos));
		if (waitWp)
		{
			waitWp.SetHoldingTime(waitTime);
			aiGroup.AddWaypoint(waitWp);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Travel to a connected PoI via vehicle: GetIn -> Move -> GetOut -> Loiter.
	protected void AssignTravelBehavior(SCR_AIGroup aiGroup, AFM_CivPopGroupData data)
	{
		data.m_eBehavior = AFM_ECivPopBehavior.TRAVEL;

		array<AFM_CivPopEntity> connected = m_aPoIs;
		if (!connected || connected.IsEmpty())
		{
			AssignLocalPatrol(aiGroup, data);
			return;
		}

		AFM_CivPopEntity targetPoI = connected.GetRandomElement();
		if (!targetPoI)
		{
			AssignLocalPatrol(aiGroup, data);
			return;
		}

		vector currentPos = aiGroup.GetOrigin();
		vector targetPos = targetPoI.GetPoIPosition();

		// 1. Get in vehicle
		AIWaypoint getInWp = SpawnWaypoint(m_pConfig.GetWaypointGetInPrefab(), currentPos);
		if (getInWp)
			aiGroup.AddWaypoint(getInWp);

		// 2. Move to target PoI
		AIWaypoint moveWp = SpawnWaypoint(m_pConfig.GetWaypointMovePrefab(), targetPos);
		if (moveWp)
			aiGroup.AddWaypoint(moveWp);

		// 3. Get out of vehicle
		AIWaypoint getOutWp = SpawnWaypoint(m_pConfig.GetWaypointGetOutPrefab(), targetPos);
		if (getOutWp)
			aiGroup.AddWaypoint(getOutWp);

		// 4. Loiter in new PoI
		vector patrolPos = GetRandomPositionInRadius(targetPos, targetPoI.GetLocationRadius());
		AIWaypoint loiterWp = SpawnWaypoint(m_pConfig.GetWaypointLoiterPrefab(), patrolPos);
		if (loiterWp)
		{
			loiterWp.SetCompletionRadius(targetPoI.GetLocationRadius());
			aiGroup.AddWaypoint(loiterWp);
		}

		// Update current PoI for next cycle
		data.m_CurrentPoI = targetPoI;
	}

	//------------------------------------------------------------------------------------------------
	// Cleanup
	//------------------------------------------------------------------------------------------------

	//! Remove tracking entries whose entity no longer exists.
	protected void CleanupDeadGroups()
	{
		array<EntityID> toRemove = {};

		foreach (EntityID groupId, AFM_CivPopGroupData data : m_mGroupData)
		{
			IEntity ent = GetGame().GetWorld().FindEntityByID(groupId);
			if (!ent)
				toRemove.Insert(groupId);
		}

		foreach (EntityID id : toRemove)
		{
			m_mGroupData.Remove(id);
		}

		if (!toRemove.IsEmpty())
			PrintFormat("[AFM_CivPop] Cleaned up %1 dead groups, %2 remaining", toRemove.Count(), m_mGroupData.Count());
	}

	//------------------------------------------------------------------------------------------------
	//! Detect groups with no waypoints and give them a new behavior.
	protected void CheckIdleGroups()
	{
		int idleCount;

		foreach (EntityID groupId, AFM_CivPopGroupData data : m_mGroupData)
		{
			IEntity ent = GetGame().GetWorld().FindEntityByID(groupId);
			SCR_AIGroup aiGroup = SCR_AIGroup.Cast(ent);
			if (!aiGroup)
				continue;

			array<AIWaypoint> wps = {};
			aiGroup.GetWaypoints(wps);

			if (wps.IsEmpty())
			{
				AssignNewBehavior(groupId);
				idleCount++;
			}
		}

		if (idleCount > 0)
			PrintFormat("[AFM_CivPop] Re-tasked %1 idle groups", idleCount, level: LogLevel.DEBUG);
	}


	//------------------------------------------------------------------------------------------------
	// Waypoint helpers
	//------------------------------------------------------------------------------------------------

	//! Spawn a waypoint entity from a prefab at the given position.
	protected AIWaypoint SpawnWaypoint(ResourceName prefab, vector pos)
	{
		if (prefab.IsEmpty())
			return null;

		IEntity ent = SpawnEntityPrefab(prefab, pos);
		if (!ent)
			return null;

		AIWaypoint wp = AIWaypoint.Cast(ent);
		if (!wp)
		{
			SCR_EntityHelper.DeleteEntityAndChildren(ent);
			return null;
		}

		return wp;
	}

	//------------------------------------------------------------------------------------------------
	protected void ClearWaypoints(SCR_AIGroup aiGroup)
	{
		array<AIWaypoint> waypoints = {};
		aiGroup.GetWaypoints(waypoints);

		foreach (AIWaypoint wp : waypoints)
		{
			aiGroup.RemoveWaypoint(wp);
			delete wp;
		}
	}

	//------------------------------------------------------------------------------------------------
	// General helpers
	//------------------------------------------------------------------------------------------------

	//! Spawn a prefab entity at a world position.
	protected IEntity SpawnEntityPrefab(ResourceName prefab, vector pos)
	{
		Resource res = Resource.Load(prefab);
		if (!res || !res.IsValid())
		{
			PrintFormat("[AFM_CivPop] Failed to load prefab: %1", prefab, level: LogLevel.ERROR);
			return null;
		}

		EntitySpawnParams params = EntitySpawnParams();
		params.TransformMode = ETransformMode.WORLD;
		params.Transform[3] = pos;

		return GetGame().SpawnEntityPrefab(res, GetGame().GetWorld(), params);
	}


	//------------------------------------------------------------------------------------------------
	//! Returns a random position on the XZ plane within radius, snapped to terrain.
	protected vector GetRandomPositionInRadius(vector center, float radius)
	{
		float angle = s_RandomGenerator.RandFloatXY(0, Math.PI2);
		float dist = s_RandomGenerator.RandFloatXY(0, radius);
		vector pos = center + Vector(Math.Cos(angle) * dist, 0, Math.Sin(angle) * dist);
		pos[1] = GetGame().GetWorld().GetSurfaceY(pos[0], pos[2]);
		return pos;
	}

	//------------------------------------------------------------------------------------------------
	//! Remove waypoints from an AI group and delete the group entity.
	protected void DeleteGroupEntity(IEntity ent)
	{
		SCR_AIGroup aiGroup = SCR_AIGroup.Cast(ent);
		if (aiGroup)
		{
			array<AIWaypoint> wps = {};
			aiGroup.GetWaypoints(wps);
			foreach (AIWaypoint wp : wps)
			{
				aiGroup.RemoveWaypoint(wp);
				delete wp;
			}
		}

		SCR_EntityHelper.DeleteEntityAndChildren(ent);
	}

	//------------------------------------------------------------------------------------------------
	// Diagnostics
	//------------------------------------------------------------------------------------------------
	override void OnDiag(float timeSlice)
	{
		DbgUI.Begin("AFM CivPop System");
		DbgUI.Text(string.Format("Active: %1", m_bIsActive));
		DbgUI.Text(string.Format("Registered PoIs: %1", m_aPoIs.Count()));
		if (m_pConfig)
			DbgUI.Text(string.Format("Managed groups: %1 / %2", m_mGroupData.Count(), m_pConfig.GetMaxCivilianCount()));

		int patrol, travel, idle;
		foreach (EntityID id, AFM_CivPopGroupData data : m_mGroupData)
		{
			switch (data.m_eBehavior)
			{
				case AFM_ECivPopBehavior.LOCAL_PATROL: patrol++; break;
				case AFM_ECivPopBehavior.TRAVEL: travel++; break;
				default: idle++; break;
			}
		}
		DbgUI.Text(string.Format("Patrol: %1  Travel: %2  Idle: %3", patrol, travel, idle));
		DbgUI.End();
	}
}