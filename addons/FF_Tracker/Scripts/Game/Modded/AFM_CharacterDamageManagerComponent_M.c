modded class SCR_CharacterDamageManagerComponent : SCR_DamageManagerComponent
{
	override void OnPostInit(IEntity owner) 
	{
		super.OnPostInit(owner);
		
		if (Replication.IsServer())
			GetOnDamageStateChanged().Insert(AFM_OnDamageStateChanged);
	}
	
	protected void AFM_OnDamageStateChanged(EDamageState state)
	{		
		Instigator instigator = GetInstigator();
		
		int playerId = instigator.GetInstigatorPlayerID();
		if (playerId == 0)
			return;
		
		PlayerManager pm = GetGame().GetPlayerManager();
		if (!pm)
			return;
		
		int ownerId = pm.GetPlayerIdFromControlledEntity(GetOwner());
		if (ownerId == 0)
			return;
		
		string instigatorName = pm.GetPlayerName(playerId);
		string ownerName = pm.GetPlayerName(ownerId);
		
		string message = string.Format("AFM_FriendlyFireTracker: %1 shot at %2. Damage state %3", instigatorName, ownerName, EDamageStateToString(state));
		
		Rpc(SendMessage, message);
		Print(message, LogLevel.WARNING);
	}
	
	protected string EDamageStateToString(EDamageState state)
	{
		switch (state)
		{
			case EDamageState.DESTROYED:
				return "DESTROYED";
			case EDamageState.UNDAMAGED:
				return "UNDAMAGED";
			case EDamageState.INTERMEDIARY:
				return "INTERMEDIARY";
		}
		
		return state.ToString();
	}
	
	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	protected void SendMessage(string message)
	{
		Print(message, LogLevel.WARNING);
	}
}