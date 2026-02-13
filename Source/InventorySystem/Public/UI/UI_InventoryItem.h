// © 2024 Daniel Münch. All Rights Reserved

#pragma once

#include "InventorySlots.h"
#include "InventorySystemComponent.h"
#include "Components/Overlay.h"
#include "TimerManager.h"
#include "UI_InventoryItem.generated.h"

#define LOCTEXT_NAMESPACE "InventorySystem"

class UItemContainerComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FInventorySlotChangedDelegate, bool, bRemoved);

/**
 * @class UUI_InventoryItem
 * @brief Use this widget to create UI elements containing the item data and inventory slot information.
 *
 * This class is responsible for representing an individual inventory item within the game's UI.
 * It extends UOverlay to allow for complex UI compositions. The widget is designed to be flexible,
 * supporting custom InventorySystemComponents and ItemContainerComponents for dynamic updates to the displayed inventory slot.
 *
 * Usage:
 * - Use this widget to create a visual representation of an item slot within a custom inventory or item container panel.
 * - Bind the InventorySlotChangedDelegate to respond to changes in the item slot.
 *
 * Example Usage:
 * @code
 * UUI_InventoryItem* InventoryItemWidget = CreateWidget<UUI_InventoryItem>(GetWorld(), InventoryItemWidgetClass);
 * InventoryItemWidget->SetCustomItemContainerComponent(YourComponentClass);
 * @endcode
 */

UCLASS(Category = "Inventory System")
class INVENTORYSYSTEM_API UUI_InventoryItem : public UOverlay
{
    GENERATED_BODY()

protected:
    UUI_InventoryItem();

    /**
     * Handle for delayed initialization of inventory slot.
     */
    FTimerHandle InitInventorySlotTimerHandle;

    /**
     * Handle removal of delegates here.
     */
    virtual void FinishDestroy() override;

    

    /**
    * Initializes the inventory slot, typically called after a delay to ensure all components are loaded.
    */
    UFUNCTION()
    void InitInventorySlot();

    /**
     * Triggers the inventory slot changed delegate with the specified inventory slots.
     *
     * @param InventorySlots 
     */
    UFUNCTION()
    void CallChangeDelegate(const TArray<int>& InventorySlots);

    /**
     * Custom item container component, if set.
     */
    UPROPERTY()
    UItemContainerComponent* CustomItemContainerComponent = nullptr;

    /**
     * Automatically detected player state inventory system component.
     */
    UPROPERTY()
    UInventorySystemComponent* PlayerStateInventorySystemComponent = nullptr;

    virtual TSharedRef<SWidget> RebuildWidget() override;
public:
    /**
     * Delegate for adding functionality after a slot was changed.
     */
    UPROPERTY(BlueprintAssignable, BlueprintCallable)
    FInventorySlotChangedDelegate InventorySlotChangedDelegate;

    /**
     * Sets a custom ItemContainerComponent. This is typically used when the inventory system is not located
     * within the PlayerState, allowing for greater flexibility in how inventory data is managed and displayed.
     * @param InventorySystemComponent The custom ItemContainerComponent to use.
     */
    UFUNCTION(BlueprintCallable, Category = "Inventory System")
    void SetCustomItemContainerComponent(UItemContainerComponent* const& ItemContainerComponent);

    /**
     * Retrieves the ItemContainerComponent currently being used by this inventory item widget. Cast to InventorySystemComponent when needed.
     * @return The active item container component.
     */
    UFUNCTION(BlueprintCallable, Category = "Inventory System")
    UItemContainerComponent* GetUsedItemContainerComponent();

    /**
     * Sets the inventory slot to be displayed by this widget. This should be set before interacting with the widget.
     */
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Inventory System")
    int InventorySlot = INDEX_NONE;

    /**
     * Retrieves the data for the currently set inventory slot and returns it.
     * @return The data structure representing the current inventory slot's state.
     */
    UFUNCTION(BlueprintCallable, Category = "Inventory System")
    FInventorySlot GetInventorySlotData();
};

#undef LOCTEXT_NAMESPACE