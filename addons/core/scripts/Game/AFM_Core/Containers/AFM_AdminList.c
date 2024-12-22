[BaseContainerProps(configRoot: true)]
class AFM_AdminList
{
	[Attribute(desc: "Unique Player UID. This is not the player ID but the the player UID obtained via BackendApi.GetPlayerIdentityId()")]
	protected ref array<ref AFM_AdminUID> m_aAdminUIDs;
};