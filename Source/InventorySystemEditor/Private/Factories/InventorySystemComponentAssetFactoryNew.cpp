// © 2024 Daniel Münch. All Rights Reserved

#include "InventorySystemComponentAssetFactoryNew.h"

#include "InventorySystemComponent.h"
#include "Kismet2/KismetEditorUtilities.h"

/* FactoryNew structors
 *****************************************************************************/

UInventorySystemComponentAssetFactoryNew::UInventorySystemComponentAssetFactoryNew(const FObjectInitializer& ObjectInitializer): Super(ObjectInitializer)
{
	SupportedClass = UInventorySystemComponent::StaticClass();
	bCreateNew = true;
	bEditAfterNew = false;
}


/* UFactory overrides
 *****************************************************************************/

UObject* UInventorySystemComponentAssetFactoryNew::FactoryCreateNew(UClass* InClass, UObject* InParent, const FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
	UBlueprint* NewBP = FKismetEditorUtilities::CreateBlueprint( UInventorySystemComponent::StaticClass(), InParent, InName, BPTYPE_Normal, UBlueprint::StaticClass(), UBlueprintGeneratedClass::StaticClass());
	return NewBP;
}


bool UInventorySystemComponentAssetFactoryNew::ShouldShowInNewMenu() const
{
	return true;
}