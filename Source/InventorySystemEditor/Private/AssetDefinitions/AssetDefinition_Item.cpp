// © 2024 Daniel Münch. All Rights Reserved

#include "AssetDefinition_Item.h"

#include "ItemDataAsset.h"

#define LOCTEXT_NAMESPACE "InventorySystem"

FLinearColor UAssetDefinition_Item::GetAssetColor() const
{
	return FColor(175, 0, 128);
}

TSoftClassPtr<> UAssetDefinition_Item::GetAssetClass() const
{
	return UItemDataAsset::StaticClass();
}

TConstArrayView<FAssetCategoryPath> UAssetDefinition_Item::GetAssetCategories() const
{
	static FAssetCategoryPath AssetPaths[] = { FAssetCategoryPath(LOCTEXT("InventorySystem", "Inventory System"), LOCTEXT("InventorySystem_SubCategoryItem", "Item")) };
	return AssetPaths;
}

FAssetSupportResponse UAssetDefinition_Item::CanDuplicate(const FAssetData& InAsset) const
{
	return FAssetSupportResponse::NotSupported();
}

bool UAssetDefinition_Item::CanMerge() const
{
	return false;
}

bool UAssetDefinition_Item::CanImport() const
{
	return false;
}

#undef LOCTEXT_NAMESPACE


