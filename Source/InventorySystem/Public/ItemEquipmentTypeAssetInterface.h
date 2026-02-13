// © 2024 Daniel Münch. All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "ItemEquipmentTypeAssetInterface.generated.h"

#define LOCTEXT_NAMESPACE "InventorySystem"

/**
 * @class UItemEquipmentTypeAssetInterface
 * @brief Defines an interface for item equipment type assets, allowing for dynamic interaction with different types of equipment.
 *
 * This interface is used to categorize and manage different equipment types within the game, providing a standardized method
 * for retrieving the name and potentially other properties specific to each equipment type. Implementing this interface in an
 * asset allows for easy identification and sorting of equipment based on its type, facilitating interactions such as equipping,
 * unequipping, and inventory management.
 *
 * General Usage:
 * - Implement this interface in any asset that represents a specific type of equipment.
 * - Override the GetName function to return the equipment type's name.
 *
 * Example Use Case:
 * @code
 * class UMyHelmetAsset : public UObject, public IItemEquipmentTypeAssetInterface
 * {
 *     virtual FText GetName() override
 *     {
 *         return LOCTEXT("MyHelmetName", "Helmet");
 *     }
 * };
 * @endcode
 */
UINTERFACE(BlueprintType, Category = "Inventory System")
class INVENTORYSYSTEM_API UItemEquipmentTypeAssetInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * Interface for Item Equipment Type Assets.
 */
class INVENTORYSYSTEM_API IItemEquipmentTypeAssetInterface
{
	GENERATED_BODY()

public:
	/**
	 * Get the name of the equipment type.
	 *
	 * @return The name of the equipment type as a localized text.
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Type")
	FText GetName();
};

#undef LOCTEXT_NAMESPACE