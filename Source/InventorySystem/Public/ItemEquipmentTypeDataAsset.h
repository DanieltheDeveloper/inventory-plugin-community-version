// © 2024 Daniel Münch. All Rights Reserved

#pragma once

#include "Engine/DataAsset.h"
#include "ItemEquipmentTypeAssetInterface.h"
#include "ItemEquipmentTypeDataAsset.generated.h"

#define LOCTEXT_NAMESPACE "InventorySystem"

/**
 * @class UItemEquipmentTypeDataAsset
 * @brief Defines equipment types for items, such as armor slots or weapon categories, to be used within the game's inventory system.
 *
 * This class acts as a blueprintable asset that game designers can use to specify different types of equipment. It includes
 * properties such as the equipment's name and category, and implements interfaces for consistent handling across the game.
 * It's crucial for categorizing equipment items and ensuring they fit into the correct slots in the inventory system.
 *
 * General Usage:
 * - Create instances in the Unreal Editor to define each unique equipment type your game will use.
 * - Reference these data assets when creating equipment items to specify their type.
 *
 * Example Use Case:
 * @code
 * // In Unreal Editor, create a new ItemEquipmentTypeDataAsset named "ArmorChest".
 * // In your game code or blueprints, you can then reference this asset to categorize chest armor items.
 *
 * // Creating a new item data asset instance in C++
 * UItemEquipmentTypeDataAsset* NewEquipmentTypeData = NewObject<UItemEquipmentTypeDataAsset>(this, UItemEquipmentTypeDataAsset::StaticClass());
 * NewEquipmentTypeData->Name = FText::FromString(TEXT("New Item"));
 * @endcode
 */
UCLASS(Abstract, Blueprintable, BlueprintType, Category = "Inventory System")
class INVENTORYSYSTEM_API UItemEquipmentTypeDataAsset : public UPrimaryDataAsset, public IItemEquipmentTypeAssetInterface
{
	GENERATED_BODY()

public:
	/**
	 * Must be set correctly, or asset won't be gathered by the Asset Manager.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Inventory System|Asset Manager", meta = (EditCondition = "IsDataOnly == false", EditConditionHides))
	FPrimaryAssetType AssetType;

#if WITH_EDITORONLY_DATA
	/**
	 * For internal use only. Allows faster check of DataAssets.
	 */
	UPROPERTY()
	bool IsDataOnly = false;

	/**
	 * Search all AItemDrop actors and rerun construction scripts.
	 *
	 * @param ObjectSaveContext 
	 */
	virtual void PreSave(FObjectPreSaveContext ObjectSaveContext) override;

	/**
	 * Search all AItemDrop actors and rerun construction scripts.
	 *
	 * @param ObjectSaveContext 
	 */
	virtual void PostSaveRoot(FObjectPostSaveRootContext ObjectSaveContext) override;

	/**
	 * Main method for rerun construction scripts.
	 */
	void RerunAllItemDropConstructionScripts();
#endif

	/**
	 * Get the logical name of the item data asset.
	 *
	 * @return The logical name as FString.
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory System")
	FString GetIdentifierString() const;

	/** Overridden to use saved type. */
	virtual FPrimaryAssetId GetPrimaryAssetId() const override;

	/**
	 * Full equipment slot type name.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory System|Description")
	FText Name;

	/**
	 * Implement Interface.
	 * 
	 * @return The name of the equipment type as a localized text.
	 */
	FText GetName_Implementation() override;
};

#undef LOCTEXT_NAMESPACE