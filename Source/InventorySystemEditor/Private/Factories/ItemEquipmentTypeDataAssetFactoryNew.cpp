// © 2024 Daniel Münch. All Rights Reserved

#include "ItemEquipmentTypeDataAssetFactoryNew.h"

#include "Kismet2/SClassPickerDialog.h" 
#include "ItemEquipmentTypeDataAsset.h"

#define LOCTEXT_NAMESPACE "InventorySystem"

UItemEquipmentTypeDataAssetFactoryNew::UItemEquipmentTypeDataAssetFactoryNew(const FObjectInitializer& ObjectInitializer): Super(ObjectInitializer)
{
	SupportedClass = UItemEquipmentTypeDataAsset::StaticClass();
	bCreateNew = true;
	bEditAfterNew = false;
}

FText UItemEquipmentTypeDataAssetFactoryNew::GetDisplayName() const
{
	return LOCTEXT("ItemEquipmentTypeClassPickerName", "Create Item Equipment Type Data Asset");
}

UObject* UItemEquipmentTypeDataAssetFactoryNew::FactoryCreateNew(UClass* InClass, UObject* InParent, const FName InName, const EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
	if (ItemClass != nullptr)
	{
		UItemEquipmentTypeDataAsset* NewDataObject = NewObject<UItemEquipmentTypeDataAsset>(InParent, ItemClass, InName, Flags | RF_Transactional);
		NewDataObject->IsDataOnly = true;
		return NewDataObject;
	}
	check(InClass->IsChildOf(UItemEquipmentTypeDataAsset::StaticClass()));
	return NewObject<UItemEquipmentTypeDataAsset>(InParent, InClass, InName, Flags | RF_Transactional);
}

bool UItemEquipmentTypeDataAssetFactoryNew::ConfigureProperties()
{
	ItemClass = nullptr;

	FClassViewerModule& ClassViewerModule = FModuleManager::LoadModuleChecked<FClassViewerModule>("ClassViewer");
	FClassViewerInitializationOptions Options;
	Options.Mode = EClassViewerMode::ClassPicker;

	const TSharedRef<FItemEquipmentTypeFilterViewer> Filter = MakeShareable<FItemEquipmentTypeFilterViewer>(new FItemEquipmentTypeFilterViewer);
	Options.ClassFilters.Add(Filter);

	Filter->DisallowedClassFlags = CLASS_Abstract | CLASS_Deprecated;
	Filter->AllowedChildrenOfClasses.Add(UItemEquipmentTypeDataAsset::StaticClass());

	const FText TitleText = LOCTEXT("ClassPickerTitleItemEquipmentTypeAsset", "Pick Item Equipment Type Data Asset Class");
	UClass* ChosenClass = nullptr;
	const bool bPressedOk = SClassPickerDialog::PickClass(TitleText, Options, ChosenClass, UItemEquipmentTypeDataAsset::StaticClass());

	if (bPressedOk)
	{
		ItemClass = ChosenClass;
	}

	return bPressedOk;
}


bool UItemEquipmentTypeDataAssetFactoryNew::ShouldShowInNewMenu() const
{
	return true;
}

#undef LOCTEXT_NAMESPACE