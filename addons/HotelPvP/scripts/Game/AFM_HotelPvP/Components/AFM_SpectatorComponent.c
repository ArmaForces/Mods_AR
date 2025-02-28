class AFM_SpectatorComponentClass : ScriptComponentClass
{
}

class AFM_SpectatorComponent : ScriptComponent
{
	[RplProp()]
	protected bool m_bIsSpectator = false;
	
	void SetState(bool state)
	{
		Rpc(RpcAsk_SetState, state);
		
		if (state)
			Rpc(RpcAsk_ShowHint);
	}
	
	bool GetState()
	{
		return m_bIsSpectator;
	}
	
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void RpcAsk_SetState(bool state)
	{
		if (m_bIsSpectator == state)
			return;
		
		m_bIsSpectator = state;
		Replication.BumpMe();
	}
	
	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	protected void RpcAsk_ShowHint()
	{
		GetGame().GetCallqueue().CallLater(Hint, 1000 * 3);
	}
	
	protected void Hint()
	{
		SCR_HintManagerComponent.GetInstance().ShowCustom("You were assigned spectator rights. To view spectator camera, press  and hold Y", "", 10, false);
	}
}