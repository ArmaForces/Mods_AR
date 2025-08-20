class AFM_PapaBearScriptComponentClass : ScriptComponentClass
{
}

class AFM_PapaBearScriptComponent : ScriptComponent
{
	[Attribute("coldBorsht", desc: "Redfor encryption key")]
	protected string m_sRedKey;
	
	[Attribute("48000", desc: "Redfor platoon frequency")]
	protected int m_iRedFreq;
	
	[Attribute("chickenNuggets", desc: "Blufor encryption key")]
	protected string m_sBluKey;
	
	[Attribute("42000", desc: "Blufor platoon frequency")]
	protected int m_iBluFreq;
	
	protected SCR_RadioComponent m_RadioComp;
	protected RplComponent m_RplComponent;
	
	override void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);
		
		m_RplComponent = RplComponent.Cast(owner.FindComponent(RplComponent));
		m_RadioComp = SCR_RadioComponent.Cast(owner.FindComponent(SCR_RadioComponent));
	}
	
	void TransmitBluHqMessage(AFM_ERadioMsg message)
	{
		TransmitMessage(message, m_iBluFreq, m_sBluKey);
	}
	
	void TransmitRedHqMessage(AFM_ERadioMsg message)
	{
		TransmitMessage(message, m_iRedFreq, m_sRedKey);
	}
	
	void TransmitMessage(AFM_ERadioMsg message, int frequency, string key)
	{
		BaseRadioComponent radio = m_RadioComp.GetRadioComponent();
		if (!radio || !radio.IsPowered())
			return;
		
		BaseTransceiver transmitter = radio.GetTransceiver(0);
		
		if (!transmitter)
			return;
		
		transmitter.SetFrequency(frequency);
		
		AFM_GMRadioMsg msg = new AFM_GMRadioMsg();
		msg.SetMessageType(AFM_ERadioMsgType.SINGLE_MESSAGE);
		msg.SetRadioMsg(message);
		msg.SetIsPublic(false);
		msg.SetEncryptionKey(key);

		transmitter.BeginTransmission(msg);
	}
	
	static AFM_PapaBearScriptComponent m_PapaBearInstance = null;
	static AFM_PapaBearScriptComponent GetInstance()
	{
		if (!m_PapaBearInstance)
		{
			IEntity ent = GetGame().GetWorld().FindEntityByName("PapaBear");
			if (!ent)
			{
				Print("PapaBear entity not found in game world!", LogLevel.ERROR);
				return null;
			}
			
			m_PapaBearInstance = AFM_PapaBearScriptComponent.Cast(ent.FindComponent(AFM_PapaBearScriptComponent));
			if (!m_PapaBearInstance)
			{
				Print("PapaBear component not found in entity!", LogLevel.ERROR);
				return null;
			}
		}
		
		return m_PapaBearInstance;
	}
		
	protected bool IsProxy()
	{
		return (m_RplComponent && m_RplComponent.IsProxy());
	}
}