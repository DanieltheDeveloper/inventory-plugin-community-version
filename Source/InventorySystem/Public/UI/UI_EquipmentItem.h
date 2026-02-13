// © 2024 Daniel Münch. All Rights Reserved

#pragma once

#include "EquipmentSlots.h"
#include "Components/Overlay.h"
#include "TimerManager.h"
#include "UI_EquipmentItem.generated.h"

#define LOCTEXT_NAMESPACE "InventorySystem"

class UInventorySystemComponent;

/**
 * Delegate for handling equipment slot changes. Can be used to bind custom functionality when the equipment slot is updated.
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FEquipmentSlotChangedDelegate, bool, bRemoved);

/**
 * @class UUI_EquipmentItem
 * @brief Use this widget to create UI elements containing the equipment data and equipment slot information.
 *
 * This class is responsible for representing an individual equipment item within the game's UI.
 * It extends UOverlay to allow for complex UI compositions. The widget is designed to be flexible,
 * supporting custom InventorySystemComponents and dynamic updates to the displayed equipment slot.
 *
 * Usage:
 * - Use this widget to create a visual representation of an equipment slot within a custom equipment panel.
 * - Bind the EquipmentSlotChangedDelegate to respond to changes in the equipment slot.
 *
 * Example Usage:
 * @code
 * UUI_EquipmentItem* EquipmentItemWidget = CreateWidget<UUI_EquipmentItem>(GetWorld(), EquipmentItemWidgetClass);
 * EquipmentItemWidget->SetCustomInventorySystemComponent(YourInventoryComponent);
 * @endcode
 */
UCLASS(Category = "Inventory System")
class INVENTORYSYSTEM_API UUI_EquipmentItem : public UOverlay
{
    GENERATED_BODY()

protected:
    UUI_EquipmentItem();

    /**
    * Handles the initialization timing for setting up the equipment slot.
    */
    FTimerHandle InitEquipmentSlotTimerHandle;

    /**
    * Handle removal of delegates here.
    */
    virtual void FinishDestroy() override;

    /**
    * Initializes the equipment slot, setting up necessary references and data.
    */
    UFUNCTION()
    void InitEquipmentSlot();

    /**
     * Calls the change delegate with a list of changed equipment slots. Primarily used internally to handle updates.
     *
     * @param EquipmentSlots 
     */
    UFUNCTION()
    void CallChangeDelegate(const TArray<int>& EquipmentSlots);

    /**
     * A custom inventory system component, set if not using the one from the player state.
     */
    UPROPERTY()
    UInventorySystemComponent* CustomInventorySystemComponent = nullptr;

    /**
     * Automatically detected inventory system component from the player state.
     */
    UPROPERTY()
    UInventorySystemComponent* PlayerStateInventorySystemComponent = nullptr;

    /**
     * Rebuilds the widget, allowing for customization of the UI components based on the equipment slot data.
     *
     * @return 
     */
    virtual TSharedRef<SWidget> RebuildWidget() override;

public:
    /**
     * Delegate for adding custom functionality after an equipment slot change. Bind to this to respond to slot updates.
     */
    UPROPERTY(BlueprintAssignable, BlueprintCallable)
    FEquipmentSlotChangedDelegate EquipmentSlotChangedDelegate;

    /**
     * Sets a custom InventorySystemComponent. This is useful for UI components that are not automatically associated with
     * a player state's inventory system. It must be set before interacting with the equipment slot.
     *
     * @param InventorySystemComponent The inventory system component to be used with this equipment item.
     */
    UFUNCTION(BlueprintCallable, Category = "Inventory System")
    void SetCustomInventorySystemComponent(UInventorySystemComponent* const& InventorySystemComponent);

    /**
     * Retrieves the InventorySystemComponent associated with this equipment item, whether it's a custom component or
     * one found in the player state.
     *
     * @return The InventorySystemComponent used by this equipment item.
     */
    UFUNCTION(BlueprintCallable, Category = "Inventory System")
    UInventorySystemComponent* GetUsedInventorySystemComponent();

    /**
     * Sets the equipment slot to be displayed by this widget. This should be set before the widget is initialized to
     * ensure the correct slot is displayed. It specifies which slot from the InventorySystemComponent's EquipmentSlots array
     * to show.
     */
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Inventory System")
    int EquipmentSlot = INDEX_NONE;

    /**
     * Retrieves the data for the currently set equipment slot and returns it.
     *
     * @return The data structure representing the current equipment slot's state.
     */
    UFUNCTION(BlueprintCallable, Category = "Inventory System")
    FEquipmentSlot GetEquipmentSlotData();
};

#undef LOCTEXT_NAMESPACE
