// © 2024 Daniel Münch. All Rights Reserved

#pragma once

#include <atomic>
#include "InventorySystemSettings.generated.h"

#define LOCTEXT_NAMESPACE "InventorySystem"

/**
 * @class UInventorySystemSettings
 * @brief Configuration settings for the Inventory System, defining global parameters like maximum stack sizes and inventory dimensions.
 *
 * This class provides a centralized location for settings that govern the behavior of the inventory system, including limits on stack sizes
 * for various item types and the overall size of inventories and containers. These settings can be adjusted in the project's configuration files
 * or through the editor to tailor the inventory system to the specific needs of your game.
 *
 * General Usage:
 * - Accessible via the project settings under the Inventory System category.
 * - Can be modified to change the fundamental behavior and limitations of the inventory system across the game.
 *
 * Example Use Case:
 * - Adjusting the maximum stack size for items to limit how many of the same item a player can carry.
 */
UCLASS(config = InventorySystem)
class INVENTORYSYSTEM_API UInventorySystemSettings : public UObject
{
	GENERATED_BODY()

public:
	explicit UInventorySystemSettings(const FObjectInitializer& OBJ);

#if WITH_EDITORONLY_DATA
	/**
	 * Internal use only. Boolean indicating whether the component is in a editor play context.
	 */
	UPROPERTY()
	unsigned bHasBegunPlayEditor = 0;
#endif

	/**
	 * The global maximum stack size for items.
	 */
	UPROPERTY(Config, EditDefaultsOnly, Category = "Inventory", meta = (ClampMin="2", EditCondition = "bHasBegunPlayEditor == 0"))
	int MaxInventoryStackSize;

	/**
	 * The global maximum size of inventories.
	 */
	UPROPERTY(Config, EditDefaultsOnly, Category = "Inventory", meta = (ClampMin="1", EditCondition = "bHasBegunPlayEditor == 0"))
	int MaxInventorySize;

	/**
	 * The global maximum stack size for equipment items.
	 */
	UPROPERTY(Config, EditDefaultsOnly, Category = "Inventory", meta = (ClampMin="2", EditCondition = "bHasBegunPlayEditor == 0"))
	int MaxItemEquipmentStackSize;

	/**
	 * The global maximum stack size for items.
	 */
	UPROPERTY(Config, EditDefaultsOnly, Category = "Container", meta = (ClampMin="2", EditCondition = "bHasBegunPlayEditor == 0"))
	int MaxItemContainerStackSize;

	/**
	* The global maximum size of item containers.
	*/
	UPROPERTY(Config, EditDefaultsOnly, Category = "Container", meta = (ClampMin="1", EditCondition = "bHasBegunPlayEditor == 0"))
	int MaxItemContainerSize;
	
	/**
	 * The global maximum stack size for item drops.
	 */
	UPROPERTY(Config, EditDefaultsOnly, Category = "Item Drop", meta = (ClampMin="2", EditCondition = "bHasBegunPlayEditor == 0"))
	int MaxItemDropStackSize;

#if WITH_EDITORONLY_DATA
	/**
	 * Propagate changes to components and actors.
	 *
	 * @param PropertyChangedEvent 
	 */
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
};

#undef LOCTEXT_NAMESPACE