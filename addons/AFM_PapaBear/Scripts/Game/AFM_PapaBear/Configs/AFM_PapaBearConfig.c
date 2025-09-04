[BaseContainerProps(configRoot: true)]
class AFM_PapaBearConfig
{
	[Attribute("", UIWidgets.Object, "Definition of radio message types")]
	protected ref array<ref AFM_PapaBearEntryConfig> m_aPapaBearEntryConfigs;
	
	AFM_PapaBearEntryConfig GetEntryConfig(string name, int index = -1)
	{
		if (!m_aPapaBearEntryConfigs || m_aPapaBearEntryConfigs.IsEmpty())
			return null;
		
		foreach(AFM_PapaBearEntryConfig cfg: m_aPapaBearEntryConfigs)
		{
			if (cfg.m_sMessageName == name && cfg.m_iSampleIndex == index)
				return cfg;
		}
		
		return null;
	}
}

[BaseContainerProps()]
class AFM_PapaBearEntryConfig
{
	[Attribute(defvalue: "", desc: "Sample name")]
	string m_sMessageName;
	
	[Attribute(defvalue: "", desc: "Message text to be shown in chat")]
	string m_sMessageText;
	
	[Attribute(defvalue: "-1", desc: "If part of multiple banks provie the sample number")]
	int m_iSampleIndex;
}