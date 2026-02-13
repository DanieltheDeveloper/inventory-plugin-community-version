// © 2024 Daniel Münch. All Rights Reserved

#include "UI/UI_EquipmentItem.h"

#include "InventorySystemComponent.h"
#include "Engine/GameInstance.h"
#include "GameFramework/PlayerState.h"

#define LOCTEXT_NAMESPACE "UUI_EquipmentItem"

UUI_EquipmentItem::UUI_EquipmentItem()
{
	if (IsValid(UWidget::GetWorld()))
	{
		UWidget::GetWorld()->GetTimerManager().SetTimer(InitEquipmentSlotTimerHandle, this, &UUI_EquipmentItem::InitEquipmentSlot, 0.25f, true, 0.01f);
	}
}

void UUI_EquipmentItem::FinishDestroy()
{
	if (UInventorySystemComponent* Component = GetUsedInventorySystemComponent(); IsValid(Component))
	{
		Component->ChangedEquipmentSlotsDelegate.RemoveDynamic(this, &UUI_EquipmentItem::CallChangeDelegate);
	}

	Super::FinishDestroy();
}

void UUI_EquipmentItem::SetCustomInventorySystemComponent(UInventorySystemComponent* const& InventorySystemComponent)
{
	if (UInventorySystemComponent* Component = GetUsedInventorySystemComponent(); IsValid(Component))
	{
		Component->ChangedEquipmentSlotsDelegate.RemoveDynamic(this, &UUI_EquipmentItem::CallChangeDelegate);
	}
	CustomInventorySystemComponent = InventorySystemComponent;
	InitEquipmentSlot();
}

FEquipmentSlot UUI_EquipmentItem::GetEquipmentSlotData()
{
	if (const UInventorySystemComponent* Component = GetUsedInventorySystemComponent(); IsValid(Component))
	{
		return Component->GetEquipmentSlot(EquipmentSlot);
	}

	return FEquipmentSlot{};
}

void UUI_EquipmentItem::InitEquipmentSlot()
{
	if (UInventorySystemComponent* Component = GetUsedInventorySystemComponent(); IsValid(Component))
	{
		Component->ChangedEquipmentSlotsDelegate.AddUniqueDynamic(this, &UUI_EquipmentItem::CallChangeDelegate);
		UWidget::GetWorld()->GetTimerManager().ClearTimer(InitEquipmentSlotTimerHandle);
		EquipmentSlotChangedDelegate.Broadcast(false);
	}
}

void UUI_EquipmentItem::CallChangeDelegate(const TArray<int>& EquipmentSlots)
{
	if (EquipmentSlots.Contains(EquipmentSlot))
	{
		if (UInventorySystemComponent* Component = GetUsedInventorySystemComponent(); IsValid(Component) && Component->GetEquipmentSlot(EquipmentSlot).Slot == INDEX_NONE)
		{
			EquipmentSlotChangedDelegate.Broadcast(true);
			return;
		}
		EquipmentSlotChangedDelegate.Broadcast(false);
	}
}

UInventorySystemComponent* UUI_EquipmentItem::GetUsedInventorySystemComponent()
{
	if (IsValid(CustomInventorySystemComponent))
	{
		return CustomInventorySystemComponent;
	}

	if (!IsValid(PlayerStateInventorySystemComponent) && GetGameInstance() != nullptr && GetGameInstance()->IsValidLowLevel()) {
		if (const APlayerController* PlayerController = GetOwningPlayer(); IsValid(PlayerController)) {
			if (const APlayerState* PlayerState = GetOwningPlayer()->GetPlayerState<APlayerState>(); IsValid(PlayerState))
			{
				if (UInventorySystemComponent* InventorySystemComponent = Cast<UInventorySystemComponent>(PlayerState->GetComponentByClass(UInventorySystemComponent::StaticClass())); IsValid(InventorySystemComponent))
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

TSharedRef<SWidget> UUI_EquipmentItem::RebuildWidget()
{
	auto OverlayWidget = Super::RebuildWidget();

	return OverlayWidget;
}

#undef LOCTEXT_NAMESPACE