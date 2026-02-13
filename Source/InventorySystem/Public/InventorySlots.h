// © 2024 Daniel Münch. All Rights Reserved

#pragma once

#include "ItemProperties.h"
#include "InventorySlots.generated.h"

#define LOCTEXT_NAMESPACE "InventorySystem"

/**
 * @struct FInventorySlot
 * @brief Represents a single slot in the inventory system, holding item data and quantity.
 *
 * This structure defines an inventory slot, including details like the slot index, the item's primary asset ID,
 * its properties, and the amount of the item in the slot. It's used throughout the inventory system to manage
 * items in the inventory, including adding, removing, and updating items.
 *
 * General Usage:
 * - Use this struct when managing inventory operations such as adding or removing items.
 * - Can be extended with additional properties or functions to suit specific game requirements.
 *
 * Example Use Case:
 * - Creating an inventory slot to add an item to the player's inventory, specifying the item type, properties, and quantity.
 */
USTRUCT(BlueprintType, Category = "Inventory System")
struct INVENTORYSYSTEM_API FInventorySlot
{
	GENERATED_BODY()
	
	/**
	 * Default constructor for FInventorySlot.
	 */
	FInventorySlot() {};

	/**
	 * Constructor for FInventorySlot with initialization parameters.
	 *
	 * @param NewSlot			 Slot.
	 * @param NewAsset           Primary asset ID.
	 * @param NewItemProperties  Item properties.
	 * @param NewAmount          Item amount.
	 */
	FInventorySlot(const int NewSlot, const FPrimaryAssetId& NewAsset, const FItemProperties& NewItemProperties, const int NewAmount)
	{
		Slot = NewSlot;
		Asset = NewAsset;
		ItemProperties = NewItemProperties;
		Amount = NewAmount;
	};

	/**
	 * Index representing the positions of item in the inventory.
	 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Inventory System")
	int Slot = INDEX_NONE;

	/**
	 * Primary asset ID representing the item in the inventory.
	 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Inventory System")
	FPrimaryAssetId Asset;

	/**
	 * Item properties associated with the item in the inventory.
	 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Inventory System", DisplayName = "Dynamic Item Properties")
	FItemProperties ItemProperties;

	/**
	 * Item amount corresponding to the item in the inventory.
	 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Inventory System")
	int Amount = 0;
};

#undef LOCTEXT_NAMESPACE