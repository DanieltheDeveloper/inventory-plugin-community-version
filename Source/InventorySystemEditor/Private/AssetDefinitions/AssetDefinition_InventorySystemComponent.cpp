// © 2024 Daniel Münch. All Rights Reserved

#include "AssetDefinition_InventorySystemComponent.h"

#include "InventorySystemComponent.h"

#define LOCTEXT_NAMESPACE "InventorySystem"

FLinearColor UAssetDefinition_InventorySystemComponent::GetAssetColor() const
{
	return FColor(117, 146, 204);
}

TSoftClassPtr<> UAssetDefinition_InventorySystemComponent::GetAssetClass() const
{
	return UInventorySystemComponent::StaticClass();
}

TConstArrayView<FAssetCategoryPath> UAssetDefinition_InventorySystemComponent::GetAssetCategories() const
{
	static FAssetCategoryPath AssetPaths[] = { FAssetCategoryPath(LOCTEXT("InventorySystem", "Inventory System")) };
	return AssetPaths;
}

#undef LOCTEXT_NAMESPACE


