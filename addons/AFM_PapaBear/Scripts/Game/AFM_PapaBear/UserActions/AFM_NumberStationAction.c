class AFM_NumberStationAction : ScriptedUserAction
{
	[Attribute(desc: "Station name")]
	string m_sStationName;
	
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
		BaseRadioComponent radio = m_RadioComp.GetRadioComponent();
		if (!radio || !radio.IsPowered())
			return;
		
		BaseTransceiver transmitter = radio.GetTransceiver(0);
		int freq = transmitter.GetFrequency();
		
		m_bIsRunning = !m_bIsRunning;
		AFM_NumberStationScriptComponent numberStation = AFM_NumberStationScriptComponent.GetInstance();
		
		if (m_bIsRunning) 
			numberStation.CreateTransmission(m_sStationName, m_sNumberStationMessage,  m_bIsMessageLooped, freq, radio.GetEncryptionKey());
		else
			numberStation.TerminateTransmission(freq);
		
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

};