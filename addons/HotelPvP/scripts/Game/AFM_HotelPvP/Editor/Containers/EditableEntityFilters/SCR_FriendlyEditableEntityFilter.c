[BaseContainerProps(), SCR_BaseContainerCustomTitleEnum(EEditableEntityState, "m_State")]
modded class SCR_FriendlyEditableEntityFilter
{
	protected AFM_SpectatorComponent m_pAFM_SpectatorComponent;
	
	override bool CanAdd(SCR_EditableEntityComponent entity)
	{
		bool canAdd = super.CanAdd(entity);
		
		// Consider all entities as friendly when in spectator mode
		return canAdd || (m_pAFM_SpectatorComponent && m_pAFM_SpectatorComponent.IsSpectatorActive());
	}
	
	override void EOnEditorActivate()
	{
		super.EOnEditorActivate();
		
		PlayerController pc = GetGame().GetPlayerController();
		m_pAFM_SpectatorComponent = AFM_SpectatorComponent.Cast(pc.FindComponent(AFM_SpectatorComponent));
	}
}