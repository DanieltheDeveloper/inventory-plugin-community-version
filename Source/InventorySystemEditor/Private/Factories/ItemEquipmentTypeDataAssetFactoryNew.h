// © 2024 Daniel Münch. All Rights Reserved

#pragma once

#include "Factories/Factory.h"
#include "ClassViewerModule.h"
#include "ClassViewerFilter.h"
#include "Factories/DataAssetFactory.h"
#include "ItemEquipmentTypeDataAssetFactoryNew.generated.h"


class UItemEquipmentTypeDataAsset;

/**
 * UItemEquipmentTypeAssetFactoryNew is a class that defines a factory for creating Item Equipment Type Assets.
 * 
 * This class provides the functionality to create new Item Equipment Type Assets when using the asset creation menu in the editor.
 */
UCLASS(hidecategories = Object)
class INVENTORYSYSTEMEDITOR_API UItemEquipmentTypeDataAssetFactoryNew : public UFactory
{
    GENERATED_UCLASS_BODY()
 
    /**
     * Name that is displayed in the asset action menu
     *
     * @return 
     */
    virtual FText GetDisplayName() const override;
 
    /**
     * The class of the Item Equipment Type Asset to be created.
     */
    UPROPERTY(EditAnywhere, Category = Item)
    TSubclassOf<UItemEquipmentTypeDataAsset> ItemClass;

    /**
     * Constructor for UItemEquipmentTypeAssetFactoryNew.
     */
    virtual UObject* FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn) override;

    /**
     * Determines whether this factory should be shown in the "New" menu.
     * 
     * @return True if this factory should be shown in the "New" menu; otherwise, false.
     */
    virtual bool ShouldShowInNewMenu() const override;

    /**
     * Configures properties for the created Item Equipment Type Asset.
     * 
     * @return True if the properties were successfully configured; otherwise, false.
     */
    virtual bool ConfigureProperties() override;
};

/**
 * FItemEquipmentTypeFilterViewer is a class that defines a filter for the Class Viewer in the Unreal Engine editor.
 * 
 * This class is used to specify which classes are allowed to be displayed in the Class Viewer.
 */
class INVENTORYSYSTEMEDITOR_API FItemEquipmentTypeFilterViewer final : public IClassViewerFilter
{
public:
    /**
     * A set of allowed child classes for filtering.
     */
    TSet<const UClass*> AllowedChildrenOfClasses;

    /**
     * Flags for disallowed classes.
     */
    uint32 DisallowedClassFlags;

    /**
     * Determines if a class is allowed to be displayed in the Class Viewer.
     * 
     * @return True if the class is allowed; otherwise, false.
     */
    virtual bool IsClassAllowed(const FClassViewerInitializationOptions& InInitOptions, const UClass* InClass, const TSharedRef<class FClassViewerFilterFuncs> InFilterFuncs) override
    {
     return !InClass->HasAnyClassFlags(CLASS_Abstract)
      && InFilterFuncs->IfInChildOfClassesSet(AllowedChildrenOfClasses, InClass) != EFilterReturn::Failed;
    }


    /**
     * Determines if an unloaded class is allowed to be displayed in the Class Viewer.
     * 
     * @return True if the unloaded class is allowed; otherwise, false.
     */
    virtual bool IsUnloadedClassAllowed(const FClassViewerInitializationOptions& InInitOptions, const TSharedRef<const IUnloadedBlueprintData> InUnloadedClassData, const TSharedRef<FClassViewerFilterFuncs> InFilterFuncs) override
    {
     return !InUnloadedClassData->HasAnyClassFlags(DisallowedClassFlags)
      && InFilterFuncs->IfInChildOfClassesSet(AllowedChildrenOfClasses, InUnloadedClassData) != EFilterReturn::Failed;
    }
};