[BaseContainerProps(namingConvention: NamingConvention.NC_MUST_HAVE_NAME)]
class AFM_AdminUID
{	
	[Attribute(desc: "Unique Player UIDs. This is not the player ID but the the player UID obtained via BackendApi.GetPlayerIdentityId()")]
	protected ref array <string> m_sAdminUIDs;
};
