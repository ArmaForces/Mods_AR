class AFM_SendRadioMessageAction : ScriptedUserAction
{
	[Attribute(desc: "Signal name")]
	string m_eMessageType;
	
	
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
		AFM_PapaBearScriptComponent.GetInstance().TransmitRedHqMessage(m_eMessageType);
	}
	
	override bool HasLocalEffectOnlyScript()
	{
		return false;
	}

};