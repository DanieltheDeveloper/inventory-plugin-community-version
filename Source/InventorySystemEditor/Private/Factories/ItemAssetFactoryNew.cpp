// © 2024 Daniel Münch. All Rights Reserved

#include "ItemAssetFactoryNew.h"

#include "ItemDataAsset.h"
#include "Kismet2/KismetEditorUtilities.h"

#define LOCTEXT_NAMESPACE "InventorySystem"

/* FactoryNew structors
 *****************************************************************************/

UItemAssetFactoryNew::UItemAssetFactoryNew(const FObjectInitializer& ObjectInitializer): Super(ObjectInitializer)
{
	SupportedClass = UItemDataAsset::StaticClass();
	bCreateNew = true;
	bEditAfterNew = false;
}


/* UFactory overrides
 *****************************************************************************/

UObject* UItemAssetFactoryNew::FactoryCreateNew(UClass* InClass, UObject* InParent, const FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
	UBlueprint* NewBP = FKismetEditorUtilities::CreateBlueprint(UItemDataAsset::StaticClass(), InParent, InName, BPTYPE_Normal, UBlueprint::StaticClass(), UBlueprintGeneratedClass::StaticClass());
	return NewBP;
}

FText UItemAssetFactoryNew::GetDisplayName() const
{
	return LOCTEXT("ItemBlueprintCreateName", "Create Item Base Class");
}


bool UItemAssetFactoryNew::ShouldShowInNewMenu() const
{
	return true;
}

#undef LOCTEXT_NAMESPACE