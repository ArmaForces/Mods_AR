modded class SCR_InventoryMenuUI
{
	protected const int MIN_RESOURCE_SPLIT_COUNT = 1;
	protected const int MAX_RESOURCE_SPLIT_COUNT = 50;
	
	protected override bool MoveItem_VirtualArsenal(inout SCR_InventoryStorageBaseUI pStorageBaseUI = null, out bool operationFailed = false)
	{
		SCR_ArsenalInventorySlotUI arsenalInventorySlotUI;
		
		if (m_pSelectedSlotUI)
			 arsenalInventorySlotUI = SCR_ArsenalInventorySlotUI.Cast(m_pSelectedSlotUI);
		else
			return false;
		
		if (!arsenalInventorySlotUI)
		{
			if (m_pActiveHoveredStorageUI && !IsStorageArsenal(m_pActiveHoveredStorageUI.GetCurrentNavigationStorage()))
				return false;
			
			// Check if slot contains any nonrefundable items
			if (DoesSlotContainNonRefundableItems(m_pSelectedSlotUI))
				return false;
			
			//! Perform refund logic.
			BaseInventoryStorageComponent storageComponent = m_pStorageLootUI.GetCurrentNavigationStorage();

			if (!storageComponent) //! Relevant for OpenStorage classes
				storageComponent = m_pActiveHoveredStorageUI.GetStorage();
			
			if (!storageComponent || !IsStorageArsenal(storageComponent) )	
				return false;
			
			IEntity arsenalEntity = storageComponent.GetOwner();
			if (arsenalEntity)
			{
				SCR_ArsenalComponent arsenalComp = SCR_ArsenalComponent.Cast(arsenalEntity.FindComponent(SCR_ArsenalComponent));
				
				//~ Arsenal is disabled so cannot refund
				if (arsenalComp && !arsenalComp.IsArsenalEnabled())
				{
					operationFailed = true;
					return true;
				}
			}

			InventoryItemComponent inventoryItemComponent	= m_pSelectedSlotUI.GetInventoryItemComponent();
			SCR_ResourceComponent resourceComponent			= SCR_ResourceComponent.FindResourceComponent(arsenalEntity);
			SCR_ResourcePlayerControllerInventoryComponent resourceInventoryComponent = SCR_ResourcePlayerControllerInventoryComponent.Cast(GetGame().GetPlayerController().FindComponent(SCR_ResourcePlayerControllerInventoryComponent));
			
			resourceInventoryComponent.RpcAsk_ArsenalRefundItem(Replication.FindId(resourceComponent), Replication.FindId(inventoryItemComponent), EResourceType.SUPPLIES);

			return true;
		}
		
		BaseInventoryStorageComponent storageComponent = m_pActiveHoveredStorageUI.GetCurrentNavigationStorage();
		if (!storageComponent)
			return false;
		if (arsenalInventorySlotUI && storageComponent.GetOwner().FindComponent(SCR_ArsenalInventoryStorageManagerComponent))
			return true;
		
		SCR_InventoryStorageManagerComponent invManagerTo	= m_pActiveHoveredStorageUI.GetInventoryManager();
		BaseInventoryStorageComponent storageTo				= m_pActiveHoveredStorageUI.GetCurrentNavigationStorage();
		IEntity arsenalEntity								= SCR_InventoryStorageBaseUI.ARSENAL_SLOT_STORAGES.Get(arsenalInventorySlotUI);
		SCR_ResourceComponent resourceComponent				= SCR_ResourceComponent.FindResourceComponent(arsenalEntity);
		ResourceName resourceName							= arsenalInventorySlotUI.GetItemResource();
		SCR_ResourcePlayerControllerInventoryComponent resourceInventoryComponent = SCR_ResourcePlayerControllerInventoryComponent.Cast(GetGame().GetPlayerController().FindComponent(SCR_ResourcePlayerControllerInventoryComponent));

		if (storageTo.IsInherited(ClothNodeStorageComponent))
		{
			storageTo = invManagerTo.FindActualStorageForItemResource(resourceName, storageTo);
			if (!storageTo)
			{
				operationFailed = true;
				return true;
			}
		}

		//! Check if item supports splitting (has resource component) - for purchasing from arsenal
		IEntity itemEntity = arsenalInventorySlotUI.GetInventoryItemComponent().GetOwner();
		if (TryShowItemSplitDialog_Purchase(itemEntity, arsenalEntity, storageTo, resourceName))
			return true;

		if (invManagerTo.CanInsertItemInStorage(arsenalInventorySlotUI.GetInventoryItemComponent().GetOwner(), storageTo))
			resourceInventoryComponent.RpcAsk_ArsenalRequestItem(Replication.FindId(resourceComponent), Replication.FindId(invManagerTo),  Replication.FindId(storageTo), resourceName, EResourceType.SUPPLIES);
		else
			operationFailed = true;

		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Attempts to show item split dialog when purchasing items from arsenal
	//! \param[in] itemEntity The item entity being purchased
	//! \param[in] arsenalEntity The arsenal entity selling the item
	//! \param[in] storageTo The target storage component
	//! \return true if split dialog was shown, false otherwise
	protected bool TryShowItemSplitDialog_Purchase(IEntity itemEntity, IEntity arsenalEntity, BaseInventoryStorageComponent storageTo, ResourceName resourceName)
	{
		if (!itemEntity || !storageTo)
			return false;
		
		//! Get max resource capacity of target storage if it also has resources
		IEntity storageOwner = storageTo.GetOwner();
		SCR_UniversalInventoryStorageComponent storageComponentTo = SCR_UniversalInventoryStorageComponent.Cast(storageOwner.FindComponent(SCR_UniversalInventoryStorageComponent));
		
		float maxStoredResources;
		if (storageComponentTo)
		{
			maxStoredResources = Math.Min(storageComponentTo.GetEstimatedCountFitForItem(itemEntity), MAX_RESOURCE_SPLIT_COUNT);
		}
		else
		{
			maxStoredResources = MAX_RESOURCE_SPLIT_COUNT;
		}
		
		//TODO: Retrun false when no SCR_UniversalInventoryStorageComponent or when CanInsertItemInStorage fails
		if (maxStoredResources <= 0)
			return false;
		
		CreateItemSplitDialog_EntityCallback(maxStoredResources, storageOwner, arsenalEntity, resourceName);
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void CreateItemSplitDialog_EntityCallback(int maxVal, IEntity entityTo, IEntity entityFrom, ResourceName resourceName)
	{
		SCR_ItemSplitDialog dialog = SCR_ItemSplitDialog.Create(maxVal, entityTo, entityFrom);
		
		dialog.SetResourceName(resourceName);
		dialog.m_OnConfirm.Insert(OnItemSplitDialogConfirm_Entity);
	}
	
		//------------------------------------------------------------------------------------------------
	protected bool OnItemSplitDialogConfirm_Entity(inout notnull SCR_ItemSplitDialog dialog)
	{
		IEntity entityFrom = dialog.GetEntityFrom();
		IEntity entityTo = dialog.GetEntityTo();
		float count = dialog.GetSliderValue();		
		ResourceName resourceName = dialog.GetResourceName();	

		if (!entityTo || !entityFrom)
			return false;
		
		SCR_ResourcePlayerControllerInventoryComponent resourceInventoryComponent = SCR_ResourcePlayerControllerInventoryComponent.Cast(GetGame().GetPlayerController().FindComponent(SCR_ResourcePlayerControllerInventoryComponent));
		SCR_ResourceComponent resourceComponent	= SCR_ResourceComponent.FindResourceComponent(entityFrom);
		SCR_InventoryStorageManagerComponent invManagerTo = SCR_InventoryStorageManagerComponent.Cast(entityTo.FindComponent(SCR_InventoryStorageManagerComponent));
		BaseInventoryStorageComponent storageTo = BaseInventoryStorageComponent.Cast(entityTo.FindComponent(BaseInventoryStorageComponent));
	
		for (int i = 0; i < count; i++)
		{
			resourceInventoryComponent.RpcAsk_ArsenalRequestItem(Replication.FindId(resourceComponent), Replication.FindId(invManagerTo),  Replication.FindId(storageTo), resourceName, EResourceType.SUPPLIES);
		}
		
		return true;
	}
}

modded class SCR_ItemSplitDialog 
{
	protected ResourceName m_sResourceName;
	void SetResourceName(ResourceName name)
	{
		m_sResourceName = name;
	}
	
	ResourceName GetResourceName()
	{
		return m_sResourceName;
	}
}