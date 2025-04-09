modded class SCR_VONController: ScriptComponent
{
	const string SOUND_START_1 = "Sounds/AFM_Core/VON/beep_sr_on.wav";
	const string SOUND_START_2 = "Sounds/AFM_Core/VON/beep_lr_on.wav";
	const string SOUND_END = "Sounds/AFM_Core/VON/click_off.wav";

	protected AFM_EVONBeep m_eBeepTypeCh1 = AFM_EVONBeep.OFF;
	protected AFM_EVONBeep m_eBeepTypeCh2 = AFM_EVONBeep.OFF;
	protected bool m_bBeepEnd = false;

	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);

		AFM_LoadSettings();
		GetGame().OnUserSettingsChangedInvoker().Insert(AFM_LoadSettings);
	}

	//------------------------------------------------------------------------------------------------
	protected void AFM_LoadSettings()
	{
		BaseContainer settings = GetGame().GetGameUserSettings().GetModule(AFM_SettingsSubMenu.MODULE_RADIO);
		if (AFM_VONSettings.Enabled())
		{
			settings.Get("m_BeepCh1", m_eBeepTypeCh1);
			settings.Get("m_BeepCh2", m_eBeepTypeCh2);
			m_bBeepEnd = true;

			PrintFormat("SCR_VONController.AFM_LoadSettings: %1, %2", m_eBeepTypeCh1, m_eBeepTypeCh2, level: LogLevel.DEBUG);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected string AFM_GetBeepSound(int transreceiverNumber)
	{
		AFM_EVONBeep beepType;
		if (transreceiverNumber == 1)
			beepType = m_eBeepTypeCh1;
		else
			beepType = m_eBeepTypeCh2;

		switch (beepType) {
			case AFM_EVONBeep.LOW:
				return SOUND_START_2;
			case AFM_EVONBeep.NORMAL:
				return SOUND_START_1;
		}

		return "";
	}

	//------------------------------------------------------------------------------------------------
	override void SetActiveTransmit(notnull SCR_VONEntry entry)
	{
		super.SetActiveTransmit(entry);

		SCR_VONEntryRadio radioEntry = SCR_VONEntryRadio.Cast(entry);
		if (!radioEntry)
			return;

		// TODO use sound event SCR_UISoundEntity.SoundEvent(SCR_SoundEvent.SOUND_RADIO_TURN_OFF);
		string beep = AFM_GetBeepSound(radioEntry.GetTransceiverNumber());
		if (beep != "")
			AudioSystem.PlaySound(beep);
	}

	//------------------------------------------------------------------------------------------------
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
