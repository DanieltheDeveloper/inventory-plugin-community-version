// © 2024 Daniel Münch. All Rights Reserved

#pragma once

#include "Engine/DataAsset.h"
#include "Engine/Texture2D.h"
#include "ItemAssetInterface.h"
#include "ItemDataAsset.generated.h"

#define LOCTEXT_NAMESPACE "InventorySystem"

/**
 * @class UItemDataAsset
 * @brief Defines the base data for items within the game, including names, icons, and stackability.
 *
 * UItemDataAsset is an abstract class used as a base for all item data assets in the game. It defines the essential properties
 * of an item, such as its name, icon, and whether it can be stacked. This class implements the IItemAssetInterface, ensuring
 * all derived assets provide a standard interface for accessing item data.
 *
 * General Usage:
 * - Create subclasses for specific item types with additional properties as needed.
 * - Use in conjunction with UInventorySystemComponent for inventory management.
 * - Accessible in Blueprints and C++ for easy integration into game systems.
 *
 * Example Use Case:
 * @code
 * // Creating a new item data asset instance in C++
 * UItemDataAsset* NewItemData = NewObject<UItemDataAsset>(this, UItemDataAsset::StaticClass());
 * NewItemData->Name = FText::FromString(TEXT("New Item"));
 * NewItemData->bCanStack = true;
 * NewItemData->Icon = MyIconTexture;
 * ...
 * @endcode
 */
UCLASS(Abstract, Blueprintable, BlueprintType, Category = "Inventory System")
class INVENTORYSYSTEM_API UItemDataAsset : public UPrimaryDataAsset, public IItemAssetInterface
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

	/** 
	 * Get the primary asset ID of this item data asset.
	 *
	 * @return The primary asset ID.
	 */
	virtual FPrimaryAssetId GetPrimaryAssetId() const override;

	/**
	 * The name of the item.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory System|Description")
	FText Name;

	/**
	 * Check if this item can be stacked.
	 *
	 * @return True if the item can be stacked, false otherwise.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory System|Behavior", AssetRegistrySearchable)
	bool bCanStack = false;

	/**
	 * The item texture for usage in the UI or 2D game world.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory System|Visuals", meta = (AssetBundles = "Visuals"))
	UTexture2D* Icon;

	// Implement Interface
	
	/**
	 * Get the name of the item.
	 *
	 * @return The name of the item as FText.
	 */
	FText GetName_Implementation() override;
	
	/**
	 * Check if the item can stack.
	 *
	 * @return True if the item can stack, false otherwise.
	 */
	bool CanStack_Implementation() override;
	
	/**
	 * Get the icon of the item.
	 *
	 * @return The icon of the item as a UTexture2D.
	 */
	UTexture2D* GetIcon_Implementation() override;
};

#undef LOCTEXT_NAMESPACE