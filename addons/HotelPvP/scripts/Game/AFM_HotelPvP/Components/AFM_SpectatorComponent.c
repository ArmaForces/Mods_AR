class AFM_SpectatorComponentClass : ScriptComponentClass
{
}

class AFM_SpectatorComponent : ScriptComponent
{
	[RplProp(onRplName: "OnIsSpectatorUpdated", condition: RplCondition.OwnerOnly)]
	protected bool m_bIsSpectator = false;
	
	void SetState(bool state)
	{
		m_bIsSpectator = state;
		Replication.BumpMe();
	}
	
	bool IsSpectatorActive()
	{
		return m_bIsSpectator;
	}
	
	void OnIsSpectatorUpdated()
	{
		if (m_bIsSpectator)
			GetGame().GetCallqueue().CallLater(Hint, 1000 * 3);
	}
	
	protected void Hint()
	{
		SCR_HintManagerComponent.GetInstance().ShowCustom("You were assigned spectator rights. To view spectator camera, press and hold Y", "", 10, false);
	}
}