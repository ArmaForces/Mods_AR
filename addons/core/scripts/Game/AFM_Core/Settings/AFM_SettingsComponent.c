[ComponentEditorProps(category: "AFM", description: "")]
class AFM_SettingsComponentClass : ScriptComponentClass
{
}

class AFM_SettingsComponent : ScriptComponent
{
	ScriptInvoker RequestMissionHeader()
	{
		Print("Requesting mission header");
		
		Rpc(RpcAsk_Request, new AFM_SettingsRequestMissionHeader());
		
		return null;
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void RpcAsk_Request(AFM_SettingsRequest request)
	{
		AFM_SettingsRequestMissionHeader requestParsed = AFM_SettingsRequestMissionHeader.Cast(request);
		if (requestParsed)
		{
			AFM_ResponseMissionHeader response = new AFM_ResponseMissionHeader();
			
			ACE_SettingsConfig config = ArmaReforgerScripted.ACE_GetSettingsConfig();
			foreach(ACE_ModSettings settings : config.GetAllModSettings())
			{
				Resource holder = BaseContainerTools.CreateContainerFromInstance(settings);
				BaseContainer settingsContainer = holder.GetResource().ToBaseContainer();
				Print("ACE Settings:");
				for (int i, count = settingsContainer.GetNumVars(); i < count; i++)
				{
					string var = settingsContainer.GetVarName(i);
					DataVarType type = settingsContainer.GetDataVarType(i);
					
					switch (type)
					{
						case DataVarType.BOOLEAN:
							bool val;
							settingsContainer.Get(var, val);
							PrintFormat("\t%1: %2", var, val);
							break;
						
						case DataVarType.SCALAR:
						case DataVarType.INTEGER:
							float val;
							settingsContainer.Get(var, val);
							PrintFormat("\t%1: %2", var, val);
							break;
						
						default:
							PrintFormat("\t%1: <unknown type, %2>", var, type.ToString());
					}
				}
			}
			
			Rpc(RpcDo_Response, response);
			return;
		}
	}

	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	protected void RpcDo_Response(AFM_SettingsResponse response)
	{

	}
}

class AFM_SettingsNetworkMessage
{
	//! Takes snapshot and encodes it into packet using as few bits as possible.
	static void Encode(SSnapSerializerBase snapshot, ScriptCtx ctx, ScriptBitSerializer packet)
	{
	}

	//! Takes packet and decodes it into snapshot. Returns true on success or false when an error occurs.
	static bool Decode(ScriptBitSerializer packet, ScriptCtx ctx, SSnapSerializerBase snapshot)
	{
	}

	//! Compares two snapshots. Returns true when they match or false otherwise.
	static bool SnapCompare(SSnapSerializerBase lhs, SSnapSerializerBase rhs, ScriptCtx ctx)
	{
		return true;
	}
}

class AFM_SettingsRequest : AFM_SettingsNetworkMessage
{
	//! Compares instance against snapshot. Returns true when they match or false otherwise.
	static bool PropCompare(AFM_SettingsRequest instance, SSnapSerializerBase snapshot, ScriptCtx ctx)
	{
		return true;
	}

	//! Writes data from an instance into snapshot. Opposite of Inject().
	static bool Extract(AFM_SettingsRequest instance, ScriptCtx ctx, SSnapSerializerBase snapshot)
	{
	}

	//! Writes data from snapshot into instance. Opposite of Extract().
	static bool Inject(SSnapSerializerBase snapshot, ScriptCtx ctx, AFM_SettingsRequest instance)
	{
	}
}
class AFM_SettingsRequestMissionHeader : AFM_SettingsRequest
{
	//! Compares instance against snapshot. Returns true when they match or false otherwise.
	static bool PropCompare(AFM_SettingsRequestMissionHeader instance, SSnapSerializerBase snapshot, ScriptCtx ctx)
	{
		return true;
	}

	//! Writes data from an instance into snapshot. Opposite of Inject().
	static bool Extract(AFM_SettingsRequestMissionHeader instance, ScriptCtx ctx, SSnapSerializerBase snapshot)
	{
	}

	//! Writes data from snapshot into instance. Opposite of Extract().
	static bool Inject(SSnapSerializerBase snapshot, ScriptCtx ctx, AFM_SettingsRequestMissionHeader instance)
	{
	}
}

class AFM_SettingsResponse : AFM_SettingsNetworkMessage
{
	//! Compares instance against snapshot. Returns true when they match or false otherwise.
	static bool PropCompare(AFM_SettingsResponse instance, SSnapSerializerBase snapshot, ScriptCtx ctx)
	{
		return true;
	}

	//! Writes data from an instance into snapshot. Opposite of Inject().
	static bool Extract(AFM_SettingsResponse instance, ScriptCtx ctx, SSnapSerializerBase snapshot)
	{
	}

	//! Writes data from snapshot into instance. Opposite of Extract().
	static bool Inject(SSnapSerializerBase snapshot, ScriptCtx ctx, AFM_SettingsResponse instance)
	{
	}
}
class AFM_ResponseMissionHeader : AFM_SettingsResponse
{
	//! Compares instance against snapshot. Returns true when they match or false otherwise.
	static bool PropCompare(AFM_ResponseMissionHeader instance, SSnapSerializerBase snapshot, ScriptCtx ctx)
	{
		return true;
	}

	//! Writes data from an instance into snapshot. Opposite of Inject().
	static bool Extract(AFM_ResponseMissionHeader instance, ScriptCtx ctx, SSnapSerializerBase snapshot)
	{
	}

	//! Writes data from snapshot into instance. Opposite of Extract().
	static bool Inject(SSnapSerializerBase snapshot, ScriptCtx ctx, AFM_ResponseMissionHeader instance)
	{
	}
}
