class AFM_SettingsSubMenu : SCR_SettingsSubMenuBase
{
	const string MODULE_RADIO = "AFM_VONSettings";

	//------------------------------------------------------------------------------------------------
	override void OnTabCreate(Widget menuRoot, ResourceName buttonsLayout, int index)
	{
		BaseContainer radioSettings = GetGame().GetGameUserSettings().GetModule(MODULE_RADIO);

		super.OnTabCreate(menuRoot, buttonsLayout, index);

		if (!radioSettings)
			return;

		// Radio beep Ch1
		SCR_SelectionWidgetComponent checkBoxBeep1 = SCR_SelectionWidgetComponent.GetSelectionComponent("Beep_Ch1", m_wRoot);
		if (checkBoxBeep1)
		{
			int value;
			radioSettings.Get("m_BeepCh1", value);

			checkBoxBeep1.SetCurrentItem(value >> 1, false, false); //--- Shift the value, because it's a flag
			checkBoxBeep1.m_OnChanged.Insert(SetBeepCh1);
		}
		else
		{
			Print("Radio setting 'Beep_Ch1' not found", LogLevel.WARNING);
		}

		// Radio beep Ch1
		SCR_SelectionWidgetComponent checkBoxBeep2 = SCR_SelectionWidgetComponent.GetSelectionComponent("Beep_Ch2", m_wRoot);
		if (checkBoxBeep2)
		{
			int value;
			radioSettings.Get("m_BeepCh2", value);

			checkBoxBeep2.SetCurrentItem(value >> 1, false, false); //--- Shift the value, because it's a flag
			checkBoxBeep2.m_OnChanged.Insert(SetBeepCh2);
		}
		else
		{
			Print("Radio setting 'Beep_Ch2' not found", LogLevel.WARNING);
		}
	}

	//------------------------------------------------------------------------------------------------
	void SetBeepCh1(SCR_SelectionWidgetComponent checkBox, int state)
	{
		SetBeepProp(checkBox, state, "m_BeepCh1");
	}

	//------------------------------------------------------------------------------------------------
	void SetBeepCh2(SCR_SelectionWidgetComponent checkBox, int state)
	{
		SetBeepProp(checkBox, state, "m_BeepCh2");
	}

	//------------------------------------------------------------------------------------------------
	void SetBeepProp(SCR_SelectionWidgetComponent checkBox, int state, string prop)
	{
		BaseContainer radioSettings = GetGame().GetGameUserSettings().GetModule(MODULE_RADIO);
		if (!radioSettings)
			return;

		state = 1 << state; //--- Shift the value, because it's a flag

		radioSettings.Set(prop, state);
		GetGame().UserSettingsChanged();
	}
}
