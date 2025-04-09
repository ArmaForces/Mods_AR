enum AFM_EVONBeep
{
	OFF		= 1 << 0,
	NORMAL	= 1 << 1,
	LOW		= 1 << 2,
}

class AFM_VONSettings : ModuleGameSettings
{
	private const string GUID_ACE_RADIO = "64475DC102F2BDA4";
	
	private static bool m_bEnabledLoaded;
	private static bool m_bEnabled;
	
	static bool Enabled()
	{
		// we can't simply use GameProject.GetAddonID as ACE_Radio can be available but not loaded
		if (!m_bEnabledLoaded)
		{
			// TODO this could be extracted into an utility class if we will use it in more places
			array<string> addonGUIDs = {};
			GameProject.GetLoadedAddons(addonGUIDs);

			m_bEnabled = !addonGUIDs.Contains(GUID_ACE_RADIO);
			m_bEnabledLoaded = true;
		}
		
		return m_bEnabled;
	}
	
	[Attribute(AFM_EVONBeep.NORMAL.ToString(), uiwidget: UIWidgets.ComboBox, desc: "Sound to play when activating VoN Ch1", enums: ParamEnumArray.FromEnum(AFM_EVONBeep))]
	AFM_EVONBeep m_BeepCh1;

	[Attribute(AFM_EVONBeep.LOW.ToString(), uiwidget: UIWidgets.ComboBox, desc: "Sound to play when activating VoN Ch1", enums: ParamEnumArray.FromEnum(AFM_EVONBeep))]
	AFM_EVONBeep m_BeepCh2;
}
