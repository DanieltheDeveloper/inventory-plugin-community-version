// © 2024 Daniel Münch. All Rights Reserved

#include "AssetDefinition_ItemDrop.h"

#include "ItemDrop.h"

#define LOCTEXT_NAMESPACE "InventorySystem"

FLinearColor UAssetDefinition_ItemDrop::GetAssetColor() const
{
	return FColor(241, 99, 6);
}

TSoftClassPtr<> UAssetDefinition_ItemDrop::GetAssetClass() const
{
	return AItemDrop::StaticClass();
}

TConstArrayView<FAssetCategoryPath> UAssetDefinition_ItemDrop::GetAssetCategories() const
{
	static FAssetCategoryPath AssetPaths[] = { FAssetCategoryPath(LOCTEXT("InventorySystem", "Inventory System")) };
	return AssetPaths;
}

#undef LOCTEXT_NAMESPACE


