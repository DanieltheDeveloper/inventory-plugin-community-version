// © 2024 Daniel Münch. All Rights Reserved

#pragma once

#include "InventorySlots.h"
#include "EquipmentSlots.generated.h"

#define LOCTEXT_NAMESPACE "InventorySystem"

/**
 * @struct FEquipmentSlot
 * @brief Defines a slot for equipment items within the inventory system, extending inventory slot capabilities.
 *
 * This structure encapsulates an equipment slot within the inventory system, holding information about the equipment type,
 * slot index, item asset ID, properties, and quantity. It is specifically used for managing equipment items, such as weapons or armor,
 * providing a means to associate them with specific types and slots within the player's inventory.
 *
 * General Usage:
 * - Utilized within inventory management systems to differentiate between regular inventory slots and specialized equipment slots.
 * - Supports the definition of equipment types to ensure items are placed in the correct slot, enhancing gameplay and UI interactions.
 *
 * Example Use Case:
 * - Defining equipment slots in player inventory to manage where equipment items can be placed or swapped.
 */
USTRUCT(BlueprintType, Category = "Inventory System")
struct INVENTORYSYSTEM_API FEquipmentSlot : public FInventorySlot
{
	GENERATED_BODY()
	
	/**
	 * Default constructor for FEquipmentSlot.
	 */
	FEquipmentSlot() {};

	/**
	 * Constructor for FEquipmentSlot with initialization parameters.
	 *
	 * @param NewEquipmentTypes	 Equipment types.
	 * @param NewSlot			 Slot.
	 * @param NewAsset           Primary asset ID.
	 * @param NewItemProperties  Item properties.
	 * @param NewAmount          Item amount.
	 */
	FEquipmentSlot(const TArray<FPrimaryAssetId>& NewEquipmentTypes, const int NewSlot, const FPrimaryAssetId& NewAsset, const FItemProperties& NewItemProperties, const int NewAmount)
	{
		EquipmentTypes = NewEquipmentTypes;
		Slot = NewSlot;
		Asset = NewAsset;
		ItemProperties = NewItemProperties;
		Amount = NewAmount;
	};

	/**
	 * Equipment type representing the direct type of item in the inventory.
	 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Inventory System")
	TArray<FPrimaryAssetId> EquipmentTypes;
};

#undef LOCTEXT_NAMESPACE

