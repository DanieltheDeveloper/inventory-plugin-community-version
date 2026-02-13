// © 2024 Daniel Münch. All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "AssetDefinitionDefault.h"

#include "AssetDefinition_InventorySystemComponent.generated.h"

#define LOCTEXT_NAMESPACE "InventorySystem"

/**
 * Custom asset definition class for Inventory System Component assets.
 */
UCLASS()
class UAssetDefinition_InventorySystemComponent : public UAssetDefinitionDefault
{
	GENERATED_BODY()

public:
	/**
	 * Implementation of the GetAssetDisplayName function to provide the display name for this asset type.
	 *
	 * @return The display name for the Inventory System Component asset type.
	 */
	virtual FText GetAssetDisplayName() const override { return LOCTEXT("InventorySystemComponent", "Inventory System Component"); }

	/**
	 * Implementation of the GetAssetColor function to provide a custom color for this asset type.
	 *
	 * @return The custom color for the Inventory System Component asset type.
	 */
	virtual FLinearColor GetAssetColor() const override;

	/**
	 * Implementation of the GetAssetClass function to specify the UClass associated with this asset type.
	 *
	 * @return A TSoftClassPtr to the UClass associated with the Inventory System Component asset type.
	 */
	virtual TSoftClassPtr<UObject> GetAssetClass() const override;

	/**
	 * Implementation of the GetAssetCategories function to provide asset categories for this asset type.
	 *
	 * @return A TConstArrayView containing the asset categories for the Inventory System Component asset type.
	 */
	virtual TConstArrayView<FAssetCategoryPath> GetAssetCategories() const override;

	UAssetDefinition_InventorySystemComponent()
	{
		IncludeClassInFilter = EIncludeClassInFilter::Always;
	}
};

#undef LOCTEXT_NAMESPACE