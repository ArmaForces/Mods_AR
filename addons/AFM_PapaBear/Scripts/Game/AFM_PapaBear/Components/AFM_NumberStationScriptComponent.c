class AFM_NumberStationScriptComponentClass : ScriptComponentClass
{
}

class AFM_NumberStationScriptComponent : ScriptComponent
{
	[Attribute(desc: "Delay between characters", defvalue: "800")]
	protected int m_iCharacterDelay;
	
	[Attribute(desc: "Delay between sequences", defvalue: "1200")]
	protected int m_iPauseDelay;
	
	protected SCR_RadioComponent m_RadioComp;
	protected RplComponent m_RplComponent;
	protected ref map<int, ref AFM_NumberStationTask> m_Tasks = new map<int, ref AFM_NumberStationTask>();
	
	override void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);
		
		m_RplComponent = RplComponent.Cast(owner.FindComponent(RplComponent));
		m_RadioComp = SCR_RadioComponent.Cast(owner.FindComponent(SCR_RadioComponent));
	}
	
	void CreateTransmission(AFM_ERadioMsg stationType, string message, bool loopMessage, int frequency, string encryptionKey)
	{
		if (m_Tasks.Contains(frequency))
		{
			Print("Number station already occupies frequency " + frequency, LogLevel.WARNING);
			return;
		}
		
		ref AFM_NumberStationTask task = new AFM_NumberStationTask();
		task.Frequency = frequency;
		task.Message = message;
		task.LoopMessage = loopMessage;
		task.EncryptionKey = encryptionKey;
		task.StationType = stationType;
		
		m_Tasks.Set(frequency, task);
		
		GetGame().GetCallqueue().CallLater(TransmitTaskLoop, m_iPauseDelay, false, frequency, 0);
	}
	
	void TerminateTransmission(int frequency)
	{
		m_Tasks.Remove(frequency);
	}
	
	bool IsTransmissionActive(int frequency)
	{
		return m_Tasks.Contains(frequency);
	}
	
	protected void TransmitTaskLoop(int frequency, int index)
	{
		if (!m_Tasks.Contains(frequency))
			return;
		
		AFM_NumberStationTask task = m_Tasks.Get(frequency);
		
		if (index == task.Message.Length())
		{
			//End of message
			if (task.LoopMessage)
				//treat the end as pause, resume from the beggining
				GetGame().GetCallqueue().CallLater(TransmitTaskLoop, m_iPauseDelay, false, frequency, 0);
			else 
				TerminateTransmission(frequency);
			return;
		}
		
		int sampleIndex = task.Message.Substring(index, 1).ToInt(-1);
		
		if (sampleIndex < 0)
		{
			//invalid character, treat as pause
			GetGame().GetCallqueue().CallLater(TransmitTaskLoop, m_iPauseDelay, false, frequency, index+1);
			return;
		}
		
		TransmitMessage(task.StationType, task.Frequency, task.EncryptionKey, sampleIndex);
		
		GetGame().GetCallqueue().CallLater(TransmitTaskLoop, m_iCharacterDelay, false, frequency, index+1);
	}
	
	protected void TransmitMessage(AFM_ERadioMsg message, int frequency, string key, int sampleIndex)
	{
		BaseRadioComponent radio = m_RadioComp.GetRadioComponent();
		if (!radio || !radio.IsPowered())
			return;
		
		BaseTransceiver transmitter = radio.GetTransceiver(0);
		
		if (!transmitter)
			return;
		
		transmitter.SetFrequency(frequency);
		
		AFM_GMRadioMsg msg = new AFM_GMRadioMsg();
		msg.SetMessageType(AFM_ERadioMsgType.NUMBER_STATION);
		msg.SetRadioMsg(message);
		msg.SetIsPublic(false);
		msg.SetEncryptionKey(key);
		msg.SetSampleIndex(sampleIndex);

		transmitter.BeginTransmission(msg);
	}
	
	static AFM_NumberStationScriptComponent m_numberStationInstance = null;
	static AFM_NumberStationScriptComponent GetInstance()
	{
		if (!m_numberStationInstance)
		{
			IEntity ent = GetGame().GetWorld().FindEntityByName("NumberStation");
			if (!ent)
			{
				Print("NumberStation entity not found in game world!", LogLevel.ERROR);
				return null;
			}
			
			m_numberStationInstance = AFM_NumberStationScriptComponent.Cast(ent.FindComponent(AFM_NumberStationScriptComponent));
			if (!m_numberStationInstance)
			{
				Print("NumberStation component not found in entity!", LogLevel.ERROR);
				return null;
			}
		}
		
		return m_numberStationInstance;
	}
		
	protected bool IsProxy()
	{
		return (m_RplComponent && m_RplComponent.IsProxy());
	}
}

class AFM_NumberStationTask
{
	int Frequency;
	bool LoopMessage;
	string Message;
	string EncryptionKey;
	AFM_ERadioMsg StationType;
}