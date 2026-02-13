// © 2024 Daniel Münch. All Rights Reserved

#include "ItemContainerComponentAssetFactoryNew.h"

#include "ItemContainerComponent.h"
#include "Kismet2/KismetEditorUtilities.h"

/* FactoryNew structors
 *****************************************************************************/

UItemContainerComponentAssetFactoryNew::UItemContainerComponentAssetFactoryNew(const FObjectInitializer& ObjectInitializer): Super(ObjectInitializer)
{
	SupportedClass = UItemContainerComponent::StaticClass();
	bCreateNew = true;
	bEditAfterNew = false;
}


/* UFactory overrides
 *****************************************************************************/

UObject* UItemContainerComponentAssetFactoryNew::FactoryCreateNew(UClass* InClass, UObject* InParent, const FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
	UBlueprint* NewBP = FKismetEditorUtilities::CreateBlueprint( UItemContainerComponent::StaticClass(), InParent, InName, BPTYPE_Normal, UBlueprint::StaticClass(), UBlueprintGeneratedClass::StaticClass());
	return NewBP;
}


bool UItemContainerComponentAssetFactoryNew::ShouldShowInNewMenu() const
{
	return true;
}