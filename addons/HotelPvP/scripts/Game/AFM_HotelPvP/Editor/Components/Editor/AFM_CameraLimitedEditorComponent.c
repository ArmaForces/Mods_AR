modded class SCR_CameraLimitedEditorComponent : SCR_CameraEditorComponent
{
	override protected ResourceName GetCameraPrefab()
	{
		SCR_EditorManagerEntity manager = SCR_EditorManagerEntity.GetInstance();
		if (!manager) 
			return ResourceName.Empty;
		
		PlayerController pc = GetGame().GetPlayerController();
		AFM_SpectatorComponent spectator = AFM_SpectatorComponent.Cast(pc.FindComponent(AFM_SpectatorComponent));
		bool isPrisoner = spectator && spectator.GetState();
		
		bool isLimited = manager.IsLimited() && !spectator;
		

		if (isLimited)
			return m_LimitedCameraPrefab;
		else
			return m_CameraPrefab;
	}
}