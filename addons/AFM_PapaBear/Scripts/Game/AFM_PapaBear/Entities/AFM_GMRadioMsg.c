//------------------------------------------------------------------------------------------------
class AFM_GMRadioMsg : ScriptedRadioMessage
{
	static const int INVALID_RADIO_MSG_PARAM = -1;
	
	protected AFM_ERadioMsg m_iRadioMsg;
	protected int m_iFactionId;
	protected float m_fSeed = Math.RandomFloat01();
	protected bool m_bIsPublic = true;
	protected int m_iParam = INVALID_RADIO_MSG_PARAM;
	protected int m_iAuthorID = SCR_CampaignMilitaryBaseComponent.INVALID_PLAYER_INDEX;
	
	//------------------------------------------------------------------------------------------------
	void SetRadioMsg(AFM_ERadioMsg msg)
	{
		m_iRadioMsg = msg;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetFactionId(int factionId)
	{
		m_iFactionId = factionId;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetIsPublic(bool public)
	{
		m_bIsPublic = public;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetParam(int param)
	{
		m_iParam = param;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetAuthorID(int playerID)
	{
		m_iAuthorID = playerID;
	}
	
	//------------------------------------------------------------------------------------------------
	float GetSeed()
	{
		return m_fSeed;
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnDelivery(BaseTransceiver receiver, int freq, float quality)
	{
		Print("Delivering message on freq " + freq);
		IEntity owner = receiver.GetRadio().GetOwner();

		ChimeraCharacter player;

		while (!player)
		{
			player = ChimeraCharacter.Cast(owner);
			
			if (player)
				break;
			
			owner = owner.GetParent();
			
			if (!owner)
				return;
			
		}

		if (player)
		{
			int playerID = GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(owner);
			PlayerController controller = GetGame().GetPlayerManager().GetPlayerController(playerID);
			
			if (!controller)
				return;
			
			AFM_RadioMessageNetworkComponent comp = AFM_RadioMessageNetworkComponent.Cast(controller.FindComponent(AFM_RadioMessageNetworkComponent));
			
			if (comp)
				comp.PlayRadioMsg(m_iRadioMsg, m_iFactionId, m_fSeed, m_bIsPublic, quality, playerID);
		}
	}
};

enum AFM_ERadioMsg
{
	NONE,
	MISSION_START,
	MONTIGNAC_CAPTURED,
	CODE_FOXTROT,
	PROVINS_START,
	PROVINS_CAPTURED,
	ARMAFORCES,
	DONUTS,
	MY_EVERON
};
