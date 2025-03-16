modded class SCR_CameraLimitedEditorComponent : SCR_CameraEditorComponent
{
	override protected ResourceName GetCameraPrefab()
	{
		SCR_EditorManagerEntity manager = SCR_EditorManagerEntity.GetInstance();
		if (!manager) 
			return ResourceName.Empty;
		
		PlayerController pc = GetGame().GetPlayerController();
		AFM_SpectatorComponent spectator = AFM_SpectatorComponent.Cast(pc.FindComponent(AFM_SpectatorComponent));
		if (!spectator)
			return m_CameraPrefab;
		
		bool isLimited = manager.IsLimited() && !spectator.IsSpectatorActive();
		
		if (isLimited)
			return m_LimitedCameraPrefab;
		else
			return m_CameraPrefab;
	}
}