class AFM_SpectatorComponentClass : ScriptComponentClass
{
}

class AFM_SpectatorComponent : ScriptComponent
{
	[RplProp(onRplName: "OnIsSpectatorUpdated", condition: RplCondition.OwnerOnly)]
	protected bool m_bIsSpectator = false;
	
	[RplProp()]
	protected vector m_vSpectatorOrigin = "0 0 0";
	
	void EnableSpectator(vector origin)
	{
		if (m_bIsSpectator)
			return;
		
		m_vSpectatorOrigin = origin;
		m_bIsSpectator = true;
		
		Replication.BumpMe();
	}
	
	void DisableSpectator()
	{
		if (!m_bIsSpectator)
			return;
		
		m_bIsSpectator = false;
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
	
	vector GetCameraOriginPos()
	{
		return m_vSpectatorOrigin;
	}
	
	protected void Hint()
	{
		SCR_HintManagerComponent.GetInstance().ShowCustom("You were assigned spectator rights. To view spectator camera, press and hold Y", "", 10, false);
	}
}