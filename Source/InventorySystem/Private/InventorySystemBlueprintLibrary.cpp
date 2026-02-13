// © 2024 Daniel Münch. All Rights Reserved

#include "InventorySystemBlueprintLibrary.h"

#include "GameFramework/PlayerController.h"
#include "Runtime/Core/Public/Async/ParallelFor.h"

#define LOCTEXT_NAMESPACE "InventorySystem"

// START ----ItemProperty Operators----

bool UInventorySystemBlueprintLibrary::EqualEqual_ItemProperty(const FItemProperty& First, const FItemProperty& Other)
{
	if (First == Other)
	{
		return true;
	}
	return false;
}

bool UInventorySystemBlueprintLibrary::Unequal_ItemProperty(const FItemProperty& First, const FItemProperty& Other)
{
	if (First != Other)
	{
		return true;
	}
	return false;
}

bool UInventorySystemBlueprintLibrary::GreaterEqual_ItemProperty(const FItemProperty& First, const FItemProperty& Other)
{
	if (First >= Other)
	{
		return true;
	}
	return false;
}

bool UInventorySystemBlueprintLibrary::LessEqual_ItemProperty(const FItemProperty& First, const FItemProperty& Other)
{
	if (First <= Other)
	{
		return true;
	}
	return false;
}

bool UInventorySystemBlueprintLibrary::Greater_ItemProperty(const FItemProperty& First, const FItemProperty& Other)
{
	if (First > Other)
	{
		return true;
	}
	return false;
}

bool UInventorySystemBlueprintLibrary::Less_ItemProperty(const FItemProperty& First, const FItemProperty& Other)
{
	if (First < Other)
	{
		return true;
	}
	return false;
}

// END ----ItemProperty Operators----


// START ----Equipment utils----

bool UInventorySystemBlueprintLibrary::ItemDataAsset_EqualEqual_ItemEquipmentDataAsset(UItemDataAsset* const& ItemDataAsset, UItemEquipmentDataAsset*& ItemEquipmentDataAsset)
{
	if (UItemEquipmentDataAsset* CastItemEquipmentDataAsset = Cast<UItemEquipmentDataAsset>(ItemDataAsset); IsValid(CastItemEquipmentDataAsset))
	{
		ItemEquipmentDataAsset = CastItemEquipmentDataAsset;
		return true;
	}

	return false;
}

// END ----Equipment utils----

#undef LOCTEXT_NAMESPACE