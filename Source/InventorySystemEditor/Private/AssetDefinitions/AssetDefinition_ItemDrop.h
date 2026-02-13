// © 2024 Daniel Münch. All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "AssetDefinitionDefault.h"

#include "AssetDefinition_ItemDrop.generated.h"

#define LOCTEXT_NAMESPACE "InventorySystem"

/**
 * Custom asset definition class for Item Drop assets.
 */
UCLASS()
class UAssetDefinition_ItemDrop : public UAssetDefinitionDefault
{
	GENERATED_BODY()

public:
	/**
	 * Implementation of the GetAssetDisplayName function to provide the display name for this asset type.
	 *
	 * @return The display name for the Item Drop asset type.
	 */
	virtual FText GetAssetDisplayName() const override { return LOCTEXT("ItemDrop", "Item Drop"); }

	/**
	 * Implementation of the GetAssetColor function to provide a custom color for this asset type.
	 *
	 * @return The custom color for the Item Drop asset type.
	 */
	virtual FLinearColor GetAssetColor() const override;

	/**
	 * Implementation of the GetAssetClass function to specify the UClass associated with this asset type.
	 *
	 * @return A TSoftClassPtr to the UClass associated with the Item Drop asset type.
	 */
	virtual TSoftClassPtr<UObject> GetAssetClass() const override;

	/**
	 * Implementation of the GetAssetCategories function to provide asset categories for this asset type.
	 *
	 * @return A TConstArrayView containing the asset categories for the Item Drop asset type.
	 */
	virtual TConstArrayView<FAssetCategoryPath> GetAssetCategories() const override;

	UAssetDefinition_ItemDrop()
	{
		IncludeClassInFilter = EIncludeClassInFilter::Always;
	}
};

#undef LOCTEXT_NAMESPACE