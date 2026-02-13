// © 2024 Daniel Münch. All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "AssetDefinitionDefault.h"

#include "AssetDefinition_ItemEquipmentType.generated.h"

#define LOCTEXT_NAMESPACE "InventorySystem"

/**
 * Custom asset definition class for Item Equipment Type assets.
 */
UCLASS()
class UAssetDefinition_ItemEquipmentType : public UAssetDefinitionDefault
{
	GENERATED_BODY()

public:
	/**
	 * Implementation of the GetAssetDisplayName function to provide the display name for this asset type.
	 *
	 * @return The display name for the Item Equipment Type asset type.
	 */
	virtual FText GetAssetDisplayName() const override { return LOCTEXT("ItemEquipmentType", "Item Equipment Type"); }

	/**
	 * Implementation of the GetAssetColor function to provide a custom color for this asset type.
	 *
	 * @return The custom color for the Item Equipment Type asset type.
	 */
	virtual FLinearColor GetAssetColor() const override;

	/**
	 * Implementation of the GetAssetClass function to specify the UClass associated with this asset type.
	 *
	 * @return A TSoftClassPtr to the UClass associated with the Item Equipment Type asset type.
	 */
	virtual TSoftClassPtr<UObject> GetAssetClass() const override;

	/**
	 * Implementation of the GetAssetCategories function to provide asset categories for this asset type.
	 *
	 * @return A TConstArrayView containing the asset categories for the Item Equipment Type asset type.
	 */
	virtual TConstArrayView<FAssetCategoryPath> GetAssetCategories() const override;

	UAssetDefinition_ItemEquipmentType()
	{
		IncludeClassInFilter = EIncludeClassInFilter::Always;
	}

	/**
	 * Can duplicate
	 * 
	 * @param InAsset 
	 * @return 
	 */
	virtual FAssetSupportResponse CanDuplicate(const FAssetData& InAsset) const override;

	/**
	 * Can merge
	 *
	 * @return 
	 */
	virtual bool CanMerge() const override;

	/**
	 * Can import
	 *
	 * @return 
	 */
	virtual bool CanImport() const override;
};

#undef LOCTEXT_NAMESPACE