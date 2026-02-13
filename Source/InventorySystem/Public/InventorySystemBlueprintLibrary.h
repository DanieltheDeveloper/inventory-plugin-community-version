// © 2024 Daniel Münch. All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "ItemEquipmentDataAsset.h"
#include "ItemProperties.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "InventorySystemBlueprintLibrary.generated.h"

#define LOCTEXT_NAMESPACE "InventorySystem"

/**
 * @class UInventorySystemBlueprintLibrary
 * @brief Provides a library of static functions for inventory system operations, such as item property comparisons and equipment checks.
 *
 * This Blueprint function library includes utilities for comparing item properties, validating item types, and casting item data assets.
 * It's designed to facilitate common inventory-related operations within Blueprints, providing a standardized way to interact with the inventory system's data structures.
 *
 * General Usage:
 * - Call these functions directly in Blueprint graphs where inventory data needs to be manipulated or evaluated.
 * - Utilize the comparison operators to streamline inventory management logic, such as sorting or validating items.
 *
 * Example Use Case:
 * - Comparing two item properties for equality in a Blueprint to determine if they are the same item.
 * - Checking if an item can be equipped in a specific equipment slot.
 */
UCLASS(Category = "Inventory System")
class INVENTORYSYSTEM_API UInventorySystemBlueprintLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

	// START ----ItemProperty Operators----

	/**
	 * Compares two item properties for equality.
	 *
	 * @param First  The first item property.
	 * @param Other  The second item property.
	 * @return       True if the item properties are equal, false otherwise.
	 */
	UFUNCTION(BlueprintPure, meta = (DisplayName = "ItemProperty == ItemProperty", CompactNodeTitle = "==", Keywords = "== equal", CommutativeAssociativeBinaryOperator = "true"), Category = "Inventory System")
	static bool EqualEqual_ItemProperty(const FItemProperty& First, const FItemProperty& Other);

	/**
	 * Compares two item properties for inequality.
	 *
	 * @param First  The first item property.
	 * @param Other  The second item property.
	 * @return       True if the item properties are not equal, false otherwise.
	 */
	UFUNCTION(BlueprintPure, meta = (DisplayName = "ItemProperty != ItemProperty", CompactNodeTitle = "!=", Keywords = "!= unequal", CommutativeAssociativeBinaryOperator = "true"), Category = "Inventory System")
	static bool Unequal_ItemProperty(const FItemProperty& First, const FItemProperty& Other);

	/**
	 * Checks if one item property is greater than or equal to another.
	 *
	 * @param First  The first item property.
	 * @param Other  The second item property.
	 * @return       True if the first item property is greater than or equal to the second, false otherwise.
	 */
	UFUNCTION(BlueprintPure, meta = (DisplayName = "ItemProperty >= ItemProperty", CompactNodeTitle = ">=", Keywords = ">= greater", CommutativeAssociativeBinaryOperator = "true"), Category = "Inventory System")
	static bool GreaterEqual_ItemProperty(const FItemProperty& First, const FItemProperty& Other);

	/**
	 * Checks if one item property is less than or equal to another.
	 *
	 * @param First  The first item property.
	 * @param Other  The second item property.
	 * @return       True if the first item property is less than or equal to the second, false otherwise.
	 */
	UFUNCTION(BlueprintPure, meta = (DisplayName = "ItemProperty <= ItemProperty", CompactNodeTitle = "<=", Keywords = "<= less", CommutativeAssociativeBinaryOperator = "true"), Category = "Inventory System")
	static bool LessEqual_ItemProperty(const FItemProperty& First, const FItemProperty& Other);

	/**
	 * Checks if one item property is greater than another.
	 *
	 * @param First  The first item property.
	 * @param Other  The second item property.
	 * @return       True if the first item property is greater than the second, false otherwise.
	 */
	UFUNCTION(BlueprintPure, meta = (DisplayName = "ItemProperty > ItemProperty", CompactNodeTitle = ">", Keywords = "> greater", CommutativeAssociativeBinaryOperator = "true"), Category = "Inventory System")
	static bool Greater_ItemProperty(const FItemProperty& First, const FItemProperty& Other);

	/**
	 * Checks if one item property is less than another.
	 *
	 * @param First  The first item property.
	 * @param Other  The second item property.
	 * @return       True if the first item property is less than the second, false otherwise.
	 */
	UFUNCTION(BlueprintPure, meta = (DisplayName = "ItemProperty < ItemProperty", CompactNodeTitle = "<", Keywords = "< less", CommutativeAssociativeBinaryOperator = "true"), Category = "Inventory System")
	static bool Less_ItemProperty(const FItemProperty& First, const FItemProperty& Other);

	// END ----ItemProperty Operators----

	// START ----Equipment utils----

	/**
	 * Checks if an ItemDataAsset can be cast to an ItemEquipmentDataAsset.
	 *
	 * @param ItemDataAsset          The item data asset to check.
	 * @param ItemEquipmentDataAsset If successful, this will contain the casted ItemEquipmentDataAsset.
	 * @return                       True if the cast is successful, false otherwise.
	 */
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "ItemDataAsset == ItemEquipmentDataAsset", CompactNodeTitle = "==", Keywords = "== equal ItemDataAsset ItemEquipmentDataAsset"), Category = "Inventory System")
	static bool ItemDataAsset_EqualEqual_ItemEquipmentDataAsset(UItemDataAsset* const& ItemDataAsset, UItemEquipmentDataAsset*& ItemEquipmentDataAsset);

	// END ----Equipment utils----
};

#undef LOCTEXT_NAMESPACE