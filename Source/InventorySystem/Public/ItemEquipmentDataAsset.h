// © 2024 Daniel Münch. All Rights Reserved

#pragma once

#include "ItemEquipmentTypeDataAsset.h"
#include "ItemDataAsset.h"
#include "ItemEquipmentDataAsset.generated.h"

#define LOCTEXT_NAMESPACE "InventorySystem"

/**
 * @class UItemEquipmentDataAsset
 * @brief Defines equipment-specific data for items, including types and properties for equippable items.
 *
 * This class extends UItemDataAsset to include equipment-specific information such as equipment types, which can be used to define where
 * and how an item can be equipped. This includes support for multiple equipment types to allow items like weapons to be equipped in
 * different slots (e.g., left hand, right hand).
 *
 * General Usage:
 * - Create subclasses for specific types of equipment with additional properties as needed.
 * - Utilize in conjunction with UInventorySystemComponent for managing equippable items.
 *
 * Example Use Case:
 * @code
 * // Creating a new equipment data asset in C++
 * UItemEquipmentDataAsset* NewEquipmentData = NewObject<UItemEquipmentDataAsset>(this, UItemEquipmentDataAsset::StaticClass());
 * NewEquipmentData->EquipmentType.Add(EquipmentTypeAssetID);
 * @endcode
 */
UCLASS(Abstract, Category = "Inventory System")
class INVENTORYSYSTEM_API UItemEquipmentDataAsset : public UItemDataAsset
{
	GENERATED_BODY()

public:

#if WITH_EDITORONLY_DATA
	/**
	 * Contains type data for this item. Please not use this directly in runtime as it will be empty. Use EquipmentType instead and load it with the AssetManager.
	 */
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Inventory System|Types", meta = (AllowedClasses = "/Script/InventorySystem.ItemEquipmentTypeDataAsset", ExactClass = false))
	TArray<UItemEquipmentTypeDataAsset*> EquipmentTypeDataAssets;

	/**
	 * Post edit change property.
	 *
	 * @param PropertyChangedEvent 
	 */
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

	/**
	 * Equipment type. Set multiple types for example weapon types to be equipped in left and right hand.
	 */
	UPROPERTY(BlueprintReadOnly, Category = "Inventory System|Types", AssetRegistrySearchable, meta = (EditCondition = "false", AllowedClasses = "/Script/InventorySystem.ItemEquipmentTypeDataAsset", ExactClass = false))
	TArray<FPrimaryAssetId> EquipmentType;

	/**
	 * Implement Interface.
	 * 
	 * @return The equipment types associated with this equipment data asset.
	 */
	TArray<FPrimaryAssetId> GetEquipmentType_Implementation() override;
};

#undef LOCTEXT_NAMESPACE