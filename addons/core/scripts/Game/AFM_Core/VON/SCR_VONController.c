modded class SCR_VONController: ScriptComponent
{
	const string SOUND_START_1 = "Sounds/AFM_Core/VON/beep_sr_on.wav";
	const string SOUND_START_2 = "Sounds/AFM_Core/VON/beep_lr_on.wav";
	const string SOUND_END = "Sounds/AFM_Core/VON/click_off.wav";
	
	override void SetActiveTransmit(notnull SCR_VONEntry entry)
	{
		super.SetActiveTransmit(entry);
		
		SCR_VONEntryRadio radioEntry = SCR_VONEntryRadio.Cast(entry);
		if (!radioEntry)
			return;
		
		string sound;
		if (radioEntry.GetTransceiverNumber() == 1)
		{
			sound = SOUND_START_1;
		}
		else
		{
			sound = SOUND_START_2;
		}
		
		// TODO use sound event SCR_UISoundEntity.SoundEvent(SCR_SoundEvent.SOUND_RADIO_TURN_OFF);
		AudioSystem.PlaySound(sound);
	}
	
	override void DeactivateVON(EVONTransmitType transmitType = EVONTransmitType.NONE)
	{		
		if (m_bIsActive)
		{
			if (transmitType != EVONTransmitType.DIRECT)
			{
				AudioSystem.PlaySound(SOUND_END);	
			}
		};
		
		// TODO use sound event SCR_UISoundEntity.SoundEvent(SCR_SoundEvent.SOUND_RADIO_TURN_OFF);
		super.DeactivateVON(transmitType);
	}
}