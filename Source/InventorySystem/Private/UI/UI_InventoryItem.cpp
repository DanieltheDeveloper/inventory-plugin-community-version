// © 2024 Daniel Münch. All Rights Reserved

#include "UI/UI_InventoryItem.h"

#include "InventorySystemComponent.h"
#include "Engine/GameInstance.h"
#include "GameFramework/PlayerState.h"

#define LOCTEXT_NAMESPACE "UUI_InventoryItem"

UUI_InventoryItem::UUI_InventoryItem()
{
	if (IsValid(UWidget::GetWorld()))
	{
		UWidget::GetWorld()->GetTimerManager().SetTimer(InitInventorySlotTimerHandle, this, &UUI_InventoryItem::InitInventorySlot, 0.25f, true, 0.01f);
	}
}

void UUI_InventoryItem::FinishDestroy()
{
	if (UItemContainerComponent* Component = GetUsedItemContainerComponent(); IsValid(Component))
	{
		Component->ChangedInventorySlotsDelegate.RemoveDynamic(this, &UUI_InventoryItem::CallChangeDelegate);
	}

	Super::FinishDestroy();
}

void UUI_InventoryItem::SetCustomItemContainerComponent(UItemContainerComponent* const& ItemContainerComponent)
{
	if (UItemContainerComponent* Component = GetUsedItemContainerComponent(); IsValid(Component))
	{
		Component->ChangedInventorySlotsDelegate.RemoveDynamic(this, &UUI_InventoryItem::CallChangeDelegate);
	}
	CustomItemContainerComponent = ItemContainerComponent;
	InitInventorySlot();
}

UItemContainerComponent* UUI_InventoryItem::GetUsedItemContainerComponent()
{
	if (IsValid(CustomItemContainerComponent))
	{
		return CustomItemContainerComponent;
	}

	if (!IsValid(PlayerStateInventorySystemComponent) && GetGameInstance() != nullptr && GetGameInstance()->IsValidLowLevel()) {
		if (const APlayerController* PlayerController = GetOwningPlayer(); IsValid(PlayerController)) {
			if (const APlayerState* PlayerState = GetOwningPlayer()->GetPlayerState<APlayerState>(); IsValid(PlayerState))
			{
				if (UItemContainerComponent* InventorySystemComponent = Cast<UItemContainerComponent>(PlayerState->GetComponentByClass(UItemContainerComponent::StaticClass())); IsValid(InventorySystemComponent))
				{
					return InventorySystemComponent;
				}
			}
		}
	}
	else
	{
		return PlayerStateInventorySystemComponent;
	}

	return nullptr;
}

FInventorySlot UUI_InventoryItem::GetInventorySlotData()
{
	if (const UItemContainerComponent* Component = GetUsedItemContainerComponent(); IsValid(Component))
	{
		return Component->GetInventorySlot(InventorySlot);
	}

	return FInventorySlot{};
}

void UUI_InventoryItem::InitInventorySlot()
{
	if (UItemContainerComponent* Component = GetUsedItemContainerComponent(); IsValid(Component))
	{
		Component->ChangedInventorySlotsDelegate.AddUniqueDynamic(this, &UUI_InventoryItem::CallChangeDelegate);
		UWidget::GetWorld()->GetTimerManager().ClearTimer(InitInventorySlotTimerHandle);
		InventorySlotChangedDelegate.Broadcast(false);
	}
}

void UUI_InventoryItem::CallChangeDelegate(const TArray<int>& InventorySlots)
{
	if (InventorySlots.Contains(InventorySlot))
	{
		if (UItemContainerComponent* Component = GetUsedItemContainerComponent(); IsValid(Component) && Component->GetInventorySlot(InventorySlot).Slot == INDEX_NONE)
		{
			InventorySlotChangedDelegate.Broadcast(true);
			return;
		}
		InventorySlotChangedDelegate.Broadcast(false);
	}
}

TSharedRef<SWidget> UUI_InventoryItem::RebuildWidget()
{
	auto OverlayWidget = Super::RebuildWidget();
	return OverlayWidget;
}

#undef LOCTEXT_NAMESPACE