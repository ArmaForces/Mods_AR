class AFM_SendRadioMessageAction : ScriptedUserAction
{
	[Attribute(desc: "Signal name", enums: ParamEnumArray.FromEnum(AFM_ERadioMsg), defvalue: AFM_ERadioMsg.NONE.ToString(), uiwidget: UIWidgets.ComboBox)]
	AFM_ERadioMsg m_eMessageType;
	
	protected SCR_RadioComponent m_RadioComp;
	protected SignalsManagerComponent m_SignalsMgr;
	
	//------------------------------------------------------------------------------------------------
	override bool CanBeShownScript(IEntity user)
	{
		if (!m_RadioComp)
			return false;

		return true;
		CharacterControllerComponent charComp = CharacterControllerComponent.Cast(user.FindComponent(CharacterControllerComponent));
		return charComp.GetInspect();
	}

	protected override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity)
	{
		int playerId = GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(pUserEntity);
		
		SendHQMessage(m_eMessageType, playerId, false);
	}

	override void Init(IEntity pOwnerEntity, GenericComponent pManagerComponent)
	{
		m_RadioComp = SCR_RadioComponent.Cast(pOwnerEntity.FindComponent(SCR_RadioComponent));
		m_SignalsMgr = SignalsManagerComponent.Cast(pOwnerEntity.FindComponent(SignalsManagerComponent));
	}
	
	override bool HasLocalEffectOnlyScript()
	{
		return false;
	}
	
	void SendHQMessage(AFM_ERadioMsg msgType, int playerId, bool public = true)
	{
		if (msgType == SCR_ERadioMsg.NONE)
			return;
		
		
		BaseRadioComponent radio = m_RadioComp.GetRadioComponent();
		if (!radio || !radio.IsPowered())
			return;
		
		BaseTransceiver transmitter = radio.GetTransceiver(0);
		
		if (!transmitter)
			return;
		
		AFM_GMRadioMsg msg = new AFM_GMRadioMsg();
		msg.SetRadioMsg(msgType);
		//msg.SetFactionId();
		msg.SetIsPublic(public);
		msg.SetAuthorID(playerId);
		msg.SetEncryptionKey(radio.GetEncryptionKey());

		transmitter.BeginTransmission(msg);
	}

};