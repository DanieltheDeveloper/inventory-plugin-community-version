// © 2024 Daniel Münch. All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Engine/Texture2D.h"
#include "ItemEquipmentTypeDataAsset.h"
#include "ItemAssetInterface.generated.h"

#define LOCTEXT_NAMESPACE "InventorySystem"

/**
 * @class UItemAssetInterface
 * @brief Defines a standard interface for item assets, providing essential item information and functionalities.
 *
 * This interface establishes a common set of functions that all item assets must implement, such as obtaining the item's name,
 * icon, stackability, and equipment type. It ensures consistency across different types of items within the game's inventory system,
 * facilitating easier management and interaction with items in game logic and UI.
 *
 * General Usage:
 * - Implement this interface in any UObject-derived class that represents an item in the game.
 * - Use the provided methods to interact with item data in a uniform way.
 *
 * Example Use Case:
 * @code
 * class UMyGameItem : public UObject, public IItemAssetInterface
 * {
 *     FText GetName() override { return FText::FromString("Example Item"); }
 *     bool CanStack() override { return true; }
 *     UTexture2D* GetIcon() override { return MyIconTexture; }
 *     TArray<UItemEquipmentTypeDataAsset*> GetEquipmentType() override { return MyEquipmentTypes; }
 * };
 * @endcode
 */
UINTERFACE(BlueprintType, Category = "Inventory System")
class INVENTORYSYSTEM_API UItemAssetInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * Interface for Item Asset
 */
class INVENTORYSYSTEM_API IItemAssetInterface
{
	GENERATED_BODY()

public:

	/**
	 * Get the name of the item.
	 *
	 * @return The name of the item as FText.
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Item")
	FText GetName();

	/**
	 * Check if the item can stack.
	 *
	 * @return True if the item can stack, false otherwise.
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Item")
	bool CanStack();

	/**
	 * Get the icon of the item.
	 *
	 * @return The icon of the item as a Texture2D.
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Item")
	UTexture2D* GetIcon();

	/**
	 * Get the equipment type of the item.
	 *
	 * @return The equipment types of the item as FPrimaryAssetId.
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Equipment")
	TArray<FPrimaryAssetId> GetEquipmentType();
};

#undef LOCTEXT_NAMESPACE