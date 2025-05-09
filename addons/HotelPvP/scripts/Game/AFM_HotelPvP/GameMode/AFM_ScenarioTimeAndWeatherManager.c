class AFM_ScenarioTimeAndWeatherManagerClass : SCR_BaseGameModeComponentClass
{}

class AFM_ScenarioTimeAndWeatherManager : SCR_BaseGameModeComponent
{
	//! If enabled custom weather Id will be used on session start. Authority only.
	[Attribute(defvalue: "0", desc: "If enabled, random weather Id will be used. Authority only.", category: "Environment")]
	bool m_bUseCustomWeather;
	
	[Attribute(desc: "Array of weather ids to choose from on game start")]
	ref array<string> m_aWeatherTemplates;

	//! If enabled custom time of the day will be used on session start. Authority only.
	[Attribute(defvalue: "0", desc: "If enabled, custom time of the day will be used. Authority only.", category: "Environment")]
	bool m_bUseCustomTime;

	//! Time of the day set on game start. Authority only.
	[Attribute(desc: "Time of the day set on game start. Authority only.", category: "Environment", params: "0 24 0.01")]
	ref array<float> m_aTimeOfDayTemplates; 

		
	//------------------------------------------------------------------------------------------------
	//! Initialize the manager.
	override void EOnInit(IEntity owner)
	{
		super.EOnInit(owner);

		if(SCR_Global.IsEditMode())
			return;
	
		if (m_bUseCustomTime)
			SetTimeOfTheDay(m_aTimeOfDayTemplates.GetRandomElement());

		if (m_bUseCustomWeather)
			SetWeather(m_aWeatherTemplates.GetRandomElement());
	}
	
	//------------------------------------------------------------------------------------------------
	//! Initializes this manager component and hooks up events.
	protected override void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);
		SetEventMask(owner, EntityEvent.INIT);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Forcefully sets weather to provided weatherId. Authority only.
	protected void SetWeather(string weatherId)
	{
		if (!m_pGameMode.IsMaster())
			return;

		if (weatherId.IsEmpty())
			return;

		ChimeraWorld world = GetOwner().GetWorld();
		if (!world)
			return;
		
		TimeAndWeatherManagerEntity weatherManager = world.GetTimeAndWeatherManager();
		if (!weatherManager)
		{
			Print("Cannot initialize weather: TimeAndWeatherManagerEntity not found!", LogLevel.WARNING);
			return;
		}

		Print("AFM_ScenarioTimeAndWeatherManager: Setting weather to " + weatherId);
		weatherManager.ForceWeatherTo(true, weatherId, 0.0);
	}

	//------------------------------------------------------------------------------------------------
	//! Forcefully sets time of the day to provided value. Authority only.
	protected void SetTimeOfTheDay(float timeOfDay)
	{
		if (!m_pGameMode.IsMaster())
			return;

		ChimeraWorld world = GetOwner().GetWorld();
		if (!world)
			return;
		
		TimeAndWeatherManagerEntity weatherManager = world.GetTimeAndWeatherManager();
		if (!weatherManager)
		{
			Print("Cannot initialize TimeOfTheDay: TimeAndWeatherManagerEntity not found!", LogLevel.WARNING);
			return;
		}

		Print("AFM_ScenarioTimeAndWeatherManager: Setting time to " + timeOfDay);
		weatherManager.SetTimeOfTheDay(timeOfDay, true);
	}
	
}