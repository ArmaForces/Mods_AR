enum AFM_EVONBeep
{
	OFF		= 1 << 0,
	NORMAL	= 1 << 1,
	LOW		= 1 << 2,
}

class AFM_VONSettings : ModuleGameSettings
{
	[Attribute(AFM_EVONBeep.NORMAL.ToString(), uiwidget: UIWidgets.ComboBox, desc: "Sound to play when activating VoN Ch1", enums: ParamEnumArray.FromEnum(AFM_EVONBeep))]
	AFM_EVONBeep m_BeepCh1;

	[Attribute(AFM_EVONBeep.LOW.ToString(), uiwidget: UIWidgets.ComboBox, desc: "Sound to play when activating VoN Ch1", enums: ParamEnumArray.FromEnum(AFM_EVONBeep))]
	AFM_EVONBeep m_BeepCh2;
}
