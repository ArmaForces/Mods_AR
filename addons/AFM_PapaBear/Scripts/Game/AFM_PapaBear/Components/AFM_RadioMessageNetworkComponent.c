class AFM_RadioMessageNetworkComponentClass : ScriptComponentClass
{
}

class AFM_RadioMessageNetworkComponent : ScriptComponent
{
	protected RplComponent m_RplComponent;
	protected AudioHandle m_PlayedRadio = AudioHandle.Invalid;
	
	void PlayMessage(AFM_ERadioMsg msg, int factionId, float seed, float quality)
	{
		SCR_PlayerController pc = SCR_PlayerController.Cast(GetGame().GetPlayerController());
		if (!pc)
			return;
		IEntity player = pc.GetMainEntity();
		if (!player)
			return;
		SCR_CommunicationSoundComponent soundComp = SCR_CommunicationSoundComponent.Cast(player.FindComponent(SCR_CommunicationSoundComponent));
		if (!soundComp)
			return;
		SignalsManagerComponent signalComp = SignalsManagerComponent.Cast(player.FindComponent(SignalsManagerComponent));
		if (!signalComp)
			return;

		int signalSeed = signalComp.AddOrFindSignal("Seed");
		int signalQuality = signalComp.AddOrFindSignal("TransmissionQuality");

		signalComp.SetSignalValue(signalSeed, seed);
		signalComp.SetSignalValue(signalQuality, quality);

		string msgName;
		string chatMessage;

		switch (msg)
		{
			case AFM_ERadioMsg.ARMAFORCES:
			{
				msgName = "ARMAFORCES";
				chatMessage = "PAPA BEAR TO ALL UNITS. ARMAFORCES ARE A BUNCH OF FAGS. OUT";
				break;
			}
			case AFM_ERadioMsg.DONUTS:
			{
				msgName = "DONUTS";
				chatMessage = "PAPA BEAR TO ALL UNITS. I REALLY LOVE DONUTS. YUMMY YUMMY DONUTS HERE I COME. OUT";
				break;
			}
			case AFM_ERadioMsg.MY_EVERON:
			{
				msgName = "MY_EVERON";
				chatMessage = "SWEATY AMERICANS AND DEFAULT DANIELS. THIS IS WHAT MY EVERON IS ALL ABOUT. EVERY INCH OF THIS PLACE SEES ACTION. FROM MORTON TO SAINT PIERRE, YOU CAN SMELL THE RESOURCES";
				break;
			}
			default:
			case AFM_ERadioMsg.NONE: 
			{
				return;
			}
		}
		
		if (!msgName.IsEmpty())
		{
			AudioSystem.TerminateSound(m_PlayedRadio);
			m_PlayedRadio = soundComp.SoundEvent(msgName);
			
			if (!chatMessage.IsEmpty())
			{
				SCR_ChatComponent.RadioProtocolMessage(chatMessage);
			}
			
			if (m_PlayedRadio == AudioHandle.Invalid)
			{
				PrintFormat("PapaBear: Invalid radio handle received. Check signal name %1", msgName, level: LogLevel.WARNING);
			}
		}
	}
	
	void PlayNumberStation(AFM_ERadioMsg msg, float seed, float quality, int sampleIndex)
	{
		SCR_PlayerController pc = SCR_PlayerController.Cast(GetGame().GetPlayerController());
		if (!pc)
			return;
		IEntity player = pc.GetMainEntity();
		if (!player)
			return;
		SCR_CommunicationSoundComponent soundComp = SCR_CommunicationSoundComponent.Cast(player.FindComponent(SCR_CommunicationSoundComponent));
		if (!soundComp)
			return;
		SignalsManagerComponent signalComp = SignalsManagerComponent.Cast(player.FindComponent(SignalsManagerComponent));
		if (!signalComp)
			return;
		
		int signalSeed = signalComp.AddOrFindSignal("Seed");
		int signalQuality = signalComp.AddOrFindSignal("TransmissionQuality");
		int signalIndex = signalComp.AddOrFindSignal("SampleIndex");

		signalComp.SetSignalValue(signalSeed, seed);
		signalComp.SetSignalValue(signalQuality, quality);
		signalComp.SetSignalValue(signalIndex, sampleIndex);
		
		string stationName;
		switch (msg)
		{
			case AFM_ERadioMsg.NUMBER_STATION_E06:
			{
				stationName = "E06";
				break;
			}
			default:
				return;
		}
		
		if (!stationName.IsEmpty())
		{
			AudioSystem.TerminateSound(m_PlayedRadio);
			m_PlayedRadio = soundComp.SoundEvent(stationName);
			
			if (m_PlayedRadio == AudioHandle.Invalid)
			{
				PrintFormat("PapaBear: Invalid radio handle received. Check signal name %1", stationName, level: LogLevel.WARNING);
			}
		}
	} 
	
	void PlayRadioMsg(AFM_ERadioMsgType msgType, AFM_ERadioMsg msg, int FactionId, float seed, bool isPublic, float quality, int playerID, int sampleIndex)
	{
		SCR_PlayerController pc = SCR_PlayerController.Cast(GetGame().GetPlayerController());
		if (isPublic || playerID == pc.GetPlayerId())
			Rpc(RpcDo_PlayRadioMsg,msgType, msg, FactionId, seed, quality, sampleIndex);
	}
	
	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	void RpcDo_PlayRadioMsg(AFM_ERadioMsgType msgType, AFM_ERadioMsg msg, int factionId, float seed, float quality, int sampleIndex)
	{
		switch (msgType)
		{
			case AFM_ERadioMsgType.SINGLE_MESSAGE:
			{
				PlayMessage(msg, factionId, seed, quality);
				return;
			}
			case AFM_ERadioMsgType.NUMBER_STATION:
			{
				PlayNumberStation(msg, seed, quality, sampleIndex);
			}
		}
	}	
}