// © 2024 Daniel Münch. All Rights Reserved

#pragma once

#include "ItemDataAsset.h"
#include "ItemProperties.h"
#include "GameFramework/Actor.h"
#include <atomic>
#include "ItemDrop.generated.h"

#define LOCTEXT_NAMESPACE "InventorySystem"

class UInventorySystemComponent;

/**
 * @class AItemDrop
 * @brief Represents an item that can be picked up from the game world, including functionality for pickup and inventory interaction.
 *
 * AItemDrop acts as a physical representation of items within the game environment that players can interact with to add to their inventory.
 * It includes properties to manage item data, stacking, and the visual representation in the game world. This class is designed to work
 * seamlessly with the UInventorySystemComponent to facilitate item pickups and inventory management.
 *
 * General Usage:
 * - Place in the game world as a spawnable actor for items that can be picked up.
 * - Configure properties such as item data, stacking behavior, and visual representation through the editor or at runtime.
 * - If a different item type is utilized, it is required to build a new AItemDrop; the only variables that can be altered are the amount and dynamic stats.
 *
 * Example Use Case:
 * @code
 * // Assuming this is called within an actor class that has a reference to an inventory system component
 * AItemDrop* ItemDrop = GetWorld()->SpawnActor<AItemDrop>(AItemDropClass, Location, Rotation, ...);
 * ItemDrop->SetItemData(ItemDataAsset);
 * ItemDrop->PickUp(MyInventorySystemComponent);
 * @endcode
 */
UCLASS(Abstract, BlueprintType, Blueprintable, Category = "Inventory System", hidecategories = (Object), ClassGroup = ("Inventory System"))
class INVENTORYSYSTEM_API AItemDrop : public AActor
{
	GENERATED_BODY()

protected:
	/**
	 * Boolean indicating whether the ItemDrop is currently processing another request.
	 */
	UPROPERTY(Replicated, BlueprintReadOnly, VisibleAnywhere, Category = "Inventory System|Settings")
	bool bIsProcessing = false;

	/**
	 * Boolean indicating if, upon successful pick up, the actor should be destroyed. Count will be reset to 1 if every item has already been retrieved.
	 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Inventory System|Settings")
	bool bDestroyAfterPickUp = true;

	/**
	 * Internal use only. Boolean indicating whether other item properties are allowed to be edited.
	 */
	UPROPERTY(VisibleAnywhere, Category = "Inventory System|Item|Edit Conditions")
	bool AllowItemEdit = false;

	/**
	 * Internal use only. Boolean indicating whether other item properties are allowed to be edited.
	 */
	UPROPERTY(VisibleAnywhere, Category = "Inventory System|Item|Edit Conditions")
	bool AllowItemAssetEdit = false;

	/**
	 * Used for internal stack logic. Read only!
	 */
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Inventory System|Item")
	bool InternalCanStack = false;

	/**
	 * Max stack size for this actor or actor type if base class. When set to 0 default plugion config will be used. Can and should not be changed while playing.
	 */
	UPROPERTY(Replicated, BlueprintReadOnly, EditAnywhere, Category = "Inventory System|Settings", meta = (ClampMin="0", ExposeOnSpawn = "true", EditCondition = "bHasBegunPlayEditor == false", EditConditionHides))
	int MaxStackSize = 0;

	/**
	 * Called when the item begins playing in the world.
	 */
	virtual void BeginPlay() override;

	AItemDrop();

#if WITH_EDITORONLY_DATA
	/**
	 * A pointer to the sprite billboard for editor internal use.
	 */
	UPROPERTY()
	UBillboardComponent* SpriteComponent;

	/**
	 * Contains data for this item. Please don't use this directly in runtime as it will be empty. Use InventoryAsset instead and load it with the AssetManager.
	 */
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Inventory System|Item", meta = (EditCondition = "AllowItemAssetEdit == false", EditConditionHides, ToolTip = "Select the ItemData or ItemEquipmentAsset you specified", AllowedClasses = "/Script/InventorySystem.ItemDataAsset", ExactClass = false))
	UItemDataAsset* InventoryDataAsset;

	/**
	 * Internal use only. Boolean indicating whether the component is in a editor play context.
	 */
	UPROPERTY(VisibleAnywhere, Category = "Inventory System|Settings|Edit Conditions")
	bool bHasBegunPlayEditor = false;
#endif

public:
#if WITH_EDITORONLY_DATA
	/**
	 * Internal use only. Add check to save.
	 *
	 * @param ObjectSaveContext 
	 */
	virtual void PreSave(FObjectPreSaveContext ObjectSaveContext) override;

	/**
	 * Post edit change property.
	 *
	 * @param PropertyChangedEvent 
	 */
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;

	/**
	 *	Set editor begun play.
	 *
	 * @param EndPlayReason 
	 */
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
#endif
	
	/**
	 * Called when the item is constructed in the world.
	 *
	 * @param Transform  The transform of the item.
	 */
	virtual void OnConstruction(const FTransform& Transform) override;
	
	/**
	 * Used to check if actor is setup correctly and prevent execution of further actions.
	 *
	 * @param bPreventExecution
	 * @param bIsSavePackageEvent 
	 */
	UFUNCTION()
	void InternalChecks(const bool bPreventExecution = false, const bool bIsSavePackageEvent = false);
	
	/**
	 * Used in the Begin Play function to generate a random amount of the selected item. Previously specified amount will be overwritten.
	 * Must be greater than or equal to 0 and less than MaxRandomAmount.
	 */
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Inventory System|Settings", meta = (ClampMin="0", ExposeOnSpawn = "true", EditCondition = "InternalCanStack && AllowItemAssetEdit == false", EditConditionHides))
	int MinRandomAmount = 0;

	/**
	 * Used in the Begin Play function to generate a random amount of the selected item. Previously specified amount will be overwritten.
	 * Must be greater than MinRandomAmount.
	 */
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Inventory System|Settings", meta = (ClampMin="0", ExposeOnSpawn = "true", EditCondition = "InternalCanStack && AllowItemAssetEdit == false", EditConditionHides))
	int MaxRandomAmount = 0;

	/**
	 * Pick up the item and add it to the inventory system.
	 *
	 * @param InventorySystemComponent Component this item should be added to.
	 * @param bCanStack Indicates whether the item can stack if already present in the inventory.
	 */
	UFUNCTION(Server, Reliable, WithValidation, BlueprintCallable, Category = "Inventory System")
	void PickUp(UInventorySystemComponent* InventorySystemComponent, const bool bCanStack = true);
	void PickUp_Implementation(UInventorySystemComponent* InventorySystemComponent, const bool bCanStack = true);
	
	/**
	 * This after pick up event allows you to add functionality before the object gets destroyed. Make sure to always call the parent function! If the object should be kept after pick up with amount 0 use the bDestroyAfterPickUp flag.
	 *
	 * @param bSuccess
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "Inventory System")
	void AfterPickUpEvent(const bool bSuccess);

	/**
	 * Contains data for this item.
	 */
	UPROPERTY(Replicated, BlueprintReadOnly, EditInstanceOnly, Category = "Inventory System|Item", meta = (EditCondition = "AllowItemAssetEdit", EditConditionHides, ToolTip = "Select the ItemData or ItemEquipmentAsset you specified",  ExposeOnSpawn = "true", AllowedClasses = "/Script/InventorySystem.ItemDataAsset", ExactClass = false))
	FPrimaryAssetId InventoryAsset;
	
	/**
	 * Amount for the item.
	 */
	UPROPERTY(Replicated, BlueprintReadWrite, EditAnywhere, Category = "Inventory System|Item", meta = (EditCondition = "AllowItemEdit && InternalCanStack", ToolTip = "Set Item Amount", ExposeOnSpawn = "true", ClampMin="1", EditConditionHides))
	int Amount = 1;

	/**
	 * Contains dynamic stats for this item.
	 */
	UPROPERTY(Replicated, BlueprintReadWrite, EditAnywhere, Category = "Inventory System|Item", meta = (EditCondition = "AllowItemEdit", ToolTip = "Select the Dynamic stats you want to add", ExposeOnSpawn = "true"))
	FItemProperties DynamicStats;
	
	/**
	 * Internal method used to get the global config value or a default set in this component.
	 * 
	 * @return 
	 */
	int GetStackSizeConfig() const;
};

#undef LOCTEXT_NAMESPACE