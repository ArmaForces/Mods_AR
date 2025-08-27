class AFM_RadioMessageNetworkComponentClass : ScriptComponentClass
{
}

class AFM_RadioMessageNetworkComponent : ScriptComponent
{
	protected RplComponent m_RplComponent;
	protected AudioHandle m_PlayedRadio = AudioHandle.Invalid;
	protected ref AFM_PapaBearConfig m_Config = null;
	
	static string m_sPapaBearCfgPath = "Configs/PapaBearConfig.conf";
	
	void PlayMessage(string msg, int factionId, float seed, float quality)
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
		
		AFM_PapaBearEntryConfig cfgEntry = GetConfig().GetEntryConfig(msg);

		if (cfgEntry)
		{
			AudioSystem.TerminateSound(m_PlayedRadio);
			m_PlayedRadio = soundComp.SoundEvent(cfgEntry.m_sMessageName);
			
			if (!cfgEntry.m_sMessageText.IsEmpty())
			{
				SCR_ChatComponent.RadioProtocolMessage(cfgEntry.m_sMessageText);
			}
			
			if (m_PlayedRadio == AudioHandle.Invalid)
			{
				PrintFormat("PapaBear: Invalid radio handle received. Check signal name %1", cfgEntry.m_sMessageName, level: LogLevel.WARNING);
			}
			array<string> signalNames = new array<string>();
			soundComp.GetSignalNames(signalNames);
			Print(signalNames);
		}
	}
	
	void PlayNumberStation(string msg, float seed, float quality, int sampleIndex)
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
		
		if (!msg.IsEmpty())
		{
			AudioSystem.TerminateSound(m_PlayedRadio);
			m_PlayedRadio = soundComp.SoundEvent(msg);
			
			if (m_PlayedRadio == AudioHandle.Invalid)
			{
				PrintFormat("PapaBear: Invalid radio handle received. Check signal name %1", msg, level: LogLevel.WARNING);
			}
		}
	} 
	
	void PlayRadioMsg(AFM_ERadioMsgType msgType, string msg, int FactionId, float seed, bool isPublic, float quality, int playerID, int sampleIndex)
	{
		SCR_PlayerController pc = SCR_PlayerController.Cast(GetGame().GetPlayerController());
		if (!pc)
			return; 
		if (isPublic || playerID == pc.GetPlayerId())
			Rpc(RpcDo_PlayRadioMsg, msgType, msg, FactionId, seed, quality, sampleIndex);
	}
	
	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	void RpcDo_PlayRadioMsg(AFM_ERadioMsgType msgType, string msg, int factionId, float seed, float quality, int sampleIndex)
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
	
	protected AFM_PapaBearConfig GetConfig()
	{
		if (m_Config)
			return m_Config;
		
		m_Config = SCR_ConfigHelperT<AFM_PapaBearConfig>.GetConfigObject(m_sPapaBearCfgPath);
		
		return m_Config;
	}
}