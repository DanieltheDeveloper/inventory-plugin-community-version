// © 2024 Daniel Münch. All Rights Reserved

#pragma once

#include "Factories/Factory.h"
#include "InventorySystemComponentAssetFactoryNew.generated.h"

/**
 * UInventorySystemComponentAssetFactoryNew is a class that defines a factory for creating Inventory System Component assets.
 * 
 * This class provides the functionality to create new Inventory System Component assets when using the asset creation menu in the editor.
 */
UCLASS(hidecategories = Object)
class INVENTORYSYSTEMEDITOR_API UInventorySystemComponentAssetFactoryNew : public UFactory
{
	GENERATED_UCLASS_BODY()

	/**
	 * Constructor for UInventorySystemComponentAssetFactoryNew.
	 */
	virtual UObject* FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn) override;

	/**
	 * Determines whether this factory should be shown in the "New" menu.
	 * 
	 * @return True if this factory should be shown in the "New" menu; otherwise, false.
	 */
	virtual bool ShouldShowInNewMenu() const override;
};