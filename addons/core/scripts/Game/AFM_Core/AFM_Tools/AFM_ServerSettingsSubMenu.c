class AFM_ServerSettingsSubMenu : AFM_SubMenuBase
{
	protected bool m_bLoadedMissionHeader = false;
	protected bool m_bLoadedAceSettings = false;
	
	protected TextWidget m_wMissionHeaderText;
	protected TextWidget m_wAceSettingsText;
	
	//------------------------------------------------------------------------------------------------
	override void OnTabCreate(Widget menuRoot, ResourceName buttonsLayout, int index)
	{
		super.OnTabCreate(menuRoot, buttonsLayout, index);
		
		m_wMissionHeaderText = TextWidget.Cast(m_wRoot.FindAnyWidget("TextMissionHeader"));
		m_wAceSettingsText = TextWidget.Cast(m_wRoot.FindAnyWidget("TextAceSettings"));
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnTabShow()
	{
		super.OnTabShow();
		
		if (!m_bLoadedMissionHeader || !m_bLoadedAceSettings)
			LoadTabData();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void LoadTabData()
	{
		SCR_PlayerController pc = SCR_PlayerController.Cast(GetGame().GetPlayerController());
		AFM_SettingsComponent settingsComponent = AFM_SettingsComponent.Cast(pc.FindComponent(AFM_SettingsComponent));
		
		if (m_wMissionHeaderText)
		{
			Print("UI: Loading mission header data");
			SCR_LoadingOverlay.ShowForWidget(m_wMissionHeaderText.GetParent());
			
			settingsComponent.m_pMissionHeaderResponseInvoker.Insert(OnMissionHeader);
			#ifdef WORKBENCH
			GetGame().GetCallqueue().CallLater(settingsComponent.RequestMissionHeader, 700);
			#else
			settingsComponent.RequestMissionHeader();
			#endif
		}
		
		if (m_wAceSettingsText)
		{
			Print("UI: Loading ACE settings data");
			SCR_LoadingOverlay.ShowForWidget(m_wAceSettingsText.GetParent());
			
			settingsComponent.m_pAceSettingsResponseInvoker.Insert(OnAceSettings);
			#ifdef WORKBENCH
			GetGame().GetCallqueue().CallLater(settingsComponent.RequestAceSettings, 1400);
			#else
			settingsComponent.RequestAceSettings();
			#endif
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Set mission header data
	protected void OnMissionHeader(AFM_ContainerTextResponse response)
	{
		m_bLoadedMissionHeader = true;
		SCR_LoadingOverlay.HideForWidget(m_wMissionHeaderText.GetParent());
		
		m_wMissionHeaderText.SetText(response.m_sContent);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Set ACE settings data
	protected void OnAceSettings(AFM_ContainerTextResponse response)
	{
		m_bLoadedAceSettings = true;
		SCR_LoadingOverlay.HideForWidget(m_wAceSettingsText.GetParent());
		
		m_wAceSettingsText.SetText(response.m_sContent);
	}
}