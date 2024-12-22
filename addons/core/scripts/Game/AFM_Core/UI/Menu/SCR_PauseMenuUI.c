modded class PauseMenuUI: ChimeraMenuBase
{
	protected SCR_ButtonTextComponent m_AFM_ServerSettingsButton;
	
	//------------------------------------------------------------------------------------------------
	override void OnMenuOpen()
	{
		super.OnMenuOpen();

		m_AFM_ServerSettingsButton = SCR_ButtonTextComponent.GetButtonText("AFM_ServerSettings", m_wRoot);
		if (m_AFM_ServerSettingsButton)		
			m_AFM_ServerSettingsButton.m_OnClicked.Insert(AFM_OnServerSettings);
	}
	
	//------------------------------------------------------------------------------------------------
	private void AFM_OnServerSettings()
	{
		GetGame().GetMenuManager().OpenMenu(ChimeraMenuPreset.AFM_ServerSettings);
	}
};



