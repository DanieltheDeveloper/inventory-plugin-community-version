// © 2024 Daniel Münch. All Rights Reserved


#include "ItemEquipmentDataAsset.h"

#include "InventorySystem.h"
#include "Engine/AssetManager.h"
#include "Engine/World.h"
#include "UObject/SavePackage.h"

#if WITH_EDITORONLY_DATA
void UItemEquipmentDataAsset::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	// Check if the changed property is InventoryAsset
	if (PropertyChangedEvent.Property && PropertyChangedEvent.Property->GetFName() == GET_MEMBER_NAME_CHECKED(UItemEquipmentDataAsset, EquipmentTypeDataAssets))
	{
		AddToRoot();
		Modify();

		EquipmentType.Reset();
		for (const UItemEquipmentTypeDataAsset* DataAssetType : EquipmentTypeDataAssets)
		{
			if (IsValid(DataAssetType))
			{
				EquipmentType.Add(DataAssetType->GetPrimaryAssetId());	
			}
		}

		if (!HasAnyFlags(RF_Transient | RF_TagGarbageTemp) && !HasAnyMarks(OBJECTMARK_EditorOnly))
		{
			if (UPackage* Package = GetOutermost(); IsValid(Package) && Package->IsDirty() && !Package->HasAnyFlags(RF_Transient | RF_TagGarbageTemp) && !Package->HasAnyMarks(OBJECTMARK_EditorOnly))
			{
				const FString PackageFileName = FPackageName::LongPackageNameToFilename(Package->GetName(), FPackageName::GetAssetPackageExtension());
				FSavePackageArgs Args = FSavePackageArgs();
				Args.TopLevelFlags = GetFlags();
			
				if (UPackage::SavePackage(Package, this, *PackageFileName, Args))
				{
					// Log automatic change of FPrimaryAssetId of component
					UE_LOG(InventorySystem, Warning, TEXT("[UItemEquipmentDataAsset|%s][PostEditChangeProperty]: EquipmentTypeDataAsset for DataAsset was changed"), *GetFName().ToString());
					RerunAllItemDropConstructionScripts();
				}
			}	
		}
		RemoveFromRoot();
	}

	Super::PostEditChangeProperty(PropertyChangedEvent);
}
#endif

TArray<FPrimaryAssetId> UItemEquipmentDataAsset::GetEquipmentType_Implementation()
{
	return EquipmentType;
}
