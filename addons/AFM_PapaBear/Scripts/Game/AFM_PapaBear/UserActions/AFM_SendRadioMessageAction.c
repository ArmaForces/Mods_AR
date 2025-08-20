class AFM_SendRadioMessageAction : ScriptedUserAction
{
	[Attribute(desc: "Signal name", enums: ParamEnumArray.FromEnum(AFM_ERadioMsg), defvalue: AFM_ERadioMsg.NONE.ToString(), uiwidget: UIWidgets.ComboBox)]
	AFM_ERadioMsg m_eMessageType;
	
	
	//------------------------------------------------------------------------------------------------
	override bool CanBeShownScript(IEntity user)
	{
		return true;
		CharacterControllerComponent charComp = CharacterControllerComponent.Cast(user.FindComponent(CharacterControllerComponent));
		return charComp.GetInspect();
	}

	protected override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity)
	{
		AFM_PapaBearScriptComponent.GetInstance().TransmitBluHqMessage(m_eMessageType);
	}
	
	override bool HasLocalEffectOnlyScript()
	{
		return false;
	}

};