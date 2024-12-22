modded enum ChimeraMenuPreset
{
	AFM_ServerSettings
}

class AFM_ServerSettingsMenu: ChimeraMenuBase
{
	//------------------------------------------------------------------------------------------------
	override void OnMenuInit()
	{
		super.OnMenuInit();		
		Print("AFM_ServerSettingsMenu - OnMenuInit");
	}
}