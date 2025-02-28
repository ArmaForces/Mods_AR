class AFM_CarryHostageAction: ACE_Carrying_BaseUserAction
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
	
	override bool CanBePerformedScript(IEntity user)
	{
		ChimeraCharacter userChar = ChimeraCharacter.Cast(user);
		if (!userChar)
			return false;
		
		// Check if user can take crouch stance
		SCR_CharacterControllerComponent userCharCtrl = SCR_CharacterControllerComponent.Cast(userChar.GetCharacterController());
		if (!userCharCtrl || !userCharCtrl.CanChangeStance(ECharacterStanceChange.STANCECHANGE_TOCROUCH))
		{
			SetCannotPerformReason("#AR-Keybind_StanceProne");
			return false;
		}
		
		ChimeraCharacter ownerChar = ChimeraCharacter.Cast(GetOwner());
		if (!ownerChar)
			return false;
	
		
		// Trying to carry while unit is ragdolling will break things
		if (ownerChar.GetAnimationComponent().IsRagdollActive())
		{
			SetCannotPerformReason("#AR-UserActionUnavailable");
			return false;
		}

		return true;
	}
	
	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity)
	{
		super.PerformAction(pOwnerEntity, pUserEntity);
		
		SCR_ChimeraCharacter userChar = SCR_ChimeraCharacter.Cast(pUserEntity);
		if (!userChar)
			return;
		
		SCR_CharacterControllerComponent userCharController = SCR_CharacterControllerComponent.Cast(userChar.GetCharacterController());
		if (!userCharController)
			return;
		
		userCharController.ACE_Carrying_CarryCasualty(SCR_ChimeraCharacter.Cast(pOwnerEntity));
	}
}