[ComponentEditorProps(category: "AFM", description: "")]
class AFM_SettingsComponentClass : ScriptComponentClass
{
}

class AFM_SettingsComponent : ScriptComponent
{
	protected ref AFM_PlainTextContainerSerializer m_pSerializer = new AFM_PlainTextContainerSerializer();
	
	ref AFM_ContainerTextResponseInvoker m_pMissionHeaderResponseInvoker = new AFM_ContainerTextResponseInvoker();
	ref AFM_ContainerTextResponseInvoker m_pAceSettingsResponseInvoker = new AFM_ContainerTextResponseInvoker();
	
	protected SCR_MissionHeader GetMissionHeader()
	{
		SCR_MissionHeader header = SCR_MissionHeader.Cast(GetGame().GetMissionHeader());
		
		#ifdef WORKBENCH
		// if world is started directly it will have no header, provide some test data
		if (!header)
			header = SCR_MissionHeader.Cast(MissionHeader.ReadMissionHeader("{83D06A42096F671C}Missions/MpTest/10_MpTest.conf"));
		#endif
		
		return header;
	}
	
	//################################################################################################
	//! Mission header
	void RequestMissionHeader()
	{
		Print("Requesting mission header");
		
		Rpc(RpcAsk_RequestMissionHeader, SCR_PlayerController.GetLocalPlayerId());
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void RpcAsk_RequestMissionHeader(int playerId)
	{
		AFM_ContainerTextResponse response = new AFM_ContainerTextResponse();
		
		SCR_MissionHeader header = GetMissionHeader();
		if (header)
		{
			Resource holder = BaseContainerTools.CreateContainerFromInstance(header);
			BaseContainer headerContainer = holder.GetResource().ToBaseContainer();
			
			response.m_sContent += m_pSerializer.SerializeContainer(headerContainer);
		}
			
		Rpc(RpcDo_RespondMissionHeader, response);
			
	}

	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	protected void RpcDo_RespondMissionHeader(AFM_ContainerTextResponse response)
	{
		Print("Mission header response");
		
		m_pMissionHeaderResponseInvoker.Invoke(response);
	}
	
	//################################################################################################
	//! ACE Settings
	void RequestAceSettings()
	{
		Print("Requesting ACE settings");
		
		Rpc(RpcAsk_RequestAceSettings, SCR_PlayerController.GetLocalPlayerId());
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void RpcAsk_RequestAceSettings(int playerId)
	{
		AFM_ContainerTextResponse response = new AFM_ContainerTextResponse();
		
		ACE_SettingsConfig config = ArmaReforgerScripted.ACE_GetSettingsConfig();
		foreach(ACE_ModSettings settings : config.GetAllModSettings())
		{
			Resource holder = BaseContainerTools.CreateContainerFromInstance(settings);
			BaseContainer settingsContainer = holder.GetResource().ToBaseContainer();
			
			response.m_sContent += m_pSerializer.SerializeContainer(settingsContainer);
			response.m_sContent += "\n";
		}
			
		Rpc(RpcDo_RespondAceSettings, response);
	}

	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	protected void RpcDo_RespondAceSettings(AFM_ContainerTextResponse response)
	{
		Print("ACE settings response");
		
		m_pAceSettingsResponseInvoker.Invoke(response);
	}
}

void ScriptInvoker_AFM_MissionHeaderResponse(AFM_ContainerTextResponse response);
typedef func ScriptInvoker_AFM_MissionHeaderResponse;
typedef ScriptInvokerBase<ScriptInvoker_AFM_MissionHeaderResponse> AFM_ContainerTextResponseInvoker;

class AFM_ContainerTextResponse
{
	string m_sContent = "";
	
	//################################################################################################
	//! Codec methods
	//------------------------------------------------------------------------------------------------
	//! Property mem to snapshot extraction.
	// Extracts relevant properties from an instance of type T into snapshot. Opposite of Inject()
	static bool Extract(AFM_ContainerTextResponse instance, ScriptCtx ctx, SSnapSerializerBase reader)
	{
		reader.SerializeString(instance.m_sContent);
		
		return true;
	}
	
	//! From snapshot to packet.
	// Takes snapshot and compresses it into packet. Opposite of Decode()
	static void Encode(SSnapSerializerBase snapshot, ScriptCtx ctx, ScriptBitSerializer packet)
	{
		snapshot.EncodeString(packet);
	}

	//! From packet to snapshot.
	// Takes packet and decompresses it into snapshot. Opposite of Encode()
	static bool Decode(ScriptBitSerializer packet, ScriptCtx ctx, SSnapSerializerBase snapshot)
	{
		snapshot.DecodeString(packet);
		
		return true;
	}
	
	//! Snapshot to property memory injection.
	// Injects relevant properties from snapshot into an instance of type T . Opposite of Extract()
	static bool Inject(SSnapSerializerBase writer, ScriptCtx ctx, AFM_ContainerTextResponse instance)
	{
		writer.SerializeString(instance.m_sContent);
		
		return true;
	}

	//! Snapshot to snapshot comparison.
	// Compares two snapshots to see whether they are the same or not
	static bool SnapCompare(SSnapSerializerBase lhs, SSnapSerializerBase rhs, ScriptCtx ctx)
	{
		return lhs.CompareStringSnapshots(rhs);
	}

	//! Property mem to snapshot comparison.
	// Compares instance and a snapshot to see if any property has changed enough to require a new snapshot
	static bool PropCompare(AFM_ContainerTextResponse instance, SSnapSerializerBase snapshot, ScriptCtx ctx)
	{
		return snapshot.CompareString(instance.m_sContent);
	}
}


