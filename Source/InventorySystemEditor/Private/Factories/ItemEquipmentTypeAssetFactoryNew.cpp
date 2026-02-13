// © 2024 Daniel Münch. All Rights Reserved

#include "ItemEquipmentTypeAssetFactoryNew.h"

#include "ItemEquipmentTypeDataAsset.h"
#include "Kismet2/KismetEditorUtilities.h"

#define LOCTEXT_NAMESPACE "InventorySystem"

/* FactoryNew structors
 *****************************************************************************/

UItemEquipmentTypeAssetFactoryNew::UItemEquipmentTypeAssetFactoryNew(const FObjectInitializer& ObjectInitializer): Super(ObjectInitializer)
{
	SupportedClass = UItemEquipmentTypeDataAsset::StaticClass();
	bCreateNew = true;
	bEditAfterNew = false;
}


/* UFactory overrides
 *****************************************************************************/

UObject* UItemEquipmentTypeAssetFactoryNew::FactoryCreateNew(UClass* InClass, UObject* InParent, const FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
	return FKismetEditorUtilities::CreateBlueprint( UItemEquipmentTypeDataAsset::StaticClass(), InParent, InName, BPTYPE_Normal, UBlueprint::StaticClass(), UBlueprintGeneratedClass::StaticClass());;
}

FText UItemEquipmentTypeAssetFactoryNew::GetDisplayName() const
{
	return LOCTEXT("ItemEquipmentTypeBlueprintCreateName", "Create Item Equipment Type Base Class");
}


bool UItemEquipmentTypeAssetFactoryNew::ShouldShowInNewMenu() const
{
	return true;
}

#undef LOCTEXT_NAMESPACE