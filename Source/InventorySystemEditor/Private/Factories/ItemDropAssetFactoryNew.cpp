// © 2024 Daniel Münch. All Rights Reserved

#include "ItemDropAssetFactoryNew.h"

#include "ItemDrop.h"
#include "Kismet2/KismetEditorUtilities.h"

/* FactoryNew structors
 *****************************************************************************/

UItemDropAssetFactoryNew::UItemDropAssetFactoryNew(const FObjectInitializer& ObjectInitializer): Super(ObjectInitializer)
{
	SupportedClass = AItemDrop::StaticClass();
	bCreateNew = true;
	bEditAfterNew = false;
}


/* UFactory overrides
 *****************************************************************************/

UObject* UItemDropAssetFactoryNew::FactoryCreateNew(UClass* InClass, UObject* InParent, const FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
	return FKismetEditorUtilities::CreateBlueprint( AItemDrop::StaticClass(), InParent, InName, BPTYPE_Normal, UBlueprint::StaticClass(), UBlueprintGeneratedClass::StaticClass());
}


bool UItemDropAssetFactoryNew::ShouldShowInNewMenu() const
{
	return true;
}