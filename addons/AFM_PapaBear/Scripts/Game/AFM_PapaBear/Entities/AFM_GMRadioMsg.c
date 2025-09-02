//------------------------------------------------------------------------------------------------
class AFM_GMRadioMsg : ScriptedRadioMessage
{
	static const int INVALID_RADIO_MSG_PARAM = -1;
	
	protected AFM_ERadioMsgType m_iMessageType;
	protected string m_sRadioMsg;
	protected int m_iFactionId;
	protected float m_fSeed = Math.RandomFloat01();
	protected bool m_bIsPublic = true;
	protected int m_iParam = INVALID_RADIO_MSG_PARAM;
	protected int m_iSampleIndex = -1;
	
	//------------------------------------------------------------------------------------------------
	void SetMessageType(AFM_ERadioMsgType msgType)
	{
		m_iMessageType = msgType;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetRadioMsg(string msg)
	{
		m_sRadioMsg = msg;
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
	float GetSeed()
	{
		return m_fSeed;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetSampleIndex(int index)
	{
		m_iSampleIndex = index;
	}
	
	
	//------------------------------------------------------------------------------------------------
	override void OnDelivery(BaseTransceiver receiver, int freq, float quality)
	{
		PrintFormat("Delivering message type %1 on freq %2", m_iMessageType, freq, level: LogLevel.NORMAL);
		IEntity owner = receiver.GetRadio().GetOwner();

		ChimeraCharacter player;

		while (!player)
		{
			player = ChimeraCharacter.Cast(owner);
			
			if (player)
				break;
			
			owner = owner.GetParent();
			
			if (!owner) {
				Print("Failed to find player character", LogLevel.ERROR);
				return;
			}
			
		}

		if (player)
		{
			int playerID = GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(owner);
			PlayerController controller = GetGame().GetPlayerManager().GetPlayerController(playerID);
			
			if (!controller)
			{
				Print("No PlayerController found", LogLevel.ERROR);
				return;
			}
			
			AFM_RadioMessageNetworkComponent comp = AFM_RadioMessageNetworkComponent.Cast(controller.FindComponent(AFM_RadioMessageNetworkComponent));
			if (!comp)
			{
				Print("No AFM_RadioMessageNetworkComponent found on player controller entity!", LogLevel.ERROR);
				return;
			}
			
			comp.PlayRadioMsg(m_iMessageType, m_sRadioMsg, m_iFactionId, m_fSeed, m_bIsPublic, quality, playerID, m_iSampleIndex);
		}
	}
};

enum AFM_ERadioMsgType
{
	SINGLE_MESSAGE,
	NUMBER_STATION
}
