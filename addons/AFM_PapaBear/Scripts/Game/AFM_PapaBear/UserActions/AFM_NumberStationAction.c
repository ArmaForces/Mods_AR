class AFM_NumberStationAction : ScriptedUserAction
{
	[Attribute(desc: "Station name", enums: ParamEnumArray.FromEnum(AFM_ERadioMsg), defvalue: AFM_ERadioMsg.NONE.ToString(), uiwidget: UIWidgets.ComboBox)]
	AFM_ERadioMsg m_eMessageType;
	
	[Attribute(desc: "Signal to transmit. Any non numeric characters will be interpreted as pause", defvalue: "1111 2137 2137 2137 1111")]
	protected string m_sNumberStationMessage;
	
	[Attribute(desc: "Delay between characters", defvalue: "800")]
	protected int m_iCharacterDelay;
	
	[Attribute(desc: "Delay between sequences", defvalue: "1200")]
	protected int m_iPauseDelay;
	
	[Attribute(desc: "Should the message loop", defvalue: "1")]
	protected bool m_bIsMessageLooped;
	
	protected bool m_bIsRunning = false;
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
		m_bIsRunning = !m_bIsRunning;
		
		if (m_bIsRunning) 
		{
			SendNumberStationMessage(0);
		}
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
	
	override bool GetActionNameScript(out string outName)
	{
		BaseRadioComponent radio = m_RadioComp.GetRadioComponent();
		if (!radio || !radio.IsPowered())
			return false;
		
		BaseTransceiver transmitter = radio.GetTransceiver(0);
		int freq = transmitter.GetFrequency();
		
		if (m_bIsRunning)
			outName = "Stop transmiting";
		else 
			outName = "Begin transmiting. Message " + m_sNumberStationMessage + ", frequency: " + freq;
		
		return true;
	}
	
	
	void SendNumberStationMessage(int index)
	{
		if (!m_bIsRunning) return;
		
		if (index == m_sNumberStationMessage.Length())
		{
			//End of message
			if (m_bIsMessageLooped)
				//treat the end as pause, resume from the beggining
				GetGame().GetCallqueue().CallLater(SendNumberStationMessage, m_iPauseDelay, false, 0);
			else 
				m_bIsRunning = false;
			return;
		}
		
		int sampleIndex = m_sNumberStationMessage.Substring(index, 1).ToInt(-1);
		
		if (sampleIndex < 0)
		{
			//invalid character, treat as pause
			GetGame().GetCallqueue().CallLater(SendNumberStationMessage, m_iPauseDelay, false, index+1);
			return;
		}
		
		//we have valid sample index, begin transmission
		BaseRadioComponent radio = m_RadioComp.GetRadioComponent();
		if (!radio || !radio.IsPowered())
			return;
		
		BaseTransceiver transmitter = radio.GetTransceiver(0);
		
		if (!transmitter)
			return;
		
		AFM_GMRadioMsg msg = new AFM_GMRadioMsg();
		msg.SetMessageType(AFM_ERadioMsgType.NUMBER_STATION);
		msg.SetRadioMsg(m_eMessageType);
		msg.SetIsPublic(false);
		msg.SetEncryptionKey(radio.GetEncryptionKey());
		msg.SetSampleIndex(sampleIndex);
		transmitter.BeginTransmission(msg);
		
		GetGame().GetCallqueue().CallLater(SendNumberStationMessage, m_iCharacterDelay, false, index+1);
	}

};