/**
* Point of Interest entity placed by mission makers in the world.
* Registers itself with AFM_CivPopSystem on initialization so
* that the system can use it for civilian spawning and waypoint assignment.
*/

enum AFM_ECivPopPoIType
{
	TOWN,
	LARGE_CITY,
	CAFE,
	MARKET,
	GAS_STATION,
	CHURCH,
	PARK
}

[EntityEditorProps(category: "AFM/CivPop", description: "CivPop Point of Interest - place on map to attract civilian activity")]
class AFM_CivPopEntityClass : GenericEntityClass
{
}

class AFM_CivPopEntity : GenericEntity
{
	[Attribute(defvalue: AFM_ECivPopPoIType.TOWN.ToString(), uiwidget: UIWidgets.ComboBox, desc: "Type of Point of Interest", enums: ParamEnumArray.FromEnum(AFM_ECivPopPoIType), category: "CivPop")]
	protected AFM_ECivPopPoIType m_ePoIType;

	[Attribute(defvalue: "100", desc: "Radius of this location in meters. Determines the area civilians will be active in.", category: "CivPop", params: "1 2000 1")]
	protected float m_fLocationRadius;

	[Attribute(desc: "Optional array of connected CivPop entities that civilians can travel between.", category: "CivPop")]
	protected ref array<AFM_CivPopEntity> m_aConnectedEntities;

	//------------------------------------------------------------------------------------------------
	AFM_ECivPopPoIType GetPoIType()
	{
		return m_ePoIType;
	}

	//------------------------------------------------------------------------------------------------
	float GetLocationRadius()
	{
		return m_fLocationRadius;
	}

	//------------------------------------------------------------------------------------------------
	//! Returns connected entities. Can be null or empty.
	array<AFM_CivPopEntity> GetConnectedEntities()
	{
		return m_aConnectedEntities;
	}

	//------------------------------------------------------------------------------------------------
	//! Returns world position of this PoI
	vector GetPoIPosition()
	{
		return GetOrigin();
	}

	//------------------------------------------------------------------------------------------------
	override void EOnInit(IEntity owner)
	{
		super.EOnInit(owner);

		AFM_CivPopSystem system = AFM_CivPopSystem.GetInstance();
		if (!system)
		{
			Print("[AFM_CivPop] AFM_CivPopSystem not found, cannot register entity!", LogLevel.ERROR);
			return;
		}

		system.RegisterPoI(this);
	}

	//------------------------------------------------------------------------------------------------
	void ~AFM_CivPopEntity()
	{
		AFM_CivPopSystem system = AFM_CivPopSystem.GetInstance();
		if (system)
			system.UnregisterPoI(this);
	}

	//------------------------------------------------------------------------------------------------
	void AFM_CivPopEntity(IEntitySource src, IEntity parent)
	{
		SetEventMask(EntityEvent.INIT);
	}
}
