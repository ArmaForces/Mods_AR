class AFM_CarryHostageAction: ACE_Carrying_CarryUserAction
{
	override bool CanBeShownScript(IEntity user)
	{
		ChimeraCharacter ownerChar = ChimeraCharacter.Cast(GetOwner());
		if (!ownerChar || ownerChar.IsInVehicle())
			return false;
		
		SCR_CharacterControllerComponent ownerCharCtrl = SCR_CharacterControllerComponent.Cast(ownerChar.GetCharacterController());
		if (!ownerCharCtrl  || ownerCharCtrl.GetLifeState() == ECharacterLifeState.DEAD)
			return false;

		return true;
	}
}