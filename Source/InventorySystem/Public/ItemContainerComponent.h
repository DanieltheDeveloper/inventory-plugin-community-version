// © 2024 Daniel Münch. All Rights Reserved

#pragma once

#include "InventorySlots.h"
#include "ItemDataAsset.h"
#include "Components/ActorComponent.h"
#include <atomic>

#include "ItemContainerComponent.generated.h"

#define LOCTEXT_NAMESPACE "InventorySystem"

// Blueprint + C++ Delegates

// Delegate declarations for various inventory actions. Each delegate is triggered upon the completion of a specific action in the inventory system, with a boolean parameter indicating the success or failure of the action.

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FSetSlotAmountSuccessDelegate, bool, bSuccess, int, Slot, bool, bIsEquipment);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FSetSlotItemPropertySuccessDelegate, bool, bSuccess, int, Slot, bool, bIsEquipment);

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FAddItemToComponentOtherComponentStartDelegate);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FAddItemToComponentSuccessDelegate, bool, bSuccess, int, Slot, int, ItemsLeft, UItemContainerComponent*, OtherComponent);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FAddItemToComponentOtherComponentSuccessDelegate, bool, bSuccess, int, Slot, int, ItemsLeft, UItemContainerComponent*, OtherComponent);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FAddItemSuccessDelegate, int, ItemsLeft, const TArray<int>&, Slots);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FAddItemFailureDelegate, FPrimaryAssetId, PrimaryAssetId, FItemProperties, DynamicStats, int, Amount);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_FiveParams(FAddItemToSlotFailureDelegate, FPrimaryAssetId, PrimaryAssetId, int, Slot, FItemProperties, DynamicStats, int, Amount, bool, bEnableFallback);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FAddItemToSlotSuccessDelegate, int, ItemsLeft, int, Slot, bool, bEnableFallback);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FSwapItemSuccessDelegate, bool, bSuccess, int, FirstSlot, int, SecondSlot, bool, bIsEquipment);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FRemoveAmountFromSlotSuccessDelegate, bool, bSuccess, FInventorySlot, OldInventorySlot, int, RemovedAmount);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FSplitItemStackSuccessDelegate, bool, bSuccess, int, SplitSlot, int, Slot);

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FSwapItemWithComponentOtherComponentStartDelegate);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FSwapItemWithComponentSuccessDelegate, bool, bSuccess, int, Slot, UItemContainerComponent*, OtherComponent);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FSwapItemWithComponentOtherComponentSuccessDelegate, bool, bSuccess, int, Slot, UItemContainerComponent*, OtherComponent);

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FCollectAllItemsOtherComponentStartDelegate);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FCollectAllItemsSuccessDelegate, bool, bSuccess, bool, bItemsLeft, UItemContainerComponent*, OtherComponent);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FCollectAllItemsOtherComponentSuccessDelegate, bool, bSuccess, bool, bItemsLeft, UItemContainerComponent*, OtherComponent);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FChangedInventorySlotsDelegate, const TArray<int>&, Slots);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSetMaxStackSizeSuccessDelegate, bool, bSuccess);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSetInventorySizeSuccessDelegate, bool, bSuccess);

/**
 * @class UItemContainerComponent
 * @brief Handles item storage, management, and interaction within an item container, including addition, removal, and property adjustment of items.
 * 
 * This component serves as the backbone for container management, capable of handling various item-related actions such as adding or removing items,
 * setting item quantities, and adjusting item properties. It supports delegation for container actions, allowing for custom reactions to container changes.
 * 
 * General Usage:
 * - Attach to any actor that requires an item container, such as NPCs and treasure chests.
 * - Use the provided functions to manipulate the container, including adding and removing items.
 * - Listen to the various delegates for container events to update the UI or game logic accordingly.
 *
 * Example Use Case:
 * @code
 * // Assuming 'InventoryComponent' is already attached to your character or actor
 * UItemContainerComponent* Inventory = Character->FindComponentByClass<UItemContainerComponent>();
 * Inventory->AddItem(InventoryAsset, DynamicStats, Amount, bCanStack, bRevertWhenFull);
 * @endcode
 */
UCLASS(meta = (BlueprintSpawnableComponent), Category = "Inventory System", Blueprintable)
class INVENTORYSYSTEM_API UItemContainerComponent : public UActorComponent
{
	GENERATED_BODY()

protected:
	/**
	 * Constructor set bAllowInventoryEdit.
	 */
	UItemContainerComponent();
	
	/**
	 * Check for settings changed on register/reload.
	 */
	virtual void OnRegister() override;

	/**
	 * Use internal checks to prevent dysfunctional component to be created.
	 */
	virtual void BeginPlay() override;

	/**
	 * Array of indices representing inventory slots.
	 */
	UPROPERTY(ReplicatedUsing = OnRep_InventoryIndices, BlueprintReadOnly, EditAnywhere, Category = "Inventory System|Inventory", meta = (EditFixedOrder, NoResetToDefault))
	TArray<int> InventoryIndices;

	UFUNCTION()
	void OnRep_InventoryIndices(TArray<int> OldInventoryIndices);

	/**
	 * Array of primary asset IDs representing inventory assets.
	 */
	UPROPERTY(ReplicatedUsing = OnRep_InventoryAssets, BlueprintReadOnly, EditAnywhere, Category = "Inventory System|Inventory", meta = (EditCondition = "bAllowInventoryEdit && bAllowInventoryAssetEdit", EditConditionHides, AllowedClasses = "/Script/InventorySystem.ItemDataAsset", ExactClass = false, EditFixedOrder))
	TArray<FPrimaryAssetId> InventoryAssets;

	UFUNCTION()
    void OnRep_InventoryAssets(TArray<FPrimaryAssetId> OldInventoryAssets);
	
#if WITH_EDITOR
	/**
	 * Internal use only. Check if item edit is in PIE context is allowed
	 */
	UFUNCTION()
	virtual void InternalCheckEditVariables(const TArray<int>& Slots);
	
	/**
	 * Save after InternalChecks.
	 */
	UFUNCTION()
	virtual void InternalSaveAfterCheck();
#endif

	/**
	 * Internal use only. Boolean indicating whether other inventory properties are allowed to be edited.
	 */
	UPROPERTY(VisibleAnywhere, Category = "Inventory System|Inventory|Edit Conditions")
	bool bAllowInventoryEdit = false;

#if	WITH_EDITORONLY_DATA

	/**
	 * Internal use only. Boolean indicating whether other inventory properties are allowed to be edited.
	 */
	UPROPERTY(VisibleAnywhere, Category = "Inventory System|Inventory|Edit Conditions")
	bool bAllowInventoryAssetEdit = false;

	/**
	 * Internal use only. Boolean indicating whether the component is in a editor play context.
	 */
	UPROPERTY(VisibleAnywhere, Category = "Inventory System|Settings|Edit Conditions")
	bool bHasBegunPlayEditor = false;

	/**
	 * Contains data for this item. Please not use this directly in runtime as it will be empty. Use InventoryAssets instead and load it with the AssetManager.
	 */
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Inventory System|Inventory", meta = (EditCondition = "bAllowInventoryEdit && !bAllowInventoryAssetEdit", EditConditionHides, ToolTip = "Select the ItemData or ItemEquipmentAsset you specified", AllowedClasses = "/Script/InventorySystem.ItemDataAsset", ExactClass = false, EditFixedOrder))
	TArray<UItemDataAsset*> InventoryDataAssets;
#endif

	/**
	 * Array of integers representing the amounts of items in inventory slots.
	 */
	UPROPERTY(ReplicatedUsing = OnRep_InventoryAmounts, BlueprintReadOnly, EditAnywhere, Category = "Inventory System|Inventory", meta = (EditConditionHides, EditCondition = "bAllowInventoryEdit", EditFixedOrder, NoResetToDefault))
	TArray<int> InventoryAmounts;

	UFUNCTION()
	void OnRep_InventoryAmounts(TArray<int> OldInventoryAmounts);

	/**
	 * Array of indices representing dynamic stats associated with items in inventory slots.
	 */
	UPROPERTY(ReplicatedUsing = OnRep_InventoryDynamicStatsIndices, BlueprintReadOnly, EditAnywhere, Category = "Inventory System|Inventory", meta = (EditConditionHides, EditCondition = "bAllowInventoryEdit", EditFixedOrder, NoResetToDefault))
	TArray<int> InventoryDynamicStatsIndices;

	UFUNCTION()
	void OnRep_InventoryDynamicStatsIndices(TArray<int> OldInventoryDynamicStatsIndices);

	/**
	 * Array of item properties representing dynamic stats associated with items in inventory. Needs an index in InventoryDynamicStatsIndices to work.
	 */
	UPROPERTY(ReplicatedUsing = OnRep_InventoryDynamicStats, BlueprintReadOnly, EditAnywhere, Category = "Inventory System|Inventory", meta = (EditConditionHides, EditCondition = "bAllowInventoryEdit", EditFixedOrder))
	TArray<FItemProperties> InventoryDynamicStats;

	UFUNCTION()
	void OnRep_InventoryDynamicStats(TArray<FItemProperties> OldInventoryDynamicStats);

	/**
	 * The maximum stack size for items.
	 */
	UPROPERTY(Replicated, BlueprintReadOnly, EditAnywhere, Category = "Inventory System|Settings", meta = (ClampMin="0", EditCondition = "!bHasBegunPlayEditor"))
	int MaxStackSize = 0;

	/**
	 * The maximum size of the inventory.
	 */
	UPROPERTY(Replicated, BlueprintReadOnly, EditAnywhere, Category = "Inventory System|Settings", meta = (ClampMin="0", EditCondition = "!bHasBegunPlayEditor"))
	int InventorySize = 0;

public:
	/**
	 * Delegate used to add functionality after the item swap method started.
	 */
	UPROPERTY(BlueprintAssignable, BlueprintCallable)
	FSwapItemWithComponentOtherComponentStartDelegate SwapItemWithComponentOtherComponentStartDelegate;

	/**
	 * Delegate used to add functionality after an item is switched between inventory components.
	 */
	UPROPERTY(BlueprintAssignable, BlueprintCallable)
	FSwapItemWithComponentSuccessDelegate SwapItemWithComponentSuccessDelegate;

	/**
	 * Delegate used to add functionality after an item is switched between inventory components.
	 */
	UPROPERTY(BlueprintAssignable, BlueprintCallable)
	FSwapItemWithComponentOtherComponentSuccessDelegate SwapItemWithComponentOtherComponentSuccessDelegate;

	/**
	 * Delegate used to add functionality after an item is added from the current to another component.
	 */
	UPROPERTY(BlueprintAssignable, BlueprintCallable)
	FAddItemToComponentOtherComponentStartDelegate AddItemToComponentOtherComponentStartDelegate;

	/**
	 * Delegate used to add functionality after an item is added from the current to another component.
	 */
	UPROPERTY(BlueprintAssignable, BlueprintCallable)
	FAddItemToComponentSuccessDelegate AddItemToComponentSuccessDelegate;

	/**
	 * Delegate used to add functionality after an item is added from the current to another component.
	 */
	UPROPERTY(BlueprintAssignable, BlueprintCallable)
	FAddItemToComponentOtherComponentSuccessDelegate AddItemToComponentOtherComponentSuccessDelegate;

	/**
	 * Delegate used to add functionality after an item failed to be added to the inventory.
	 */
	UPROPERTY(BlueprintAssignable, BlueprintCallable)
	FAddItemFailureDelegate AddItemFailureDelegate;

	/**
	 * Delegate used to add functionality after an item is added to the inventory.
	 */
	UPROPERTY(BlueprintAssignable, BlueprintCallable)
	FAddItemSuccessDelegate AddItemSuccessDelegate;

	/**
	 * Delegate used to add functionality after an item is added to a certain inventory slot.
	 */
	UPROPERTY(BlueprintAssignable, BlueprintCallable)
	FAddItemToSlotSuccessDelegate AddItemToSlotSuccessDelegate;

	/**
	 * Delegate used to add functionality after an item failed to be added to a certain inventory slot.
	 */
	UPROPERTY(BlueprintAssignable, BlueprintCallable)
	FAddItemToSlotFailureDelegate AddItemToSlotFailureDelegate;

	/**
	 * Delegate used to add functionality after an item stack is split.
	 */
	UPROPERTY(BlueprintAssignable, BlueprintCallable)
	FSplitItemStackSuccessDelegate SplitItemStackSuccessDelegate;

	/**
	 * Delegate used to add functionality after a whole item or a certain amount is removed.
	 */
	UPROPERTY(BlueprintAssignable, BlueprintCallable)
	FRemoveAmountFromSlotSuccessDelegate RemoveAmountFromSlotSuccessDelegate;

	/**
	 * Delegate used to add functionality after an item is switched between slots.
	 */
	UPROPERTY(BlueprintAssignable, BlueprintCallable)
	FSwapItemSuccessDelegate SwapItemSuccessDelegate;

	/**
	 * Delegate to add functionality after the amount of an item is changed.
	 */
	UPROPERTY(BlueprintAssignable, BlueprintCallable)
	FSetSlotAmountSuccessDelegate SetSlotAmountSuccessDelegate;

	/**
	 * Delegate to add functionality after an item property is set.
	 */
	UPROPERTY(BlueprintAssignable, BlueprintCallable)
	FSetSlotItemPropertySuccessDelegate SetSlotItemPropertySuccessDelegate;

	/**
	 * Delegate to add functionality after when the collection of all items in this container started.
	 */
	UPROPERTY(BlueprintAssignable, BlueprintCallable)
	FCollectAllItemsOtherComponentStartDelegate CollectAllItemsOtherComponentStartDelegate;

	/**
	 * Delegate to add functionality after the collection of all items in this container.
	 */
	UPROPERTY(BlueprintAssignable, BlueprintCallable)
	FCollectAllItemsSuccessDelegate CollectAllItemsSuccessDelegate;

	/**
	 * Delegate to add functionality after the collection of all items in this container.
	 */
	UPROPERTY(BlueprintAssignable, BlueprintCallable)
	FCollectAllItemsOtherComponentSuccessDelegate CollectAllItemsOtherComponentSuccessDelegate;

	/**
	 * Delegate used to add functionality after an item was changed in any way.
	 */
	UPROPERTY(BlueprintAssignable, BlueprintCallable)
	FChangedInventorySlotsDelegate ChangedInventorySlotsDelegate;

	/**
	 * Delegate used to add functionality after MaxStackSize was set.
	 */
	UPROPERTY(BlueprintAssignable, BlueprintCallable)
	FSetMaxStackSizeSuccessDelegate SetMaxStackSizeSuccessDelegate;

	/**
	 * Delegate used to add functionality after InventorySize was set.
	 */
	UPROPERTY(BlueprintAssignable, BlueprintCallable)
	FSetInventorySizeSuccessDelegate SetInventorySizeSuccessDelegate;
	
	/**
	 * Boolean indicating whether the component is currently processing another request.
	 */
	UPROPERTY(Replicated, BlueprintReadOnly, VisibleAnywhere, Category = "Inventory System|Settings")
	bool bIsProcessing = false;
	
	/**
	 * GetLifetimeReplicatedProps override to specify replicated properties.
	 *
	 * @param OutLifetimeProps  The array to hold replicated property information.
	 */
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	/**
	 * Used to check if actor is setup correctly and prevent execution of further actions.
	 *
	 * @return
	 */
	UFUNCTION()
	virtual bool InternalChecks(const bool bIsSavePackageEvent = false);

#if WITH_EDITOR
	/**
	 *	Set editor begun play.
	 *
	 * @param EndPlayReason 
	 */
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	/**
	 * Post edit change property.
	 *
	 * @param PropertyChangedEvent 
	 */
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;

	/**
	 * Internal use only. Add check to save.
	 *
	 * @param ObjectSaveContext 
	 */
	virtual void PreSave(FObjectPreSaveContext ObjectSaveContext) override;
#endif

	/**
	 * Get the InventorySlots.
	 *
	 * @return The InventorySlots containing the inventory data.
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory System")
	TArray<FInventorySlot> GetInventorySlots() const;

	/**
	 * Get an InventorySlot.
	 * @param Slot The target slot index
	 *
	 * @return The InventorySlot.
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory System")
	FInventorySlot GetInventorySlot(const int Slot) const;

	/**
	 * Check if a slot has a specific item property.
	 *
	 * @param Slot           The slot to check.
	 * @param Name           The name of the item property.
	 * @param bIsEquipment   Boolean indicating if the slot is an equipment slot.
	 * @return               True if the item property is found, false otherwise.
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory System")
	virtual bool HasItemProperty(const int Slot, const FName Name, const bool bIsEquipment = false);

	/**
	 * Get item property.
	 *
	 * @param Slot           The slot to check.
	 * @param Name           The name of the item property.
	 * @param bIsEquipment   Boolean indicating if the slot is an equipment slot.
	 * @return               True if the item property is found, false otherwise.
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory System")
	virtual FItemProperty GetItemProperty(const int Slot, const FName Name, const bool bIsEquipment = false);

	/**
	 * Set the amount of items in a slot. Use this only if you plan to add to the item or remove less then the max amount. If you want to remove items use RemoveAmountFromSlot, this has special logic to remove items if possible!
	 *
	 * @param Slot           The slot to set the amount for.
	 * @param Amount         The new amount.
	 * @param bIsEquipment   Boolean indicating if the slot is an equipment slot.
	 */
	UFUNCTION(BlueprintCallable, Server, WithValidation, Reliable, Category = "Inventory System")
	virtual void SetSlotAmount(const int Slot, const int Amount, const bool bIsEquipment = false);
	virtual void SetSlotAmount_Implementation(const int Slot, const int Amount, const bool bIsEquipment = false);

	/**
	 * Set an ItemProperty of a given slot. Remove, add or edit item properties.
	 *
	 * @param Slot           The slot to set the item property for.
	 * @param Name           The name of the item property.
	 * @param DisplayName	 The display name of the property.
	 * @param Value          The value of the item property.
	 * @param bIsEquipment   Boolean indicating if the slot is an equipment slot.
	 */
	UFUNCTION(Server, WithValidation, Reliable, BlueprintCallable, Category = "Inventory System")
	void SetSlotItemProperty(const int Slot, const FName Name, const FText& DisplayName, const FText& Value, const bool bIsEquipment = false);
	virtual void SetSlotItemProperty_Implementation(const int Slot, const FName Name, const FText& DisplayName, const FText& Value, const bool bIsEquipment = false);

	/**
	 * Find a stack of items in the inventory.
	 *
	 * @param InventoryAsset  	The primary asset ID of the inventory item.
	 * @param Index           	The index where the item stack was found.
	 * @param Amount          	The amount found in the stack.
	 * @param bSuccess        	Boolean indicating if the search was successful.
	 * @param DynamicStats    	The dynamic stats associated with the item.
	 * @param ItemAmount      	The quantity of the object to locate. Only returns this amount if it is less than the stack size configuration after adding it to the found stack amount. Use INDEX_NONE (-1) to only check if there is a stack, ignore space.
	 * @param bReturnFullStack 	Boolean indicating if a full stack should be returned.
	 * @param IgnoreInventorySlotsSlots Array of slots that will be ignored while searching for item stacks.
	 */
	//UFUNCTION(BlueprintCallable, Category = "Inventory System")
	void FindItemStack(const FPrimaryAssetId& InventoryAsset, int& Index, int& Amount, bool& bSuccess, const FItemProperties DynamicStats = FItemProperties(), const int& ItemAmount = -1, const bool bReturnFullStack = false, const TArray<int> IgnoreInventorySlots = {});

	/**
	 * Find the next empty slot in the inventory.
	 *
	 * @param Slot      				The slot where the empty slot was found.
	 * @param bSuccess  				Boolean indicating if an empty slot was found.
	 * @param IgnoreInventorySlotsSlots Array of slots that will be ignored while searching for empty slots.
	 */
	//UFUNCTION(BlueprintCallable, Category = "Inventory System")
	void FindNextEmptySlot(int& Slot, bool& bSuccess, const TArray<int> IgnoreInventorySlots = {}) const;

	/**
	 * Add an item to another component. Moves an item from the current inventory system with an assigned amount to another component.
	 * Broadcasts assigned delegates on both components.
	 *
	 * @param Slot						The slot index of the item to move.
	 * @param ItemContainerComponent	The target item container component to which the item should be moved.
	 * @param Amount					The amount of items to move (default is 1).
	 * @param bCanStack					Specifies if stacking is allowed (default is false).
	 * @param bRevertWhenFull			Should revert already added items when full.
	 */
	UFUNCTION(Server, WithValidation, Reliable, BlueprintCallable, Category = "Inventory System")
	void AddItemToComponent(const int Slot, UItemContainerComponent* ItemContainerComponent, const int Amount = 1, const bool bCanStack = false, const bool bRevertWhenFull = false);
	virtual void AddItemToComponent_Implementation(const int Slot, UItemContainerComponent* ItemContainerComponent, const int Amount = 1, const bool bCanStack = false, const bool bRevertWhenFull = false);

	/**
	 * Internal with return. Dont use for implementation!!! Add an item to another component. Moves an item from the current inventory system with an assigned amount to another component.
	 *
	 * @param Slot						The slot index of the item to move.
	 * @param ItemContainerComponent	The target item container component to which the item should be moved.
	 * @param Amount					The amount of items to move (default is 1).
	 * @param bCanStack					Specifies if stacking is allowed (default is false).
	 * @param bIsEquipment				Is equipment.
	 * @param bRevertWhenFull			Should revert already added items when full.
	 *
	 * @return
	 */
	virtual TArray<int> AddItemToComponentInternal(const int Slot, UItemContainerComponent* ItemContainerComponent, int& Amount, const bool bCanStack = false, const bool bIsEquipment = false, const bool bRevertWhenFull = false);

	/**
	 * Add an item to the inventory if possible. Checks for stack and empty spaces.
	 * (For item "Pickup," use the Pickup function in the ItemDrop object; additional checks are used!)
	 *
	 * @param InventoryAsset      The primary asset ID of the item to add.
	 * @param DynamicStats        The dynamic properties of the item (default is empty).
	 * @param Amount              The amount of items to add (default is 1).
	 * @param bCanStack           Specifies if stacking is allowed (default is false).
	 * @param bRevertWhenFull	  Should revert already added items when full.
	 */
	UFUNCTION(Server, WithValidation, Reliable, BlueprintCallable, Category = "Inventory System")
	void AddItem(const FPrimaryAssetId InventoryAsset, const FItemProperties DynamicStats = FItemProperties(), const int Amount = 1, const bool bCanStack = false, const bool bRevertWhenFull = true);
	virtual void AddItem_Implementation(const FPrimaryAssetId InventoryAsset, const FItemProperties DynamicStats = FItemProperties(), const int Amount = 1, const bool bCanStack = false, const bool bRevertWhenFull = true);

	/**
	 * Internal with return. Dont use for implementation!!! Add an item to the inventory if possible. Checks for stack and empty spaces.
	 * (For item "Pickup," use the Pickup function in the ItemDrop object; additional checks are used!)
	 *
	 * @param InventoryAsset      The primary asset ID of the item to add.
	 * @param DynamicStats        The dynamic properties of the item (default is empty).
	 * @param Amount              The amount of items to add (default is 1).
	 * @param bCanStack           Specifies if stacking is allowed (default is false).
	 * @param bRevertWhenFull	  Should revert already added items when full.
	 *
	 * @return
	 */
	TArray<int> AddItemInternal(const FPrimaryAssetId& InventoryAsset, const FItemProperties& DynamicStats, int& Amount, bool bCanStack, bool bRevertWhenFull);

	/**
	 * Add an item to a specific slot if possible or use a fallback slot.
	 * Evaluates if the supplied index may be utilized and, if not, falls back to a default.
	 * (For item "Pickup," use the Pickup function in the ItemDrop object; additional checks are used!)
	 *
	 * @param InventoryAsset      The primary asset ID of the item to add.
	 * @param Slot                The target slot index.
	 * @param DynamicStats        The dynamic properties of the item (default is empty).
	 * @param Amount              The amount of items to add (default is 1).
	 * @param bCanStack           Specifies if stacking is allowed (default is false).
	 * @param bEnableFallback     Specifies if the fallback function AddItem should be used. This will call the AddItem function and its delegates if not space on the specified slot is available (default is true).
	 */
	UFUNCTION(Server, WithValidation, Reliable, BlueprintCallable, Category = "Inventory System")
	void AddItemToSlot(const FPrimaryAssetId InventoryAsset, const int Slot, const FItemProperties DynamicStats = FItemProperties(), const int Amount = 1, const bool bCanStack = false, const bool bEnableFallback = true);
	virtual void AddItemToSlot_Implementation(const FPrimaryAssetId InventoryAsset, const int Slot, const FItemProperties DynamicStats = FItemProperties(), const int Amount = 1, const bool bCanStack = false, const bool bEnableFallback = true);

	/**
	 * Swap items in the inventory. Merge item stacks if the first and second index have the same static and dynamic properties.
	 *
	 * @param First               The index of the first item to swap.
	 * @param Second              The index of the second item to swap.
	 * @param bCanStack           Specifies if stacking is allowed (default is false).
	 * @param bIsEquipment		  Is equipment.
	 */
	UFUNCTION(Server, WithValidation, Reliable, BlueprintCallable, Category = "Inventory System")
	void SwapItems(const int First, const int Second, const bool bCanStack = false, const bool bIsEquipment = false);
	virtual void SwapItems_Implementation(const int First, const int Second, const bool bCanStack = false, const bool bIsEquipment = false);

	/**
	 * Remove a specified amount of items from a slot. Removes the entire item if the amount is equal to or greater than the item's quantity.
	 *
	 * @param Slot                The slot index from which to remove items.
	 * @param Amount              The amount of items to remove (default is 1).
	 */
	UFUNCTION(Server, WithValidation, Reliable, BlueprintCallable, Category = "Inventory System")
	void RemoveAmountFromSlot(const int Slot, const int Amount = 1);
	virtual void RemoveAmountFromSlot_Implementation(const int Slot, const int Amount = 1);

	/**
	 * Split items in the inventory. Search the next empty slot and split the item by the given amount.
	 *
	 * @param Slot                The slot index to split.
	 * @param SplitAmount         The amount by which to split the item (default is 1).
	 */
	UFUNCTION(Server, WithValidation, Reliable, BlueprintCallable, Category = "Inventory System")
	void SplitItemStack(const int Slot, const int SplitAmount = 1);
	virtual void SplitItemStack_Implementation(const int Slot, const int SplitAmount = 1);

	/**
	 * Swap items between ItemContainerComponent. Merge item stacks if the first and second index have the same static and dynamic properties.
	 *
	 * @param First						The index of the first item to swap.
	 * @param Second					The index of the second item to swap.
	 * @param ItemContainerComponent	The target item container component for swapping.
	 * @param bCanMergeStack			Specifies if stacking is allowed (default is false).
	 */
	UFUNCTION(Server, WithValidation, Reliable, BlueprintCallable, Category = "Inventory System")
	void SwapItemWithComponent(const int First, const int Second, UItemContainerComponent* ItemContainerComponent, const bool bCanMergeStack = false);
	virtual void SwapItemWithComponent_Implementation(const int First, const int Second, UItemContainerComponent* ItemContainerComponent, const bool bCanMergeStack = false);

	/**
	 * Collect all items from this container and add them to the specified ItemContainerComponent.
	 *
	 * This function collects all items present in this container and adds them to the provided ItemContainerComponent.
	 * The items are added one by one, and if the ItemContainerComponent allows stacking, items of the same type can stack together.
	 *
	 * @param ItemContainerComponent The target ItemContainerComponent where items will be collected.
	 * @param bCanStack Indicates whether items can stack within the ItemContainerComponent (default: true).
	 *
	 * @note This function should be called on the server for proper replication in multiplayer scenarios.
	 */
	UFUNCTION(Server, WithValidation, Reliable, BlueprintCallable, Category = "Inventory System")
	void CollectAllItems(UItemContainerComponent* ItemContainerComponent, const bool bCanStack = true);
	virtual void CollectAllItems_Implementation(UItemContainerComponent* ItemContainerComponent, const bool bCanStack = true);

	/**
	 * Internal method used to get the global config value or a default set in this component.
	 * 
	 * @return 
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory System")
	virtual int GetStackSizeConfig() const;

	/**
	 * Set MaxStackSize. Checks all items and stops execution if an item would have overflow. Please split, set items manually or active force execution with the bForce parameter to use this function.
	 * By forcing the execution you will delete all items that are overflowing!
	 * 
	 * @return 
	 */
	UFUNCTION(Server, WithValidation, Reliable, BlueprintCallable, Category = "Inventory System")
	void SetStackSizeConfig(const int NewMaxStackSize, const bool bForce = false);
	virtual void SetStackSizeConfig_Implementation(const int NewMaxStackSize, const bool bForce = false);

	/**
	 * Internal method used to get the global config value or a default set in this component.
	 * 
	 * @return 
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory System")
	virtual int GetInventorySizeConfig() const;

	/**
	 * Set InventorySize. Checks all items and stops execution if the item container would have overflow. Please set items manually or active force execution with the bForce parameter to use this function.
	 * By forcing the execution you will delete all items that are overflowing!
	 * 
	 * @return 
	 */
	UFUNCTION(Server, WithValidation, Reliable, BlueprintCallable, Category = "Inventory System")
	void SetInventorySizeConfig(const int NewInventorySize, const bool bForce = false);
	virtual void SetInventorySizeConfig_Implementation(const int NewInventorySize, const bool bForce = false);
};

#undef LOCTEXT_NAMESPACE
