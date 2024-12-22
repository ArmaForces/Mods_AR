modded enum ChimeraMenuPreset
{
	AFM_ToolsMenu
}

class AFM_ToolsMenu: SCR_SuperMenuBase
{
	protected Widget m_wRoot;
	protected SCR_TabViewComponent m_Tabs;
	
	//------------------------------------------------------------------------------------------------
	override void OnMenuOpen()
	{
		super.OnMenuOpen();		
		Print("AFM_ServerSettingsMenu - OnMenuInit");
		
		m_wRoot = GetRootWidget();
		
		SCR_InputButtonComponent back = SCR_InputButtonComponent.GetInputButtonComponent("Back", m_wRoot);
		back.m_OnActivated.Insert(Close);
		
		m_Tabs = GetSuperMenu().GetTabView();
		
		#ifdef WORKBENCH
		AddTestTab();
		#endif
		
		m_Tabs.ShowTab(0, playSound: false);
	}
	
#ifdef WORKBENCH
	private void AddTestTab()
	{
		m_Tabs.AddTab(
			layout: "{0C2FFE60CB37F8CA}UI/layouts/AFM_ServerSettings/AFM_Tools_TabContentBase.layout",
			identifier: "test1",
			content: "Test 1",
		);
		m_Tabs.AddTab(
			layout: "{7C8EFF077F074206}UI/layouts/Menus/SettingsMenu/EditorSettings.layout",
			identifier: "test2",
			content: "Test 2",
		);
	}
#endif
}