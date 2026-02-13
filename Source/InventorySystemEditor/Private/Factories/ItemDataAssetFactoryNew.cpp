// © 2024 Daniel Münch. All Rights Reserved

#include "ItemDataAssetFactoryNew.h"

#include "InventorySystemEditor.h"
#include "Kismet2/SClassPickerDialog.h" 
#include "ItemDataAsset.h"
#include "UObject/SavePackage.h"

#define LOCTEXT_NAMESPACE "InventorySystem"

UItemDataAssetFactoryNew::UItemDataAssetFactoryNew(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SupportedClass = UItemDataAsset::StaticClass();
	bCreateNew = true;
	bEditAfterNew = false;
}


FText UItemDataAssetFactoryNew::GetDisplayName() const
{
	return LOCTEXT("ItemClassPickerName", "Create Item Data Asset");
}

UObject* UItemDataAssetFactoryNew::FactoryCreateNew(UClass* InClass, UObject* InParent, const FName InName, const EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
	if (ItemClass != nullptr)
	{
		UItemDataAsset* NewDataObject = NewObject<UItemDataAsset>(InParent, ItemClass, InName, Flags | RF_Transactional);
		NewDataObject->IsDataOnly = true;
		return NewDataObject;
	}
	check(InClass->IsChildOf(UItemDataAsset::StaticClass()));
	return NewObject<UItemDataAsset>(InParent, InClass, InName, Flags);
}

bool UItemDataAssetFactoryNew::ConfigureProperties()
{
	ItemClass = nullptr;

	FClassViewerModule& ClassViewerModule = FModuleManager::LoadModuleChecked<FClassViewerModule>("ClassViewer");
	FClassViewerInitializationOptions Options;
	Options.Mode = EClassViewerMode::ClassPicker;

	const TSharedRef<FItemFilterViewer> Filter = MakeShareable<FItemFilterViewer>(new FItemFilterViewer);
	Options.ClassFilters.Add(Filter);

	Filter->DisallowedClassFlags = CLASS_Abstract | CLASS_Deprecated;
	Filter->AllowedChildrenOfClasses.Add(UItemDataAsset::StaticClass());

	const FText TitleText = LOCTEXT("ClassPickerTitleItemDataAsset", "Pick Item Data Asset Class");
	UClass* ChosenClass = nullptr;
	const bool bPressedOk = SClassPickerDialog::PickClass(TitleText, Options, ChosenClass, UItemDataAsset::StaticClass());

	if (bPressedOk)
	{
		ItemClass = ChosenClass;
	}

	return bPressedOk;
}


bool UItemDataAssetFactoryNew::ShouldShowInNewMenu() const
{
	return true;
}

#undef LOCTEXT_NAMESPACE