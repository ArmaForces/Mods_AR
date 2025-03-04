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
	
	protected SCR_FactionManager m_FactionManager;
	
	protected ref ScriptInvoker m_OnMatchSituationChanged;
	
	[RplProp(onRplName: "OnMatchSituationChanged")]
	protected WorldTimestamp m_fVictoryTimestamp;
	
	[RplProp(onRplName: "OnMatchSituationChanged")]
	protected WorldTimestamp m_fStartTimestamp;
	
	[RplProp(onRplName: "OnMatchSituationChanged")]
	protected bool m_bHasGameStarted = false;
	
	[RplProp(onRplName: "OnMatchSituationChanged")]
	protected int m_iDefendersRemaining = 0;
	
	[RplProp(onRplName: "OnMatchSituationChanged")]
	protected int m_iAttackersRemaining = 0;
	
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
		
		if (!m_bHasGameStarted)
			return;
		
		FactionAffiliationComponent fac = FactionAffiliationComponent.Cast(entity.FindComponent(FactionAffiliationComponent));
		if (!fac) 
		{
			Print("Unknown victim faction!", LogLevel.WARNING);
			return;
		}
		
		FactionKey fKey = fac.GetAffiliatedFactionKey();	
		if (fKey == m_sAttackerFactionKey || fKey == m_sDefenderFactionKey)
		{
			HandlePlayerDeath(entity);
		}
		else if (fKey == "CIV")
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
		
		ChimeraWorld world = GetGame().GetWorld();
		m_fStartTimestamp = world.GetServerTimestamp().PlusSeconds(m_iGameStartDelaySeconds);
		m_fVictoryTimestamp = m_fStartTimestamp.PlusSeconds(m_iGameOverTimeMinutes * 60);
		GetGame().GetCallqueue().CallLater(StartGame, m_iGameStartDelaySeconds * 1000);
		GetGame().GetCallqueue().CallLater(TimeoutGameEnd, m_iGameOverTimeMinutes * 60 * 1000 + m_iGameStartDelaySeconds * 1000, false);
	}
	
	void StartGame()
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
	
	void GameEndCheck() 
	{
		if (!m_bHasGameStarted)
			return;
		
		m_iAttackersRemaining = GetFactionRemainingPlayers(m_sAttackerFactionKey);
		m_iDefendersRemaining = GetFactionRemainingPlayers(m_sDefenderFactionKey);
		
		bool attackersDead = m_iAttackersRemaining == 0;
		bool defendersDead = m_iDefendersRemaining == 0;
		bool hostageStatus = AllHostagesFreed();
		
		PrintFormat("GameEndCheck: A:%1 D:%2 H:%3", attackersDead, defendersDead, hostageStatus, level:LogLevel.SPAM);
		
		if (AllHostagesFreed())
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
	
	protected bool AllHostagesFreed()
	{
		return GetGame().GetWorld().QueryEntitiesBySphere(
				GetOrigin(),
				m_iGameAreaRadius,
				ProcessFoundHostages,
				FilterHostageEntities,
				EQueryEntitiesFlags.DYNAMIC
		);
	}
	
	protected bool ProcessFoundHostages(IEntity entity)
	{
		return false;
	}
	
	protected bool FilterHostageEntities(IEntity entity)
	{
		if(entity.Type() != SCR_ChimeraCharacter)
			return false; 
		
		FactionAffiliationComponent fac = FactionAffiliationComponent.Cast(entity.FindComponent(FactionAffiliationComponent));
		return fac && fac.GetAffiliatedFactionKey() == "CIV";
	}
	
	void TimeoutGameEnd()
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
	
	
	protected void GameEnd(FactionKey fKey)
	{
		m_bHasGameStarted = false;
		
		Faction faction = m_FactionManager.GetFactionByKey(fKey);
		int factionId = m_FactionManager.GetFactionIndex(faction);
		SCR_GameModeEndData endData = SCR_GameModeEndData.CreateSimple(EGameOverTypes.ENDREASON_SCORELIMIT, winnerFactionId:factionId);
		EndGameMode(endData);
	}
	
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
	
	int GetFactionRemainingPlayers(FactionKey fKey)
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
				if (!damageManager.IsDestroyed() && !spectator.GetState()) {
					remainingPlayers++;
				}
			}
		}
		PrintFormat("Found %1 players in faction %2", remainingPlayers, fKey, level: LogLevel.SPAM);
		return remainingPlayers;
	}
	
	void HandlePlayerDeath(IEntity unit)
	{
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
			spectatorComponent.SetState(true);
		}
		else 
		{
			Print("No spectator component found on unit!!!", LogLevel.ERROR);
		}
		
		respawnComponent.RequestSpawn(spsd);
	}
	
	void DisableRestrictionZones()
	{
		SCR_PlayersRestrictionZoneManagerComponent restrictionZoneManager = SCR_PlayersRestrictionZoneManagerComponent.Cast(FindComponent(SCR_PlayersRestrictionZoneManagerComponent));
		if (!restrictionZoneManager)
			return;
		
		SCR_EditorRestrictionZoneEntity attackersZone = SCR_EditorRestrictionZoneEntity.Cast(GetWorld().FindEntityByName("attackers_zone"));
		if (attackersZone)
		{
			Print("Removing attacker restiction zone");
			restrictionZoneManager.RemoveRestrictionZone(attackersZone);
		}
		
		SCR_EditorRestrictionZoneEntity defendersZone = SCR_EditorRestrictionZoneEntity.Cast(GetWorld().FindEntityByName("defenders_zone"));
		if (defendersZone)
		{
			Print("Removing defender restiction zone");
			restrictionZoneManager.RemoveRestrictionZone(defendersZone);
		}
	}
	
	
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

