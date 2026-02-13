// © 2024 Daniel Münch. All Rights Reserved

#pragma once

#include "Factories/Factory.h"
#include "ItemEquipmentTypeAssetFactoryNew.generated.h"

/**
 * UItemEquipmentTypeAssetFactoryNew is a class that defines a factory for creating Item Equipment Type Assets.
 * 
 * This class provides the functionality to create new Item Equipment Type Assets when using the asset creation menu in the editor.
 */
UCLASS(hidecategories = Object)
class INVENTORYSYSTEMEDITOR_API UItemEquipmentTypeAssetFactoryNew : public UFactory
{
	GENERATED_UCLASS_BODY()
	
	/**
	 * Constructor for UItemEquipmentTypeAssetFactoryNew.
	 */
	virtual UObject* FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn) override;

	/**
	 * Name that is displayed in the asset action menu
	 *
	 * @return 
	 */
	virtual FText GetDisplayName() const override;

	/**
	 * Determines whether this factory should be shown in the "New" menu.
	 * 
	 * @return True if this factory should be shown in the "New" menu; otherwise, false.
	 */
	virtual bool ShouldShowInNewMenu() const override;
};