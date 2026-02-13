// © 2024 Daniel Münch. All Rights Reserved

#include "AssetDefinition_ItemEquipmentType.h"

#include "ItemEquipmentTypeDataAsset.h"

 #define LOCTEXT_NAMESPACE "InventorySystem"

 FLinearColor UAssetDefinition_ItemEquipmentType::GetAssetColor() const
 {
 	return FColor(185, 78, 72);
 }

 TSoftClassPtr<> UAssetDefinition_ItemEquipmentType::GetAssetClass() const
 {
 	return UItemEquipmentTypeDataAsset::StaticClass();
 }

 TConstArrayView<FAssetCategoryPath> UAssetDefinition_ItemEquipmentType::GetAssetCategories() const
 {
 	static FAssetCategoryPath AssetPaths[] = { FAssetCategoryPath(LOCTEXT("InventorySystem", "Inventory System"), LOCTEXT("InventorySystem_SubCategoryItemEquipmentType", "Item Equipment Type")) };
 	return AssetPaths;
 }

FAssetSupportResponse UAssetDefinition_ItemEquipmentType::CanDuplicate(const FAssetData& InAsset) const
{
	return FAssetSupportResponse::NotSupported();
}

bool UAssetDefinition_ItemEquipmentType::CanMerge() const
{
	return false;
}

bool UAssetDefinition_ItemEquipmentType::CanImport() const
{
	return false;
}

#undef LOCTEXT_NAMESPACE


