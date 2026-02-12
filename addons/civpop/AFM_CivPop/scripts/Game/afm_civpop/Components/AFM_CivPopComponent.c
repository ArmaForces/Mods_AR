/**
* Gamemode component that holds configuration for the AFM CivPop system.
* Attach this to the game mode entity to enable civilian population spawning.
*/

[ComponentEditorProps(category: "AFM/CivPop", description: "Configuration component for the CivPop system. Attach to GameMode.")]
class AFM_CivPopComponentClass : ScriptComponentClass
{
}

class AFM_CivPopComponent : ScriptComponent
{
	//--- Spawn settings
	[Attribute(defvalue: "20", desc: "Maximum number of civilians that can be alive at once", category: "CivPop Spawning", params: "0 200 1")]
	protected int m_iMaxCivilianCount;

	[Attribute(defvalue: "5", desc: "Interval in seconds between spawn update ticks", category: "CivPop Spawning", params: "1 60 1")]
	protected float m_fSpawnTickInterval;

	//--- Waypoint prefabs
	[Attribute(desc: "Prefab for Wait waypoint", category: "CivPop Waypoints", params: "et")]
	protected ResourceName m_sWaypointWaitPrefab;

	[Attribute(desc: "Prefab for Move waypoint", category: "CivPop Waypoints", params: "et")]
	protected ResourceName m_sWaypointMovePrefab;

	[Attribute(desc: "Prefab for Loiter / wander waypoint", category: "CivPop Waypoints", params: "et")]
	protected ResourceName m_sWaypointLoiterPrefab;

	[Attribute(desc: "Prefab for GetIn vehicle waypoint", category: "CivPop Waypoints", params: "et")]
	protected ResourceName m_sWaypointGetInPrefab;

	[Attribute(desc: "Prefab for GetOut vehicle waypoint", category: "CivPop Waypoints", params: "et")]
	protected ResourceName m_sWaypointGetOutPrefab;

	//--- Civilian group prefabs
	[Attribute(desc: "Array of civilian group prefabs to spawn from. A random one is chosen each time.", category: "CivPop Prefabs", params: "et")]
	protected ref array<ResourceName> m_aCivilianGroupPrefabs;

	//--- Vehicle prefabs
	[Attribute(desc: "Array of civilian vehicle prefabs. Used when civilians travel between connected PoIs.", category: "CivPop Prefabs", params: "et")]
	protected ref array<ResourceName> m_aVehiclePrefabs;

	//--- Getters ------------------------------------------------------------------

	//------------------------------------------------------------------------------------------------
	int GetMaxCivilianCount()
	{
		return m_iMaxCivilianCount;
	}

	//------------------------------------------------------------------------------------------------
	float GetSpawnTickInterval()
	{
		return m_fSpawnTickInterval;
	}

	//------------------------------------------------------------------------------------------------
	ResourceName GetWaypointWaitPrefab()
	{
		return m_sWaypointWaitPrefab;
	}

	//------------------------------------------------------------------------------------------------
	ResourceName GetWaypointMovePrefab()
	{
		return m_sWaypointMovePrefab;
	}

	//------------------------------------------------------------------------------------------------
	ResourceName GetWaypointLoiterPrefab()
	{
		return m_sWaypointLoiterPrefab;
	}

	//------------------------------------------------------------------------------------------------
	ResourceName GetWaypointGetInPrefab()
	{
		return m_sWaypointGetInPrefab;
	}

	//------------------------------------------------------------------------------------------------
	ResourceName GetWaypointGetOutPrefab()
	{
		return m_sWaypointGetOutPrefab;
	}

	//------------------------------------------------------------------------------------------------
	array<ResourceName> GetCivilianGroupPrefabs()
	{
		return m_aCivilianGroupPrefabs;
	}

	//------------------------------------------------------------------------------------------------
	array<ResourceName> GetVehiclePrefabs()
	{
		return m_aVehiclePrefabs;
	}
}
