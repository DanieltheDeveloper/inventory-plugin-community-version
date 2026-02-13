// © 2024 Daniel Münch. All Rights Reserved

#include "Settings/InventorySystemSettings.h"

#include <functional>

#include "InventorySystem.h"
#include "InventorySystemComponent.h"
#include "ItemContainerComponent.h"
#include "ItemDrop.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Misc/ScopedSlowTask.h"
#include "UObject/SavePackage.h"

UInventorySystemSettings::UInventorySystemSettings(const FObjectInitializer& OBJ)
{
	MaxInventoryStackSize = 99;
	MaxInventorySize = 200;
	MaxItemEquipmentStackSize = 99;
	MaxItemContainerStackSize = 99;
	MaxItemContainerSize = 20;
	MaxItemDropStackSize = 99;
}

#if WITH_EDITORONLY_DATA
void UInventorySystemSettings::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	UObject::PostEditChangeProperty(PropertyChangedEvent);
	
	const FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");

	if (!AssetRegistryModule.IsValid())
	{
		UE_LOG(InventorySystem, Warning, TEXT("[UInventorySystemSettings|%s][PostEditChangeProperty]: AssetRegistryModule is invalid"), *GetFName().ToString());
		return;
	}

	const IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();

	const FText TaskTitle = FText(NSLOCTEXT("InventorySystem", "PropagateSystemSettingsChange", "Propagate Settings Change"));
	FScopedSlowTask SlowTask(10, TaskTitle);
	SlowTask.MakeDialog();
	SlowTask.Visibility = ESlowTaskVisibility::ForceVisible;
	SlowTask.EnterProgressFrame(1);
	
	FARFilter Filter;
	Filter.bRecursiveClasses = true;
	Filter.bRecursivePaths = true;
	Filter.PackagePaths.Add(FName("/Game"));
	Filter.ClassPaths= TArray{FTopLevelAssetPath(UBlueprint::StaticClass()), FTopLevelAssetPath(AActor::StaticClass()), FTopLevelAssetPath(UActorComponent::StaticClass()), FTopLevelAssetPath(AItemDrop::StaticClass()), FTopLevelAssetPath(StaticClass()), FTopLevelAssetPath(UItemEquipmentDataAsset::StaticClass()), FTopLevelAssetPath(UItemEquipmentTypeDataAsset::StaticClass())};
	TArray<FAssetData> AssetDataList;
	AssetRegistry.GetAssets(Filter, AssetDataList);

	const std::function ReSaveBatchOfAssets = [&](TArray<FAssetData>& AssetDataListHandle)
	{
		for (const FAssetData& AssetData : AssetDataListHandle)
		{
			if (UObject* Asset = AssetData.GetAsset(); IsValid(Asset) && !Asset->HasAnyFlags(RF_Transient | RF_TagGarbageTemp) && !Asset->HasAnyMarks(OBJECTMARK_EditorOnly))
			{
				if (const UBlueprint* Blueprint = Cast<UBlueprint>(Asset); IsValid(Blueprint))
				{
					if (!Blueprint->GeneratedClass->IsChildOf(UItemDataAsset::StaticClass()) && !Blueprint->GeneratedClass->IsChildOf(UItemEquipmentDataAsset::StaticClass()) && !Blueprint->GeneratedClass->IsChildOf(AActor::StaticClass()) && !Blueprint->GeneratedClass->IsChildOf(AItemDrop::StaticClass()) && !Blueprint->GeneratedClass->IsChildOf(UItemContainerComponent::StaticClass()) && !Blueprint->GeneratedClass->IsChildOf(UInventorySystemComponent::StaticClass()))
					{
						// Skip - Object should not be saved
						continue;
					}
				}
				
				if (UPackage* Package = Asset->GetOutermost(); IsValid(Package) && !Package->HasAnyFlags(RF_Transient | RF_TagGarbageTemp) && !Package->HasAnyMarks(OBJECTMARK_EditorOnly))
				{
					if (Package->IsFullyLoaded())
					{
						Package->FullyLoad();
					}
					
					// Save the package
					FString PackageFileName = FPackageName::LongPackageNameToFilename(Package->GetName(), FPackageName::GetAssetPackageExtension());
					FSavePackageArgs Args = FSavePackageArgs();
					Args.TopLevelFlags = RF_Public | RF_Standalone | RF_MarkAsNative | RF_Transactional | RF_ClassDefaultObject | RF_ArchetypeObject;
					Args.SaveFlags = SAVE_KeepEditorOnlyCookedPackages | SAVE_FromAutosave;
					if (UPackage::SavePackage(Package, Asset, *PackageFileName, Args))
					{
						// Log success or handle appropriately
						UE_LOG(InventorySystem, Log, TEXT("Saved Asset: %s"), *Asset->GetName());
					}
					else
					{
						// Log failure or handle appropriately
						UE_LOG(InventorySystem, Warning, TEXT("Failed to Save Asset: %s"), *Asset->GetName());
					}
				}
			}
		}
	};

	// Define batch size
	constexpr int32 BatchSize = 15;
	SlowTask.TotalAmountOfWork = AssetDataList.Num() / BatchSize + 2;
	for (int32 StartIndex = 0; StartIndex < AssetDataList.Num(); StartIndex += BatchSize)
	{
		SlowTask.EnterProgressFrame(1, FText::Format(FText::FromString(TEXT("{0}")), NSLOCTEXT("InventorySystem", "PropagateSystemSettingsChangeDesc", "Propagate Settings Change for Assets")));
		const int32 EndIndex = FMath::Min(StartIndex + BatchSize, AssetDataList.Num());
		TArray<FAssetData> Batch = TArray<FAssetData>(&AssetDataList[StartIndex], EndIndex - StartIndex);

		ReSaveBatchOfAssets(Batch);
	}

	SlowTask.CompletedWork = SlowTask.TotalAmountOfWork;
}
#endif
