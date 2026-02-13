// © 2024 Daniel Münch. All Rights Reserved

#include "ItemEquipmentAssetFactoryNew.h"

#include "ItemEquipmentDataAsset.h"
#include "Kismet2/KismetEditorUtilities.h"

#define LOCTEXT_NAMESPACE "InventorySystem"

/* FactoryNew structors
 *****************************************************************************/

UItemEquipmentAssetFactoryNew::UItemEquipmentAssetFactoryNew(const FObjectInitializer& ObjectInitializer): Super(ObjectInitializer)
{
	SupportedClass = UItemEquipmentDataAsset::StaticClass();
	bCreateNew = true;
	bEditAfterNew = false;
}


/* UFactory overrides
 *****************************************************************************/

UObject* UItemEquipmentAssetFactoryNew::FactoryCreateNew(UClass* InClass, UObject* InParent, const FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
	return FKismetEditorUtilities::CreateBlueprint( UItemEquipmentDataAsset::StaticClass(), InParent, InName, BPTYPE_Normal, UBlueprint::StaticClass(), UBlueprintGeneratedClass::StaticClass());
}

FText UItemEquipmentAssetFactoryNew::GetDisplayName() const
{
	return LOCTEXT("ItemEquipmentBlueprintCreateName", "Create Item Equipment Base Class");
}


bool UItemEquipmentAssetFactoryNew::ShouldShowInNewMenu() const
{
	return true;
}

#undef LOCTEXT_NAMESPACE