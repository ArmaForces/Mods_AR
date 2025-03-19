class AFM_FactoryPvPGameModeClass: SCR_BaseGameModeClass
{
}

class AFM_FactoryPvPGameMode : SCR_BaseGameMode
{	
	[Attribute("FIA", UIWidgets.EditBox, "Defenders faction key")]
	protected FactionKey m_sDefenderFactionKey;
	
	[Attribute("US", UIWidgets.EditBox, "Attackers faction key")]
	protected FactionKey m_sAttackerFactionKey;	
	
	[Attribute( defvalue: "180", desc: "Time in seconds to delay game start")]
	int m_iGameStartDelaySeconds;
	
	[Attribute( defvalue: "15", desc: "Time in minutes before defenders win")]
	int m_iGameOverTimeMinutes;
	
	[Attribute("FIA", UIWidgets.EditBox, "Faction that wins on timeout. Empty for a tie")]
	protected FactionKey m_TimeoutWinnerFactionKey;	
	
	[Attribute( defvalue: "500", desc: "Game area radius")]
	int m_iGameAreaRadius;
	
	[Attribute( defvalue: "0", desc: "Disable faction spawn after game start?")]
	bool m_bDisableSpawnAfterStart;
	
	[Attribute(uiwidget: UIWidgets.ResourceNamePicker, desc: "Spectator loadout prefab", params: "et", category: "Prefabs")]
	ResourceName m_pPrisonerPrefab;
	
	[Attribute(desc: "Array of restriction zone names to be disabled at game start")]
	ref array<string> m_aRestrictionZoneNames;
	
	protected SCR_FactionManager m_FactionManager;
	
	protected ref ScriptInvoker m_OnMatchSituationChanged;
	
	[RplProp(onRplName: "OnMatchSituationChanged")]
	protected WorldTimestamp m_fVictoryTimestamp;
	
	[RplProp(onRplName: "OnMatchSituationChanged")]
	protected WorldTimestamp m_fStartTimestamp;
	
	[RplProp(onRplName: "OnMatchSituationChanged")]
	protected bool m_bHasGameStarted = false;
	
	[RplProp(onRplName: "OnMatchSituationChanged")]
	protected bool m_bHasPreGameStarted = false;
	
	[RplProp(onRplName: "OnMatchSituationChanged")]
	protected int m_iDefendersRemaining = 0;
	
	[RplProp(onRplName: "OnMatchSituationChanged")]
	protected int m_iAttackersRemaining = 0;
	
	//------------------------------------------------------------------------------------------------
	ScriptInvoker GetOnMatchSituationChanged()
	{
		if (!m_OnMatchSituationChanged)
			m_OnMatchSituationChanged = new ScriptInvoker();

		return m_OnMatchSituationChanged;
	}
	
	void OnMatchSituationChanged()
	{
		if (m_OnMatchSituationChanged)
			m_OnMatchSituationChanged.Invoke();
	}
	
	override void OnControllableDestroyed(IEntity entity, IEntity killerEntity, notnull Instigator instigator)
	{
		super.OnControllableDestroyed(entity, killerEntity, instigator);
		
		if (!IsMaster())
			return;
		
		FactionAffiliationComponent fac = FactionAffiliationComponent.Cast(entity.FindComponent(FactionAffiliationComponent));
		if (!fac) 
		{
			Print("Unknown victim faction!", LogLevel.WARNING);
			return;
		}
				
		FactionKey fKey = fac.GetAffiliatedFactionKey();
		if (fKey == "CIV")
			HandleHostageKill(killerEntity);
		else if (fKey == m_sAttackerFactionKey || fKey == m_sDefenderFactionKey)
			HandlePlayerDeath(entity);
		else
			PrintFormat("Unknown victim faction key %1", fKey, level: LogLevel.WARNING);
		
		GameEndCheck();
		Replication.BumpMe();
	}
	
	override void EOnInit(IEntity owner)
	{
		super.EOnInit(owner);
		
		if (SCR_Global.IsEditMode())
			return;
		
		m_FactionManager = SCR_FactionManager.Cast(GetGame().GetFactionManager());
		if (!m_FactionManager)
		{
			Print("Faction manager component is missing!", LogLevel.ERROR);
		}
		
		GetOnPlayerSpawned().Insert(OnPlayerSpawn);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Server-side method to handle player spawns
	//! This will launch the scenario into pregame state when first player connects 
	//! and update scoring table on every player joining game
	protected void OnPlayerSpawn(int playerId, IEntity player)
	{
		if (!IsMaster()) return;
		
		PrintFormat("Player %1 has spawned", playerId, level: LogLevel.DEBUG);
		if (!m_bHasPreGameStarted)
			PreGameStart();
		
		GameEndCheck();
		OnMatchSituationChanged();
		Replication.BumpMe();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Server-side method to handle pre game start, eg. when at least 1 player has joined
	//! This will set up timers and call queue for game start later
	protected void PreGameStart()
	{
		if (m_bHasPreGameStarted) return;
		m_bHasPreGameStarted = true;
		Print("PreGame starting");
		
		ChimeraWorld world = GetGame().GetWorld();
		m_fStartTimestamp = world.GetServerTimestamp().PlusSeconds(m_iGameStartDelaySeconds);
		m_fVictoryTimestamp = m_fStartTimestamp.PlusSeconds(m_iGameOverTimeMinutes * 60);
		GetGame().GetCallqueue().CallLater(StartGame, m_iGameStartDelaySeconds * 1000);
		GetGame().GetCallqueue().CallLater(GameEndTimeout, m_iGameOverTimeMinutes * 60 * 1000 + m_iGameStartDelaySeconds * 1000, false);
	}
	

	//------------------------------------------------------------------------------------------------
	//! Server-side method to handle game start
	protected void StartGame()
	{
		if (!IsMaster())
			return;
		
		Print("Gamemode starting");
		DisableRestrictionZones();
		GetGame().GetCallqueue().CallLater(GameEndCheck, 1000 * 5, true);
		Rpc(RPC_DoStartGame);
		RPC_DoStartGame();
		Replication.BumpMe();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Proxy-side method to handle game start event
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	void RPC_DoStartGame()
	{
		if (m_bHasGameStarted) return;
		m_bHasGameStarted = true;
		
		SCR_HintManagerComponent.GetInstance().ShowCustom("Game is starting", "", 10, false);

		if (m_bDisableSpawnAfterStart) 
		{
			array<SCR_SpawnPoint> spawnPoints = SCR_SpawnPoint.GetSpawnPoints();
			foreach(SCR_SpawnPoint sp: spawnPoints)
			{
				sp.SetSpawnPointEnabled_S(!sp.IsSpawnPointEnabled());
			}
		}
		OnMatchSituationChanged();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void GameEndCheck() 
	{
		m_iAttackersRemaining = GetFactionRemainingPlayersCount(m_sAttackerFactionKey);
		m_iDefendersRemaining = GetFactionRemainingPlayersCount(m_sDefenderFactionKey);
		
		if (!m_bHasGameStarted)
			return;
		
		bool attackersDead = m_iAttackersRemaining == 0;
		bool defendersDead = m_iDefendersRemaining == 0;
		bool hostageStatus = AreAllHostagesFree();
		
		PrintFormat("GameEndCheck: A:%1 D:%2 H:%3", attackersDead, defendersDead, hostageStatus, level:LogLevel.SPAM);
		
		if (hostageStatus)
		{
			Print("All hostages escaped", LogLevel.NORMAL);
			GameEnd(m_sAttackerFactionKey);
			return;
		}
		
		if (attackersDead && defendersDead)
		{
			GameEndTie();
			return;
		}
		
		if (attackersDead)
		{
			GameEndDefendersWin();
			return;	
		}
		
		if (defendersDead) 
		{
			GameEndAttackersWin();
			return;
		}
	}
	
	protected bool AreAllHostagesFree()
	{
		return GetGame().GetWorld().QueryEntitiesBySphere(
				GetOrigin(),
				m_iGameAreaRadius,
				AFM_HostageQueryCollector.ProcessFoundHostages,
				AFM_HostageQueryCollector.FilterHostageEntities,
				EQueryEntitiesFlags.DYNAMIC
		);
	}
	
	protected void HandleHostageKill(IEntity killerEntity)
	{
		if (!killerEntity)
		{
			Print("Unknown killer entity!", LogLevel.WARNING);
			GameEndTie();
		}
		else
		{
			Faction killerFaction = FactionAffiliationComponent.Cast(killerEntity.FindComponent(FactionAffiliationComponent))
				.GetAffiliatedFaction();
			FactionKey killerFactionKey = killerFaction.GetFactionKey();
			
			if (killerFactionKey == m_sAttackerFactionKey) 
			{
				GameEndDefendersWin();
			}
			else if (killerFactionKey == m_sDefenderFactionKey)
			{
				GameEndAttackersWin();
			}
			else
			{
				PrintFormat("Unsuported killer faction! %1", killerFactionKey, level:LogLevel.ERROR);
				GameEndTie();
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Server side method to end game in case of timeout
	protected void GameEndTimeout()
	{
		if (!IsMaster())
			return;
		
		PrintFormat("Timeout reached, ending game with {%1} faction win", m_TimeoutWinnerFactionKey);
		SCR_Faction faction = SCR_Faction.Cast(m_FactionManager.GetFactionByKey(m_TimeoutWinnerFactionKey));
		
		
		if (!faction)
		{
			GameEndTie();
			return;
		}
	
		GameEnd(m_TimeoutWinnerFactionKey);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Server side method to end game with a tie
	protected void GameEndTie()
	{
		Print("Game tie", LogLevel.NORMAL);
		SCR_GameModeEndData endData = SCR_GameModeEndData.Create(
			EGameOverTypes.FACTION_DRAW,
			{}, {}
		);
		m_bHasGameStarted = false;
		EndGameMode(endData);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Server side method to end game with winningFactionKey faction victory
	protected void GameEnd(FactionKey winningFactionKey)
	{
		m_bHasGameStarted = false;
		
		Faction faction = m_FactionManager.GetFactionByKey(winningFactionKey);
		int factionId = m_FactionManager.GetFactionIndex(faction);
		SCR_GameModeEndData endData = SCR_GameModeEndData.CreateSimple(EGameOverTypes.ENDREASON_SCORELIMIT, winnerFactionId:factionId);
		EndGameMode(endData);
	}
	
	protected void GameEndDefendersWin()
	{
		Print("Defenders win!");
		GameEnd(m_sDefenderFactionKey);
	}

	protected void GameEndAttackersWin()
	{
		Print("Attackers win!");
		GameEnd(m_sAttackerFactionKey);
	}
	
	//------------------------------------------------------------------------------------------------
	int GetFactionRemainingPlayersCount(FactionKey fKey)
	{
		SCR_Faction faction = SCR_Faction.Cast(m_FactionManager.GetFactionByKey(fKey));
		if (!faction)
		{
			PrintFormat("Could not find faction %1", fKey, level:LogLevel.WARNING);
			return 0;
		}
		array<int> playerIds = new array<int>;
		
		faction.GetPlayersInFaction(playerIds);
		int remainingPlayers = 0;
		
		foreach(int id: playerIds)
		{
			PlayerController pc = GetGame().GetPlayerManager().GetPlayerController(id);
			if (pc)
			{
				SCR_ChimeraCharacter ent = SCR_ChimeraCharacter.Cast(pc.GetControlledEntity());		
				AFM_SpectatorComponent spectator = AFM_SpectatorComponent.Cast(pc.FindComponent(AFM_SpectatorComponent));
				SCR_DamageManagerComponent damageManager = ent.GetDamageManager();
				if (!damageManager.IsDestroyed() && !spectator.IsSpectatorActive()) {
					remainingPlayers++;
				}
			}
		}
		PrintFormat("Found %1 players in faction %2", remainingPlayers, fKey, level: LogLevel.SPAM);
		return remainingPlayers;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Server-side method to handle player death, respawn and spectator rights
	protected void HandlePlayerDeath(IEntity unit)
	{
		if (!m_bHasGameStarted)
			return;
		
		int playerId = GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(unit);
		if (playerId == 0)
		{
			Print("No player id found on unit!!!", LogLevel.ERROR);
			return;
		}
		
		PlayerController pc = GetGame().GetPlayerManager().GetPlayerController(playerId);
		if (!pc) 
		{
			Print("No player controller found on unit!!!", LogLevel.ERROR);
			return;
		}
		
		SCR_RespawnComponent respawnComponent = SCR_RespawnComponent.Cast(pc.FindComponent(SCR_RespawnComponent));
		if (!respawnComponent)
		{
			Print("No respawn component found on unit!!!", LogLevel.ERROR);
			return;
		}
		
		IEntity sp = GetWorld().FindEntityByName("spectator_spawn");
		if (!sp)
		{
			Print("No spectator entity found in world! Please make sure spawn point exists and its named spectator_spawn", LogLevel.ERROR);
			return;
		}
		
		SCR_SpawnPoint spawnPoint = SCR_SpawnPoint.Cast(sp);
		SCR_SpawnPointSpawnData spsd = new SCR_SpawnPointSpawnData(m_pPrisonerPrefab, spawnPoint.GetRplId());
		AFM_SpectatorComponent spectatorComponent = AFM_SpectatorComponent.Cast(pc.FindComponent(AFM_SpectatorComponent));
		if (spectatorComponent)
		{
			spectatorComponent.EnableSpectator(unit.GetOrigin());
		}
		else 
		{
			Print("No spectator component found on unit!!!", LogLevel.ERROR);
		}
		
		respawnComponent.RequestSpawn(spsd);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Server side removal of starting restriction zones
	protected void DisableRestrictionZones()
	{
		SCR_PlayersRestrictionZoneManagerComponent restrictionZoneManager = SCR_PlayersRestrictionZoneManagerComponent.Cast(FindComponent(SCR_PlayersRestrictionZoneManagerComponent));
		if (!restrictionZoneManager)
			return;
		
		foreach(string zoneName: m_aRestrictionZoneNames)
		{
			SCR_EditorRestrictionZoneEntity zone = SCR_EditorRestrictionZoneEntity.Cast(GetWorld().FindEntityByName(zoneName));
			if (!zone)
			{
				PrintFormat("Could not cast %1 zone to restriction zone entity - please verify list of restriction zones", zone, level: LogLevel.WARNING);
				continue;
			}
			
			PrintFormat("Removing %1 restiction zone", zone, level: LogLevel.DEBUG);
			restrictionZoneManager.RemoveRestrictionZone(zone);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	SCR_Faction GetBluforFaction()
	{
		return SCR_Faction.Cast(m_FactionManager.GetFactionByKey(m_sDefenderFactionKey));
	}
	
	SCR_Faction GetRedforFaction()
	{
		return SCR_Faction.Cast(m_FactionManager.GetFactionByKey(m_sAttackerFactionKey));
	}

	int GetBluforScore()
	{
		return m_iDefendersRemaining;
	}

	int GetRedforScore()
	{
		return m_iAttackersRemaining;
	}
	
	bool IsGameRunning()
	{
		return m_bHasGameStarted;
	}

	WorldTimestamp GetVictoryTimestamp()
	{
		return m_fVictoryTimestamp;
	}

	WorldTimestamp GetGameStartTimestamp()
	{
		return m_fStartTimestamp;
	}
	
	//------------------------------------------------------------------------------------------------ 
	override bool RplLoad(ScriptBitReader reader)
	{
		super.RplLoad(reader);
		reader.ReadBool(m_bHasGameStarted);
		reader.ReadInt(m_iAttackersRemaining);
		reader.ReadInt(m_iDefendersRemaining);
		
		return true;
	}
	
	override bool RplSave(ScriptBitWriter writer)
	{
		super.RplSave(writer);
		writer.WriteBool(m_bHasGameStarted);
		writer.WriteInt(m_iAttackersRemaining);
		writer.WriteInt(m_iDefendersRemaining);
		
		return true;
	}

}

//------------------------------------------------------------------------------------------------
class AFM_HostageQueryCollector
{
	static bool ProcessFoundHostages(IEntity entity)
	{
		return false;
	}
	
	static bool FilterHostageEntities(IEntity entity)
	{
		if(entity.Type() != SCR_ChimeraCharacter)
			return false; 
		
		FactionAffiliationComponent fac = FactionAffiliationComponent.Cast(entity.FindComponent(FactionAffiliationComponent));
		return fac && fac.GetAffiliatedFactionKey() == "CIV";
	}
}

