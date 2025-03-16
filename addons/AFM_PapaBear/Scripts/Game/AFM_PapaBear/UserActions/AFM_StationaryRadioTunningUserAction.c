class AFM_StationaryRadioTuningUserAction : SCR_RadioTuningUserAction
{
	override bool CanBeShownScript(IEntity user)
	{
		if (!m_RadioComp)
			return false;
		
		return true;
	}
}