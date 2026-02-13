// © 2024 Daniel Münch. All Rights Reserved

#include "AssetDefinition_ItemContainerComponent.h"

#include "ItemContainerComponent.h"

#define LOCTEXT_NAMESPACE "InventorySystem"

FLinearColor UAssetDefinition_ItemContainerComponent::GetAssetColor() const
{
	return FColor(198, 115, 255);
}

TSoftClassPtr<> UAssetDefinition_ItemContainerComponent::GetAssetClass() const
{
	return UItemContainerComponent::StaticClass();
}

TConstArrayView<FAssetCategoryPath> UAssetDefinition_ItemContainerComponent::GetAssetCategories() const
{
	static FAssetCategoryPath AssetPaths[] = { FAssetCategoryPath(LOCTEXT("InventorySystem", "Inventory System")) };
	return AssetPaths;
}

#undef LOCTEXT_NAMESPACE


