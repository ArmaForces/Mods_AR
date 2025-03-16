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
	
		m_vInitCameraTransform[3] = spectator.GetCameraOriginPos();
				
		if (isLimited)
			return m_LimitedCameraPrefab;
		else
			return m_CameraPrefab;
	}
	
	override protected void CreateCamera()
	{
		delete m_Camera;
		
		if (GetOwner().IsDeleted())
			return;
		
		EntitySpawnParams spawnParams = new EntitySpawnParams();
		spawnParams.Transform = m_vInitCameraTransform;

		ResourceName prefab = GetCameraPrefab();
		if (prefab.IsEmpty())
		{
			m_Camera = SCR_ManualCamera.Cast(GetGame().SpawnEntity(SCR_ManualCamera, GetGame().GetWorld(), spawnParams));
		}
		else
		{
			m_Camera = SCR_ManualCamera.Cast(GetGame().SpawnEntityPrefab(Resource.Load(prefab), GetGame().GetWorld(), spawnParams));
		}
		
		m_Camera.GetOnCameraDeactivate().Insert(OnCameraDectivate);
		Math3D.MatrixIdentity4(m_vInitCameraTransform);
		
		if (m_CameraData)
			m_CameraData.LoadComponents(m_Camera);
		
		m_OnCameraCreate.Invoke(m_Camera);
		
		//--- When opening the editor, move the camera above player
		if (m_bMoveUpOnInit && !m_bIsReplacingCamera)
		{
			if (m_PreActivateControlledEntity)
			{
				SCR_TeleportToCursorManualCameraComponent teleportComponent = SCR_TeleportToCursorManualCameraComponent.Cast(m_Camera.FindCameraComponent(SCR_TeleportToCursorManualCameraComponent));
				if (teleportComponent)
				{
					if (m_vPreActivateCameraTransform[3] == vector.Zero)
						m_Camera.GetWorldTransform(m_vPreActivateCameraTransform);
					//--- Reset roll
					vector angles = Math3D.MatrixToAngles(m_vPreActivateCameraTransform);
					angles[2] = 0;
					Math3D.AnglesToMatrix(angles, m_vPreActivateCameraTransform);
					
					PlayerController pc = GetGame().GetPlayerController();
					AFM_SpectatorComponent spectator = AFM_SpectatorComponent.Cast(pc.FindComponent(AFM_SpectatorComponent));
					if (spectator)
					{
						PrintFormat("AFM_Spectator: Setting spectator camera position to %1", spectator.GetCameraOriginPos(), level: LogLevel.DEBUG);
						m_vPreActivateCameraTransform[3] = spectator.GetCameraOriginPos();
					}
					
					m_Camera.SetTransform(m_vPreActivateCameraTransform);
					bool disableInterruption = !GetGame().GetInputManager().IsUsingMouseAndKeyboard();
					teleportComponent.TeleportCamera(m_vPreActivateCameraTransform[3], true, true, true, disableInterruption, m_fDistance, true);
				}
			}
		}
		m_bIsReplacingCamera = false;
		m_PreActivateControlledEntity = null;
	}
}