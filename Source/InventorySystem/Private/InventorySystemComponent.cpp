// © 2024 Daniel Münch. All Rights Reserved

#include "InventorySystemComponent.h"

#include <functional>
#include "InventorySystem.h"
#include "Net/UnrealNetwork.h"
#include "Engine/AssetManager.h"
#include "AssetRegistry/AssetData.h"
#include "Settings/InventorySystemSettings.h"
#include "Kismet/KismetMathLibrary.h"
#include "UObject/SavePackage.h"

#define LOCTEXT_NAMESPACE "InventorySystem"


UInventorySystemComponent::UInventorySystemComponent()
{
#if WITH_EDITORONLY_DATA
	AllowEquipmentEdit = !EquipmentIndices.IsEmpty();
	AllowEquipmentTypeEdit = !EquipmentTypeIndices.IsEmpty();
	AllowEquipmentIndexEdit = !EquipmentTypes.IsEmpty();
	AllowEquipmentTypeAssetEdit = HasBegunPlay();
	AllowEquipmentAssetEdit = HasBegunPlay();
#endif
}

#if WITH_EDITOR
void UInventorySystemComponent::InternalCheckEditVariables(const TArray<int>& Slots)
{
	Super::InternalCheckEditVariables(Slots);

	AllowEquipmentEdit = !EquipmentIndices.IsEmpty();
	AllowEquipmentTypeEdit = !EquipmentTypeIndices.IsEmpty();
	AllowEquipmentIndexEdit = !EquipmentTypes.IsEmpty();
	AllowEquipmentTypeAssetEdit = HasBegunPlay();
	AllowEquipmentAssetEdit = HasBegunPlay();
}

void UInventorySystemComponent::InternalSaveAfterCheck()
{
	// Only save changes when in editor otherwise it could lead to problems and unexpected behavior
	if (HasAnyFlags(RF_Transient | RF_TagGarbageTemp) || HasAnyMarks(OBJECTMARK_EditorOnly) || HasBegunPlay() || IsBeingDestroyed())
	{
		return;
	}

	if (AActor* ActorOwner = GetOwner(); IsValid(ActorOwner))
	{
		if (UPackage* Package = ActorOwner->GetPackage(); IsValid(Package) && !Package->HasAnyFlags(RF_Transient | RF_TagGarbageTemp) && !Package->HasAnyMarks(OBJECTMARK_EditorOnly))
		{
			const FString PackageFileName = FPackageName::LongPackageNameToFilename(Package->GetName(), FPackageName::GetAssetPackageExtension());
			FSavePackageArgs Args = FSavePackageArgs();
			Args.TopLevelFlags = ActorOwner->GetFlags();

			if (UPackage::SavePackage(Package, ActorOwner, *PackageFileName, Args))
			{
				UE_LOG(InventorySystem, Warning, TEXT("[UInventorySystemComponent|%s][InternalSaveAfterCheck]: A mistake in setup resulted in data being altered... saving"), *GetFName().ToString());
			}
		}

		return;
	}

	if (UPackage* Package = GetOutermost(); IsValid(Package) && !Package->HasAnyFlags(RF_Transient | RF_TagGarbageTemp) && !Package->HasAnyMarks(OBJECTMARK_EditorOnly))
	{
		const FString PackageFileName = FPackageName::LongPackageNameToFilename(Package->GetName(), FPackageName::GetAssetPackageExtension());
		FSavePackageArgs Args = FSavePackageArgs();
		Args.TopLevelFlags = GetFlags();

		if (UPackage::SavePackage(Package, this, *PackageFileName, Args))
		{
			UE_LOG(InventorySystem, Warning, TEXT("[UInventorySystemComponent|%s][InternalSaveAfterCheck]: A mistake in setup resulted in data being altered... saving"), *GetFName().ToString());
		}
	}
}
#endif

void UInventorySystemComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(UInventorySystemComponent, EquipmentIndices, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UInventorySystemComponent, EquipmentAssets, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UInventorySystemComponent, EquipmentAmounts, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UInventorySystemComponent, EquipmentDynamicStatsIndices, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UInventorySystemComponent, EquipmentDynamicStats, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UInventorySystemComponent, EquipmentTypes, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UInventorySystemComponent, EquipmentTypeIndices, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME(UInventorySystemComponent, MaxEquipmentStackSize);
}

void UInventorySystemComponent::OnRep_EquipmentTypeIndices(TArray<int> OldEquipmentTypeIndices)
{
	// Check for changes in the array
	for (int Index = 0; Index < EquipmentTypeIndices.Num(); Index++)
	{
		// Check if index was added
		if (OldEquipmentTypeIndices.Find(EquipmentTypeIndices[Index]) == INDEX_NONE || EquipmentTypeIndices[Index] != OldEquipmentTypeIndices[Index])
		{
			ChangedEquipmentSlotsDelegate.Broadcast({EquipmentTypeIndices[Index]});
		}
	}

	for (int Index = 0; Index < OldEquipmentTypeIndices.Num(); Index++)
	{
		// Check if index was removed
		if (EquipmentTypeIndices.Find(OldEquipmentTypeIndices[Index]) == INDEX_NONE)
		{
			ChangedEquipmentSlotsDelegate.Broadcast({OldEquipmentTypeIndices[Index]});
		}
	}
}

void UInventorySystemComponent::OnRep_EquipmentTypes(TArray<FPrimaryAssetId> OldEquipmentTypes)
{
	// Check for changes in the array
	for (int Index = 0; Index < EquipmentTypes.Num(); Index++)
	{
		if (EquipmentTypeIndices.IsValidIndex(Index))
		{
			// Check if index was added or changed
			if (!OldEquipmentTypes.IsValidIndex(Index) || EquipmentTypes[Index] != OldEquipmentTypes[Index])
			{
				ChangedEquipmentSlotsDelegate.Broadcast({EquipmentTypeIndices[Index]});
			}	
		}
	}
}

void UInventorySystemComponent::OnRep_EquipmentIndices(TArray<int> OldEquipmentIndices)
{
	// Check for changes in the array
	for (int Index = 0; Index < EquipmentIndices.Num(); Index++)
	{
		// Check if index was added
		if (OldEquipmentIndices.Find(EquipmentIndices[Index]) == INDEX_NONE || EquipmentIndices[Index] != OldEquipmentIndices[Index])
		{
			ChangedEquipmentSlotsDelegate.Broadcast({EquipmentIndices[Index]});
		}
	}

	for (int Index = 0; Index < OldEquipmentIndices.Num(); Index++)
	{
		// Check if index was removed
		if (EquipmentIndices.Find(OldEquipmentIndices[Index]) == INDEX_NONE)
		{
			ChangedEquipmentSlotsDelegate.Broadcast({OldEquipmentIndices[Index]});
		}
	}
}

void UInventorySystemComponent::OnRep_EquipmentAssets(TArray<FPrimaryAssetId> OldEquipmentAssets)
{
	// Check for changes in the array
	for (int Index = 0; Index < EquipmentAssets.Num(); Index++)
	{
		if (EquipmentIndices.IsValidIndex(Index))
		{
			// Check if index was added or changed
			if (!OldEquipmentAssets.IsValidIndex(Index) || EquipmentAssets[Index] != OldEquipmentAssets[Index])
			{
				ChangedEquipmentSlotsDelegate.Broadcast({EquipmentIndices[Index]});
			}	
		}
	}
}

void UInventorySystemComponent::OnRep_EquipmentAmounts(TArray<int> OldEquipmentAmounts)
{
	// Check for changes in the array
	for (int Index = 0; Index < EquipmentAmounts.Num(); Index++)
	{
		if (EquipmentIndices.IsValidIndex(Index))
		{
			// Check if index was added or changed
			if (!OldEquipmentAmounts.IsValidIndex(Index) || EquipmentAmounts[Index] != OldEquipmentAmounts[Index])
			{
				ChangedEquipmentSlotsDelegate.Broadcast({EquipmentIndices[Index]});
			}
		}
	}
}

void UInventorySystemComponent::OnRep_EquipmentDynamicStatsIndices(TArray<int> OldEquipmentDynamicStatsIndices)
{
	// Check for changes in the array
	for (int Index = 0; Index < EquipmentDynamicStatsIndices.Num(); Index++)
	{
		if (const int RealEquipmentIndex = EquipmentIndices.Find(EquipmentDynamicStatsIndices[Index]); RealEquipmentIndex != INDEX_NONE)
		{
			// Check if index was added
			if (OldEquipmentDynamicStatsIndices.Find(EquipmentDynamicStatsIndices[Index]) == INDEX_NONE || EquipmentDynamicStatsIndices[Index] != OldEquipmentDynamicStatsIndices[Index])
			{
				ChangedEquipmentSlotsDelegate.Broadcast({EquipmentIndices[RealEquipmentIndex]});
			}
		}
	}

	for (int Index = 0; Index < OldEquipmentDynamicStatsIndices.Num(); Index++)
	{
		// Check if index was removed
		if (EquipmentDynamicStatsIndices.Find(OldEquipmentDynamicStatsIndices[Index]) == INDEX_NONE)
		{
			ChangedEquipmentSlotsDelegate.Broadcast({OldEquipmentDynamicStatsIndices[Index]});
		}
	}
}

void UInventorySystemComponent::OnRep_EquipmentDynamicStats(TArray<FItemProperties> OldEquipmentDynamicStats)
{
	// Check for changes in the array
	for (int Index = 0; Index < EquipmentDynamicStats.Num(); Index++)
	{
		if (!OldEquipmentDynamicStats.IsValidIndex(Index))
		{
			continue;
		}
		
		if (const int* RealEquipmentDynamicStatsSlot = EquipmentDynamicStatsIndices.FindByKey(Index); RealEquipmentDynamicStatsSlot != nullptr)
		{
			if (const int RealEquipmentIndex = EquipmentIndices.Find(*RealEquipmentDynamicStatsSlot); RealEquipmentIndex != INDEX_NONE)
			{
				// Check if value changed
				if (EquipmentDynamicStats[Index] != OldEquipmentDynamicStats[Index])
				{
					ChangedEquipmentSlotsDelegate.Broadcast({EquipmentIndices[RealEquipmentIndex]});
				}		
			}
		}
	}
}

#if WITH_EDITOR
void UInventorySystemComponent::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	FName PropertyName{};
	if (PropertyChangedEvent.Property)
	{
		PropertyName = PropertyChangedEvent.Property->GetFName();
	}


	// Check if the changed property is EquipmentDataAssets or EquipmentDataAssetTypes
	if (!HasBegunPlay() && !IsBeingDestroyed() && (PropertyName == GET_MEMBER_NAME_CHECKED(UInventorySystemComponent, EquipmentDataAssets) || PropertyName == GET_MEMBER_NAME_CHECKED(UInventorySystemComponent, EquipmentDataAssetTypes) || PropertyName ==
		GET_MEMBER_NAME_CHECKED(UInventorySystemComponent, EquipmentIndices) || PropertyName == GET_MEMBER_NAME_CHECKED(UInventorySystemComponent, EquipmentTypeIndices)))
	{
		AddToRoot();
		Modify();

		// EquipmentDataAssetTypes
		// Remove invalid assets first
		for (int I = 0; I < EquipmentDataAssetTypes.Num(); I++)
		{
			if (!EquipmentTypeIndices.IsValidIndex(I))
			{
				UE_LOG(InventorySystem, Warning, TEXT("[UInventorySystemComponent|%s][PostEditChangeProperty]: DataAsset with key %d. No valid EquipmentTypeIndicies entry found. Entry was deleted"), *GetFName().ToString(), I);
				EquipmentDataAssetTypes.RemoveAt(I);
			}
		}

		EquipmentTypes.Reset();
		for (int I = 0; I < EquipmentDataAssetTypes.Num(); I++)
		{
			if (const UItemEquipmentTypeDataAsset* DataAsset = EquipmentDataAssetTypes[I]; IsValid(DataAsset))
			{
				EquipmentTypes.Add(DataAsset->GetPrimaryAssetId());
				continue;
			}
			EquipmentTypes.Add(FPrimaryAssetId()); // Set to invalid FPrimaryAssetId
			UE_LOG(InventorySystem, Warning, TEXT("[UInventorySystemComponent|%s][PostEditChangeProperty]: DataAsset with key %d. No valid object could be cast. FPrimaryAssetId was set to empty"), *GetFName().ToString(), I);
		}

		EquipmentAssets.Empty();
		for (int I = 0; I < EquipmentDataAssets.Num(); I++)
		{
			if (const UItemEquipmentDataAsset* DataAsset = EquipmentDataAssets[I]; IsValid(DataAsset) && DataAsset->GetPrimaryAssetId().IsValid() && DataAsset->GetPrimaryAssetId() != FPrimaryAssetId())
			{
				EquipmentAssets.Add(DataAsset->GetPrimaryAssetId());
				continue;
			}
			EquipmentAssets.Add(FPrimaryAssetId()); // Set to invalid FPrimaryAssetId
			UE_LOG(InventorySystem, Warning, TEXT("[UInventorySystemComponent|%s][PostEditChangeProperty]: DataAsset with key %d. No valid object could be cast. FPrimaryAssetId was set to empty"), *GetFName().ToString(), I);
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
					UE_LOG(InventorySystem, Warning, TEXT("[UInventorySystemComponent|%s][PostEditChangeProperty]: DataAsset data was changed. Reconstructing FPrimaryAssetsIds"), *GetFName().ToString());
				}
			}
		}
		RemoveFromRoot();
		// Run internal checks
		InternalChecks();
		Super::PostEditChangeProperty(PropertyChangedEvent);
		return;
	}

	if (PropertyName == GET_MEMBER_NAME_CHECKED(UInventorySystemComponent, EquipmentTypeIndices) || PropertyName == GET_MEMBER_NAME_CHECKED(UInventorySystemComponent, EquipmentAssets) || PropertyName ==
		GET_MEMBER_NAME_CHECKED(UInventorySystemComponent, EquipmentIndices) || PropertyName == GET_MEMBER_NAME_CHECKED(UInventorySystemComponent, EquipmentAmounts) || PropertyName ==
		GET_MEMBER_NAME_CHECKED(UInventorySystemComponent, EquipmentTypes) || PropertyName == GET_MEMBER_NAME_CHECKED(UInventorySystemComponent, EquipmentDynamicStats) || PropertyName ==
		GET_MEMBER_NAME_CHECKED(UInventorySystemComponent, EquipmentDynamicStatsIndices) || PropertyName == GET_MEMBER_NAME_CHECKED(UInventorySystemComponent, MaxStackSize) || PropertyName ==
		GET_MEMBER_NAME_CHECKED(UInventorySystemComponent, InventorySize) || PropertyName == GET_MEMBER_NAME_CHECKED(UInventorySystemComponent, MaxEquipmentStackSize))
	{
		InternalChecks();
	}

	Super::PostEditChangeProperty(PropertyChangedEvent);
}
#endif

bool UInventorySystemComponent::InternalChecks(const bool bIsSavePackageEvent)
{
	bool InternalPreventExecution = Super::InternalChecks(bIsSavePackageEvent);

	AddToRoot();
	UAssetManager& Manager = UAssetManager::Get();
	const FName AssetRegistrySearchablePropertyName = GET_MEMBER_NAME_CHECKED(UItemDataAsset, bCanStack);

	if (!Manager.IsValid())
	{
		UE_LOG(InventorySystem, Error, TEXT("[UInventorySystemComponent|%s][InternalChecks]: AssetManager is not initialized"), *GetFName().ToString());
		RemoveFromRoot();
		return true;
	}

	if (EquipmentTypeIndices.IsEmpty() || EquipmentTypes.IsEmpty())
	{
		bool IsEmpty = false;
		if (!EquipmentDynamicStatsIndices.IsEmpty() || !EquipmentDynamicStats.IsEmpty() || !EquipmentAmounts.IsEmpty() || !EquipmentAssets.IsEmpty())
		{
			IsEmpty = true;
		}
		EquipmentDynamicStatsIndices.Empty();
		EquipmentDynamicStats.Empty();
		EquipmentAmounts.Empty();
		EquipmentAssets.Empty();
#if WITH_EDITORONLY_DATA
		AllowEquipmentEdit = false;
		AllowEquipmentTypeEdit = false;
		AllowEquipmentIndexEdit = false;
		if (!EquipmentDataAssets.IsEmpty() || !EquipmentDataAssetTypes.IsEmpty())
		{
			IsEmpty = true;
		}
		EquipmentDataAssets.Empty();
		EquipmentDataAssetTypes.Empty();
#endif
		EquipmentTypes.Empty();
		EquipmentIndices.Empty();

		if (IsEmpty && !bIsSavePackageEvent)
		{
			UE_LOG(InventorySystem, Warning, TEXT("[UInventorySystemComponent|%s][InternalChecks]: No valid equipment types or indices found. EquipmentAssets and EquipmentDataAssets reseted. Please fill the EquipmentTypes TMap and EquipmentIndicies TArray!"), *GetFName().ToString());
#if WITH_EDITORONLY_DATA
			InternalSaveAfterCheck();
#endif
			RemoveFromRoot();
			return true;
		}
	}

	if (!EquipmentIndices.IsEmpty() && (EquipmentAmounts.IsEmpty() || EquipmentAssets.IsEmpty()))
	{
		AllowEquipmentEdit = true;
	}

	bool IsDataChanged = false;

	// EquipmentTypeIndices
	if (EquipmentTypeIndices.Contains(0))
	{
		if (EquipmentTypeIndices.Num() == 1)
		{
			EquipmentTypeIndices[0] = 1;
			IsDataChanged = true;
		}
		else
		{
			bool IsChanged = false;
			if (const int RealIndex = EquipmentTypeIndices.Find(0); RealIndex != INDEX_NONE)
			{
				for (int I = 1; I <= 999; I++)
				{
					if (!EquipmentTypeIndices.Contains(I))
					{
						IsChanged = true;
						EquipmentTypeIndices[RealIndex] = I;
						IsDataChanged = true;
						UE_LOG(InventorySystem, Warning, TEXT("[UInventorySystemComponent|%s][InternalChecks]: EquipmentTypeIndices slot 0 is not a valid slot. Entry was changed to first available slot"), *GetFName().ToString());
						break;
					}
				}
			}
			if (!IsChanged)
			{
				InternalPreventExecution = true;
				IsDataChanged = true;
				UE_LOG(InventorySystem, Warning, TEXT("[UInventorySystemComponent|%s][InternalChecks]: EquipmentTypeIndices no valid or free slot found. Entry was deleted"), *GetFName().ToString());
				EquipmentTypeIndices.Remove(0);
			}
		}
	}

	TSet<int> UniqueSet;
	TArray<int> UniqueArray;

	for (int Element : EquipmentTypeIndices)
	{
		if (UniqueSet.Contains(Element))
		{
			UE_LOG(InventorySystem, Warning, TEXT("[UInventorySystemComponent|%s][InternalChecks]: EquipmentTypeIndices should be unique, element was removed"), *GetFName().ToString());
			InternalPreventExecution = true;
			IsDataChanged = true;
			continue;
		}

		if (Element <= 0)
		{
			InternalPreventExecution = true;
			UE_LOG(InventorySystem, Warning, TEXT("[UInventorySystemComponent|%s][InternalChecks]: EquipmentTypeIndices should be bigger or equal to 1. Negativ value found, element was removed"), *GetFName().ToString());
			IsDataChanged = true;
			continue;
		}

		UniqueSet.Add(Element);
		UniqueArray.AddUnique(Element);
	}

	EquipmentTypeIndices = UniqueArray;

	if (EquipmentTypeIndices.Num() > 999)
	{
		UE_LOG(InventorySystem, Warning, TEXT("[UInventorySystemComponent|%s][InternalChecks]: EquipmentTypeIndices slots out of range. All indicies above max inventory size were removed"), *GetFName().ToString());
		EquipmentTypeIndices.RemoveAt(999, EquipmentTypeIndices.Num() - 999);
		IsDataChanged = true;
	}

#if WITH_EDITOR
	if (EquipmentTypeIndices.Num())
	{
		AllowEquipmentTypeAssetEdit = HasBegunPlay();
		AllowEquipmentTypeEdit = true;
	}
	else
	{
		AllowEquipmentTypeAssetEdit = false;
		AllowEquipmentTypeEdit = false;
	}
#endif

	// EquipmentTypes
	for (int I = 0; I < EquipmentTypes.Num(); I++)
	{
		if (!EquipmentTypeIndices.IsValidIndex(I))
		{
			InternalPreventExecution = true;
			UE_LOG(InventorySystem, Warning, TEXT("[UInventorySystemComponent|%s][InternalChecks]: EquipmentTypeIndices has no valid EquipmentIndices. Element was removed"), *GetFName().ToString());
			EquipmentTypes.RemoveAt(I);
			IsDataChanged = true;
			continue;
		}

		if (!EquipmentTypes[I].IsValid() || EquipmentTypes[I] == FPrimaryAssetId())
		{
			InternalPreventExecution = true;
			UE_LOG(InventorySystem, Warning, TEXT("[UInventorySystemComponent|%s][InternalChecks]: EquipmentType is not valid. Check EquipmentDataAssetTypes before play"), *GetFName().ToString());
		}
	}

#if WITH_EDITORONLY_DATA
	if (EquipmentTypes.Num())
	{
		AllowEquipmentIndexEdit = true;
	}
	else
	{
		AllowEquipmentIndexEdit = false;
	}
#endif

	if (EquipmentIndices.IsEmpty())
	{
		bool IsEmpty = false;
		if (!EquipmentDynamicStatsIndices.IsEmpty() || !EquipmentDynamicStats.IsEmpty() || !EquipmentAmounts.IsEmpty() || !EquipmentAssets.IsEmpty())
		{
			IsEmpty = true;
		}
		EquipmentDynamicStatsIndices.Empty();
		EquipmentDynamicStats.Empty();
		EquipmentAmounts.Empty();
		EquipmentAssets.Empty();
#if WITH_EDITORONLY_DATA
		AllowEquipmentEdit = false;
		AllowEquipmentAssetEdit = false;
		if (!EquipmentDataAssets.IsEmpty())
		{
			IsEmpty = true;
		}
		EquipmentDataAssets.Empty();
#endif
		EquipmentIndices.Empty();

		if (IsEmpty && !bIsSavePackageEvent)
		{
			UE_LOG(InventorySystem, Warning,
			       TEXT("[UInventorySystemComponent|%s][InternalChecks]: No valid equipment types or indices found. EquipmentAssets and EquipmentDataAssets reseted. Please fill the EquipmentTypes TMap and EquipmentIndicies TArray!"),
			       *GetFName().ToString());
#if WITH_EDITORONLY_DATA
			InternalSaveAfterCheck();
#endif
			RemoveFromRoot();
			return true;
		}
	}

	// EquipmentIndices
	if (EquipmentIndices.Contains(0))
	{
		if (EquipmentIndices.Num() == 1)
		{
			EquipmentIndices[0] = 1;
			IsDataChanged = true;
		}
		else
		{
			bool IsChanged = false;
			if (const int RealIndex = EquipmentIndices.Find(0); RealIndex != INDEX_NONE)
			{
				for (int I = 1; I <= 999; I++)
				{
					if (!EquipmentIndices.Contains(I))
					{
						IsChanged = true;
						EquipmentIndices[RealIndex] = I;
						IsDataChanged = true;
						UE_LOG(InventorySystem, Warning, TEXT("[UInventorySystemComponent|%s][InternalChecks]: EquipmentIndices slot 0 is not a valid slot. Entry was changed to first available slot"), *GetFName().ToString());
						break;
					}
				}
			}
			if (!IsChanged)
			{
				InternalPreventExecution = true;
				IsDataChanged = true;
				UE_LOG(InventorySystem, Warning, TEXT("[UInventorySystemComponent|%s][InternalChecks]: EquipmentIndices no valid or free slot found. Entry was deleted"), *GetFName().ToString());
				EquipmentIndices.Remove(0);
			}
		}
	}

	UniqueSet.Reset();
	UniqueArray.Reset();

	for (int Element : EquipmentIndices)
	{
		if (UniqueSet.Contains(Element))
		{
			UE_LOG(InventorySystem, Warning, TEXT("[UInventorySystemComponent|%s][InternalChecks]: EquipmentIndices should be unique, element was removed"), *GetFName().ToString());
			InternalPreventExecution = true;
			IsDataChanged = true;
			continue;
		}

		if (Element <= 0)
		{
			InternalPreventExecution = true;
			UE_LOG(InventorySystem, Warning, TEXT("[UInventorySystemComponent|%s][InternalChecks]: EquipmentIndices should be bigger or equal to 1. Negativ value found, element was removed"), *GetFName().ToString());
			IsDataChanged = true;
			continue;
		}

		if (!EquipmentTypeIndices.Contains(Element))
		{
			InternalPreventExecution = true;
			UE_LOG(InventorySystem, Warning, TEXT("[UInventorySystemComponent|%s][InternalChecks]: EquipmentIndices should be in EquipmentTypeIndices. No value found, element was removed"), *GetFName().ToString());
			IsDataChanged = true;
			continue;
		}

		UniqueSet.Add(Element);
		UniqueArray.AddUnique(Element);
	}

	EquipmentIndices = UniqueArray;

	if (EquipmentIndices.Num() > 999)
	{
		UE_LOG(InventorySystem, Warning, TEXT("[UInventorySystemComponent|%s][InternalChecks]: EquipmentIndices slots out of range. All indicies above max size were removed"), *GetFName().ToString());
		IsDataChanged = true;
		EquipmentIndices.RemoveAt(999, EquipmentIndices.Num() - 999);
	}

#if WITH_EDITORONLY_DATA
	if (EquipmentIndices.Num())
	{
		AllowEquipmentAssetEdit = HasBegunPlay();
		AllowEquipmentEdit = true;
	}
	else
	{
		AllowEquipmentAssetEdit = false;
		AllowEquipmentEdit = false;
	}
#endif

	// EquipmentAmounts
	for (int I = 0; I < EquipmentAmounts.Num(); I++)
	{
		if (!EquipmentIndices.IsValidIndex(I))
		{
			InternalPreventExecution = true;
			UE_LOG(InventorySystem, Warning, TEXT("[UInventorySystemComponent|%s][InternalChecks]: EquipmentAmounts has no valid EquipmentIndices parent slot. All entries deleted"), *GetFName().ToString());
			IsDataChanged = true;
			EquipmentAmounts.RemoveAt(I);
			break;
		}

		if (EquipmentAmounts[I] <= 0)
		{
			UE_LOG(InventorySystem, Warning, TEXT("[UInventorySystemComponent|%s][InternalChecks]: EquipmentAmounts can't be smaller or equal to 0. Entry was changed to 1"), *GetFName().ToString());
			IsDataChanged = true;
			EquipmentAmounts[I] = 1;
		}

		FAssetData AssetData;
		if (EquipmentAssets.IsValidIndex(I))
		{
			Manager.GetPrimaryAssetData(EquipmentAssets[I], AssetData);
		}

		if (AssetData.IsValid())
		{
			if (bool TempCanStack = false; AssetData.GetTagValue(AssetRegistrySearchablePropertyName, TempCanStack))
			{
				Manager.UnloadPrimaryAsset(EquipmentAssets[I]);

				if (!TempCanStack && EquipmentAmounts[I] > 1)
				{
					UE_LOG(InventorySystem, Warning, TEXT("[UInventorySystemComponent|%s][InternalChecks]: EquipmentAmounts can't be greater then 1 if parent DataAsset disallows stacking. Entry was changed to 1"), *GetFName().ToString());
					EquipmentAmounts[I] = 1;
				}
			}
		}

		if (EquipmentAmounts[I] > GetEquipmentStackSizeConfig())
		{
			UE_LOG(InventorySystem, Warning, TEXT("[UInventorySystemComponent|%s][InternalChecks]: EquipmentAmounts can't be greater then max stack config. Amount was changed to max stack size"), *GetFName().ToString());
			EquipmentAmounts[I] = GetEquipmentStackSizeConfig();
			IsDataChanged = true;
		}
	}

	// EquipmentDynamicStatsIndices
	if (EquipmentDynamicStatsIndices.Contains(0))
	{
		if (EquipmentDynamicStatsIndices.Num() == 1)
		{
			EquipmentDynamicStatsIndices[0] = 1;
			IsDataChanged = true;
		}
		else
		{
			bool IsChanged = false;
			if (const int RealEquipmentDynamicStatsIndex = EquipmentDynamicStatsIndices.Find(0); RealEquipmentDynamicStatsIndex != INDEX_NONE)
			{
				for (int I = 1; I <= 999; I++)
				{
					if (EquipmentIndices.Contains(I) && !EquipmentDynamicStatsIndices.Contains(I))
					{
						IsChanged = true;
						EquipmentDynamicStatsIndices[RealEquipmentDynamicStatsIndex] = I;
						IsDataChanged = true;
						UE_LOG(InventorySystem, Warning, TEXT("[UInventorySystemComponent|%s][InternalChecks]: EquipmentDynamicStatsIndices slot 0 is not a valid slot. Entry was changed to first available slot"), *GetFName().ToString());
						break;
					}
				}
			}
			if (!IsChanged)
			{
				InternalPreventExecution = true;
				UE_LOG(InventorySystem, Warning, TEXT("[UInventorySystemComponent|%s][InternalChecks]: EquipmentDynamicStatsIndices no valid or free slot found. Entry was deleted. Please add more slots to the EquipmentIndices"),
				       *GetFName().ToString());
				EquipmentDynamicStatsIndices.Remove(0);
				IsDataChanged = true;
			}
		}
	}

	UniqueSet.Empty();
	UniqueArray.Empty();

	for (int Element : EquipmentDynamicStatsIndices)
	{
		// Check if valid slot
		if (const int RealIndex = EquipmentIndices.Find(Element); RealIndex == INDEX_NONE)
		{
			InternalPreventExecution = true;
			IsDataChanged = true;
			UE_LOG(InventorySystem, Warning, TEXT("[UInventorySystemComponent|%s][InternalChecks]: EquipmentDynamicStatsIndices slot is not a valid slot, element was removed"), *GetFName().ToString());
			continue;
		}

		// Check unique
		if (UniqueSet.Contains(Element))
		{
			InternalPreventExecution = true;
			IsDataChanged = true;
			UE_LOG(InventorySystem, Warning, TEXT("[UInventorySystemComponent|%s][InternalChecks]: EquipmentDynamicStatsIndices should be unique, element was removed"), *GetFName().ToString());
			continue;
		}

		// Check negative
		if (Element <= 0)
		{
			InternalPreventExecution = true;
			IsDataChanged = true;
			UE_LOG(InventorySystem, Warning, TEXT("[UInventorySystemComponent|%s][InternalChecks]: EquipmentDynamicStatsIndices should be postive, element was removed"), *GetFName().ToString());
			continue;
		}

		UniqueSet.Add(Element);
		UniqueArray.AddUnique(Element);
	}

	EquipmentDynamicStatsIndices = UniqueArray;

	// EquipmentDynamicStats
	if (EquipmentDynamicStatsIndices.IsEmpty() && !EquipmentDynamicStats.IsEmpty())
	{
		EquipmentDynamicStats.Empty();
		UE_LOG(InventorySystem, Warning, TEXT("[UInventorySystemComponent|%s][InternalChecks]: EquipmentDynamicStats EquipmentDynamicStatsIndices has no entries. All elements removed"), *GetFName().ToString());
		IsDataChanged = true;
	}

	for (int I = 0; I < EquipmentDynamicStats.Num(); I++)
	{
		if (!EquipmentDynamicStatsIndices.IsValidIndex(I))
		{
			InternalPreventExecution = true;
			UE_LOG(InventorySystem, Warning, TEXT("[UInventorySystemComponent|%s][InternalChecks]: EquipmentDynamicStats has no valid EquipmentDynamicStatsIndices parent entry. Element was removed"), *GetFName().ToString());
			EquipmentDynamicStats.RemoveAt(I);
			IsDataChanged = true;
		}
	}

	// EquipmentAssets
	for (int I = 0; I < EquipmentAssets.Num(); I++)
	{
		if (!EquipmentIndices.IsValidIndex(I))
		{
			InternalPreventExecution = true;
			UE_LOG(InventorySystem, Warning, TEXT("[UInventorySystemComponent|%s][InternalChecks]: InventoryAsset has no valid InventoryIndices. Element was removed"), *GetFName().ToString());
			EquipmentAssets.RemoveAt(I);
			IsDataChanged = true;
			continue;
		}

		if (!EquipmentAssets[I].IsValid() || EquipmentAssets[I] == FPrimaryAssetId())
		{
			InternalPreventExecution = true;
			UE_LOG(InventorySystem, Warning, TEXT("[UInventorySystemComponent|%s][InternalChecks]: EquipmentAsset is not valid. Check EquipmentDataAssets before play"), *GetFName().ToString());
		}
	}

#if WITH_EDITORONLY_DATA
	if (IsDataChanged && !bIsSavePackageEvent)
	{
		InternalSaveAfterCheck();
	}
#endif

	RemoveFromRoot();
	return InternalPreventExecution;
}

void UInventorySystemComponent::OnRegister()
{
	Super::OnRegister();

	InternalChecks(true);
}

void UInventorySystemComponent::BeginPlay()
{
#if WITH_EDITORONLY_DATA
	ChangedEquipmentSlotsDelegate.AddDynamic(this, &UInventorySystemComponent::InternalCheckEditVariables);
	
	// Additional check if EquipmentDataAssets is empty
	if (EquipmentDataAssetTypes.IsEmpty())
	{
		if (!EquipmentDynamicStatsIndices.IsEmpty() || !EquipmentDynamicStats.IsEmpty() || !EquipmentAmounts.IsEmpty() || !EquipmentAssets.IsEmpty() || !EquipmentTypes.IsEmpty() || !EquipmentDataAssets.IsEmpty() || !EquipmentDataAssetTypes.IsEmpty())
		{
			UE_LOG(InventorySystem, Error, TEXT("[UInventorySystemComponent|%s][BeginPlay]: No valid equipment types or indices found but data arrays filled"), *GetFName().ToString());
			UE_LOG(InventorySystem, Error, TEXT("[UInventorySystemComponent|%s][BeginPlay]: Is not setup correctly. Destroying component..."), *GetFName().ToString());
			DestroyComponent();
			return;
		}
	}

	if (EquipmentDataAssets.IsEmpty())
	{
		if (!EquipmentDynamicStatsIndices.IsEmpty() || !EquipmentDynamicStats.IsEmpty() || !EquipmentAmounts.IsEmpty() || !EquipmentAssets.IsEmpty())
		{
			UE_LOG(InventorySystem, Error, TEXT("[UInventorySystemComponent|%s][BeginPlay]: No valid equipment types or indices found but data arrays filled"), *GetFName().ToString());
			UE_LOG(InventorySystem, Error, TEXT("[UInventorySystemComponent|%s][BeginPlay]: Is not setup correctly. Destroying component..."), *GetFName().ToString());
			DestroyComponent();
			return;
		}
	}
#endif

	if (InternalChecks())
	{
		UE_LOG(InventorySystem, Error, TEXT("[UInventorySystemComponent|%s][BeginPlay]: Is not setup correctly. Destroying component..."), *GetFName().ToString());
		DestroyComponent();
		return;
	}

#if WITH_EDITORONLY_DATA
	EquipmentDataAssets.Empty();
	EquipmentDataAssetTypes.Empty();
	AllowEquipmentTypeAssetEdit = AllowEquipmentTypeEdit;
#endif

	Super::BeginPlay();
}

#if WITH_EDITOR
void UInventorySystemComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	ChangedEquipmentSlotsDelegate.RemoveAll(this);
	Super::EndPlay(EndPlayReason);
}
#endif

TArray<FEquipmentSlot> UInventorySystemComponent::GetEquipmentSlots() const
{
	if (EquipmentTypes.IsEmpty())
	{
		return {};
	}

	TArray<FEquipmentSlot> EquipmentSlots;
	for (const int Slot : EquipmentTypeIndices)
	{
		if (FEquipmentSlot NewSlot = GetEquipmentSlot(Slot); NewSlot.Slot != INDEX_NONE)
		{
			EquipmentSlots.Add(NewSlot);
			continue;
		}

		EquipmentSlots.Empty();
		break;
	}
		
	return EquipmentSlots;
}

FEquipmentSlot UInventorySystemComponent::GetEquipmentSlot(const int Slot) const
{
	UAssetManager& Manager = UAssetManager::Get();
	const FName AssetRegistrySearchableEquipmentTypePropertyName = GET_MEMBER_NAME_CHECKED(UItemEquipmentDataAsset, EquipmentType);

	if (!Manager.IsValid())
	{
		UE_LOG(InventorySystem, Error, TEXT("[UInventorySystemComponent|%s][GetEquipmentSlot]: AssetManager is not initialized"), *GetFName().ToString());
		return FEquipmentSlot{};
	}

	TArray<FPrimaryAssetId> NewEquipmentTypes;
	int NewSlot = INDEX_NONE;
	FPrimaryAssetId NewAsset;
	FItemProperties DynamicStats;
	int NewAmount = INDEX_NONE;
	
	if (const int RealEquipmentTypeIndex = EquipmentTypeIndices.Find(Slot); RealEquipmentTypeIndex !=INDEX_NONE)
	{
		NewSlot = Slot;
		if (const int RealEquipmentIndex = EquipmentIndices.Find(Slot); RealEquipmentIndex != INDEX_NONE)
		{
			if (const int RealEquipmentDynamicStatsIndex = EquipmentDynamicStatsIndices.Find(Slot); RealEquipmentDynamicStatsIndex != INDEX_NONE)
			{
				if (!EquipmentDynamicStats.IsValidIndex(RealEquipmentDynamicStatsIndex))
				{
					UE_LOG(InventorySystem, Error, TEXT("[UInventorySystemComponent|%s][GetEquipmentSlot]: EquipmentDynamicStats is not filled but has an EquipmentDynamicStatsIndices entry"), *GetFName().ToString());
					return FEquipmentSlot{};
				}
			
				DynamicStats = EquipmentDynamicStats[RealEquipmentDynamicStatsIndex];
			}

			FAssetData CurrentItemData;
			Manager.GetPrimaryAssetData(EquipmentAssets[RealEquipmentIndex], CurrentItemData);
		
			FAssetDataTagMapSharedView::FFindTagResult EquipmentTypesTagValue = CurrentItemData.TagsAndValues.FindTag(AssetRegistrySearchableEquipmentTypePropertyName);

			if (EquipmentTypesTagValue.IsSet())
			{
				TArray<FString> AssetEquipmentTypeStrings{};
				UE_LOG(InventorySystem, Log, TEXT("[UInventorySystemComponent|%s][GetEquipmentSlot]: %s"), *GetFName().ToString(), *EquipmentTypesTagValue.GetValue());
				FString AssetEquipmentTypeBaseString = ReplaceEquipmentArrayString(EquipmentTypesTagValue.GetValue());
				AssetEquipmentTypeBaseString.ParseIntoArray(AssetEquipmentTypeStrings, TEXT(","));
				for (FString AssetEquipmentTypeString : AssetEquipmentTypeStrings)
				{
					NewEquipmentTypes.Add(FPrimaryAssetId(AssetEquipmentTypeString));
				}
			}
			
			NewAsset = EquipmentAssets[RealEquipmentIndex];
			NewAmount = EquipmentAmounts[RealEquipmentIndex];
		}	
	}

	return FEquipmentSlot{NewEquipmentTypes, NewSlot, NewAsset, DynamicStats, NewAmount};
}

bool UInventorySystemComponent::SetEquipmentType_Validate(const int Slot, const FPrimaryAssetId EquipmentType)
{
	return true;
}

void UInventorySystemComponent::SetEquipmentType_Implementation(const int Slot, const FPrimaryAssetId EquipmentType)
{
	if (const AActor* Owner = GetOwner(); !IsValid(Owner) || !Owner->HasAuthority())
	{
		UE_LOG(InventorySystem, Error, TEXT("[UItemContainerComponent|%s][SetEquipmentType]: Component owner has no authority"), *GetFName().ToString());
		SetEquipmentTypeFailureDelegate.Broadcast(Slot, EquipmentType);
		return;
	}

	if (bIsProcessing)
	{
		UE_LOG(InventorySystem, Error, TEXT("[UInventorySystemComponent|%s][SetEquipmentType]: Component is still processing previous request"), *GetFName().ToString());
		SetEquipmentTypeFailureDelegate.Broadcast(Slot, EquipmentType);
		return;
	}

	bIsProcessing = true;
	
	// Check if PrimaryAssetId is the correct type
	if (EquipmentType.PrimaryAssetType.GetName() != UItemEquipmentTypeDataAsset::StaticClass()->GetFName())
	{
		UE_LOG(InventorySystem, Error, TEXT("[UInventorySystemComponent|%s][SetEquipmentType]: EquipmentType is not of type UItemEquipmentTypeDataAsset"), *GetFName().ToString());
		SetEquipmentTypeFailureDelegate.Broadcast(Slot, EquipmentType);
		bIsProcessing = false;
		return;
	}

	// Check slot valid
	const int RealEquipmentTypeIndices = EquipmentTypeIndices.Find(Slot);
	TArray<int> ChangedSlots;
	if (RealEquipmentTypeIndices == INDEX_NONE)
	{
		if (EquipmentType == FPrimaryAssetId() || !EquipmentType.IsValid())
		{
			UE_LOG(InventorySystem, Error, TEXT("[UInventorySystemComponent|%s][SetEquipmentType]: No EquipmentTypeIndices found and EquipmentType empty"), *GetFName().ToString());
			SetEquipmentTypeFailureDelegate.Broadcast(Slot, EquipmentType);
			bIsProcessing = false;
			return;
		}

		EquipmentTypeIndices.AddUnique(Slot);
		EquipmentTypes.Add(EquipmentType);
		SetEquipmentTypeSuccessDelegate.Broadcast(Slot);
		ChangedEquipmentSlotsDelegate.Broadcast({Slot});
		bIsProcessing = false;
		return;
	}

	if (!EquipmentTypes.IsValidIndex(RealEquipmentTypeIndices))
	{
		UE_LOG(InventorySystem, Error, TEXT("[UInventorySystemComponent|%s][SetEquipmentType]: EquipmentTypeIndices has an entry but EquipmentTypes entry is invalid"), *GetFName().ToString());
		SetEquipmentTypeFailureDelegate.Broadcast(Slot, EquipmentType);
		bIsProcessing = false;
		return;
	}

	// Unequip... If impossible stop!
	if (EquipmentIndices.Contains(Slot))
	{
		ChangedSlots = ItemUnequipInternal(Slot, {}, true);
		if (ChangedSlots.IsEmpty())
		{
			UE_LOG(InventorySystem, Error, TEXT("[UInventorySystemComponent|%s][SetEquipmentType]: Equipment item for slot %d could not be unequipped"), *GetFName().ToString(), Slot);
			SetEquipmentTypeFailureDelegate.Broadcast(Slot, EquipmentType);
			bIsProcessing = false;
			return;
		}
	}

	if (EquipmentType == FPrimaryAssetId())
	{
		EquipmentTypes.RemoveAt(RealEquipmentTypeIndices);
		EquipmentTypeIndices.RemoveAt(RealEquipmentTypeIndices);

		SetEquipmentTypeSuccessDelegate.Broadcast(Slot);
		ChangedEquipmentSlotsDelegate.Broadcast({Slot});
		ChangedInventorySlotsDelegate.Broadcast(ChangedSlots);
		bIsProcessing = false;
		return;
	}

	EquipmentTypes[RealEquipmentTypeIndices] = EquipmentType;

	SetEquipmentTypeSuccessDelegate.Broadcast(Slot);
	ChangedEquipmentSlotsDelegate.Broadcast({Slot});
	ChangedInventorySlotsDelegate.Broadcast(ChangedSlots);
	bIsProcessing = false;
}

bool UInventorySystemComponent::HasItemProperty(const int Slot, const FName Name, const bool bIsEquipment)
{
	if (!bIsEquipment)
	{
		return Super::HasItemProperty(Slot, Name, bIsEquipment);
	}

	const TArray<int>& IndicesArray = bIsEquipment ? EquipmentIndices : InventoryIndices;

	if (const int Index = IndicesArray.Find(Slot); Index == INDEX_NONE || Name.IsNone())
	{
		UE_LOG(InventorySystem, Error, TEXT("[UInventorySystemComponent|%s][HasItemProperty]: Data invalid for equipment slot: %d"), *GetFName().ToString(), Slot);
		return false;
	}

	if (const int EquipmentDynamicStatsIndex = EquipmentDynamicStatsIndices.Find(Slot); EquipmentDynamicStatsIndex != INDEX_NONE && EquipmentDynamicStats.IsValidIndex(EquipmentDynamicStatsIndex))
	{
		const TArray<FItemProperty>* DynamicStatsItemProperties = &EquipmentDynamicStats[EquipmentDynamicStatsIndex].ItemProperties;
		for (const FItemProperty& ItemProperty : *DynamicStatsItemProperties)
		{
			if (ItemProperty.Name == Name)
			{
				return true;
			}
		}

		// None found. Return nothing
		return false;
	}
	
	return false;
}

FItemProperty UInventorySystemComponent::GetItemProperty(const int Slot, const FName Name, const bool bIsEquipment)
{
	if (!bIsEquipment)
	{
		return Super::GetItemProperty(Slot, Name, bIsEquipment);
	}

	if (const int Index = EquipmentIndices.Find(Slot); Index == INDEX_NONE || Name.IsNone())
	{
		UE_LOG(InventorySystem, Error, TEXT("[UInventorySystemComponent|%s][GetItemProperty]: Data invalid for equipment slot: %d"), *GetFName().ToString(), Slot);
		return {};
	}

	if (const int EquipmentDynamicStatsIndex = EquipmentDynamicStatsIndices.Find(Slot); EquipmentDynamicStatsIndex != INDEX_NONE && EquipmentDynamicStats.IsValidIndex(EquipmentDynamicStatsIndex))
	{
		const TArray<FItemProperty>* DynamicStatsItemProperties = &EquipmentDynamicStats[EquipmentDynamicStatsIndex].ItemProperties;
		for (const FItemProperty& ItemProperty : *DynamicStatsItemProperties)
		{
			if (ItemProperty.Name == Name)
			{
				return ItemProperty;
			}
		}

		// None found. Return nothing
		return {};
	}

	return {};
}

void UInventorySystemComponent::SetSlotAmount_Implementation(const int Slot, const int Amount, const bool bIsEquipment)
{
	if (const AActor* Owner = GetOwner(); !IsValid(Owner) || !Owner->HasAuthority())
	{
		UE_LOG(InventorySystem, Error, TEXT("[UItemContainerComponent|%s][SetSlotAmount]: Component owner has no authority"), *GetFName().ToString());
		SetSlotAmountSuccessDelegate.Broadcast(false, Slot, bIsEquipment);
		return;
	}

	if (bIsProcessing)
	{
		UE_LOG(InventorySystem, Error, TEXT("[UInventorySystemComponent|%s][SetSlotAmount]: Component is still processing previous request"), *GetFName().ToString());
		SetSlotAmountSuccessDelegate.Broadcast(false, Slot, bIsEquipment);
		return;
	}

	if (!bIsEquipment)
	{
		return Super::SetSlotAmount_Implementation(Slot, Amount, bIsEquipment);
	}

	bIsProcessing = true;

	// Equipment
	if (const int AmountIndex = EquipmentIndices.Find(Slot); AmountIndex != INDEX_NONE && EquipmentAssets.IsValidIndex(AmountIndex) && Amount > 0 && Amount <= GetEquipmentStackSizeConfig())
	{
		bool TempCanStack = false;
		UAssetManager& Manager = UAssetManager::Get();
		if (!Manager.IsValid())
		{
			UE_LOG(InventorySystem, Error, TEXT("[UInventorySystemComponent|%s][SetSlotAmount]: AssetManager is not initialized. Unable to set TempCanStack value"), *GetFName().ToString());
			SetSlotAmountSuccessDelegate.Broadcast(false, Slot, bIsEquipment);
			bIsProcessing = false;
			return;
		}

		FAssetData AssetData;
		Manager.GetPrimaryAssetData(EquipmentAssets[AmountIndex], AssetData);
		if (!AssetData.IsValid())
		{
			UE_LOG(InventorySystem, Error, TEXT("[UInventorySystemComponent|%s][SetSlotAmount]: AssetData is not valid. Unable to set TempCanStack value"), *GetFName().ToString());
			SetSlotAmountSuccessDelegate.Broadcast(false, Slot, bIsEquipment);
			bIsProcessing = false;
			return;
		}

		const FName AssetRegistrySearchablePropertyName = GET_MEMBER_NAME_CHECKED(UItemDataAsset, bCanStack);
		AssetData.GetTagValue(AssetRegistrySearchablePropertyName, TempCanStack);

		if (!TempCanStack && Amount > 1)
		{
			UE_LOG(InventorySystem, Log, TEXT("[UInventorySystemComponent|%s][SetSlotAmount]: Amount was set to 1 as equipment item is not stackable!"), *GetFName().ToString());
			EquipmentAmounts[AmountIndex] = 1;
		}
		else
		{
			EquipmentAmounts[AmountIndex] = Amount;
		}

		SetSlotAmountSuccessDelegate.Broadcast(true, Slot, bIsEquipment);
		ChangedEquipmentSlotsDelegate.Broadcast({Slot});
		bIsProcessing = false;
		return;
	}

	UE_LOG(InventorySystem, Log, TEXT("[UInventorySystemComponent|%s][SetSlotAmount]: Amount of equipment item could not be set: %d"), *GetFName().ToString(), Slot);
	SetSlotAmountSuccessDelegate.Broadcast(false, Slot, bIsEquipment);
	bIsProcessing = false;
}

void UInventorySystemComponent::SetSlotItemProperty_Implementation(const int Slot, const FName Name, const FText& DisplayName, const FText& Value, const bool bIsEquipment)
{
	if (const AActor* Owner = GetOwner(); !IsValid(Owner) || !Owner->HasAuthority())
	{
		UE_LOG(InventorySystem, Error, TEXT("[UItemContainerComponent|%s][SetSlotItemProperty]: Component owner has no authority"), *GetFName().ToString());
		SetSlotItemPropertySuccessDelegate.Broadcast(false, Slot, bIsEquipment);
		return;
	}

	if (bIsProcessing)
	{
		UE_LOG(InventorySystem, Warning, TEXT("[UInventorySystemComponent|%s][SetSlotItemProperty]: Component is still processing previous request"), *GetFName().ToString());
		SetSlotItemPropertySuccessDelegate.Broadcast(false, Slot, bIsEquipment);
		return;
	}

	if (!bIsEquipment)
	{
		return Super::SetSlotItemProperty_Implementation(Slot, Name, DisplayName, Value, bIsEquipment);
	}

	bIsProcessing = true;

	const int EquipmentDynamicStatsIndex = EquipmentDynamicStatsIndices.Find(Slot);
	if (const int EquipmentIndex = EquipmentIndices.Find(Slot); EquipmentIndex == INDEX_NONE || Name.IsNone())
	{
		UE_LOG(InventorySystem, Error, TEXT("[UInventorySystemComponent|%s][SetSlotItemProperty]: Equipment data invalid for slot %d"), *GetFName().ToString(), Slot);
		SetSlotItemPropertySuccessDelegate.Broadcast(false, Slot, bIsEquipment);
		bIsProcessing = false;
		return;
	}

	if (EquipmentDynamicStatsIndex == INDEX_NONE)
	{
		TArray<FItemProperty> NewItemProperties;
		NewItemProperties.Add(FItemProperty{Name, DisplayName, Value});
		// Something is wrong! Should not be filled without index
		if (const int NewEquipmentDynamicStatsIndex = EquipmentDynamicStatsIndices.AddUnique(Slot); EquipmentDynamicStats.IsValidIndex(NewEquipmentDynamicStatsIndex))
		{
			UE_LOG(InventorySystem, Error, TEXT("[UInventorySystemComponent|%s][SetSlotItemProperty]: EquipmentDynamicStats should not be filled. Index was just created"), *GetFName().ToString());
			// Revert back
			EquipmentDynamicStatsIndices.RemoveAt(NewEquipmentDynamicStatsIndex);
			SetSlotItemPropertySuccessDelegate.Broadcast(false, Slot, bIsEquipment);
			bIsProcessing = false;
			return;
		}

		EquipmentDynamicStats.Add(FItemProperties{NewItemProperties});
		SetSlotItemPropertySuccessDelegate.Broadcast(true, Slot, bIsEquipment);
		ChangedEquipmentSlotsDelegate.Broadcast({Slot});
		bIsProcessing = false;
		return;
	}

	// Something is wrong! Should be filled with stats
	if (!EquipmentDynamicStats.IsValidIndex(EquipmentDynamicStatsIndex))
	{
		UE_LOG(InventorySystem, Error, TEXT("[UInventorySystemComponent|%s][SetSlotItemProperty]: EquipmentDynamicStats is not filled but has an EquipmentDynamicStatsIndices entry"), *GetFName().ToString());
		SetSlotItemPropertySuccessDelegate.Broadcast(false, Slot, bIsEquipment);
		bIsProcessing = false;
		return;
	}

	FItemProperty DeleteItemProperty; 
	for (FItemProperty& ItemProperty : EquipmentDynamicStats[EquipmentDynamicStatsIndex].ItemProperties)
	{
		if (ItemProperty.Name == Name)
		{
			if (Value.IsEmpty())
			{
				DeleteItemProperty = ItemProperty;
				break;
			}

			ItemProperty.Value = Value;
			ItemProperty.DisplayName = DisplayName;
			SetSlotItemPropertySuccessDelegate.Broadcast(true, Slot, bIsEquipment);
			ChangedEquipmentSlotsDelegate.Broadcast({Slot});
			bIsProcessing = false;
			return;
		}
	}

	if (!DeleteItemProperty.Name.IsNone())
	{
		EquipmentDynamicStats[EquipmentDynamicStatsIndex].ItemProperties.Remove(DeleteItemProperty);
		if (EquipmentDynamicStats[EquipmentDynamicStatsIndex].ItemProperties.IsEmpty())
		{
			EquipmentDynamicStatsIndices.RemoveAt(EquipmentDynamicStatsIndex);
			EquipmentDynamicStats.RemoveAt(EquipmentDynamicStatsIndex);
		}
		SetSlotItemPropertySuccessDelegate.Broadcast(true, Slot, bIsEquipment);
		ChangedEquipmentSlotsDelegate.Broadcast({Slot});
		bIsProcessing = false;
		return;	
	}

	EquipmentDynamicStats[EquipmentDynamicStatsIndex].ItemProperties.Add(FItemProperty{Name, DisplayName, Value});
	SetSlotItemPropertySuccessDelegate.Broadcast(true, Slot, bIsEquipment);
	ChangedEquipmentSlotsDelegate.Broadcast({Slot});
	bIsProcessing = false;
}

void UInventorySystemComponent::SwapItems_Implementation(const int First, const int Second, const bool bCanStack, const bool bIsEquipment)
{
	if (!bIsEquipment)
	{
		return Super::SwapItems_Implementation(First, Second, bCanStack, bIsEquipment);
	}

	if (const AActor* Owner = GetOwner(); !IsValid(Owner) || !Owner->HasAuthority())
	{
		UE_LOG(InventorySystem, Error, TEXT("[UItemContainerComponent|%s][SwapItems]: Component owner has no authority"), *GetFName().ToString());
		SwapItemSuccessDelegate.Broadcast(false, First, Second, bIsEquipment);
		return;
	}

	if (bIsProcessing)
	{
		UE_LOG(InventorySystem, Warning, TEXT("[UInventorySystemComponent|%s][SwapItems]: Component is still processing previous request"), *GetFName().ToString());
		SwapItemSuccessDelegate.Broadcast(false, First, Second, bIsEquipment);
		return;
	}

	bIsProcessing = true;

	const int FirstIndex = EquipmentIndices.Find(First);
	const int SecondIndex = EquipmentIndices.Find(Second);

	UAssetManager& Manager = UAssetManager::Get();
	if ((FirstIndex == INDEX_NONE && SecondIndex == INDEX_NONE) || !Manager.IsValid() || !EquipmentTypeIndices.Contains(First) || !EquipmentTypeIndices.Contains(Second))
	{
		UE_LOG(InventorySystem, Error, TEXT("[UInventorySystemComponent|%s][SwapItems]: AssetManager is not initialized or item data is invalid"), *GetFName().ToString());
		SwapItemSuccessDelegate.Broadcast(false, First, Second, bIsEquipment);
		ChangedEquipmentSlotsDelegate.Broadcast({First, Second});
		bIsProcessing = false;
		return;
	}

	const int RealFirstEquipmentTypeIndex = EquipmentTypeIndices.Find(First);
	const int RealSecondEquipmentTypeIndex = EquipmentTypeIndices.Find(Second);

	if (RealFirstEquipmentTypeIndex == INDEX_NONE || RealSecondEquipmentTypeIndex == INDEX_NONE || !EquipmentTypes.IsValidIndex(RealFirstEquipmentTypeIndex) || !EquipmentTypes.IsValidIndex(RealSecondEquipmentTypeIndex))
	{
		UE_LOG(InventorySystem, Error, TEXT("[UInventorySystemComponent|%s][SwapItems]: Equipment slot or slots could not be found"), *GetFName().ToString());
		SwapItemSuccessDelegate.Broadcast(false, First, Second, bIsEquipment);
		bIsProcessing = false;
		return;
	}

	const FName AssetRegistrySearchableEquipmentTypePropertyName = GET_MEMBER_NAME_CHECKED(UItemEquipmentDataAsset, EquipmentType);

	const int RealFirstEquipmentStatsIndex = EquipmentDynamicStatsIndices.Find(First);
	const int RealSecondEquipmentStatsIndex = EquipmentDynamicStatsIndices.Find(Second);

	if ((RealFirstEquipmentStatsIndex != INDEX_NONE && !EquipmentDynamicStats.IsValidIndex(RealFirstEquipmentStatsIndex)) || (RealSecondEquipmentStatsIndex != INDEX_NONE && !EquipmentDynamicStats.
		IsValidIndex(RealSecondEquipmentStatsIndex)))
	{
		UE_LOG(InventorySystem, Error, TEXT("[UInventorySystemComponent|%s][SwapItems]: EquipmentDynamicStats is not filled but has an EquipmentDynamicStatsIndices entry"), *GetFName().ToString());
		SwapItemSuccessDelegate.Broadcast(false, First, Second, bIsEquipment);
		bIsProcessing = false;
		return;
	}

	// Both slots in use
	if (FirstIndex != INDEX_NONE && SecondIndex != INDEX_NONE)
	{
		bool DataInvalid = false;
		if (!EquipmentAssets.IsValidIndex(FirstIndex) || !EquipmentAmounts.IsValidIndex(FirstIndex))
		{
			UE_LOG(InventorySystem, Error, TEXT("[UInventorySystemComponent|%s][SwapItems]: Data invalid for slot %d"), *GetFName().ToString(), First);
			DataInvalid = true;
		}

		if (!EquipmentAssets.IsValidIndex(SecondIndex) || !EquipmentAmounts.IsValidIndex(SecondIndex))
		{
			UE_LOG(InventorySystem, Error, TEXT("[UInventorySystemComponent|%s][SwapItems]: Data invalid for slot %d"), *GetFName().ToString(), Second);
			DataInvalid = true;
		}

		if (DataInvalid)
		{
			SwapItemSuccessDelegate.Broadcast(false, First, Second, bIsEquipment);
			bIsProcessing = false;
			return;
		}

		FAssetData FirstAssetData;
		FAssetData SecondAssetData;
		Manager.GetPrimaryAssetData(EquipmentAssets[FirstIndex], FirstAssetData);
		Manager.GetPrimaryAssetData(EquipmentAssets[SecondIndex], SecondAssetData);

		if (!FirstAssetData.IsValid() || !SecondAssetData.IsValid())
		{
			UE_LOG(InventorySystem, Error, TEXT("[UInventorySystemComponent|%s][SwapItems]: AssetData is not valid. Unable to set TempCanStack value"), *GetFName().ToString());
			SwapItemSuccessDelegate.Broadcast(false, First, Second, bIsEquipment);
			bIsProcessing = false;
			return;
		}

		const FName AssetRegistrySearchablePropertyName = GET_MEMBER_NAME_CHECKED(UItemDataAsset, bCanStack);
		bool FirstTempCanStack = false;
		FirstAssetData.GetTagValue(AssetRegistrySearchablePropertyName, FirstTempCanStack);

		bool SecondTempCanStack = false;
		SecondAssetData.GetTagValue(AssetRegistrySearchablePropertyName, SecondTempCanStack);

		if (bCanStack && FirstTempCanStack && FirstAssetData == SecondAssetData)
		{
			bool bIsSameDynamicStatsItem = false;
			if (RealFirstEquipmentStatsIndex != INDEX_NONE && RealSecondEquipmentStatsIndex != INDEX_NONE)
			{
				if (!EquipmentDynamicStats.IsValidIndex(RealFirstEquipmentStatsIndex) || !EquipmentDynamicStats.IsValidIndex(RealSecondEquipmentStatsIndex))
				{
					UE_LOG(InventorySystem, Error, TEXT("[UInventorySystemComponent|%s][SwapItems]: EquipmentDynamicStats is not filled but has an EquipmentDynamicStatsIndices entry"), *GetFName().ToString());
					SwapItemSuccessDelegate.Broadcast(false, First, Second, bIsEquipment);
					bIsProcessing = false;
					return;
				}

				if (EquipmentDynamicStats[RealFirstEquipmentStatsIndex] == EquipmentDynamicStats[RealSecondEquipmentStatsIndex])
				{
					bIsSameDynamicStatsItem = true;
				}
			}

			if ((!EquipmentDynamicStats.IsValidIndex(RealFirstEquipmentStatsIndex) && !EquipmentDynamicStats.IsValidIndex(RealSecondEquipmentStatsIndex)) || bIsSameDynamicStatsItem)
			{
				// If same item and first can be stacked on second completely
				if (EquipmentAmounts[SecondIndex] + EquipmentAmounts[FirstIndex] <= GetStackSizeConfig())
				{
					EquipmentAmounts[SecondIndex] += EquipmentAmounts[FirstIndex];

					EquipmentIndices.RemoveAt(FirstIndex);
					EquipmentAmounts.RemoveAt(FirstIndex);
					EquipmentAssets.RemoveAt(FirstIndex);

					if (RealFirstEquipmentStatsIndex != INDEX_NONE)
					{
						EquipmentDynamicStatsIndices.RemoveAt(RealFirstEquipmentStatsIndex);
						EquipmentDynamicStats.RemoveAt(RealFirstEquipmentStatsIndex);
					}

					SwapItemSuccessDelegate.Broadcast(true, First, Second, bIsEquipment);
					ChangedEquipmentSlotsDelegate.Broadcast({First, Second});
					bIsProcessing = false;
					return;
				}

				if (const int AmountLeft = EquipmentAmounts[SecondIndex] + EquipmentAmounts[FirstIndex] - GetStackSizeConfig(); GetStackSizeConfig() > EquipmentAmounts[SecondIndex])
				{
					EquipmentAmounts[SecondIndex] = GetStackSizeConfig();
					EquipmentAmounts[FirstIndex] = AmountLeft;

					SwapItemSuccessDelegate.Broadcast(true, First, Second, bIsEquipment);
					ChangedEquipmentSlotsDelegate.Broadcast({First, Second});
					bIsProcessing = false;
					return;
				}	
			}
		}

		TArray<FPrimaryAssetId> FirstAssetEquipmentType{};
		FAssetDataTagMapSharedView::FFindTagResult FirstTagValue = FirstAssetData.TagsAndValues.FindTag(AssetRegistrySearchableEquipmentTypePropertyName);
		TArray<FPrimaryAssetId> SecondAssetEquipmentType{};
		FAssetDataTagMapSharedView::FFindTagResult SecondTagValue = SecondAssetData.TagsAndValues.FindTag(AssetRegistrySearchableEquipmentTypePropertyName);

		if (FirstTagValue.IsSet())
		{
			TArray<FString> AssetEquipmentTypeStrings{};
			UE_LOG(InventorySystem, Log, TEXT("[UInventorySystemComponent|%s][SwapItems]: %s"), *GetFName().ToString(), *FirstTagValue.GetValue());
			FString AssetEquipmentTypeBaseString = ReplaceEquipmentArrayString(FirstTagValue.GetValue());
			AssetEquipmentTypeBaseString.ParseIntoArray(AssetEquipmentTypeStrings, TEXT(","));
			for (FString AssetEquipmentTypeString : AssetEquipmentTypeStrings)
			{
				FirstAssetEquipmentType.Add(FPrimaryAssetId(AssetEquipmentTypeString));
			}
		}

		if (SecondTagValue.IsSet())
		{
			TArray<FString> AssetEquipmentTypeStrings{};
			UE_LOG(InventorySystem, Log, TEXT("[UInventorySystemComponent|%s][SwapItems]: %s"), *GetFName().ToString(), *SecondTagValue.GetValue());
			FString AssetEquipmentTypeBaseString = ReplaceEquipmentArrayString(SecondTagValue.GetValue());
			AssetEquipmentTypeBaseString.ParseIntoArray(AssetEquipmentTypeStrings, TEXT(","));
			for (FString AssetEquipmentTypeString : AssetEquipmentTypeStrings)
			{
				SecondAssetEquipmentType.Add(FPrimaryAssetId(AssetEquipmentTypeString));
			}
		}

		if (FirstAssetEquipmentType.IsEmpty() || SecondAssetEquipmentType.IsEmpty())
		{
			UE_LOG(InventorySystem, Error, TEXT("[UInventorySystemComponent|%s][SwapItems]: AssetData has no valid equipment type"), *GetFName().ToString());
			SwapItemSuccessDelegate.Broadcast(false, First, Second, bIsEquipment);
			bIsProcessing = false;
			return;
		}

		if (EquipmentTypes[RealFirstEquipmentTypeIndex] == EquipmentTypes[RealSecondEquipmentTypeIndex] || (FirstAssetEquipmentType.Contains(EquipmentTypes[RealSecondEquipmentTypeIndex]) && SecondAssetEquipmentType.Contains(EquipmentTypes[RealFirstEquipmentTypeIndex])))
		{
			Swap(EquipmentAssets[FirstIndex], EquipmentAssets[SecondIndex]);
			Swap(EquipmentAmounts[FirstIndex], EquipmentAmounts[SecondIndex]);

			if (RealFirstEquipmentStatsIndex != INDEX_NONE && RealSecondEquipmentStatsIndex != INDEX_NONE)
			{
				Swap(EquipmentDynamicStats[RealFirstEquipmentStatsIndex], EquipmentDynamicStats[RealSecondEquipmentStatsIndex]);
			}
			else if (RealFirstEquipmentStatsIndex != INDEX_NONE)
			{
				EquipmentDynamicStatsIndices[RealFirstEquipmentStatsIndex] = Second;
			}
			else if (RealSecondEquipmentStatsIndex != INDEX_NONE)
			{
				EquipmentDynamicStatsIndices[RealSecondEquipmentStatsIndex] = First;
			}

			SwapItemSuccessDelegate.Broadcast(true, First, Second, bIsEquipment);
			ChangedEquipmentSlotsDelegate.Broadcast({First, Second});
			bIsProcessing = false;
			return;
		}

		UE_LOG(InventorySystem, Warning, TEXT("[UInventorySystemComponent|%s][SwapItems]: Items could not be swapped. Maxium stack size already reached or invalid EquipmentType"), *GetFName().ToString());
		SwapItemSuccessDelegate.Broadcast(false, First, Second, bIsEquipment);
		bIsProcessing = false;
		return;
	}

	// Only first slot valid
	if (FirstIndex != INDEX_NONE)
	{
		FAssetData FirstAssetData;
		Manager.GetPrimaryAssetData(EquipmentAssets[FirstIndex], FirstAssetData);
		if (!FirstAssetData.IsValid())
		{
			UE_LOG(InventorySystem, Error, TEXT("[UInventorySystemComponent|%s][SwapItems]: AssetData is not valid. Unable to set TempCanStack value"), *GetFName().ToString());
			SwapItemSuccessDelegate.Broadcast(false, First, Second, bIsEquipment);
			bIsProcessing = false;
			return;
		}

		TArray<FPrimaryAssetId> FirstAssetEquipmentType{};
		FAssetDataTagMapSharedView::FFindTagResult FirstTagValue = FirstAssetData.TagsAndValues.FindTag(AssetRegistrySearchableEquipmentTypePropertyName);
		if (FirstTagValue.IsSet())
		{
			TArray<FString> AssetEquipmentTypeStrings{};
			UE_LOG(InventorySystem, Log, TEXT("[UInventorySystemComponent|%s][SwapItems]: %s"), *GetFName().ToString(), *FirstTagValue.GetValue());
			FString AssetEquipmentTypeBaseString = FirstTagValue.GetValue();
			AssetEquipmentTypeBaseString = AssetEquipmentTypeBaseString.Replace(TEXT("("), TEXT(""));
			AssetEquipmentTypeBaseString = AssetEquipmentTypeBaseString.Replace(TEXT(")"), TEXT(""));
			AssetEquipmentTypeBaseString = AssetEquipmentTypeBaseString.Replace(TEXT("\""), TEXT(""));
			AssetEquipmentTypeBaseString.ParseIntoArray(AssetEquipmentTypeStrings, TEXT(","));
			for (FString AssetEquipmentTypeString : AssetEquipmentTypeStrings)
			{
				FirstAssetEquipmentType.Add(FPrimaryAssetId(AssetEquipmentTypeString));
			}
		}

		if (FirstAssetEquipmentType.IsEmpty())
		{
			UE_LOG(InventorySystem, Error, TEXT("[UInventorySystemComponent|%s][SwapItems]: AssetData has no valid equipment type"), *GetFName().ToString());
			SwapItemSuccessDelegate.Broadcast(false, First, Second, bIsEquipment);
			bIsProcessing = false;
			return;
		}

		if (!FirstAssetEquipmentType.Contains(EquipmentTypes[RealSecondEquipmentTypeIndex]))
		{
			UE_LOG(InventorySystem, Warning, TEXT("[UInventorySystemComponent|%s][SwapItems]: AssetData equipment type is incorrect"), *GetFName().ToString());
			SwapItemSuccessDelegate.Broadcast(false, First, Second, bIsEquipment);
			bIsProcessing = false;
			return;
		}

		EquipmentIndices[FirstIndex] = Second;
		if (RealFirstEquipmentStatsIndex != INDEX_NONE)
		{
			EquipmentDynamicStatsIndices[RealFirstEquipmentStatsIndex] = Second;
		}

		SwapItemSuccessDelegate.Broadcast(true, First, Second, bIsEquipment);
		ChangedEquipmentSlotsDelegate.Broadcast({First, Second});
		bIsProcessing = false;
		return;
	}

	// Only second slot valid
	if (SecondIndex != INDEX_NONE)
	{
		FAssetData SecondAssetData;
		Manager.GetPrimaryAssetData(EquipmentAssets[SecondIndex], SecondAssetData);
		if (!SecondAssetData.IsValid())
		{
			UE_LOG(InventorySystem, Error, TEXT("[UInventorySystemComponent|%s][SwapItems]: AssetData is not valid. Unable to set TempCanStack value"), *GetFName().ToString());
			SwapItemSuccessDelegate.Broadcast(false, First, Second, bIsEquipment);
			bIsProcessing = false;
			return;
		}

		TArray<FPrimaryAssetId> SecondAssetEquipmentType{};
		FAssetDataTagMapSharedView::FFindTagResult SecondTagValue = SecondAssetData.TagsAndValues.FindTag(AssetRegistrySearchableEquipmentTypePropertyName);
		if (SecondTagValue.IsSet())
		{
			TArray<FString> AssetEquipmentTypeStrings{};
			UE_LOG(InventorySystem, Log, TEXT("[UInventorySystemComponent|%s][SwapItems]: %s"), *GetFName().ToString(), *SecondTagValue.GetValue());
			FString AssetEquipmentTypeBaseString = SecondTagValue.GetValue();
			AssetEquipmentTypeBaseString = AssetEquipmentTypeBaseString.Replace(TEXT("("), TEXT(""));
			AssetEquipmentTypeBaseString = AssetEquipmentTypeBaseString.Replace(TEXT(")"), TEXT(""));
			AssetEquipmentTypeBaseString = AssetEquipmentTypeBaseString.Replace(TEXT("\""), TEXT(""));
			AssetEquipmentTypeBaseString.ParseIntoArray(AssetEquipmentTypeStrings, TEXT(","));
			for (FString AssetEquipmentTypeString : AssetEquipmentTypeStrings)
			{
				SecondAssetEquipmentType.Add(FPrimaryAssetId(AssetEquipmentTypeString));
			}
		}

		if (SecondAssetEquipmentType.IsEmpty())
		{
			UE_LOG(InventorySystem, Error, TEXT("[UInventorySystemComponent|%s][SwapItems]: AssetData has no valid equipment type"), *GetFName().ToString());
			SwapItemSuccessDelegate.Broadcast(false, First, Second, bIsEquipment);
			bIsProcessing = false;
			return;
		}

		if (!SecondAssetEquipmentType.Contains(EquipmentTypes[RealFirstEquipmentTypeIndex]))
		{
			UE_LOG(InventorySystem, Warning, TEXT("[UInventorySystemComponent|%s][SwapItems]: AssetData equipment type is incorrect"), *GetFName().ToString());
			SwapItemSuccessDelegate.Broadcast(false, First, Second, bIsEquipment);
			bIsProcessing = false;
			return;
		}

		EquipmentIndices[SecondIndex] = First;
		if (RealSecondEquipmentStatsIndex != INDEX_NONE)
		{
			EquipmentDynamicStatsIndices[RealSecondEquipmentStatsIndex] = First;
		}

		SwapItemSuccessDelegate.Broadcast(true, First, Second, bIsEquipment);
		ChangedEquipmentSlotsDelegate.Broadcast({First, Second});
		bIsProcessing = false;
		return;
	}

	UE_LOG(InventorySystem, Error, TEXT("[UInventorySystemComponent|%s][SwapItems]: Items could not be swapped"), *GetFName().ToString());
	SwapItemSuccessDelegate.Broadcast(false, First, Second, bIsEquipment);
	bIsProcessing = false;
}

bool UInventorySystemComponent::PickUpItemDrop_Validate(AItemDrop* const& Item, const bool bCanStack)
{
	return true;
}

void UInventorySystemComponent::PickUpItemDrop_Implementation(AItemDrop* const& Item, const bool bCanStack)
{
	if (!IsValid(Item))
	{
		UE_LOG(InventorySystem, Error, TEXT("[UItemContainerComponent|%s][PickUpItemDrop]: Item invalid"), *GetFName().ToString());
		PickUpItemFailureDelegate.Broadcast(Item);
		return;
	}

	if (const AActor* Owner = GetOwner(); !IsValid(Owner) || !Owner->HasAuthority())
	{
		UE_LOG(InventorySystem, Error, TEXT("[UItemContainerComponent|%s][PickUpItemDrop]: Component owner has no authority"), *GetFName().ToString());
		PickUpItemFailureDelegate.Broadcast(Item);
		Item->AfterPickUpEvent(false);
		return;
	}

	if (bIsProcessing)
	{
		UE_LOG(InventorySystem, Error, TEXT("[UInventorySystemComponent|%s][PickUpItemDrop]: Component is still processing previous request"), *GetFName().ToString());
		PickUpItemFailureDelegate.Broadcast(Item);
		Item->AfterPickUpEvent(false);
		return;
	}

	bIsProcessing = true;

	TArray<int> ChangedSlots;
	const bool AllAdded = PickUpItemDropInternal(Item, bCanStack, ChangedSlots);
	if (ChangedSlots.IsEmpty())
	{
		PickUpItemFailureDelegate.Broadcast(Item);
		Item->AfterPickUpEvent(false);
		bIsProcessing = false;
		return;
	}

	if (!AllAdded)
	{
		UE_LOG(InventorySystem, Warning, TEXT("[UInventorySystemComponent|%s][PickUpItemDrop]: Part of the item was added. Not enough space to add all"), *GetFName().ToString());
		PickUpItemSuccessDelegate.Broadcast(Item, ChangedSlots);
		ChangedInventorySlotsDelegate.Broadcast(ChangedSlots);
		Item->AfterPickUpEvent(true);
		bIsProcessing = false;
		return;
	}

	PickUpItemSuccessDelegate.Broadcast(Item, ChangedSlots);
	ChangedInventorySlotsDelegate.Broadcast(ChangedSlots);
	Item->AfterPickUpEvent(true);
	bIsProcessing = false;
}

bool UInventorySystemComponent::PickUpItemDropInternal(AItemDrop* const& Item, const bool bCanStack, TArray<int>& ChangedSlots)
{
	if (!IsValid(Item) || Item->Amount <= 0 || !Item->InventoryAsset.IsValid() || Item->InventoryAsset == FPrimaryAssetId())
	{
		UE_LOG(InventorySystem, Error, TEXT("[UInventorySystemComponent|%s][PickUpItemDrop]: Item data or amount invalid"), *GetFName().ToString());
		return false;
	}

	bool TempCanStack = false;
	UAssetManager& Manager = UAssetManager::Get();
	if (!Manager.IsValid())
	{
		UE_LOG(InventorySystem, Error, TEXT("[UInventorySystemComponent|%s][PickUpItemDrop]: AssetManager is not initialized. Unable to set TempCanStack value"), *GetFName().ToString());
		return false;
	}

	FAssetData AssetData;
	Manager.GetPrimaryAssetData(Item->InventoryAsset, AssetData);
	if (!AssetData.IsValid())
	{
		UE_LOG(InventorySystem, Error, TEXT("[UInventorySystemComponent|%s][PickUpItemDrop]: AssetData is not valid. Unable to set TempCanStack value"), *GetFName().ToString());
		return false;
	}

	const FName AssetRegistrySearchablePropertyName = GET_MEMBER_NAME_CHECKED(UItemDataAsset, bCanStack);
	AssetData.GetTagValue(AssetRegistrySearchablePropertyName, TempCanStack);

	int Index = INDEX_NONE;
	if (bCanStack && TempCanStack)
	{
		bool bCanMerge = false;
		int Amount = 0;
		FindItemStack(Item->InventoryAsset, Index, Amount, bCanMerge, Item->DynamicStats);
		if (bCanMerge)
		{
			const int ItemsLeft = Amount + Item->Amount - GetStackSizeConfig();
			ChangedSlots.Add(InventoryIndices[Index]);
			if (ItemsLeft > 0)
			{
				// Try to add item again to ensure all items are taken
				Item->Amount = ItemsLeft;
				InventoryAmounts[Index] = GetStackSizeConfig();
				return PickUpItemDropInternal(Item, bCanStack, ChangedSlots);
			}

			InventoryAmounts[Index] = Amount + Item->Amount;
			Item->Amount = 0;
			return true;
		}
	}

	bool bSuccess = false;
	FindNextEmptySlot(Index, bSuccess);
	if (bSuccess)
	{
		const int ItemsLeft = Item->Amount - GetStackSizeConfig();
		InventoryIndices.Add(Index);
		InventoryAssets.Add(Item->InventoryAsset);
		ChangedSlots.Add(Index);
		if (!Item->DynamicStats.ItemProperties.IsEmpty())
		{
			InventoryDynamicStatsIndices.Add(Index);
			InventoryDynamicStats.Add(Item->DynamicStats);
		}

		if (ItemsLeft > 0)
		{
			// Try to add item again to ensure all items are taken
			InventoryAmounts.Add(GetStackSizeConfig());
			Item->Amount = ItemsLeft;
			return PickUpItemDropInternal(Item, bCanStack, ChangedSlots);
		}

		InventoryAmounts.Add(Item->Amount);
		Item->Amount = 0;
		return true;
	}

	UE_LOG(InventorySystem, Error, TEXT("[UInventorySystemComponent|%s][PickUpItemDrop]: Item could not be added. Unexpected behavior"), *GetFName().ToString());
	return false;
}

bool UInventorySystemComponent::AddItemToEquipmentSlot_Validate(const FPrimaryAssetId InventoryAsset, const int EquipmentSlot, const FItemProperties DynamicStats, int Amount, const bool bCanUnequippedItemStack, const bool bCanStack)
{
	return true;
}

void UInventorySystemComponent::AddItemToEquipmentSlot_Implementation(const FPrimaryAssetId InventoryAsset, const int EquipmentSlot, const FItemProperties DynamicStats, int Amount, const bool bCanUnequippedItemStack, const bool bCanStack)
{
	if (const AActor* Owner = GetOwner(); !IsValid(Owner) || !Owner->HasAuthority())
	{
		UE_LOG(InventorySystem, Error, TEXT("[UItemContainerComponent|%s][AddItemToEquipmentSlot]: Component owner has no authority"), *GetFName().ToString());
		AddItemToEquipmentSlotFailureDelegate.Broadcast(InventoryAsset, EquipmentSlot, DynamicStats, Amount);
		return;
	}

	if (bIsProcessing)
	{
		UE_LOG(InventorySystem, Error, TEXT("[UInventorySystemComponent|%s][AddItemToEquipmentSlot]: Component is still processing previous request"), *GetFName().ToString());
		AddItemToEquipmentSlotFailureDelegate.Broadcast(InventoryAsset, EquipmentSlot, DynamicStats, Amount);
		return;
	}

	bIsProcessing = true;

	const int RealEquipmentTypeIndicesIndex = EquipmentTypeIndices.Find(EquipmentSlot);
	if (!InventoryAsset.IsValid() || InventoryAsset == FPrimaryAssetId() || Amount <= 0 || RealEquipmentTypeIndicesIndex == INDEX_NONE || !EquipmentTypes.IsValidIndex(RealEquipmentTypeIndicesIndex) || !EquipmentTypes[
		RealEquipmentTypeIndicesIndex].IsValid() || EquipmentTypes[RealEquipmentTypeIndicesIndex] == FPrimaryAssetId())
	{
		UE_LOG(InventorySystem, Error, TEXT("[UInventorySystemComponent|%s][AddItemToEquipmentSlot]: Invalid InventoryAsset, EquipmentType data or amount is out of range"), *GetFName().ToString());
		AddItemToEquipmentSlotFailureDelegate.Broadcast(InventoryAsset, EquipmentSlot, DynamicStats, Amount);
		bIsProcessing = false;
		return;
	}

	// Reset Amount to max item stack size if amount was more then allowed
	int NewAmount = Amount > GetEquipmentStackSizeConfig() ? GetEquipmentStackSizeConfig() : Amount;
	TArray<int> ChangedSlots;

	bool TempCanStack = false;
	TArray<FPrimaryAssetId> AssetEquipmentType{};
	UAssetManager& Manager = UAssetManager::Get();
	if (!Manager.IsValid())
	{
		UE_LOG(InventorySystem, Error, TEXT("[UInventorySystemComponent|%s][AddItemToEquipmentSlot]: AssetManager is not initialized. Unable to set TempCanStack value"), *GetFName().ToString());
		AddItemToEquipmentSlotFailureDelegate.Broadcast(InventoryAsset, EquipmentSlot, DynamicStats, Amount);
		bIsProcessing = false;
		return;
	}

	FAssetData AssetData;
	Manager.GetPrimaryAssetData(InventoryAsset, AssetData);
	if (!AssetData.IsValid())
	{
		UE_LOG(InventorySystem, Error, TEXT("[UInventorySystemComponent|%s][AddItemToEquipmentSlot]: AssetData is not valid. Unable to set TempCanStack value"), *GetFName().ToString());
		AddItemToEquipmentSlotFailureDelegate.Broadcast(InventoryAsset, EquipmentSlot, DynamicStats, Amount);
		bIsProcessing = false;
		return;
	}

	const FName AssetRegistrySearchablePropertyName = GET_MEMBER_NAME_CHECKED(UItemDataAsset, bCanStack);
	const FName AssetRegistrySearchableEquipmentTypePropertyName = GET_MEMBER_NAME_CHECKED(UItemEquipmentDataAsset, EquipmentType);
	AssetData.GetTagValue(AssetRegistrySearchablePropertyName, TempCanStack);

	FAssetDataTagMapSharedView::FFindTagResult TagValue = AssetData.TagsAndValues.FindTag(AssetRegistrySearchableEquipmentTypePropertyName);
	if (TagValue.IsSet())
	{
		TArray<FString> AssetEquipmentTypeStrings{};
		UE_LOG(InventorySystem, Log, TEXT("[UInventorySystemComponent|%s][AddItemToEquipmentSlot]: %s"), *GetFName().ToString(), *TagValue.GetValue());
		FString AssetEquipmentTypeBaseString = ReplaceEquipmentArrayString(TagValue.GetValue());
		AssetEquipmentTypeBaseString.ParseIntoArray(AssetEquipmentTypeStrings, TEXT(","));
		for (FString AssetEquipmentTypeString : AssetEquipmentTypeStrings)
		{
			AssetEquipmentType.Add(FPrimaryAssetId(AssetEquipmentTypeString));
		}
	}

	if (AssetEquipmentType.IsEmpty())
	{
		UE_LOG(InventorySystem, Error, TEXT("[UInventorySystemComponent|%s][AddItemToEquipmentSlot]: AsseData has no valid equipment type"), *GetFName().ToString());
		AddItemToEquipmentSlotFailureDelegate.Broadcast(InventoryAsset, EquipmentSlot, DynamicStats, Amount);
		bIsProcessing = false;
		return;
	}

	if (!AssetEquipmentType.Contains(EquipmentTypes[RealEquipmentTypeIndicesIndex]))
	{
		UE_LOG(InventorySystem, Warning, TEXT("[UInventorySystemComponent|%s][AddItemToEquipmentSlot]: AssetData equipment type is incorrect"), *GetFName().ToString());
		AddItemToEquipmentSlotFailureDelegate.Broadcast(InventoryAsset, EquipmentSlot, DynamicStats, Amount);
		bIsProcessing = false;
		return;
	}

	if (const int RealEquipmentIndex = EquipmentIndices.Find(EquipmentSlot); RealEquipmentIndex != INDEX_NONE)
	{
		if (bCanStack && EquipmentAssets[RealEquipmentIndex] == InventoryAsset)
		{
			if (TempCanStack)
			{
				if (EquipmentAmounts[RealEquipmentIndex] == GetEquipmentStackSizeConfig())
				{
					// Success
					int ItemAmount = Amount;
					ChangedSlots.Append(AddItemInternal(InventoryAsset, DynamicStats, ItemAmount, bCanStack, false));
					if (ItemAmount > 0)
					{
						UE_LOG(InventorySystem, Warning, TEXT("[UInventorySystemComponent|%s][AddItemToEquipmentSlot]: Overflow of %d. The rest of the items was not used"), *GetFName().ToString(), ItemAmount);	
					}
					
					AddItemToEquipmentSlotSuccessDelegate.Broadcast(EquipmentSlot, ChangedSlots, ItemAmount);
					ChangedEquipmentSlotsDelegate.Broadcast({EquipmentSlot});
					ChangedInventorySlotsDelegate.Broadcast(ChangedSlots);
					bIsProcessing = false;
					return;
				}
				
				const int RealEquipmentDynamicStatsIndex = EquipmentDynamicStatsIndices.Find(EquipmentSlot);
				if ((RealEquipmentDynamicStatsIndex != INDEX_NONE && EquipmentDynamicStats[RealEquipmentDynamicStatsIndex] == DynamicStats) || (DynamicStats.ItemProperties.IsEmpty() && RealEquipmentDynamicStatsIndex == INDEX_NONE))
				{
					const int ClampedAmount = FMath::Clamp(EquipmentAmounts[RealEquipmentIndex] + Amount, 1, GetEquipmentStackSizeConfig());
					int Overflow = EquipmentAmounts[RealEquipmentIndex] + Amount - GetEquipmentStackSizeConfig(); 
					EquipmentAmounts[RealEquipmentIndex] = ClampedAmount;
					
					if (Overflow > 0)
					{
						ChangedSlots.Append(AddItemInternal(InventoryAsset, DynamicStats, Overflow, bCanStack, false));
			
						if (Overflow > 0)
						{
							UE_LOG(InventorySystem, Warning, TEXT("[UInventorySystemComponent|%s][AddItemToEquipmentSlot]: Overflow of %d. The rest of the items was not used"), *GetFName().ToString(), Overflow);	
						}
					}

					// Success
					AddItemToEquipmentSlotSuccessDelegate.Broadcast(EquipmentSlot, ChangedSlots, Overflow);
					ChangedEquipmentSlotsDelegate.Broadcast({EquipmentSlot});
					ChangedInventorySlotsDelegate.Broadcast(ChangedSlots);
					bIsProcessing = false;
					return;
				}
			}
		}

		bool EquippedTempCanStack = false;
		if (bCanUnequippedItemStack)
		{
			FAssetData EquippedAssetData;
			Manager.GetPrimaryAssetData(EquipmentAssets[RealEquipmentIndex], EquippedAssetData);
			if (!EquippedAssetData.IsValid())
			{
				UE_LOG(InventorySystem, Error, TEXT("[UInventorySystemComponent|%s][AddItemToEquipmentSlot]: EquippedAssetData is not valid. Unable to set EquippedTempCanStack value"), *GetFName().ToString());
				AddItemToEquipmentSlotFailureDelegate.Broadcast(InventoryAsset, EquipmentSlot, DynamicStats, Amount);
				bIsProcessing = false;
				return;
			}
			
			EquippedAssetData.GetTagValue(AssetRegistrySearchablePropertyName, EquippedTempCanStack);
			if (EquippedTempCanStack)
			{
				FItemProperties EquippedEquipmentDynamicStats;
				if (const int RealEquipmentDynamicStatsIndex = EquipmentDynamicStatsIndices.Find(EquipmentSlot); RealEquipmentDynamicStatsIndex != INDEX_NONE)
				{
					if (!EquipmentDynamicStats.IsValidIndex(RealEquipmentDynamicStatsIndex))
					{
						UE_LOG(InventorySystem, Error, TEXT("[UInventorySystemComponent|%s][AddItemToEquipmentSlot]: EquipmentDynamicStats is not filled but has an EquipmentDynamicStatsIndices entry"), *GetFName().ToString());
						AddItemToEquipmentSlotFailureDelegate.Broadcast(InventoryAsset, EquipmentSlot, DynamicStats, Amount);
						bIsProcessing = false;
						return;
					}

					EquippedEquipmentDynamicStats = EquipmentDynamicStats[RealEquipmentDynamicStatsIndex];
				}
	
				const TArray<int> UnequipChangeSlots = AddItemInternal(EquipmentAssets[RealEquipmentIndex], EquippedEquipmentDynamicStats, EquipmentAmounts[RealEquipmentIndex], bCanUnequippedItemStack, true);
				if (UnequipChangeSlots.IsEmpty())
				{
					UE_LOG(InventorySystem, Error, TEXT("[UInventorySystemComponent|%s][AddItemToEquipmentSlot]: Equipment could not be added. Slot is full and already equipped item could not be unequipped"), *GetFName().ToString());
					AddItemToEquipmentSlotFailureDelegate.Broadcast(InventoryAsset, EquipmentSlot, DynamicStats, Amount);
					bIsProcessing = false;
					return;
				}

				ChangedSlots.Append(UnequipChangeSlots);
			}
		}

		if (!EquippedTempCanStack)
		{
			bool bSuccess = false;
			// Use default can not stack behavior
			int FoundSlot = INDEX_NONE;
			FindNextEmptySlot(FoundSlot, bSuccess);

			if (!bSuccess)
			{
				UE_LOG(InventorySystem, Error, TEXT("[UInventorySystemComponent|%s][AddItemToEquipmentSlot]: Equipment could not be added. Slot is full and already equipped item could not be unequipped"), *GetFName().ToString());
				AddItemToEquipmentSlotFailureDelegate.Broadcast(InventoryAsset, EquipmentSlot, DynamicStats, Amount);
				bIsProcessing = false;
				return;
			}
			
			// Unequip item to new slot
			if (const int RealEquipmentDynamicStatsIndicesIndex = EquipmentDynamicStatsIndices.Find(EquipmentSlot); RealEquipmentDynamicStatsIndicesIndex != INDEX_NONE)
			{
				if (!EquipmentDynamicStats.IsValidIndex(RealEquipmentDynamicStatsIndicesIndex))
				{
					UE_LOG(InventorySystem, Error, TEXT("[UInventorySystemComponent|%s][AddItemToEquipmentSlot]: EquipmentDynamicStats is not filled but has an EquipmentDynamicStatsIndices entry"), *GetFName().ToString());
					AddItemToEquipmentSlotFailureDelegate.Broadcast(InventoryAsset, EquipmentSlot, DynamicStats, Amount);
					bIsProcessing = false;
					return;
				}

				if (const int NewInventoryDynamicStatsIndex = InventoryDynamicStatsIndices.AddUnique(FoundSlot); InventoryDynamicStats.IsValidIndex(NewInventoryDynamicStatsIndex))
				{
					UE_LOG(InventorySystem, Error, TEXT("[UInventorySystemComponent|%s][AddItemToEquipmentSlot]: InventoryDynamicStats should not be filled. Index was just created"), *GetFName().ToString());
					// Revert back
					InventoryDynamicStatsIndices.RemoveAt(NewInventoryDynamicStatsIndex);
					AddItemToEquipmentSlotFailureDelegate.Broadcast(InventoryAsset, EquipmentSlot, DynamicStats, Amount);
					bIsProcessing = false;
					return;
				}

				InventoryDynamicStats.Add(EquipmentDynamicStats[RealEquipmentDynamicStatsIndicesIndex]);
			}

			InventoryIndices.AddUnique(FoundSlot);
			InventoryAmounts.Add(EquipmentAmounts[RealEquipmentIndex]);
			InventoryAssets.Add(EquipmentAssets[RealEquipmentIndex]);
			ChangedSlots.Add(FoundSlot);
		}

		// Equip if exits
		if (const int RealEquipmentDynamicStatsIndicesIndex = EquipmentDynamicStatsIndices.Find(EquipmentSlot); RealEquipmentDynamicStatsIndicesIndex != INDEX_NONE)
		{
			if (!EquipmentDynamicStats.IsValidIndex(RealEquipmentDynamicStatsIndicesIndex))
			{
				UE_LOG(InventorySystem, Error, TEXT("[UInventorySystemComponent|%s][AddItemToEquipmentSlot]: EquipmentDynamicStats is not filled but has an EquipmentDynamicStatsIndices entry"), *GetFName().ToString());
				AddItemToEquipmentSlotFailureDelegate.Broadcast(InventoryAsset, EquipmentSlot, DynamicStats, Amount);
				ChangedInventorySlotsDelegate.Broadcast(ChangedSlots);
				bIsProcessing = false;
				return;
			}

			if (DynamicStats.ItemProperties.IsEmpty())
			{
				EquipmentDynamicStatsIndices.RemoveAt(RealEquipmentDynamicStatsIndicesIndex);
				EquipmentDynamicStats.RemoveAt(RealEquipmentDynamicStatsIndicesIndex);
			}
			else
			{
				EquipmentDynamicStats[RealEquipmentDynamicStatsIndicesIndex] = DynamicStats;
			}
		}
		else
		{
			if (!DynamicStats.ItemProperties.IsEmpty())
			{
				if (const int NewEquipmentDynamicStatsIndex = EquipmentDynamicStatsIndices.AddUnique(EquipmentSlot); EquipmentDynamicStats.IsValidIndex(NewEquipmentDynamicStatsIndex))
				{
					UE_LOG(InventorySystem, Error, TEXT("[UInventorySystemComponent|%s][AddItemToEquipmentSlot]: EquipmentDynamicStats should not be filled. Index was just created"), *GetFName().ToString());
					// Revert back
					EquipmentDynamicStatsIndices.RemoveAt(NewEquipmentDynamicStatsIndex);
					AddItemToEquipmentSlotFailureDelegate.Broadcast(InventoryAsset, EquipmentSlot, DynamicStats, Amount);
					bIsProcessing = false;
					return;
				}

				EquipmentDynamicStats.Add(DynamicStats);
			}
		}

		EquipmentAssets[RealEquipmentIndex] = InventoryAsset;
		EquipmentAmounts[RealEquipmentIndex] = NewAmount;

		// Success
		int ItemAmount = Amount - NewAmount;
		if (ItemAmount > 0)
		{
			ChangedSlots.Append(AddItemInternal(InventoryAsset, DynamicStats, ItemAmount, bCanStack, false));
			
			if (ItemAmount > 0)
			{
				UE_LOG(InventorySystem, Warning, TEXT("[UInventorySystemComponent|%s][AddItemToEquipmentSlot]: Overflow of %d. The rest of the items was not used"), *GetFName().ToString(), ItemAmount);	
			}
		}
		
		AddItemToEquipmentSlotSuccessDelegate.Broadcast(EquipmentSlot, ChangedSlots, ItemAmount);
		ChangedInventorySlotsDelegate.Broadcast(ChangedSlots);
		ChangedEquipmentSlotsDelegate.Broadcast({EquipmentSlot});
		bIsProcessing = false;
		return;
	}

	// Equip item here - Create new
	if (!DynamicStats.ItemProperties.IsEmpty())
	{
		if (const int NewEquipmentDynamicStatsIndex = EquipmentDynamicStatsIndices.AddUnique(EquipmentSlot); EquipmentDynamicStats.IsValidIndex(NewEquipmentDynamicStatsIndex))
		{
			UE_LOG(InventorySystem, Error, TEXT("[UInventorySystemComponent|%s][AddItemToEquipmentSlot]: EquipmentDynamicStats should not be filled. Index was just created"), *GetFName().ToString());
			// Revert back
			EquipmentDynamicStatsIndices.RemoveAt(NewEquipmentDynamicStatsIndex);
			AddItemToEquipmentSlotFailureDelegate.Broadcast(InventoryAsset, EquipmentSlot, DynamicStats, Amount);
			ChangedInventorySlotsDelegate.Broadcast(ChangedSlots);
			bIsProcessing = false;
			return;
		}

		EquipmentDynamicStats.Add(DynamicStats);
	}

	EquipmentIndices.AddUnique(EquipmentSlot);
	EquipmentAmounts.Add(Amount);
	EquipmentAssets.Add(InventoryAsset);

	// Success
	int ItemAmount = Amount - NewAmount;
	if (ItemAmount > 0)
	{
		ChangedSlots.Append(AddItemInternal(InventoryAsset, DynamicStats, ItemAmount, bCanStack, false));
			
		if (ItemAmount > 0)
		{
			UE_LOG(InventorySystem, Warning, TEXT("[UInventorySystemComponent|%s][AddItemToEquipmentSlot]: Overflow of %d. The rest of the items was not used"), *GetFName().ToString(), ItemAmount);	
		}
	}
	
	AddItemToEquipmentSlotSuccessDelegate.Broadcast(EquipmentSlot, ChangedSlots, ItemAmount);
	ChangedEquipmentSlotsDelegate.Broadcast({EquipmentSlot});
	ChangedInventorySlotsDelegate.Broadcast(ChangedSlots);
	bIsProcessing = false;
}

bool UInventorySystemComponent::RemoveEquipmentAmountFromSlot_Validate(const int EquipmentSlot, const int Amount)
{
	return true;
}

void UInventorySystemComponent::RemoveEquipmentAmountFromSlot_Implementation(const int EquipmentSlot, const int Amount)
{
	UAssetManager& Manager = UAssetManager::Get();
	const FName AssetRegistrySearchableEquipmentTypePropertyName = GET_MEMBER_NAME_CHECKED(UItemEquipmentDataAsset, EquipmentType);

	if (!Manager.IsValid())
	{
		UE_LOG(InventorySystem, Error, TEXT("[UInventorySystemComponent|%s][RemoveEquipmentAmountFromSlot]: AssetManager is not initialized"), *GetFName().ToString());
		return;
	}

	TArray<FPrimaryAssetId> NewEquipmentTypes;

	if (const AActor* Owner = GetOwner(); !IsValid(Owner) || !Owner->HasAuthority())
	{
		UE_LOG(InventorySystem, Error, TEXT("[UInventorySystemComponent|%s][RemoveEquipmentAmountFromSlot]: Component owner has no authority"), *GetFName().ToString());
		RemoveEquipmentAmountFromSlotSuccessDelegate.Broadcast(false, FEquipmentSlot{{}, EquipmentSlot, FPrimaryAssetId{}, FItemProperties{}, -1}, Amount);
		return;
	}

	if (bIsProcessing)
	{
		UE_LOG(InventorySystem, Error, TEXT("[UInventorySystemComponent|%s][RemoveEquipmentAmountFromSlot]: Component is still processing previous request"), *GetFName().ToString());
		RemoveEquipmentAmountFromSlotSuccessDelegate.Broadcast(false, FEquipmentSlot{{}, EquipmentSlot, FPrimaryAssetId{}, FItemProperties{}, -1}, Amount);
		return;
	}

	bIsProcessing = true;

	const int RealEquipmentIndex = EquipmentIndices.Find(EquipmentSlot);
	if (Amount <= 0 || Amount > GetEquipmentStackSizeConfig() || RealEquipmentIndex == INDEX_NONE || !EquipmentAmounts.IsValidIndex(RealEquipmentIndex))
	{
		UE_LOG(InventorySystem, Error, TEXT("[UInventorySystemComponent|%s][RemoveEquipmentAmountFromSlot]: Equipment data invalid for slot %d"), *GetFName().ToString(), EquipmentSlot);
		RemoveEquipmentAmountFromSlotSuccessDelegate.Broadcast(false, FEquipmentSlot{{}, EquipmentSlot, FPrimaryAssetId{}, FItemProperties{}, -1}, Amount);
		bIsProcessing = false;
		return;
	}

	const int NewAmount = EquipmentAmounts[RealEquipmentIndex] - Amount;

	if (NewAmount < 0)
	{
		UE_LOG(InventorySystem, Error, TEXT("[UInventorySystemComponent|%s][RemoveEquipmentAmountFromSlot]: New amount is smaller then 0. Aborting action"), *GetFName().ToString());
		RemoveEquipmentAmountFromSlotSuccessDelegate.Broadcast(false, FEquipmentSlot{{}, EquipmentSlot, FPrimaryAssetId{}, FItemProperties{}, -1}, Amount);
		bIsProcessing = false;
		return;
	}

	TArray<FPrimaryAssetId> TempEquipmentTypes;
	const int TempAmount = EquipmentAmounts[RealEquipmentIndex];
	const FPrimaryAssetId TempAsset = EquipmentAssets[RealEquipmentIndex];
	int RealEquipmentStatsIndex = INDEX_NONE;
	FItemProperties TempDynamicStats;
	if (RealEquipmentStatsIndex = EquipmentDynamicStatsIndices.Find(EquipmentSlot); RealEquipmentStatsIndex != INDEX_NONE)
	{
		if (!EquipmentDynamicStats.IsValidIndex(RealEquipmentStatsIndex))
		{
			UE_LOG(InventorySystem, Error, TEXT("[UInventorySystemComponent|%s][RemoveEquipmentAmountFromSlot]: EquipmentDynamicStats is not filled but has an EquipmentDynamicStatsIndices entry"), *GetFName().ToString());
			RemoveEquipmentAmountFromSlotSuccessDelegate.Broadcast(false, FEquipmentSlot{{}, EquipmentSlot, FPrimaryAssetId{}, FItemProperties{}, -1}, Amount);
			bIsProcessing = false;
			return;
		}

		TempDynamicStats = EquipmentDynamicStats[RealEquipmentStatsIndex];
	}

	FAssetData CurrentItemData;
	Manager.GetPrimaryAssetData(EquipmentAssets[RealEquipmentIndex], CurrentItemData);

	FAssetDataTagMapSharedView::FFindTagResult EquipmentTypesTagValue = CurrentItemData.TagsAndValues.FindTag(AssetRegistrySearchableEquipmentTypePropertyName);

	if (EquipmentTypesTagValue.IsSet())
	{
		TArray<FString> AssetEquipmentTypeStrings{};
		UE_LOG(InventorySystem, Log, TEXT("[UInventorySystemComponent|%s][GetEquipmentSlot]: %s"), *GetFName().ToString(), *EquipmentTypesTagValue.GetValue());
		FString AssetEquipmentTypeBaseString = ReplaceEquipmentArrayString(EquipmentTypesTagValue.GetValue());
		AssetEquipmentTypeBaseString.ParseIntoArray(AssetEquipmentTypeStrings, TEXT(","));
		for (FString AssetEquipmentTypeString : AssetEquipmentTypeStrings)
		{
			TempEquipmentTypes.Add(FPrimaryAssetId(AssetEquipmentTypeString));
		}
	}

	if (NewAmount == 0)
	{
		if (RealEquipmentStatsIndex != INDEX_NONE)
		{
			EquipmentDynamicStatsIndices.RemoveAt(RealEquipmentStatsIndex);
			EquipmentDynamicStats.RemoveAt(RealEquipmentStatsIndex);
		}

		EquipmentAmounts.RemoveAt(RealEquipmentIndex);
		EquipmentAssets.RemoveAt(RealEquipmentIndex);

		EquipmentIndices.RemoveAt(RealEquipmentIndex);
		RemoveEquipmentAmountFromSlotSuccessDelegate.Broadcast(true, FEquipmentSlot{TempEquipmentTypes, EquipmentSlot, TempAsset, TempDynamicStats, TempAmount}, Amount);
		ChangedEquipmentSlotsDelegate.Broadcast({EquipmentSlot});
		bIsProcessing = false;
		return;
	}

	EquipmentAmounts[RealEquipmentIndex] = NewAmount;

	RemoveEquipmentAmountFromSlotSuccessDelegate.Broadcast(true, FEquipmentSlot{TempEquipmentTypes, EquipmentSlot, TempAsset, TempDynamicStats, TempAmount}, Amount);
	ChangedEquipmentSlotsDelegate.Broadcast({EquipmentSlot});
	bIsProcessing = false;
}

bool UInventorySystemComponent::ItemEquipFromInventory_Validate(const int Slot, const int EquipmentSlot, const bool bCanUnequippedItemStack, const bool bCanStack)
{
	return true;
}

void UInventorySystemComponent::ItemEquipFromInventory_Implementation(const int Slot, const int EquipmentSlot, const bool bCanUnequippedItemStack, const bool bCanStack)
{
	UAssetManager& Manager = UAssetManager::Get();
	if (!Manager.IsValid())
	{
		UE_LOG(InventorySystem, Error, TEXT("[UInventorySystemComponent|%s][ItemEquipFromInventory]: AssetManager is not initialized"), *GetFName().ToString());
		ItemEquipFromInventorySuccessDelegate.Broadcast(false, EquipmentSlot, Slot);
		bIsProcessing = false;
		return;
	}

	if (const AActor* Owner = GetOwner(); !IsValid(Owner) || !Owner->HasAuthority())
	{
		UE_LOG(InventorySystem, Error, TEXT("[UItemContainerComponent|%s][ItemEquipFromInventory]: Component owner has no authority"), *GetFName().ToString());
		ItemEquipFromInventorySuccessDelegate.Broadcast(false, EquipmentSlot, Slot);
		return;
	}

	if (bIsProcessing)
	{
		UE_LOG(InventorySystem, Error, TEXT("[UInventorySystemComponent|%s][ItemEquipFromInventory]: Component is still processing previous request"), *GetFName().ToString());
		ItemEquipFromInventorySuccessDelegate.Broadcast(false, EquipmentSlot, Slot);
		return;
	}

	bIsProcessing = true;

	const int RealIndex = InventoryIndices.Find(Slot);
	if (RealIndex == INDEX_NONE)
	{
		UE_LOG(InventorySystem, Error, TEXT("[UInventorySystemComponent|%s][ItemEquipFromInventory]: Invalid item or EquipmentType data"), *GetFName().ToString());
		ItemEquipFromInventorySuccessDelegate.Broadcast(false, EquipmentSlot, Slot);
		bIsProcessing = false;
		return;
	}

	bool TempCanStack = false;
	TArray<FPrimaryAssetId> AssetEquipmentType{};
	FAssetData AssetData;
	Manager.GetPrimaryAssetData(InventoryAssets[RealIndex], AssetData);
	if (!AssetData.IsValid())
	{
		UE_LOG(InventorySystem, Error, TEXT("[UInventorySystemComponent|%s][ItemEquipFromInventory]: AssetData is not valid. Unable to set TempCanStack value"), *GetFName().ToString());
		ItemEquipFromInventorySuccessDelegate.Broadcast(false, EquipmentSlot, Slot);
		bIsProcessing = false;
		return;
	}

	const FName AssetRegistrySearchablePropertyName = GET_MEMBER_NAME_CHECKED(UItemDataAsset, bCanStack);
	const FName AssetRegistrySearchableEquipmentTypePropertyName = GET_MEMBER_NAME_CHECKED(UItemEquipmentDataAsset, EquipmentType);
	AssetData.GetTagValue(AssetRegistrySearchablePropertyName, TempCanStack);

	FAssetDataTagMapSharedView::FFindTagResult TagValue = AssetData.TagsAndValues.FindTag(AssetRegistrySearchableEquipmentTypePropertyName);
	if (TagValue.IsSet())
	{
		TArray<FString> AssetEquipmentTypeStrings{};
		UE_LOG(InventorySystem, Log, TEXT("[UInventorySystemComponent|%s][ItemEquipFromInventory]: %s"), *GetFName().ToString(), *TagValue.GetValue());
		FString AssetEquipmentTypeBaseString = ReplaceEquipmentArrayString(TagValue.GetValue());
		AssetEquipmentTypeBaseString.ParseIntoArray(AssetEquipmentTypeStrings, TEXT(","));
		for (FString AssetEquipmentTypeString : AssetEquipmentTypeStrings)
		{
			AssetEquipmentType.Add(FPrimaryAssetId(AssetEquipmentTypeString));
		}
	}

	if (AssetEquipmentType.IsEmpty())
	{
		UE_LOG(InventorySystem, Error, TEXT("[UInventorySystemComponent|%s][ItemEquipFromInventory]: AssetData has no valid equipment type"), *GetFName().ToString());
		ItemEquipFromInventorySuccessDelegate.Broadcast(false, EquipmentSlot, Slot);
		bIsProcessing = false;
		return;
	}

	int RealEquipmentSlot = EquipmentSlot;
	int CreatedEquipmentIndicesIndex = INDEX_NONE;
	if (EquipmentSlot == INDEX_NONE)
	{
		bool bHasValidEquipmentType = false;
		for (int I = 0; I < EquipmentTypes.Num(); I++)
		{
			// Found first slot this item can be equipped to
			if (EquipmentTypes[I] != FPrimaryAssetId{} && AssetEquipmentType.Contains(EquipmentTypes[I]))
			{
				if (!EquipmentIndices.Contains(EquipmentTypeIndices[I]))
				{
					CreatedEquipmentIndicesIndex = EquipmentIndices.AddUnique(EquipmentTypeIndices[I]);
					EquipmentAmounts.Add(1);
					EquipmentAssets.Add(FPrimaryAssetId{});
				}

				RealEquipmentSlot = EquipmentTypeIndices[I];
				bHasValidEquipmentType = true;
				break;
			}
		}

		// None found
		if (!bHasValidEquipmentType)
		{
			UE_LOG(InventorySystem, Warning, TEXT("[UInventorySystemComponent|%s][ItemEquipFromInventory]: No valid equipment slot of any type found"), *GetFName().ToString());
			ItemEquipFromInventorySuccessDelegate.Broadcast(false, EquipmentSlot, Slot);
			bIsProcessing = false;
			return;
		}
	}

	int RealEquipmentIndex = EquipmentIndices.Find(RealEquipmentSlot);
	const int RealEquipmentTypeIndex = EquipmentTypeIndices.Find(RealEquipmentSlot);
	TArray ChangedSlots = {Slot};
	if (RealEquipmentTypeIndex == INDEX_NONE || !EquipmentTypes.IsValidIndex(RealEquipmentTypeIndex) || !EquipmentTypes[RealEquipmentTypeIndex].IsValid() || EquipmentTypes[RealEquipmentTypeIndex] == FPrimaryAssetId() || !InventoryAmounts.IsValidIndex(RealIndex) || !InventoryAssets.IsValidIndex(RealIndex))
	{
		UE_LOG(InventorySystem, Error, TEXT("[UInventorySystemComponent|%s][ItemEquipFromInventory]: Invalid item or EquipmentType data"), *GetFName().ToString());
		if (CreatedEquipmentIndicesIndex != INDEX_NONE)
		{
			EquipmentAmounts.RemoveAt(CreatedEquipmentIndicesIndex);
			EquipmentAssets.RemoveAt(CreatedEquipmentIndicesIndex);
			EquipmentIndices.RemoveAt(CreatedEquipmentIndicesIndex);
		}
		ItemEquipFromInventorySuccessDelegate.Broadcast(false, RealEquipmentSlot, Slot);
		bIsProcessing = false;
		return;
	}

	if (!AssetEquipmentType.Contains(EquipmentTypes[RealEquipmentTypeIndex]))
	{
		UE_LOG(InventorySystem, Warning, TEXT("[UInventorySystemComponent|%s][ItemEquipFromInventory]: AssetData equipment type is incorrect"), *GetFName().ToString());
		if (CreatedEquipmentIndicesIndex != INDEX_NONE)
		{
			EquipmentAmounts.RemoveAt(CreatedEquipmentIndicesIndex);
			EquipmentAssets.RemoveAt(CreatedEquipmentIndicesIndex);
			EquipmentIndices.RemoveAt(CreatedEquipmentIndicesIndex);
		}
		ItemEquipFromInventorySuccessDelegate.Broadcast(false, EquipmentSlot, Slot);
		bIsProcessing = false;
		return;
	}

	const int FoundInventoryDynamicStatsIndex = InventoryDynamicStatsIndices.Find(Slot);
	const int FoundEquipmentDynamicStatsIndex = EquipmentDynamicStatsIndices.Find(RealEquipmentSlot);

	if (FoundInventoryDynamicStatsIndex != INDEX_NONE && !InventoryDynamicStats.IsValidIndex(FoundInventoryDynamicStatsIndex))
	{
		UE_LOG(InventorySystem, Error, TEXT("[UInventorySystemComponent|%s][ItemEquipFromInventory]: InventoryDynamicStats is not filled but has an InventoryDynamicStatsIndices entry"), *GetFName().ToString());
		if (CreatedEquipmentIndicesIndex != INDEX_NONE)
		{
			EquipmentAmounts.RemoveAt(CreatedEquipmentIndicesIndex);
			EquipmentAssets.RemoveAt(CreatedEquipmentIndicesIndex);
			EquipmentIndices.RemoveAt(CreatedEquipmentIndicesIndex);
		}
		ItemEquipFromInventorySuccessDelegate.Broadcast(false, RealEquipmentSlot, Slot);
		bIsProcessing = false;
		return;
	}

	if (FoundEquipmentDynamicStatsIndex != INDEX_NONE && !EquipmentDynamicStats.IsValidIndex(FoundEquipmentDynamicStatsIndex))
	{
		UE_LOG(InventorySystem, Error, TEXT("[UInventorySystemComponent|%s][ItemEquipFromInventory]: EquipmentDynamicStats is not filled but has an EquipmentDynamicStatsIndices entry"), *GetFName().ToString());
		if (CreatedEquipmentIndicesIndex != INDEX_NONE)
		{
			EquipmentAmounts.RemoveAt(CreatedEquipmentIndicesIndex);
			EquipmentAssets.RemoveAt(CreatedEquipmentIndicesIndex);
			EquipmentIndices.RemoveAt(CreatedEquipmentIndicesIndex);
		}
		ItemEquipFromInventorySuccessDelegate.Broadcast(false, RealEquipmentSlot, Slot);
		bIsProcessing = false;
		return;
	}

	// Unequip item before equipping new one or merge if possible
	if (RealEquipmentIndex != INDEX_NONE && CreatedEquipmentIndicesIndex == INDEX_NONE)
	{
		if (EquipmentAssets[RealEquipmentIndex] == InventoryAssets[RealIndex])
		{
			if (((FoundInventoryDynamicStatsIndex != INDEX_NONE && FoundEquipmentDynamicStatsIndex != INDEX_NONE && InventoryDynamicStats[FoundInventoryDynamicStatsIndex] == EquipmentDynamicStats[FoundEquipmentDynamicStatsIndex]) || (FoundInventoryDynamicStatsIndex == INDEX_NONE && FoundEquipmentDynamicStatsIndex == INDEX_NONE)))
			{
				if (bCanStack && TempCanStack)
				{
					ChangedSlots.Add(Slot);
					const int NewAmount = EquipmentAmounts[RealEquipmentIndex] + InventoryAmounts[RealIndex];
					if (NewAmount <= GetEquipmentStackSizeConfig())
					{
						InventoryIndices.Remove(Slot);
						InventoryAmounts.RemoveAt(RealIndex);
						InventoryAssets.RemoveAt(RealIndex);

						if (FoundInventoryDynamicStatsIndex != INDEX_NONE)
						{
							InventoryDynamicStatsIndices.RemoveAt(FoundInventoryDynamicStatsIndex);
							InventoryDynamicStats.RemoveAt(FoundInventoryDynamicStatsIndex);
						}

						EquipmentAmounts[RealEquipmentIndex] = NewAmount;

						ItemEquipFromInventorySuccessDelegate.Broadcast(true, RealEquipmentSlot, Slot);
						ChangedEquipmentSlotsDelegate.Broadcast({RealEquipmentSlot});
						ChangedInventorySlotsDelegate.Broadcast(ChangedSlots);
						bIsProcessing = false;
						return;
					}

					InventoryAmounts[RealIndex] -= GetEquipmentStackSizeConfig() - EquipmentAmounts[RealEquipmentIndex];
					EquipmentAmounts[RealEquipmentIndex] = FMath::Clamp(NewAmount, 1, GetEquipmentStackSizeConfig());
					ItemEquipFromInventorySuccessDelegate.Broadcast(true, RealEquipmentSlot, Slot);
					ChangedEquipmentSlotsDelegate.Broadcast({RealEquipmentSlot});
					ChangedInventorySlotsDelegate.Broadcast(ChangedSlots);
					bIsProcessing = false;
					return;
				}
				else
				{
					ItemEquipFromInventorySuccessDelegate.Broadcast(true, RealEquipmentSlot, Slot);
					ChangedEquipmentSlotsDelegate.Broadcast({RealEquipmentSlot});
					ChangedInventorySlotsDelegate.Broadcast(ChangedSlots);
					bIsProcessing = false;
					return;
				}
			}
		}
		
		// Add temp values then remove inventory slot. Allow item to be unequipped on the same slot if possible. Check beforehand!
		const int TempInventorySlot = InventoryIndices[RealIndex];
		int TempInventoryAmount = InventoryAmounts[RealIndex];
		const FPrimaryAssetId TempInventoryAsset = InventoryAssets[RealIndex];
		FItemProperties TempDynamicStats;
		if (FoundInventoryDynamicStatsIndex != INDEX_NONE)
		{
			TempDynamicStats = InventoryDynamicStats[FoundInventoryDynamicStatsIndex];
			InventoryDynamicStatsIndices.RemoveAt(FoundInventoryDynamicStatsIndex);
			InventoryDynamicStats.RemoveAt(FoundInventoryDynamicStatsIndex);
		}

		InventoryAssets.RemoveAt(RealIndex);
		InventoryAmounts.RemoveAt(RealIndex);
		InventoryIndices.RemoveAt(RealIndex);

		if (bCanStack && TempInventoryAmount <= GetEquipmentStackSizeConfig())
		{
			if (ChangedSlots.Append(ItemUnequipInternal(RealEquipmentSlot, {}, bCanUnequippedItemStack, Slot)); ChangedSlots.IsEmpty())
			{
				// Fallback
				const int FallbackInventoryIndicesIndex = InventoryIndices.AddUnique(TempInventorySlot);
				InventoryAssets.Add(TempInventoryAsset);
				InventoryAmounts.Add(TempInventoryAmount);

				if (FoundInventoryDynamicStatsIndex != INDEX_NONE)
				{
					InventoryDynamicStatsIndices.AddUnique(TempInventorySlot);
					InventoryDynamicStats.Add(TempDynamicStats);
				}

				if (CreatedEquipmentIndicesIndex != INDEX_NONE)
				{
					EquipmentAmounts.RemoveAt(CreatedEquipmentIndicesIndex);
					EquipmentAssets.RemoveAt(CreatedEquipmentIndicesIndex);
					EquipmentIndices.RemoveAt(CreatedEquipmentIndicesIndex);
				}
				ItemEquipFromInventorySuccessDelegate.Broadcast(false, RealEquipmentSlot, Slot);
				bIsProcessing = false;
				return;
			}
		}
		else
		{
			if (ChangedSlots.Append(ItemUnequipInternal(RealEquipmentSlot, {Slot}, bCanUnequippedItemStack, Slot)); ChangedSlots.IsEmpty())
			{
				// Fallback
				const int FallbackInventoryIndicesIndex = InventoryIndices.AddUnique(TempInventorySlot);
				InventoryAssets.Add(TempInventoryAsset);
				InventoryAmounts.Add(TempInventoryAmount);

				if (FoundInventoryDynamicStatsIndex != INDEX_NONE)
				{
					InventoryDynamicStatsIndices.AddUnique(TempInventorySlot);
					InventoryDynamicStats.Add(TempDynamicStats);
				}

				if (CreatedEquipmentIndicesIndex != INDEX_NONE)
				{
					EquipmentAmounts.RemoveAt(CreatedEquipmentIndicesIndex);
					EquipmentAssets.RemoveAt(CreatedEquipmentIndicesIndex);
					EquipmentIndices.RemoveAt(CreatedEquipmentIndicesIndex);
				}
				ItemEquipFromInventorySuccessDelegate.Broadcast(false, RealEquipmentSlot, Slot);
				bIsProcessing = false;
				return;
			}
		}

		// Now add the new item
		EquipmentIndices.AddUnique(RealEquipmentSlot);
		EquipmentAssets.Add(TempInventoryAsset);
		const int NewEquipmentAmountIndex = EquipmentAmounts.Add(1);

		if (FoundInventoryDynamicStatsIndex != INDEX_NONE)
		{
			EquipmentDynamicStatsIndices.AddUnique(RealEquipmentSlot);
			EquipmentDynamicStats.Add(TempDynamicStats);
		}

		if (bCanStack && TempInventoryAmount > 1)
		{
			// Add as much as possible then remove item if needed
			EquipmentAmounts[NewEquipmentAmountIndex]= FMath::Clamp(TempInventoryAmount, 1, GetEquipmentStackSizeConfig());
			if (EquipmentAmounts[NewEquipmentAmountIndex] != TempInventoryAmount)
			{
				if (EquipmentAmounts[NewEquipmentAmountIndex] == GetEquipmentStackSizeConfig())
				{
					TempInventoryAmount -= GetEquipmentStackSizeConfig();
				}
				else
				{
					TempInventoryAmount -= 1;
				}

				InventoryIndices.AddUnique(TempInventorySlot);
				InventoryAssets.Add(TempInventoryAsset);
				InventoryAmounts.Add(TempInventoryAmount);

				if (FoundInventoryDynamicStatsIndex != INDEX_NONE)
				{
					InventoryDynamicStatsIndices.AddUnique(TempInventorySlot);
					InventoryDynamicStats.Add(TempDynamicStats);
				}
			}

			ItemEquipFromInventorySuccessDelegate.Broadcast(true, RealEquipmentSlot, Slot);
			ChangedEquipmentSlotsDelegate.Broadcast({RealEquipmentSlot});
			ChangedInventorySlotsDelegate.Broadcast(ChangedSlots);
			bIsProcessing = false;
			return;
		}
		else
		{
			// Only add one and dont remove item if bigger then
			if (TempInventoryAmount > 1)
			{
				TempInventoryAmount -= 1;
				InventoryIndices.AddUnique(TempInventorySlot);
				InventoryAssets.Add(TempInventoryAsset);
				InventoryAmounts.Add(TempInventoryAmount);

				if (FoundInventoryDynamicStatsIndex != INDEX_NONE)
				{
					InventoryDynamicStatsIndices.AddUnique(TempInventorySlot);
					InventoryDynamicStats.Add(TempDynamicStats);
				}
			}
		}

		ItemEquipFromInventorySuccessDelegate.Broadcast(true, RealEquipmentSlot, Slot);
		ChangedEquipmentSlotsDelegate.Broadcast({RealEquipmentSlot});
		ChangedInventorySlotsDelegate.Broadcast(ChangedSlots);
		bIsProcessing = false;
		return;
	}
	else
	{
		// Equip item when nothing is equipped
		int NewEquipmentAmountIndex = INDEX_NONE;
		if (CreatedEquipmentIndicesIndex == INDEX_NONE)
		{
			EquipmentIndices.AddUnique(RealEquipmentSlot);
			EquipmentAssets.Add(InventoryAssets[RealIndex]);
			NewEquipmentAmountIndex = EquipmentAmounts.Add(1);
		}
		else
		{
			EquipmentAssets[CreatedEquipmentIndicesIndex] = InventoryAssets[RealIndex];
			NewEquipmentAmountIndex = CreatedEquipmentIndicesIndex;
		}

		if (FoundInventoryDynamicStatsIndex != INDEX_NONE)
		{
			EquipmentDynamicStatsIndices.AddUnique(RealEquipmentSlot);
			EquipmentDynamicStats.Add(InventoryDynamicStats[FoundInventoryDynamicStatsIndex]);
		}

		if (bCanStack && InventoryAmounts[RealIndex] > 1)
		{
			// Add as much as possible then remove item if needed
			EquipmentAmounts[NewEquipmentAmountIndex]= FMath::Clamp(InventoryAmounts[RealIndex], 1, GetEquipmentStackSizeConfig());
			if (EquipmentAmounts[NewEquipmentAmountIndex] == InventoryAmounts[RealIndex])
			{
				// Remove DynamicStats
				if (FoundInventoryDynamicStatsIndex != INDEX_NONE)
				{
					InventoryDynamicStats.RemoveAt(FoundInventoryDynamicStatsIndex);
					InventoryDynamicStatsIndices.RemoveAt(FoundInventoryDynamicStatsIndex);
				}

				// Remove Item
				InventoryAmounts.RemoveAt(RealIndex);
				InventoryAssets.RemoveAt(RealIndex);
				InventoryIndices.RemoveAt(RealIndex);
			}
			else if (EquipmentAmounts[NewEquipmentAmountIndex] == GetEquipmentStackSizeConfig())
			{
				InventoryAmounts[RealIndex] -= GetEquipmentStackSizeConfig();
			}
			else
			{
				InventoryAmounts[RealIndex] -= 1;
			}

			ItemEquipFromInventorySuccessDelegate.Broadcast(true, RealEquipmentSlot, Slot);
			ChangedEquipmentSlotsDelegate.Broadcast({RealEquipmentSlot});
			ChangedInventorySlotsDelegate.Broadcast(ChangedSlots);
			bIsProcessing = false;
			return;
		}
		else
		{
			// Only add one and dont remove item if bigger then 1;
			{
				// Remove DynamicStats
				if (FoundInventoryDynamicStatsIndex != INDEX_NONE)
				{
					InventoryDynamicStats.RemoveAt(FoundInventoryDynamicStatsIndex);
					InventoryDynamicStatsIndices.RemoveAt(FoundInventoryDynamicStatsIndex);
				}

				// Remove Item
				InventoryAmounts.RemoveAt(RealIndex);
				InventoryAssets.RemoveAt(RealIndex);
				InventoryIndices.RemoveAt(RealIndex);
			}
		}

		ItemEquipFromInventorySuccessDelegate.Broadcast(true, RealEquipmentSlot, Slot);
		ChangedEquipmentSlotsDelegate.Broadcast({RealEquipmentSlot});
		ChangedInventorySlotsDelegate.Broadcast(ChangedSlots);
		bIsProcessing = false;
		return;
	}


	UE_LOG(InventorySystem, Error, TEXT("[UInventorySystemComponent|%s][ItemEquipFromInventory]: Unknown error"), *GetFName().ToString());
	ItemEquipFromInventorySuccessDelegate.Broadcast(false, RealEquipmentSlot, Slot);
	if (CreatedEquipmentIndicesIndex != INDEX_NONE)
	{
		EquipmentAmounts.RemoveAt(CreatedEquipmentIndicesIndex);
		EquipmentAssets.RemoveAt(CreatedEquipmentIndicesIndex);
		EquipmentIndices.RemoveAt(CreatedEquipmentIndicesIndex);
	}
	bIsProcessing = false;
}

bool UInventorySystemComponent::ItemUnequip_Validate(const int EquipmentSlot, const TArray<int>& IgnoreInventorySlots, const bool bCanStack, const int SpecificInventorySlot)
{
	return true;
}

void UInventorySystemComponent::ItemUnequip_Implementation(const int EquipmentSlot, const TArray<int>& IgnoreInventorySlots, const bool bCanStack, const int SpecificInventorySlot)
{
	if (const AActor* Owner = GetOwner(); !IsValid(Owner) || !Owner->HasAuthority())
	{
		UE_LOG(InventorySystem, Error, TEXT("[UItemContainerComponent|%s][ItemUnequip]: Component owner has no authority"), *GetFName().ToString());
		ItemUnequipSuccessDelegate.Broadcast(false, EquipmentSlot, {});
		return;
	}

	if (bIsProcessing)
	{
		UE_LOG(InventorySystem, Error, TEXT("[UInventorySystemComponent|%s][ItemUnequip]: Component is still processing previous request"), *GetFName().ToString());
		ItemUnequipSuccessDelegate.Broadcast(false, EquipmentSlot, {});
		return;
	}

	bIsProcessing = true;
	const TArray<int> ChangedSlots = ItemUnequipInternal(EquipmentSlot, IgnoreInventorySlots, bCanStack, SpecificInventorySlot);
	if (ChangedSlots.IsEmpty())
	{
		ItemUnequipSuccessDelegate.Broadcast(false, EquipmentSlot, {});
		bIsProcessing = false;
		return;
	}
	
	ItemUnequipSuccessDelegate.Broadcast(true, EquipmentSlot, ChangedSlots);
	ChangedEquipmentSlotsDelegate.Broadcast({EquipmentSlot});
	ChangedInventorySlotsDelegate.Broadcast(ChangedSlots);
	bIsProcessing = false;
}

TArray<int> UInventorySystemComponent::ItemUnequipInternal(const int& EquipmentSlot, const TArray<int> IgnoreInventorySlots, const bool bCanStack, const int SpecificInventorySlot)
{
	const int RealEquipmentIndex = EquipmentIndices.Find(EquipmentSlot);
	UAssetManager& Manager = UAssetManager::Get();
	if (RealEquipmentIndex == INDEX_NONE || !Manager.IsValid() || !EquipmentTypeIndices.Contains(EquipmentSlot) || !EquipmentAssets.IsValidIndex(RealEquipmentIndex) || !EquipmentAmounts.IsValidIndex(RealEquipmentIndex))
	{
		UE_LOG(InventorySystem, Error, TEXT("[UInventorySystemComponent|%s][ItemUnequip]: Invalid item or EquipmentType data"), *GetFName().ToString());
		return {};
	}

	if (!IgnoreInventorySlots.IsEmpty())
	{
		int MaxArrayValue = INDEX_NONE;
		int IndexMaxValue = INDEX_NONE;
		UKismetMathLibrary::MaxOfIntArray(IgnoreInventorySlots, IndexMaxValue, MaxArrayValue);
		if (GetInventorySizeConfig() < MaxArrayValue)
		{
			UE_LOG(InventorySystem, Error, TEXT("[UInventorySystemComponent|%s][ItemUnequip]: One or more slots in the IgnoreInventorySlot array could not be found"), *GetFName().ToString());
			return {};
		}
	}

	const int FoundEquipmentDynamicStatsIndex = EquipmentDynamicStatsIndices.Find(EquipmentSlot);
	if (FoundEquipmentDynamicStatsIndex != INDEX_NONE && !EquipmentDynamicStats.IsValidIndex(FoundEquipmentDynamicStatsIndex))
	{
		UE_LOG(InventorySystem, Error, TEXT("[UInventorySystemComponent|%s][ItemUnequip]: EquipmentDynamicStats is not filled but has an EquipmentDynamicStatsIndices entry"), *GetFName().ToString());
		return {};
	}

	bool TempCanStack = false;
	FAssetData AssetData;
	Manager.GetPrimaryAssetData(EquipmentAssets[RealEquipmentIndex], AssetData);
	if (!AssetData.IsValid())
	{
		UE_LOG(InventorySystem, Error, TEXT("[UInventorySystemComponent|%s][ItemUnequip]: AssetData is not valid. Unable to set TempCanStack value"), *GetFName().ToString());
		return {};
	}

	const FName AssetRegistrySearchablePropertyName = GET_MEMBER_NAME_CHECKED(UItemDataAsset, bCanStack);
	AssetData.GetTagValue(AssetRegistrySearchablePropertyName, TempCanStack);

	TArray<int> ChangedSlots;
	std::function<bool()> UnequipItemWithoutSpecificSlot = [&]
	{
		// No slot specified or not possible to add to specified slot. Search next stack or empty slot
		bool bSuccess = false;
		int FoundIndex = INDEX_NONE;
		if (bCanStack && TempCanStack)
		{
			int Amount = 0;
			FItemProperties DynamicStats = FItemProperties{};
			if (FoundEquipmentDynamicStatsIndex != INDEX_NONE)
			{
				DynamicStats = EquipmentDynamicStats[FoundEquipmentDynamicStatsIndex];
			}

			FindItemStack(EquipmentAssets[RealEquipmentIndex], FoundIndex, Amount, bSuccess, DynamicStats, -1, false, IgnoreInventorySlots);
			if (bSuccess && Amount < GetStackSizeConfig())
			{
				// Add another item as this is not enough
				if (EquipmentAmounts[RealEquipmentIndex] + InventoryAmounts[FoundIndex] > GetStackSizeConfig())
				{
					EquipmentAmounts[RealEquipmentIndex] = EquipmentAmounts[RealEquipmentIndex] + InventoryAmounts[FoundIndex] - GetStackSizeConfig();
					InventoryAmounts[FoundIndex] = GetStackSizeConfig();
					ChangedSlots.Add(InventoryIndices[FoundIndex]);
					return UnequipItemWithoutSpecificSlot();
				}

				InventoryAmounts[FoundIndex] = EquipmentAmounts[RealEquipmentIndex] + InventoryAmounts[FoundIndex];
				if (FoundEquipmentDynamicStatsIndex != INDEX_NONE)
				{
					EquipmentDynamicStats.RemoveAt(FoundEquipmentDynamicStatsIndex);
					EquipmentDynamicStatsIndices.RemoveAt(FoundEquipmentDynamicStatsIndex);
				}

				EquipmentIndices.RemoveAt(RealEquipmentIndex);
				EquipmentAmounts.RemoveAt(RealEquipmentIndex);
				EquipmentAssets.RemoveAt(RealEquipmentIndex);
				ChangedSlots.Add(InventoryIndices[FoundIndex]);

				return true;
			}
		}

		if (bCanStack && TempCanStack)
		{
			// If item is stackable and not enough space for full item extraction. Return early
			if (EquipmentAmounts[RealEquipmentIndex] > GetStackSizeConfig() && GetInventorySizeConfig() - InventoryIndices.Num() < EquipmentAmounts[RealEquipmentIndex] / GetStackSizeConfig())
			{
				UE_LOG(InventorySystem, Warning, TEXT("[UInventorySystemComponent|%s][ItemUnequip]: Item could not be unequipped. Not enough space"), *GetFName().ToString());
				return false;
			}
		}
		else
		{
			// If item is not stackable and not enough space for single item extraction. Return early
			if (GetInventorySizeConfig() - InventoryIndices.Num() < EquipmentAmounts[RealEquipmentIndex])
			{
				UE_LOG(InventorySystem, Warning, TEXT("[UInventorySystemComponent|%s][ItemUnequip]: Item could not be unequipped. Not enough space"), *GetFName().ToString());
				return false;
			}
		}

		FindNextEmptySlot(FoundIndex, bSuccess, IgnoreInventorySlots);
		if (bSuccess)
		{
			InventoryIndices.Add(FoundIndex);
			InventoryAssets.Add(EquipmentAssets[RealEquipmentIndex]);

			if (FoundEquipmentDynamicStatsIndex != INDEX_NONE)
			{
				InventoryDynamicStatsIndices.Add(FoundIndex);
				InventoryDynamicStats.Add(EquipmentDynamicStats[FoundEquipmentDynamicStatsIndex]);
			}
			
			ChangedSlots.Add(FoundIndex);
			
			InventoryAmounts.Add(1);

			if (EquipmentAmounts[RealEquipmentIndex] - 1 == 0)
			{
				EquipmentIndices.RemoveAt(RealEquipmentIndex);
				EquipmentAmounts.RemoveAt(RealEquipmentIndex);
				EquipmentAssets.RemoveAt(RealEquipmentIndex);

				if (FoundEquipmentDynamicStatsIndex != INDEX_NONE)
				{
					EquipmentDynamicStatsIndices.RemoveAt(FoundEquipmentDynamicStatsIndex);
					EquipmentDynamicStats.RemoveAt(FoundEquipmentDynamicStatsIndex);
				}

				return true;
			}

			EquipmentAmounts[RealEquipmentIndex] -= 1;
			return UnequipItemWithoutSpecificSlot();
		}

		UE_LOG(InventorySystem, Warning, TEXT("[UInventorySystemComponent|%s][ItemUnequip]: Item could not be unequipped entirely. Nothing or only part of the item was removed from the equipment slot"), *GetFName().ToString());
		return false;
	};

	// Slot must be empty or the item must be stackable. Otherwise just search the next empty slot
	std::function<bool()> UnequipItemSpecificSlot = [&]
	{
		if (SpecificInventorySlot != INDEX_NONE && SpecificInventorySlot <= GetInventorySizeConfig() && !IgnoreInventorySlots.Contains(SpecificInventorySlot))
		{
			if (const int RealSpecificInventoryIndex = InventoryIndices.Find(SpecificInventorySlot); RealSpecificInventoryIndex != INDEX_NONE)
			{
				const int FoundInventoryDynamicStatsIndex = InventoryDynamicStatsIndices.Find(SpecificInventorySlot);
				if (FoundInventoryDynamicStatsIndex != INDEX_NONE && !InventoryDynamicStats.IsValidIndex(FoundInventoryDynamicStatsIndex))
				{
					UE_LOG(InventorySystem, Error, TEXT("[UInventorySystemComponent|%s][ItemUnequip]: InventoryDynamicStats is not filled but has an InventoryDynamicStatsIndices entry"), *GetFName().ToString());
					return false;
				}

				// Same item. Return early. If enough space combine
				if (bCanStack && TempCanStack && InventoryAssets[RealSpecificInventoryIndex] == EquipmentAssets[RealEquipmentIndex])
				{
					if ((FoundInventoryDynamicStatsIndex != INDEX_NONE && FoundEquipmentDynamicStatsIndex != INDEX_NONE && EquipmentDynamicStats[FoundEquipmentDynamicStatsIndex] == InventoryDynamicStats[FoundInventoryDynamicStatsIndex]) || (FoundInventoryDynamicStatsIndex == INDEX_NONE && FoundEquipmentDynamicStatsIndex == INDEX_NONE))
					{
						if (EquipmentAmounts[RealEquipmentIndex] + InventoryAmounts[RealSpecificInventoryIndex] <= GetStackSizeConfig())
						{
							InventoryAmounts[RealSpecificInventoryIndex] += EquipmentAmounts[RealEquipmentIndex];
							ChangedSlots.Add(InventoryIndices[RealSpecificInventoryIndex]);
							if (FoundEquipmentDynamicStatsIndex != INDEX_NONE)
							{
								EquipmentDynamicStatsIndices.RemoveAt(FoundEquipmentDynamicStatsIndex);
								EquipmentDynamicStats.RemoveAt(FoundEquipmentDynamicStatsIndex);	
							}
							EquipmentIndices.RemoveAt(RealEquipmentIndex);
							EquipmentAmounts.RemoveAt(RealEquipmentIndex);
							EquipmentAssets.RemoveAt(RealEquipmentIndex);
							return true;
						}

						EquipmentAmounts[RealEquipmentIndex] = EquipmentAmounts[RealEquipmentIndex] + InventoryAmounts[RealSpecificInventoryIndex] - GetStackSizeConfig();
						InventoryAmounts[RealSpecificInventoryIndex] = GetStackSizeConfig();
						ChangedSlots.Add(InventoryIndices[RealSpecificInventoryIndex]);
						return UnequipItemWithoutSpecificSlot();
					}
				}
			}
			else
			{
				// No item in slot
				if (InventoryIndices.Num() == GetInventorySizeConfig() || InventoryIndices.Num() + FMath::CeilToInt(static_cast<float>(EquipmentAmounts[RealEquipmentIndex]) / static_cast<float>(GetStackSizeConfig())) > GetInventorySizeConfig())
				{
					UE_LOG(InventorySystem, Error, TEXT("[UInventorySystemComponent|%s][ItemUnequip]: Not enough space in inventory"), *GetFName().ToString());
					return false;
				}

				int ItemsLeft = INDEX_NONE;
				if (bCanStack)
				{
					ItemsLeft = EquipmentAmounts[RealEquipmentIndex] - GetStackSizeConfig();
				}
				else
				{
					ItemsLeft = EquipmentAmounts[RealEquipmentIndex] - 1;
				}

				InventoryIndices.AddUnique(SpecificInventorySlot);
				InventoryAssets.Add(EquipmentAssets[RealEquipmentIndex]);

				if (FoundEquipmentDynamicStatsIndex != INDEX_NONE)
				{
					InventoryDynamicStatsIndices.Add(SpecificInventorySlot);
					InventoryDynamicStats.Add(EquipmentDynamicStats[FoundEquipmentDynamicStatsIndex]);
				}

				if (ItemsLeft > 0)
				{
					InventoryAmounts.Add(EquipmentAmounts[RealEquipmentIndex] - ItemsLeft);
					EquipmentAmounts[RealEquipmentIndex] = ItemsLeft;
					return UnequipItemWithoutSpecificSlot();
				}

				InventoryAmounts.Add(EquipmentAmounts[RealEquipmentIndex]);
				if (FoundEquipmentDynamicStatsIndex != INDEX_NONE)
				{
					EquipmentDynamicStats.RemoveAt(FoundEquipmentDynamicStatsIndex);
					EquipmentDynamicStatsIndices.RemoveAt(FoundEquipmentDynamicStatsIndex);
				}

				EquipmentIndices.RemoveAt(RealEquipmentIndex);
				EquipmentAmounts.RemoveAt(RealEquipmentIndex);
				EquipmentAssets.RemoveAt(RealEquipmentIndex);

				return true;
			}
		}

		return UnequipItemWithoutSpecificSlot();
	};

	if (UnequipItemSpecificSlot())
	{
		ItemUnequipSuccessDelegate.Broadcast(true, EquipmentSlot, ChangedSlots);
		return ChangedSlots;
	}

	ItemUnequipSuccessDelegate.Broadcast(false, EquipmentSlot, ChangedSlots);
	return ChangedSlots;
}

TArray<int> UInventorySystemComponent::AddItemToComponentInternal(const int Slot, UItemContainerComponent* ItemContainerComponent, int& Amount, const bool bCanStack, const bool bIsEquipment, const bool bRevertWhenFull)
{
	if (!bIsEquipment)
	{
		return Super::AddItemToComponentInternal(Slot, ItemContainerComponent, Amount, bCanStack, bIsEquipment, bRevertWhenFull);	
	}

	const int Index = EquipmentIndices.Find(Slot);
	TArray<int> ChangedSlots;
	if (Amount <= 0 || Index == INDEX_NONE || !EquipmentAssets.IsValidIndex(Index) || !EquipmentAssets[Index].IsValid() || EquipmentAssets[Index] == FPrimaryAssetId() || !EquipmentAmounts.IsValidIndex(Index) || EquipmentAmounts[Index] <= 0 || Amount > EquipmentAmounts[Index])
	{
		UE_LOG(InventorySystem, Error, TEXT("[UInventorySystemComponent|%s][AddItemToComponentInternal]: Data invalid for slot %d"), *GetFName().ToString(), Slot);
		return ChangedSlots;;
	}

	if (!IsValid(ItemContainerComponent))
	{
		UE_LOG(InventorySystem, Error, TEXT("[UInventorySystemComponent|%s][AddItemToComponentInternal]: Other component is invalid"), *GetFName().ToString());
		return ChangedSlots;
	}

	FItemProperties DynamicStats{};
	const int RealEquipmentDynamicStatsIndicesIndex = EquipmentDynamicStatsIndices.Find(Slot);
	if (RealEquipmentDynamicStatsIndicesIndex != INDEX_NONE)
	{
		if (!EquipmentDynamicStats.IsValidIndex(RealEquipmentDynamicStatsIndicesIndex))
		{
			UE_LOG(InventorySystem, Error, TEXT("[UInventorySystemComponent|%s][AddItemToComponentInternal]: EquipmentDynamicStats is not filled but has an EquipmentDynamicStatsIndices entry"), *GetFName().ToString());
			return ChangedSlots;
		}

		DynamicStats = EquipmentDynamicStats[RealEquipmentDynamicStatsIndicesIndex];
	}
	
	int ItemsLeft = Amount;
	ChangedSlots = ItemContainerComponent->AddItemInternal(EquipmentAssets[Index], DynamicStats, ItemsLeft, bCanStack, bRevertWhenFull);
	if (!ChangedSlots.IsEmpty())
	{
		EquipmentAmounts[Index] -= Amount - ItemsLeft;
		Amount = ItemsLeft;
		if (EquipmentAmounts[Index] == 0)
		{
			EquipmentIndices.RemoveAt(Index);
			EquipmentAssets.RemoveAt(Index);
			EquipmentAmounts.RemoveAt(Index);

			if (RealEquipmentDynamicStatsIndicesIndex != INDEX_NONE)
			{
				EquipmentDynamicStatsIndices.RemoveAt(RealEquipmentDynamicStatsIndicesIndex);
				EquipmentDynamicStats.RemoveAt(RealEquipmentDynamicStatsIndicesIndex);
			}
			
			return ChangedSlots;
		}
		
		return ChangedSlots;
	}

	UE_LOG(InventorySystem, Log, TEXT("[UInventorySystemComponent|%s][AddItemToComponentInternal]: Item could not be added to other component"), *GetFName().ToString());
	return ChangedSlots;
}

void UInventorySystemComponent::CollectAllItems_Implementation(UItemContainerComponent* ItemContainerComponent, const bool bCanStack)
{
	if (const AActor* Owner = GetOwner(); !IsValid(Owner) || !Owner->HasAuthority())
	{
		UE_LOG(InventorySystem, Error, TEXT("[UInventorySystemComponent|%s][CollectAllItems]: Component owner has no authority"), *GetFName().ToString());
		CollectAllItemsSuccessDelegate.Broadcast(false, true, nullptr);
		return;
	}

	if (bIsProcessing)
	{
		UE_LOG(InventorySystem, Warning, TEXT("[UInventorySystemComponent|%s][CollectAllItems]: Component is still processing previous request"), *GetFName().ToString());
		CollectAllItemsSuccessDelegate.Broadcast(false, true, nullptr);
		return;
	}

	bIsProcessing = true;

	// Is valid component?
	if (!IsValid(ItemContainerComponent) || ItemContainerComponent->bIsProcessing)
	{
		UE_LOG(InventorySystem, Error, TEXT("[UInventorySystemComponent|%s][CollectAllItems]: Other component is invalid"), *GetFName().ToString());
		CollectAllItemsSuccessDelegate.Broadcast(false, true, nullptr);
		bIsProcessing = false;
		return;
	}

	ItemContainerComponent->bIsProcessing = true;
	ItemContainerComponent->CollectAllItemsOtherComponentStartDelegate.Broadcast();

	bool bAddedOnce = false;
	bool bItemsLeft = false;
	TArray<int> ChangedEquipmentSlots;
	TArray<int> ChangedSlotsOtherComponent;
	const std::function<bool(int, int)> AddEquipmentItemToComponent = [&](const int Slot, const int Amount)
	{
		const int Index = EquipmentIndices.Find(Slot);
		if (Amount <= 0 || Index == INDEX_NONE || !EquipmentAssets.IsValidIndex(Index) || !EquipmentAssets[Index].IsValid() || EquipmentAssets[Index] == FPrimaryAssetId() || !EquipmentAmounts.IsValidIndex(Index) || EquipmentAmounts[Index]<= 0 || Amount > EquipmentAmounts[Index])
		{
			UE_LOG(InventorySystem, Error, TEXT("[UInventorySystemComponent|%s][CollectAllItems]: Data invalid for slot %d"), *GetFName().ToString(), Slot);
			bItemsLeft = true;
			return false;
		}

		FItemProperties DynamicStats{};
		const int RealEquipmentDynamicStatsIndicesIndex = EquipmentDynamicStatsIndices.Find(Slot);
		if (RealEquipmentDynamicStatsIndicesIndex != INDEX_NONE)
		{
			if (!EquipmentDynamicStats.IsValidIndex(RealEquipmentDynamicStatsIndicesIndex))
			{
				UE_LOG(InventorySystem, Error, TEXT("[UInventorySystemComponent|%s][CollectAllItems]: EquipmentDynamicStats is not filled but has an EquipmentDynamicStatsIndices entry"), *GetFName().ToString());
				bItemsLeft = true;
				return false;
			}
		}

		int ItemsLeft = Amount;
		const TArray<int> AddedArray = AddItemToComponentInternal(Slot, ItemContainerComponent, ItemsLeft, bCanStack, true, false);
		ChangedSlotsOtherComponent.Append(AddedArray);
		if (!AddedArray.IsEmpty())
		{
			bAddedOnce = true;
			ChangedEquipmentSlots.Add(Slot);
			if (!ItemsLeft)
			{
				return true;
			}
		}

		UE_LOG(InventorySystem, Log, TEXT("[UInventorySystemComponent|%s][CollectAllItems]: equipment item could not be added to other component"), *GetFName().ToString());
		bItemsLeft = true;
		return false;
	};

	// Start transferring all items, skip items that are not in other container if full and bCanStack
	const int EquipmentIndicesLength = EquipmentIndices.Num();
	for (int I = 0; I < EquipmentIndicesLength; I++)
	{
		if (!EquipmentIndices.IsValidIndex(0))
		{
			continue;
		}

		if (!EquipmentAmounts.IsValidIndex(0) || !EquipmentAssets.IsValidIndex(0))
		{
			// Slot no longer available? That should not happen
			UE_LOG(InventorySystem, Error, TEXT("[UInventorySystemComponent|%s][CollectAllItems]: Data invalid for equipment slot %d"), *GetFName().ToString(), EquipmentIndices[0]);
			bItemsLeft = true;
			continue;
		}

		AddEquipmentItemToComponent(EquipmentIndices[0], EquipmentAmounts[0]);
	}

	// Start transferring all items, skip items that are not in other container if full and bCanStack
	TArray<int> ChangedSlots;
	const int InventoryIndicesLength = InventoryIndices.Num();
	for (int I = 0; I < InventoryIndicesLength; I++)
	{
		if (!InventoryIndices.IsValidIndex(0))
		{
			continue;
		}

		if (!InventoryAmounts.IsValidIndex(0) || !InventoryAssets.IsValidIndex(0))
		{
			// Slot no longer available? That should not happen
			UE_LOG(InventorySystem, Error, TEXT("[UInventorySystemComponent|%s][CollectAllItems]: Data invalid for slot %d"), *GetFName().ToString(), InventoryIndices[0]);
			continue;
		}
		
		int ItemsLeft = InventoryAmounts[0];
		const int AddedSlot = InventoryIndices[0];
		TArray<int> AddedArray = AddItemToComponentInternal(InventoryIndices[0], ItemContainerComponent, ItemsLeft, bCanStack, false, false);
		ChangedSlotsOtherComponent.Append(AddedArray);
		if (!AddedArray.IsEmpty())
		{
			bAddedOnce = true;
			ChangedSlots.Add(AddedSlot);
			if (!ItemsLeft)
			{
				continue;
			}
		}
		
		bItemsLeft = true;
	}

	if (!bAddedOnce)
	{
		bItemsLeft = true;
		UE_LOG(InventorySystem, Warning, TEXT("[UInventorySystemComponent|%s][CollectAllItems]: Could not collect any item"), *GetFName().ToString());
	}

	CollectAllItemsSuccessDelegate.Broadcast(bAddedOnce, bItemsLeft, ItemContainerComponent);
	ItemContainerComponent->CollectAllItemsOtherComponentSuccessDelegate.Broadcast(bAddedOnce, bItemsLeft, this);
	ChangedInventorySlotsDelegate.Broadcast(ChangedSlots);
	ChangedEquipmentSlotsDelegate.Broadcast(ChangedEquipmentSlots);
	ItemContainerComponent->ChangedInventorySlotsDelegate.Broadcast(ChangedSlotsOtherComponent);
	ItemContainerComponent->bIsProcessing = false;
	bIsProcessing = false;
}

int UInventorySystemComponent::GetEquipmentStackSizeConfig() const
{
	const UInventorySystemSettings* InventorySettings = GetMutableDefault<UInventorySystemSettings>();
	return MaxEquipmentStackSize > 1 ? MaxEquipmentStackSize : InventorySettings->MaxItemEquipmentStackSize;
}

bool UInventorySystemComponent::SetEquipmentStackSizeConfig_Validate(const int NewInventorySize, const bool bForce)
{
	return true;
}

void UInventorySystemComponent::SetEquipmentStackSizeConfig_Implementation(const int NewMaxEquipmentStackSize, const bool bForce)
{
	if (bIsProcessing)
	{
		UE_LOG(InventorySystem, Warning, TEXT("[UInventorySystemComponent|%s][SetEquipmentStackSizeConfig]: Component is still processing previous request"), *GetFName().ToString());
		SetMaxEquipmentStackSizeSuccessDelegate.Broadcast(false);
		return;
	}

	bIsProcessing = true;
	
	if (bForce)
	{
		MaxEquipmentStackSize = NewMaxEquipmentStackSize;
		InternalChecks();
		SetMaxEquipmentStackSizeSuccessDelegate.Broadcast(true);
		ChangedEquipmentSlotsDelegate.Broadcast(EquipmentTypeIndices);
		bIsProcessing = false;
		return;
	}
	
	if (!EquipmentAmounts.IsEmpty())
	{
		for (const int Amount : EquipmentAmounts)
		{
			if (Amount > NewMaxEquipmentStackSize)
			{
				UE_LOG(InventorySystem, Warning, TEXT("[UInventorySystemComponent|%s][SetEquipmentStackSizeConfig]: Aborted action! Item overflow detected"), *GetFName().ToString());
				SetMaxEquipmentStackSizeSuccessDelegate.Broadcast(false);
				bIsProcessing = false;
				return;
			}
		}
	}

	MaxEquipmentStackSize = NewMaxEquipmentStackSize;
	SetMaxEquipmentStackSizeSuccessDelegate.Broadcast(true);
	ChangedEquipmentSlotsDelegate.Broadcast(EquipmentTypeIndices);
	bIsProcessing = false;
}

int UInventorySystemComponent::GetStackSizeConfig() const
{
	const UInventorySystemSettings* InventorySettings = GetMutableDefault<UInventorySystemSettings>();
	return MaxStackSize > 1 ? MaxStackSize : InventorySettings->MaxInventorySize;
}

int UInventorySystemComponent::GetInventorySizeConfig() const
{
	const UInventorySystemSettings* InventorySettings = GetMutableDefault<UInventorySystemSettings>();
	return InventorySize > 0 ? InventorySize : InventorySettings->MaxInventorySize;
}

FString UInventorySystemComponent::ReplaceEquipmentArrayString(FString OriginalArrayString)
{
	OriginalArrayString = OriginalArrayString.Replace(TEXT("("), TEXT(""));
	OriginalArrayString = OriginalArrayString.Replace(TEXT(")"), TEXT(""));
	return OriginalArrayString.Replace(TEXT("\""), TEXT(""));
}

#undef LOCTEXT_NAMESPACE
