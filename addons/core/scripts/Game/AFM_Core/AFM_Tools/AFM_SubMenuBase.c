class AFM_SubMenuBase: SCR_SubMenuBase
{
	protected ScrollLayoutWidget m_wScroll;

	//------------------------------------------------------------------------------------------------
	override void OnTabCreate(Widget menuRoot, ResourceName buttonsLayout, int index)
	{
		super.OnTabCreate(menuRoot, buttonsLayout, index);
		m_wScroll = ScrollLayoutWidget.Cast(m_wRoot.FindAnyWidget("ScrollLayout0"));
	}

	//------------------------------------------------------------------------------------------------
	override void OnTabShow()
	{
		super.OnTabShow();

		// Reset the focused Widget, to prevent that a Widget from a previous tab is still focused and controlled.
		GetGame().GetWorkspace().SetFocusedWidget(null);
		
		if (m_wScroll)
			m_wScroll.SetSliderPos(0, 0);
	}
}
