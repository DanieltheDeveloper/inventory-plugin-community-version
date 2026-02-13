// © 2024 Daniel Münch. All Rights Reserved

#pragma once

#include "Factories/Factory.h"
#include "ItemDropAssetFactoryNew.generated.h"

/**
 * UItemDropAssetFactoryNew is a class that defines a factory for creating Item Drop Assets.
 * 
 * This class provides the functionality to create new Item Drop Assets when using the asset creation menu in the editor.
 */
UCLASS(hidecategories = Object)
class INVENTORYSYSTEMEDITOR_API UItemDropAssetFactoryNew : public UFactory
{
	GENERATED_UCLASS_BODY()
	
	/**
	 * Constructor for UItemDropAssetFactoryNew.
	 */
	virtual UObject* FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn) override;

	/**
	 * Determines whether this factory should be shown in the "New" menu.
	 * 
	 * @return True if this factory should be shown in the "New" menu; otherwise, false.
	 */
	virtual bool ShouldShowInNewMenu() const override;
};