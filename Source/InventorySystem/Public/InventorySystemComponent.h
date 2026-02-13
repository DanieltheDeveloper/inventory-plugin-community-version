// © 2024 Daniel Münch. All Rights Reserved

#pragma once

#include "EquipmentSlots.h"
#include "ItemContainerComponent.h"
#include "ItemDrop.h"
#include "ItemEquipmentDataAsset.h"
#include "InventorySystemComponent.generated.h"

#define LOCTEXT_NAMESPACE "InventorySystem"

// Blueprint + C++ Delegates

// Delegate declarations for various inventory actions. Each delegate is triggered upon the completion of a specific action in the inventory system, with a boolean parameter indicating the success or failure of the action.

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FPickUpItemSuccessDelegate, AItemDrop*, ItemDrop, const TArray<int>&, Slots);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPickUpItemFailureDelegate, AItemDrop*, ItemDrop);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FAddItemToEquipmentSlotSuccessDelegate, int, EquipmentSlot, const TArray<int>&, Slots, int, Overflow);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FAddItemToEquipmentSlotFailureDelegate, FPrimaryAssetId, InventoryAsset, int, EquipmentSlot, FItemProperties, DynamicStats, int, Amount);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FRemoveEquipmentAmountFromSlotSuccessDelegate, bool, bSuccess, FEquipmentSlot, EquipmentSlot, int, RemovedAmount);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FItemEquipFromInventorySuccessDelegate, bool, bSuccess, int, EquipmentSlot, int, Slot);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FItemUnequipSuccessDelegate, bool, bSuccess, int, EquipmentSlot, const TArray<int>&, Slots);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSetEquipmentTypeSuccessDelegate, int, EquipmentSlot);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FSetEquipmentTypeFailureDelegate, int, EquipmentSlot, FPrimaryAssetId, EquipmentType);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FChangedEquipmentSlotsDelegate, const TArray<int>&, Slots);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSetMaxEquipmentStackSizeSuccessDelegate, bool, bSuccess);

/**
 * @class UInventorySystemComponent
 * @brief Manages inventory for a player or NPC, handling item addition, removal, and equipment management.
 *
 * This component is a central part of the inventory system, offering a comprehensive solution for inventory management,
 * including support for equipment slots, item stacking, and dynamic item properties. It interfaces with item drops, allowing
 * for items to be picked up and added to the inventory, and provides functionality for equipping items to specific slots.
 *
 * General Usage:
 * - Add to a PlayerState or NPC for inventory management.
 * - Interact with through UI for inventory operations.
 * - Extend or customize for game-specific inventory logic.
 *
 * Example Use Case:
 * @code
 * // Pick up an item
 * UInventorySystemComponent* Inventory = PlayerState->GetInventorySystemComponent();
 * Inventory->PickUpItemDrop(ItemDrop);
 *
 * // Equipping an item from the inventory
 * Inventory->ItemEquipFromInventory(Slot, EquipmentSlot, bCanUnequippedItemStack, bCanStack);
 * @endcode
 */
UCLASS(meta = (BlueprintSpawnableComponent), Category = "Inventory System", Blueprintable)
class INVENTORYSYSTEM_API UInventorySystemComponent : public UItemContainerComponent
{
	GENERATED_BODY()

protected:
	/**
	 * Constructor set AllowEquipmentEdit.
	 */
	UInventorySystemComponent();
	
	/**
	 * Check for settings changed on register/reload.
	 */
	virtual void OnRegister() override;

	/**
	 * Use internal checks to prevent dysfunctional component to be created.
	 */
	virtual void BeginPlay() override;

	/**
	 * The maximum stack size for equipment items.
	 */
	UPROPERTY(Replicated, BlueprintReadOnly, EditAnywhere, Category = "Inventory System|Settings", meta=(ClampMin="0", EditCondition = "!bHasBegunPlayEditor"))
	int MaxEquipmentStackSize = 0;
	
#if WITH_EDITOR
	/**
	 * Internal use only. Check if item edit is in PIE context is allowed
	 */
	virtual void InternalCheckEditVariables(const TArray<int>& Slots) override;
	
	/**
	 * Save after InternalChecks.
	 */
	virtual void InternalSaveAfterCheck() override;
#endif

	/**
	 * Internal use only. Boolean indicating whether other equipment properties are allowed to be edited.
	 */
	UPROPERTY(VisibleAnywhere, Category = "Inventory System|Equipment|Edit Conditions")
	bool AllowEquipmentEdit = false;

#if WITH_EDITORONLY_DATA
	/**
	 * Internal use only. Boolean indicating whether other equipment properties are allowed to be edited.
	 */
	UPROPERTY(VisibleAnywhere, Category = "Inventory System|Equipment|Edit Conditions")
	bool AllowEquipmentIndexEdit = false;

	/**
	 * Internal use only. Boolean indicating whether other equipment properties are allowed to be edited.
	 */
	UPROPERTY(VisibleAnywhere, Category = "Inventory System|Equipment|Edit Conditions")
	bool AllowEquipmentTypeEdit = false;

	/**
	 * Internal use only. Boolean indicating whether other equipment properties are allowed to be edited.
	 */
	UPROPERTY(VisibleAnywhere, Category = "Inventory System|Equipment|Edit Conditions")
	bool AllowEquipmentTypeAssetEdit = false;

	/**
	 * Internal use only. Boolean indicating whether other inventory properties are allowed to be edited.
	 */
	UPROPERTY(VisibleAnywhere, Category = "Inventory System|Equipment|Edit Conditions")
	bool AllowEquipmentAssetEdit = false;
#endif
	
	/**
	 * Override of GetLifetimeReplicatedProps to specify replicated properties.
	 *
	 * @param OutLifetimeProps The array of replicated properties.
	 */
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	/**
	 * Array of indices representing equipment types.
	 */
	UPROPERTY(ReplicatedUsing = OnRep_EquipmentTypeIndices, BlueprintReadOnly, EditAnywhere, Category = "Inventory System|Equipment", meta = (EditFixedOrder, NoResetToDefault))
	TArray<int> EquipmentTypeIndices;

	UFUNCTION()
	void OnRep_EquipmentTypeIndices(TArray<int> OldEquipmentTypeIndices);

	/**
	 * Array of primary asset IDs representing equipment types.
	 */
	UPROPERTY(ReplicatedUsing = OnRep_EquipmentTypes, BlueprintReadOnly, EditAnywhere, Category = "Inventory System|Equipment", meta = (EditCondition = "AllowEquipmentTypeEdit && AllowEquipmentTypeAssetEdit", EditConditionHides, AllowedClasses = "/Script/InventorySystem.ItemEquipmentTypeDataAsset", ExactClass = false, EditFixedOrder))
	TArray<FPrimaryAssetId> EquipmentTypes;

	UFUNCTION()
	void OnRep_EquipmentTypes(TArray<FPrimaryAssetId> OldEquipmentTypes);

#if WITH_EDITORONLY_DATA
	/**
	 * Contains data for the allowed equipment type. Please not use this directly in runtime as it will be empty. Use EquipmentTypes instead and load it with the AssetManager.
	 */
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Inventory System|Equipment", meta = (EditCondition = "AllowEquipmentTypeEdit && !AllowEquipmentTypeAssetEdit", EditConditionHides, AllowedClasses = "/Script/InventorySystem.ItemEquipmentTypeDataAsset", ExactClass = false, EditFixedOrder))
	TArray<UItemEquipmentTypeDataAsset*> EquipmentDataAssetTypes;
#endif

	/**
	 * Array of indices representing equipment slots.
	 */
	UPROPERTY(ReplicatedUsing = OnRep_EquipmentIndices, BlueprintReadOnly, EditAnywhere, Category = "Inventory System|Equipment", meta = (EditConditionHides, EditCondition = "AllowEquipmentIndexEdit", EditFixedOrder, NoResetToDefault))
	TArray<int> EquipmentIndices;

	UFUNCTION()
	void OnRep_EquipmentIndices(TArray<int> OldEquipmentIndices);

	/**
	 * Array of primary asset IDs representing equipment assets.
	 */
	UPROPERTY(ReplicatedUsing = OnRep_EquipmentAssets, BlueprintReadOnly, EditInstanceOnly, Category = "Inventory System|Equipment", meta = (EditCondition = "AllowEquipmentEdit && AllowEquipmentAssetEdit", EditConditionHides, AllowedClasses = "/Script/InventorySystem.ItemEquipmentDataAsset", ExactClass = false, EditFixedOrder))
	TArray<FPrimaryAssetId> EquipmentAssets;

	UFUNCTION()
	void OnRep_EquipmentAssets(TArray<FPrimaryAssetId> OldEquipmentAssets);

#if WITH_EDITORONLY_DATA
	/**
	 * Contains data for this item. Please not use this directly in runtime as it will be empty. Use EquipmentAsset instead and load it with the AssetManager.
	 */
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Inventory System|Equipment", meta = (EditCondition = "AllowEquipmentEdit && !AllowEquipmentAssetEdit", EditConditionHides, ToolTip = "Select the ItemData or ItemEquipmentAsset you specified", AllowedClasses = "/Script/InventorySystem.ItemEquipmentDataAsset", ExactClass = false, EditFixedOrder))
	TArray<UItemEquipmentDataAsset*> EquipmentDataAssets;
#endif

	/**
	 * Array of integers representing the amounts of items in equipment slots.
	 */
	UPROPERTY(ReplicatedUsing = OnRep_EquipmentAmounts, BlueprintReadOnly, EditAnywhere, Category = "Inventory System|Equipment", meta = (EditConditionHides, EditCondition = "AllowEquipmentEdit", EditFixedOrder, NoResetToDefault))
	TArray<int> EquipmentAmounts;

	UFUNCTION()
	void OnRep_EquipmentAmounts(TArray<int> OldEquipmentAmounts);

	/**
	 * Array of indices representing dynamic stats associated with items in equipment.
	 */
	UPROPERTY(ReplicatedUsing = OnRep_EquipmentDynamicStatsIndices, BlueprintReadOnly, EditAnywhere, Category = "Inventory System|Equipment", meta = (EditConditionHides, EditCondition = "AllowEquipmentEdit", EditFixedOrder, NoResetToDefault))
	TArray<int> EquipmentDynamicStatsIndices;

	UFUNCTION()
	void OnRep_EquipmentDynamicStatsIndices(TArray<int> OldEquipmentDynamicStatsIndices);

	/**
	 * Array of item properties representing dynamic stats associated with items in equipment. Needs an index in EquipmentDynamicStatsIndices to work.
	 */
	UPROPERTY(ReplicatedUsing = OnRep_EquipmentDynamicStats, BlueprintReadOnly, EditAnywhere, Category = "Inventory System|Equipment", meta = (EditConditionHides, EditCondition = "AllowEquipmentEdit", EditFixedOrder))
	TArray<FItemProperties> EquipmentDynamicStats;

	UFUNCTION()
	void OnRep_EquipmentDynamicStats(TArray<FItemProperties> OldEquipmentDynamicStats);

public:
#if WITH_EDITOR
	/**
	 *	Set editor begun play.
	 *
	 * @param EndPlayReason 
	 */
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	
	/**
	 * Post edit change Property.
	 *
	 * @param PropertyChangedEvent 
	 */
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

	/**
	 * Used to check if actor is setup correctly and prevent execution of further actions.
	 *
	 * @return
	 */
	virtual bool InternalChecks(const bool bIsSavePackageEvent = false) override;

	/**
	 * Delegate to add functionality after item pick up.
	 */
	UPROPERTY(BlueprintAssignable, BlueprintCallable)
	FPickUpItemSuccessDelegate PickUpItemSuccessDelegate;

	/**
	 * Delegate to add functionality after item pick up failed.
	 */
	UPROPERTY(BlueprintAssignable, BlueprintCallable)
	FPickUpItemFailureDelegate PickUpItemFailureDelegate;

	/**
	 * Delegate to add functionality after item was equipped.
	 */
	UPROPERTY(BlueprintAssignable, BlueprintCallable)
	FItemEquipFromInventorySuccessDelegate ItemEquipFromInventorySuccessDelegate;

	/**
	 * Delegate to add functionality after item was unequipped.
	 */
	UPROPERTY(BlueprintAssignable, BlueprintCallable)
	FItemUnequipSuccessDelegate ItemUnequipSuccessDelegate;

	/**
	 * Delegate to add functionality after a whole item or a certain amount is removed.
	 */
	UPROPERTY(BlueprintAssignable, BlueprintCallable)
	FRemoveEquipmentAmountFromSlotSuccessDelegate RemoveEquipmentAmountFromSlotSuccessDelegate;

	/**
	 * Delegate to add functionality after item was directly added to equipment slot.
	 */
	UPROPERTY(BlueprintAssignable, BlueprintCallable)
	FAddItemToEquipmentSlotSuccessDelegate AddItemToEquipmentSlotSuccessDelegate;

	/**
	 * Delegate to add functionality after item has failed to be added directly to the equipment slot.
	 */
	UPROPERTY(BlueprintAssignable, BlueprintCallable)
	FAddItemToEquipmentSlotFailureDelegate AddItemToEquipmentSlotFailureDelegate;

	/**
	 * Delegate to add functionality after equipment type was set.
	 */
	UPROPERTY(BlueprintAssignable, BlueprintCallable)
	FSetEquipmentTypeSuccessDelegate SetEquipmentTypeSuccessDelegate;

	/**
	 * Delegate to add functionality after equipment type set failed.
	 */
	UPROPERTY(BlueprintAssignable, BlueprintCallable)
	FSetEquipmentTypeFailureDelegate SetEquipmentTypeFailureDelegate;

	/**
	 * Delegate used to add functionality after an item was changed in any way.
	 */
	UPROPERTY(BlueprintAssignable, BlueprintCallable)
	FChangedEquipmentSlotsDelegate ChangedEquipmentSlotsDelegate;

	/**
	 * Delegate used to add functionality after an MaxEquipmentStackSize was set.
	 */
	UPROPERTY(BlueprintAssignable, BlueprintCallable)
	FSetMaxEquipmentStackSizeSuccessDelegate SetMaxEquipmentStackSizeSuccessDelegate;
	
	/**
	 * Get the EquipmentSlots.
	 *
	 * @return The equipment slots structure.
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory System")
	TArray<FEquipmentSlot> GetEquipmentSlots() const;

	/**
	 * Get an EquipmentSlot.
	 * @param Slot The target slot index
	 *
	 * @return The EquipmentSlot.
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory System")
	FEquipmentSlot GetEquipmentSlot(int Slot) const;

	/**
	 * Set, remove or add equipment type of given slot if valid and in range. This will unequip an item if the new equipment type is different!
	 *
	 * @param Slot
	 * @param EquipmentType 
	 */
	UFUNCTION(Server, WithValidation, Reliable, BlueprintCallable, Category = "Inventory System")
	void SetEquipmentType(const int Slot, const FPrimaryAssetId EquipmentType = FPrimaryAssetId());
	void SetEquipmentType_Implementation(const int Slot, const FPrimaryAssetId EquipmentType = FPrimaryAssetId());

	/**
	 * Check if a slot has a specific item property.
	 *
	 * @param Slot			The slot index to check.
	 * @param Name			The name of the item property.
	 * @param bIsEquipment	Indicates whether the slot is for equipment (default is false).
	 *
	 * @return True if the item property exists in the slot, false otherwise.
	 */
	virtual bool HasItemProperty(const int Slot, const FName Name, const bool bIsEquipment = false) override;

	/**
	 * Get item property.
	 *
	 * @param Slot           The slot to check.
	 * @param Name           The name of the item property.
	 * @param bIsEquipment   Boolean indicating if the slot is an equipment slot.
	 * @return               True if the item property is found, false otherwise.
	 */
	virtual FItemProperty GetItemProperty(const int Slot, const FName Name, const bool bIsEquipment = false) override;

	/**
	 * Set the amount of a slot. Must be a number greater than 0 and less than MaxStackSize.
	 *
	 * @param Slot			The slot index to set the amount for.
	 * @param Amount		The new amount value.
	 * @param bIsEquipment	Indicates whether the slot is for equipment (default is false).
	 */
	virtual void SetSlotAmount_Implementation(const int Slot, const int Amount, const bool bIsEquipment = false) override;

	/**
	 * Set an ItemProperty of a given slot. Remove, add or edit item properties.
	 *
	 * @param Slot           The slot to set the item property for.
	 * @param Name           The name of the item property.
	 * @param DisplayName	 The display name of the property.
	 * @param Value          The value of the item property.
	 * @param bIsEquipment   Boolean indicating if the slot is an equipment slot.
	 */
	virtual void SetSlotItemProperty_Implementation(const int Slot, const FName Name, const FText& DisplayName, const FText& Value, const bool bIsEquipment = false) override;

	/**
	 * Swap items in the inventory or equipment. Merge item stacks if the first and second index have the same static and dynamic properties.
	 *
	 * @param First               The index of the first item to swap.
	 * @param Second              The index of the second item to swap.
	 * @param bCanStack           Specifies if stacking is allowed (default is false).
	 * @param bIsEquipment		  Is equipment.
	 */
	virtual void SwapItems_Implementation(const int First, const int Second, const bool bCanStack, const bool bIsEquipment) override;

	/**
	 * Pick up an ItemDrop and destroy it after successfully adding it to the inventory.
	 *
	 * @param Item       The ItemDrop to pick up.
	 * @param bCanStack  Specifies if stacking is allowed (default is true).
	 */
	UFUNCTION(Server, WithValidation, Reliable, Category = "Inventory System")
	void PickUpItemDrop(AItemDrop* const& Item, const bool bCanStack = true);
	virtual void PickUpItemDrop_Implementation(AItemDrop* const& Item, const bool bCanStack = true);

	/**
	 * Internal with return. Dont use for implementation!!! Pick up an ItemDrop and destroy it after successfully adding it to the inventory.
	 *
	 * @param Item			The ItemDrop to pick up.
	 * @param bCanStack		Specifies if stacking is allowed (default is true).
	 * @param ChangedSlots	Was item added at least once to the inventory.
	 */
	bool PickUpItemDropInternal(AItemDrop* const& Item, const bool bCanStack, TArray<int>& ChangedSlots);

	/**
	 * Add an item to a specified equipment slot. This should only be used for items outside the inventory.
	 * Please keep track of your amount and stack size as this will always reset the amount to the max amount allowed for the slot
	 *
	 * @param InventoryAsset			The primary asset ID of the item to add.
	 * @param EquipmentSlot				The target equipment slot index.
	 * @param DynamicStats				The dynamic properties of the item.
	 * @param Amount					The amount of items to add. Will be reset GetEquipmentStackSizeConfig if higher (default is 1).
	 * @param bCanUnequippedItemStack	Specifies if stacking is allowed for unequipped items (default is true).
	 * @param bCanStack					Is stacking allowed (default false).
	 */
	UFUNCTION(Server, WithValidation, Reliable, BlueprintCallable, Category = "Inventory System")
	void AddItemToEquipmentSlot(const FPrimaryAssetId InventoryAsset, const int EquipmentSlot, const FItemProperties DynamicStats = FItemProperties(), int Amount = 1, const bool bCanUnequippedItemStack = true, const bool bCanStack = false);
	virtual void AddItemToEquipmentSlot_Implementation(const FPrimaryAssetId InventoryAsset, const int EquipmentSlot, const FItemProperties DynamicStats = FItemProperties(), int Amount = 1, const bool bCanUnequippedItemStack = true, const bool bCanStack = false);

	/**
	 * Remove items from an equipment slot.
	 *
	 * @param EquipmentSlot		The equipment slot index.
	 * @param Amount			The amount of items to remove (default is 1).
	 */
	UFUNCTION(Server, WithValidation, Reliable, BlueprintCallable, Category = "Inventory System")
	void RemoveEquipmentAmountFromSlot(const int EquipmentSlot, const int Amount = 1);
	virtual void RemoveEquipmentAmountFromSlot_Implementation(const int EquipmentSlot, const int Amount = 1);

	/**
	 * Equip an item from the inventory. If an item is already equipped, it will be switched with the item in the inventory or inserted in the next empty slot.
	 *
	 * @param Slot						The inventory slot index of the item to equip.
	 * @param EquipmentSlot				The target equipment slot index.
	 * @param bCanUnequippedItemStack	Specifies if stacking is allowed for unequipped items (default is true).
	 * @param bCanStack					Specifies if stacking is allowed (default is true).
	 */
	UFUNCTION(Server, WithValidation, Reliable, BlueprintCallable, Category = "Inventory System")
	void ItemEquipFromInventory(const int Slot, const int EquipmentSlot = -1, const bool bCanUnequippedItemStack = true, const bool bCanStack = true);
	virtual void ItemEquipFromInventory_Implementation(const int Slot, const int EquipmentSlot = -1, const bool bCanUnequippedItemStack = true, const bool bCanStack = true);

	/**
	 * Unequip an item from an equipment slot. The slot must be empty or the item must be stackable. Otherwise, it searches for the next empty or stackable slot.
	 *
	 * @param EquipmentSlot          The equipment slot index to unequip from.
	 * @param IgnoreInventorySlots   Slots that should be ignored and will not be filled.
	 * @param bCanStack              Specifies if stacking is allowed (default is false).
	 * @param SpecificInventorySlot  The specific inventory slot index to store the unequipped item (default is -1).
	 */
	UFUNCTION(Server, WithValidation, Reliable, BlueprintCallable, Category = "Inventory System")
	void ItemUnequip(const int EquipmentSlot, const TArray<int>& IgnoreInventorySlots, const bool bCanStack = false, const int SpecificInventorySlot = -1);
	virtual void ItemUnequip_Implementation(const int EquipmentSlot, const TArray<int>& IgnoreInventorySlots, const bool bCanStack = false, const int SpecificInventorySlot = -1);

	/**
	 * Internal with return. Dont use for implementation!!! Unequip an item from an equipment slot. The slot must be empty or the item must be stackable. Otherwise, it searches for the next empty or stackable slot.
	 *
	 * @param EquipmentSlot          The equipment slot index to unequip from.
	 * @param IgnoreInventorySlots   Slots that should be ignored and will not be filled.
	 * @param bCanStack              Specifies if stacking is allowed (default is false).
	 * @param SpecificInventorySlot  The specific inventory slot index to store the unequipped item (default is -1).
	 * @return 
	 */
	TArray<int> ItemUnequipInternal(const int& EquipmentSlot, const TArray<int> IgnoreInventorySlots = {}, const bool bCanStack = false, const int SpecificInventorySlot = -1);

	/**
	 * Internal with return. Dont use for implementation!!! Add an item to another component. Moves an item from the current component with an assigned amount to another component.
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
	virtual TArray<int> AddItemToComponentInternal(const int Slot, UItemContainerComponent* ItemContainerComponent, int& Amount, const bool bCanStack = false, const bool bIsEquipment = false, const bool bRevertWhenFull = true) override;

	/**
	 * Collect all items from this container and add them to the specified ItemContainerComponent.
	 *
	 * This function collects all items present in this container (Equipment included) and adds them to the provided ItemContainerComponent.
	 * The items are added one by one, and if the ItemContainerComponent allows stacking, items of the same type can stack together.
	 *
	 * @param ItemContainerComponent The target ItemContainerComponent where items will be collected.
	 * @param bCanStack Indicates whether items can stack within the ItemContainerComponent (default: true).
	 *
	 * @note This function should be called on the server for proper replication in multiplayer scenarios.
	 */
	virtual void CollectAllItems_Implementation(UItemContainerComponent* ItemContainerComponent, const bool bCanStack) override;

	/**
	 * Internal method used to get the global config value or a default set in this component.
	 * 
	 * @return 
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory System")
	virtual int GetEquipmentStackSizeConfig() const;

	/**
	 * Set EquipmentStackSize. Checks all items and stops execution if an item would have overflow. Please split, set items manually or active force execution with the bForce parameter to use this function.
	 * By forcing the execution you will delete all items that are overflowing!
	 * 
	 * @return 
	 */
	UFUNCTION(Server, WithValidation, Reliable, BlueprintCallable, Category = "Inventory System")
	void SetEquipmentStackSizeConfig(const int NewMaxEquipmentStackSize, const bool bForce = false);
	virtual void SetEquipmentStackSizeConfig_Implementation(const int NewMaxEquipmentStackSize, const bool bForce = false);

	/**
	 * Internal method used to get the global config value or a default set in this component.
	 * 
	 * @return 
	 */
	virtual int GetStackSizeConfig() const override;

	/**
	 * Internal method used to get the global config value or a default set in this component.
	 * 
	 * @return 
	 */
	virtual int GetInventorySizeConfig() const override;

	/**
	 * Internal method used to replace ", ( and ) string parts with a space.
	 *
	 * @param OriginalArrayString 
	 * @return 
	 */
	static FString ReplaceEquipmentArrayString(FString OriginalArrayString);
};
#undef LOCTEXT_NAMESPACE
