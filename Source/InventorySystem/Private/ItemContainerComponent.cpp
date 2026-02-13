// © 2024 Daniel Münch. All Rights Reserved

#include "ItemContainerComponent.h"

#include <functional>

#include "InventorySystem.h"
#include "Net/UnrealNetwork.h"
#include "Engine/AssetManager.h"
#include "AssetRegistry/AssetData.h"
#include "InventorySystemComponent.h"
#include "Settings/InventorySystemSettings.h"
#include "UObject/ObjectSaveContext.h"
#include "Kismet/KismetMathLibrary.h"
#include "UObject/SavePackage.h"

#define LOCTEXT_NAMESPACE "InventorySystem"

UItemContainerComponent::UItemContainerComponent()
{
#if WITH_EDITORONLY_DATA
	if (InventoryIndices.IsEmpty())
	{
		bAllowInventoryEdit = false;
		bAllowInventoryAssetEdit = false;
		return;
	}

	bHasBegunPlayEditor = HasBegunPlay();
	bAllowInventoryAssetEdit = HasBegunPlay();
	bAllowInventoryEdit = true;
#endif
}

void UItemContainerComponent::OnRep_InventoryIndices(TArray<int> OldInventoryIndices)
{
	// Check for changes in the array
	for (int Index = 0; Index < InventoryIndices.Num(); Index++)
	{
		// Check if index was added
		if (OldInventoryIndices.Find(InventoryIndices[Index]) == INDEX_NONE || InventoryIndices[Index] != OldInventoryIndices[Index])
		{
			ChangedInventorySlotsDelegate.Broadcast({InventoryIndices[Index]});
		}
	}

	for (int Index = 0; Index < OldInventoryIndices.Num(); Index++)
	{
		// Check if index was removed
		if (InventoryIndices.Find(OldInventoryIndices[Index]) == INDEX_NONE)
		{
			ChangedInventorySlotsDelegate.Broadcast({OldInventoryIndices[Index]});
		}
	}
}

void UItemContainerComponent::OnRep_InventoryAssets(TArray<FPrimaryAssetId> OldInventoryAssets)
{
	// Check for changes in the array
	for (int Index = 0; Index < InventoryAssets.Num(); Index++)
	{
		if (InventoryIndices.IsValidIndex(Index))
		{
			// Check if index was added or changed
			if (!OldInventoryAssets.IsValidIndex(Index) || InventoryAssets[Index] != OldInventoryAssets[Index])
			{
				ChangedInventorySlotsDelegate.Broadcast({InventoryIndices[Index]});
			}
		}
	}
}

#if WITH_EDITOR
void UItemContainerComponent::InternalCheckEditVariables(const TArray<int>& Slots)
{
	bAllowInventoryEdit = !InventoryIndices.IsEmpty();
	bAllowInventoryAssetEdit = HasBegunPlay();
}

void UItemContainerComponent::InternalSaveAfterCheck()
{
#if WITH_EDITORONLY_DATA
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
				UE_LOG(InventorySystem, Warning, TEXT("[UItemContainerComponent|%s][InternalSaveAfterCheck]: A mistake in setup resulted in data being altered... saving"), *GetFName().ToString());
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
			UE_LOG(InventorySystem, Warning, TEXT("[UItemContainerComponent|%s][InternalSaveAfterCheck]: A mistake in setup resulted in data being altered... saving"), *GetFName().ToString());
		}
	}
#endif
}
#endif

void UItemContainerComponent::OnRep_InventoryAmounts(TArray<int> OldInventoryAmounts)
{
	// Check for changes in the array
	for (int Index = 0; Index < InventoryAmounts.Num(); Index++)
	{
		if (InventoryIndices.IsValidIndex(Index))
		{
			// Check if index was added or changed
			if (!OldInventoryAmounts.IsValidIndex(Index) || InventoryAmounts[Index] != OldInventoryAmounts[Index])
			{
				ChangedInventorySlotsDelegate.Broadcast({InventoryIndices[Index]});
			}	
		}
	}
}

void UItemContainerComponent::OnRep_InventoryDynamicStatsIndices(TArray<int> OldInventoryDynamicStatsIndices)
{
	// Check for changes in the array
	for (int Index = 0; Index < InventoryDynamicStatsIndices.Num(); Index++)
	{
		if (const int RealInventoryIndex = InventoryIndices.Find(InventoryDynamicStatsIndices[Index]); RealInventoryIndex != INDEX_NONE)
		{
			// Check if index was added or changed
			if (OldInventoryDynamicStatsIndices.Find(InventoryDynamicStatsIndices[Index]) == INDEX_NONE || InventoryDynamicStatsIndices[Index] != OldInventoryDynamicStatsIndices[Index])
			{
				ChangedInventorySlotsDelegate.Broadcast({InventoryIndices[RealInventoryIndex]});
			}
		}
	}

	for (int Index = 0; Index < OldInventoryDynamicStatsIndices.Num(); Index++)
	{
		// Check if index was removed
		if (InventoryDynamicStatsIndices.Find(OldInventoryDynamicStatsIndices[Index]) == INDEX_NONE)
		{
			ChangedInventorySlotsDelegate.Broadcast({OldInventoryDynamicStatsIndices[Index]});
		}
	}
}

void UItemContainerComponent::OnRep_InventoryDynamicStats(TArray<FItemProperties> OldInventoryDynamicStats)
{
	// Check for changes in the array
	for (int Index = 0; Index < InventoryDynamicStats.Num(); Index++)
	{
		if (!OldInventoryDynamicStats.IsValidIndex(Index))
		{
			continue;
		}
		
		if (const int* RealInventoryDynamicStatsSlot = InventoryDynamicStatsIndices.FindByKey(Index); RealInventoryDynamicStatsSlot != nullptr)
		{
			if (const int RealInventoryIndex = InventoryIndices.Find(*RealInventoryDynamicStatsSlot); RealInventoryIndex != INDEX_NONE)
			{
				// Check if value added or changed
				if (InventoryDynamicStats[Index] != OldInventoryDynamicStats[Index])
				{
					ChangedInventorySlotsDelegate.Broadcast({InventoryIndices[RealInventoryIndex]});
				}		
			}
		}
	}
}

#if WITH_EDITOR
void UItemContainerComponent::PreSave(FObjectPreSaveContext SaveContext)
{
	if (IsRunningCommandlet())
	{
		Super::PreSave(SaveContext);
		return;
	}

	InternalChecks(true);

	Super::PreSave(SaveContext);

	if (!IsTemplate(RF_ClassDefaultObject | RF_ArchetypeObject))
	{
		if (AActor* ActorOwner = GetOwner(); IsValid(ActorOwner))
		{
			ActorOwner->RerunConstructionScripts();
		}
	}
}
#endif

void UItemContainerComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(UItemContainerComponent, InventoryIndices, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UItemContainerComponent, InventoryAssets, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UItemContainerComponent, InventoryAmounts, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UItemContainerComponent, InventoryDynamicStatsIndices, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UItemContainerComponent, InventoryDynamicStats, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME(UItemContainerComponent, bIsProcessing);
	DOREPLIFETIME(UItemContainerComponent, MaxStackSize);
	DOREPLIFETIME(UItemContainerComponent, InventorySize);
}

#if WITH_EDITOR
void UItemContainerComponent::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	FName PropertyName{};
	if (PropertyChangedEvent.Property)
	{
		PropertyName = PropertyChangedEvent.Property->GetFName();
	}

	// Check if the changed property is InventoryDataAssets
	if (!HasBegunPlay() && !IsBeingDestroyed() && !InventoryIndices.IsEmpty() && (PropertyName == GET_MEMBER_NAME_CHECKED(UItemContainerComponent, InventoryDataAssets) || PropertyName == GET_MEMBER_NAME_CHECKED(UItemContainerComponent, InventoryIndices)))
	{
		AddToRoot();
		Modify();

		// Remove invalid assets first
		for (int I = 0; I < InventoryDataAssets.Num(); I++)
		{
			if (!InventoryIndices.IsValidIndex(I))
			{
				UE_LOG(InventorySystem, Warning, TEXT("[UItemContainerComponent|%s][PostEditChangeProperty]: DataAsset with key %d. No valid InventoryIndicies entry found. Entry was deleted"), *GetFName().ToString(), I);
				InventoryDataAssets.RemoveAt(I);
			}
		}

		InventoryAssets.Empty();
		for (int I = 0; I < InventoryDataAssets.Num(); I++)
		{
			if (InventoryDataAssets.IsValidIndex(I))
			{
				if (const UItemDataAsset* DataAsset = InventoryDataAssets[I]; IsValid(DataAsset) && DataAsset->GetPrimaryAssetId().IsValid() && DataAsset->GetPrimaryAssetId() != FPrimaryAssetId())
				{
					InventoryAssets.Add(DataAsset->GetPrimaryAssetId());
					continue;
				}
			}
			InventoryAssets.Add(FPrimaryAssetId()); // Set to invalid FPrimaryAssetId
			UE_LOG(InventorySystem, Warning, TEXT("[UItemContainerComponent|%s][PostEditChangeProperty]: DataAsset with key %d. No valid object could be cast. FPrimaryAssetId was set to empty"), *GetFName().ToString(), I);
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
					UE_LOG(InventorySystem, Warning, TEXT("[UItemContainerComponent|%s][PostEditChangeProperty]: DataAsset data was changed. Reconstructing FPrimaryAssetsIds"), *GetFName().ToString());
				}
			}
		}
		RemoveFromRoot();

		// Run internal checks
		InternalChecks();
		Super::PostEditChangeProperty(PropertyChangedEvent);
		return;
	}

	if (PropertyName == GET_MEMBER_NAME_CHECKED(UItemContainerComponent, InventoryAssets) || PropertyName == GET_MEMBER_NAME_CHECKED(UItemContainerComponent, InventoryIndices) || PropertyName ==
		GET_MEMBER_NAME_CHECKED(UItemContainerComponent, InventoryAmounts) || PropertyName == GET_MEMBER_NAME_CHECKED(UItemContainerComponent, InventoryDynamicStatsIndices) || PropertyName ==
		GET_MEMBER_NAME_CHECKED(UItemContainerComponent, InventoryDynamicStats) || PropertyName == GET_MEMBER_NAME_CHECKED(UItemContainerComponent, MaxStackSize) || PropertyName == GET_MEMBER_NAME_CHECKED(
			UItemContainerComponent, InventorySize))
	{
		InternalChecks();
	}

	Super::PostEditChangeProperty(PropertyChangedEvent);
}
#endif

bool UItemContainerComponent::InternalChecks(const bool bIsSavePackageEvent)
{
	AddToRoot();
	UAssetManager& Manager = UAssetManager::Get();
	const FName AssetRegistrySearchablePropertyName = GET_MEMBER_NAME_CHECKED(UItemDataAsset, bCanStack);

	if (!Manager.IsValid())
	{
		UE_LOG(InventorySystem, Error, TEXT("[UItemContainerComponent|%s][InternalChecks]: AssetManager is not initialized"), *GetFName().ToString());
		RemoveFromRoot();
		return false;
	}

	if (InventoryIndices.IsEmpty())
	{
		bool IsEmpty = false;
		if (!InventoryDynamicStatsIndices.IsEmpty() || !InventoryDynamicStats.IsEmpty() || !InventoryAmounts.IsEmpty() || !InventoryAssets.IsEmpty())
		{
			IsEmpty = true;
		}
		InventoryDynamicStatsIndices.Empty();
		InventoryDynamicStats.Empty();
		InventoryAmounts.Empty();
		InventoryAssets.Empty();
#if WITH_EDITORONLY_DATA
		if (!InventoryDataAssets.IsEmpty())
		{
			IsEmpty = true;
		}
		InventoryDataAssets.Empty();
		bAllowInventoryEdit = false;
		bAllowInventoryAssetEdit = false;

		if (IsEmpty && !bIsSavePackageEvent)
		{
			InternalSaveAfterCheck();
		}
#endif

		if (IsEmpty)
		{
			UE_LOG(InventorySystem, Warning, TEXT("[UItemContainerComponent|%s][InternalChecks]: No valid indices found. All other properties were reseted. Please fill the InventoryIndices TArray!"), *GetFName().ToString());
			RemoveFromRoot();
			return true;
		}

		RemoveFromRoot();
		return false;
	}
	
	if (!InventoryIndices.IsEmpty() && (InventoryAmounts.IsEmpty() || InventoryAssets.IsEmpty()))
	{
		bAllowInventoryEdit = true;
	}

	bool InternalPreventExecution = false;
	bool IsDataChanged = false;

	// InventoryIndices
	if (InventoryIndices.Contains(0))
	{
		if (InventoryIndices.Num() == 1)
		{
			InventoryIndices[0] = 1;
			IsDataChanged = true;
		}
		else
		{
			bool IsChanged = false;
			if (const int RealIndex = InventoryIndices.Find(0); RealIndex != INDEX_NONE)
			{
				for (int I = 1; I <= GetInventorySizeConfig(); I++)
				{
					if (!InventoryIndices.Contains(I))
					{
						IsChanged = true;
						IsDataChanged = true;
						InventoryIndices[RealIndex] = I;
						UE_LOG(InventorySystem, Warning, TEXT("[UItemContainerComponent|%s][InternalChecks]: InventoryIndices slot 0 is not a valid slot. Entry was changed to first available slot"), *GetFName().ToString());
						break;
					}
				}
			}
			if (!IsChanged)
			{
				InternalPreventExecution = true;
				IsDataChanged = true;
				UE_LOG(InventorySystem, Warning, TEXT("[UItemContainerComponent|%s][InternalChecks]: InventoryIndices no valid or free slot found. Entry was deleted"), *GetFName().ToString());
				InventoryIndices.Remove(0);
			}
		}
	}

	TSet<int> UniqueSet;
	TArray<int> UniqueArray;

	for (int Element : InventoryIndices)
	{
		if (UniqueSet.Contains(Element))
		{
			UE_LOG(InventorySystem, Warning, TEXT("[UItemContainerComponent|%s][InternalChecks]: InventoryIndices should be unique, element was removed"), *GetFName().ToString());
			InternalPreventExecution = true;
			IsDataChanged = true;
			continue;
		}

		if (Element <= 0)
		{
			InternalPreventExecution = true;
			IsDataChanged = true;
			UE_LOG(InventorySystem, Warning, TEXT("[UItemContainerComponent|%s][InternalChecks]: InventoryIndices should be bigger or equal to 1. Negativ value found, element was removed"), *GetFName().ToString());
			continue;
		}

		UniqueSet.Add(Element);
		UniqueArray.AddUnique(Element);
	}

	InventoryIndices = UniqueArray;

	if (InventoryIndices.Num() > GetInventorySizeConfig())
	{
		UE_LOG(InventorySystem, Warning, TEXT("[UItemContainerComponent|%s][InternalChecks]: InventoryIndices slots out of range. All indicies above max inventory size were removed"), *GetFName().ToString());
		InventoryIndices.RemoveAt(GetInventorySizeConfig(), InventoryIndices.Num() - GetInventorySizeConfig());
		IsDataChanged = true;
	}

#if WITH_EDITORONLY_DATA
	if (InventoryIndices.Num())
	{
		bAllowInventoryAssetEdit = HasBegunPlay();
		bAllowInventoryEdit = true;
	}
	else
	{
		bAllowInventoryAssetEdit = false;
		bAllowInventoryEdit = false;
	}
#endif

	// InventoryAmount
	for (int I = 0; I < InventoryAmounts.Num(); I++)
	{
		if (!InventoryIndices.IsValidIndex(I))
		{
			InternalPreventExecution = true;
			UE_LOG(InventorySystem, Warning, TEXT("[UItemContainerComponent|%s][InternalChecks]: InventoryAmounts has no valid InventoryIndices parent slot. All entries deleted"), *GetFName().ToString());
			InventoryAmounts.RemoveAt(I);
			IsDataChanged = true;
			break;
		}

		if (InventoryAmounts[I] <= 0)
		{
			UE_LOG(InventorySystem, Warning, TEXT("[UItemContainerComponent|%s][InternalChecks]: InventoryAmounts can't be smaller or equal to 0. Entry was changed to 1"), *GetFName().ToString());
			InventoryAmounts[I] = 1;
			IsDataChanged = true;
		}


		FAssetData AssetData;
		if (InventoryAssets.IsValidIndex(I))
		{
			Manager.GetPrimaryAssetData(InventoryAssets[I], AssetData);
		}

		if (AssetData.IsValid())
		{
			if (bool TempCanStack = false; AssetData.GetTagValue(AssetRegistrySearchablePropertyName, TempCanStack))
			{
				Manager.UnloadPrimaryAsset(InventoryAssets[I]);

				if (!TempCanStack && InventoryAmounts[I] > 1)
				{
					UE_LOG(InventorySystem, Warning, TEXT("[UItemContainerComponent|%s][InternalChecks]: InventoryAmounts can't be greater then 1 if parent DataAsset disallows stacking. Entry was changed to 1"), *GetFName().ToString());
					InventoryAmounts[I] = 1;
				}
			}
		}


		if (InventoryAmounts[I] > GetStackSizeConfig())
		{
			UE_LOG(InventorySystem, Warning, TEXT("[UItemContainerComponent|%s][InternalChecks]: InventoryAmounts can't be greater then max stack config. Amount was changed to max stack size"), *GetFName().ToString());
			InventoryAmounts[I] = GetStackSizeConfig();
			IsDataChanged = true;
		}
	}

	// InventoryDynamicStatsIndices
	if (InventoryDynamicStatsIndices.Contains(0))
	{
		if (InventoryDynamicStatsIndices.Num() == 1)
		{
			IsDataChanged = true;
			InventoryDynamicStatsIndices[0] = 1;
		}
		else
		{
			bool IsChanged = false;
			if (const int RealInventoryDynamicStatsIndex = InventoryDynamicStatsIndices.Find(0); RealInventoryDynamicStatsIndex != INDEX_NONE)
			{
				for (int I = 1; I <= GetInventorySizeConfig(); I++)
				{
					if (InventoryIndices.Contains(I) && !InventoryDynamicStatsIndices.Contains(I))
					{
						IsChanged = true;
						IsDataChanged = true;
						InventoryDynamicStatsIndices[RealInventoryDynamicStatsIndex] = I;
						UE_LOG(InventorySystem, Warning, TEXT("[UItemContainerComponent|%s][InternalChecks]: InventoryDynamicStatsIndices slot 0 is not a valid slot. Entry was changed to first available slot"), *GetFName().ToString());
						break;
					}
				}
			}
			if (!IsChanged)
			{
				InternalPreventExecution = true;
				IsDataChanged = true;
				UE_LOG(InventorySystem, Warning, TEXT("[UItemContainerComponent|%s][InternalChecks]: InventoryDynamicStatsIndices no valid or free slot found. Entry was deleted. Please add more slots to the InventoryIndicies"),
				       *GetFName().ToString());
				InventoryDynamicStatsIndices.Remove(0);
			}
		}
	}

	UniqueSet.Empty();
	UniqueArray.Empty();

	for (int Element : InventoryDynamicStatsIndices)
	{
		// Check if valid slot
		if (const int RealIndex = InventoryIndices.Find(Element); RealIndex == INDEX_NONE)
		{
			InternalPreventExecution = true;
			IsDataChanged = true;
			UE_LOG(InventorySystem, Warning, TEXT("[UItemContainerComponent|%s][InternalChecks]: InventoryDynamicStatsIndices slot is not a valid slot, element was removed"), *GetFName().ToString());
			continue;
		}

		// Check unique
		if (UniqueSet.Contains(Element))
		{
			InternalPreventExecution = true;
			IsDataChanged = true;
			UE_LOG(InventorySystem, Warning, TEXT("[UItemContainerComponent|%s][InternalChecks]: InventoryDynamicStatsIndices should be unique, element was removed"), *GetFName().ToString());
			continue;
		}

		// Check negative
		if (Element <= 0)
		{
			InternalPreventExecution = true;
			IsDataChanged = true;
			UE_LOG(InventorySystem, Warning, TEXT("[UItemContainerComponent|%s][InternalChecks]: InventoryDynamicStatsIndices should be postive, element was removed"), *GetFName().ToString());
			continue;
		}

		UniqueSet.Add(Element);
		UniqueArray.AddUnique(Element);
	}

	InventoryDynamicStatsIndices = UniqueArray;

	// InventoryDynamicStats
	if (InventoryDynamicStatsIndices.IsEmpty() && !InventoryDynamicStats.IsEmpty())
	{
		InventoryDynamicStats.Empty();
		UE_LOG(InventorySystem, Warning, TEXT("[UItemContainerComponent|%s][InternalChecks]: InventoryDynamicStats InventoryDynamicStatsIndices has no entries. All elements removed"), *GetFName().ToString());
		IsDataChanged = true;
	}

	for (int I = 0; I < InventoryDynamicStats.Num(); I++)
	{
		if (!InventoryDynamicStatsIndices.IsValidIndex(I))
		{
			InternalPreventExecution = true;
			UE_LOG(InventorySystem, Warning, TEXT("[UItemContainerComponent|%s][InternalChecks]: InventoryDynamicStats has no valid InventoryDynamicStatsIndices parent entry. Element was removed"), *GetFName().ToString());
			InventoryDynamicStats.RemoveAt(I);
			IsDataChanged = true;
		}
	}

	// InventoryAssets
	for (int I = 0; I < InventoryAssets.Num(); I++)
	{
		if (!InventoryIndices.IsValidIndex(I))
		{
			InternalPreventExecution = true;
			UE_LOG(InventorySystem, Warning, TEXT("[UItemContainerComponent|%s][InternalChecks]: InventoryAsset has no valid InventoryIndices. Element was removed"), *GetFName().ToString());
			InventoryAssets.RemoveAt(I);
			IsDataChanged = true;
			continue;
		}

		if (!InventoryAssets[I].IsValid() || InventoryAssets[I] == FPrimaryAssetId())
		{
			InternalPreventExecution = true;
			UE_LOG(InventorySystem, Warning, TEXT("[UItemContainerComponent|%s][InternalChecks]: InventoryAsset is not valid. Check InventoryDataAssets before play"), *GetFName().ToString());
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

void UItemContainerComponent::OnRegister()
{
	Super::OnRegister();

	InternalChecks(true);
}

#if WITH_EDITOR
void UItemContainerComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	bHasBegunPlayEditor = false;
	ChangedInventorySlotsDelegate.RemoveAll(this);
	InternalChecks();
	Super::EndPlay(EndPlayReason);
}
#endif

void UItemContainerComponent::BeginPlay()
{
	Super::BeginPlay();

#if WITH_EDITORONLY_DATA
	bHasBegunPlayEditor = true;
	ChangedInventorySlotsDelegate.AddDynamic(this, &UItemContainerComponent::InternalCheckEditVariables);

	// Additional check if InventoryDataAssets is empty
	if (InventoryDataAssets.IsEmpty())
	{
		if (!InventoryDynamicStatsIndices.IsEmpty() || !InventoryDynamicStats.IsEmpty() || !InventoryAmounts.IsEmpty() || !InventoryAssets.IsEmpty())
		{
			UE_LOG(InventorySystem, Error, TEXT("[UItemContainerComponent|%s][BeginPlay]: No item indices found but data is arrays are filled"), *GetFName().ToString());
			UE_LOG(InventorySystem, Error, TEXT("[UItemContainerComponent|%s][BeginPlay]: Is not setup correctly. Destroying component..."), *GetFName().ToString());
			DestroyComponent();
			return;
		}
	}
#endif

	if (InternalChecks())
	{
		UE_LOG(InventorySystem, Error, TEXT("[UItemContainerComponent|%s][BeginPlay]: Is not setup correctly. Destroying component..."), *GetFName().ToString());
		DestroyComponent();
		return;
	}

#if WITH_EDITORONLY_DATA
	InventoryDataAssets.Empty();
#endif

	bIsProcessing = false;
}

TArray<FInventorySlot> UItemContainerComponent::GetInventorySlots() const
{
	if (InventoryIndices.IsEmpty())
	{
		return {};
	}

	TArray<FInventorySlot> InventorySlots;
	for (const int Slot: InventoryIndices)
	{
		if (FInventorySlot NewSlot = GetInventorySlot(Slot); NewSlot.Slot != INDEX_NONE)
		{
			InventorySlots.Add(NewSlot);
			continue;
		}

		InventorySlots.Empty();
		break;
	}

	return InventorySlots;
}

FInventorySlot UItemContainerComponent::GetInventorySlot(const int Slot) const
{
	if (const int RealInventoryIndex = InventoryIndices.Find(Slot); RealInventoryIndex != INDEX_NONE && InventoryAmounts.IsValidIndex(RealInventoryIndex) && InventoryAssets.IsValidIndex(RealInventoryIndex))
	{
		FItemProperties DynamicStats;
		if (const int RealInventoryDynamicStatsIndex = InventoryDynamicStatsIndices.Find(Slot); RealInventoryDynamicStatsIndex != INDEX_NONE)
		{
			if (!InventoryDynamicStats.IsValidIndex(RealInventoryDynamicStatsIndex))
			{
				UE_LOG(InventorySystem, Error, TEXT("[UItemContainerComponent|%s][GetInventorySlot]: InventoryDynamicStats is not filled but has an InventoryDynamicStatsIndices entry"), *GetFName().ToString());
				return FInventorySlot{};
			}
			
			DynamicStats = InventoryDynamicStats[RealInventoryDynamicStatsIndex];
		}
		return FInventorySlot{InventoryIndices[RealInventoryIndex], InventoryAssets[RealInventoryIndex], DynamicStats, InventoryAmounts[RealInventoryIndex]};
	}

	return FInventorySlot{};
}

bool UItemContainerComponent::HasItemProperty(const int Slot, const FName Name, const bool bIsEquipment)
{
	if (const int Index = InventoryIndices.Find(Slot); Index == INDEX_NONE || Name.IsNone() || bIsEquipment)
	{
		UE_LOG(InventorySystem, Error, TEXT("[UItemContainerComponent|%s][HasItemProperty]: Data invalid for slot %d"), *GetFName().ToString(), Slot);
		return false;
	}

	if (const int InventoryDynamicStatsIndex = InventoryDynamicStatsIndices.Find(Slot); InventoryDynamicStatsIndex != INDEX_NONE && InventoryDynamicStats.IsValidIndex(InventoryDynamicStatsIndex))
	{
		const TArray<FItemProperty>* DynamicStatsItemProperties = &InventoryDynamicStats[InventoryDynamicStatsIndex].ItemProperties;
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

FItemProperty UItemContainerComponent::GetItemProperty(const int Slot, const FName Name, const bool bIsEquipment)
{
	if (const int Index = InventoryIndices.Find(Slot); Index == INDEX_NONE || Name.IsNone() || bIsEquipment)
	{
		UE_LOG(InventorySystem, Error, TEXT("[UItemContainerComponent|%s][GetItemProperty]: Data invalid for slot %d"), *GetFName().ToString(), Slot);
		return {};
	}

	if (const int InventoryDynamicStatsIndex = InventoryDynamicStatsIndices.Find(Slot); InventoryDynamicStatsIndex != INDEX_NONE && InventoryDynamicStats.IsValidIndex(InventoryDynamicStatsIndex))
	{
		const TArray<FItemProperty>* DynamicStatsItemProperties = &InventoryDynamicStats[InventoryDynamicStatsIndex].ItemProperties;
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

bool UItemContainerComponent::SetSlotAmount_Validate(const int Slot, const int Amount, const bool bIsEquipment)
{
	return true;
}

void UItemContainerComponent::SetSlotAmount_Implementation(const int Slot, const int Amount, const bool bIsEquipment)
{
	if (bIsProcessing)
	{
		UE_LOG(InventorySystem, Error, TEXT("[UItemContainerComponent|%s][SetSlotAmount]: Component is still processing previous request"), *GetFName().ToString());
		SetSlotAmountSuccessDelegate.Broadcast(false, Slot, bIsEquipment);
		return;
	}

	bIsProcessing = true;

	if (bIsEquipment)
	{
		UE_LOG(InventorySystem, Error, TEXT("[UItemContainerComponent|%s][SetSlotAmount]: Data invalid for slot %d"), *GetFName().ToString(), Slot);
		return;
	}

	// Inventory
	if (const int AmountIndex = InventoryIndices.Find(Slot); AmountIndex != INDEX_NONE && InventoryAssets.IsValidIndex(AmountIndex) && Amount > 0 && Amount <= GetStackSizeConfig())
	{
		bool TempCanStack = false;
		UAssetManager& Manager = UAssetManager::Get();
		if (!Manager.IsValid())
		{
			UE_LOG(InventorySystem, Error, TEXT("[UItemContainerComponent|%s][SetSlotAmount]: AssetManager is not initialized. Unable to set TempCanStack value"), *GetFName().ToString());
			SetSlotAmountSuccessDelegate.Broadcast(false, Slot, bIsEquipment);
			bIsProcessing = false;
			return;
		}

		FAssetData AssetData;
		Manager.GetPrimaryAssetData(InventoryAssets[AmountIndex], AssetData);
		if (!AssetData.IsValid())
		{
			UE_LOG(InventorySystem, Error, TEXT("[UItemContainerComponent|%s][SetSlotAmount]: AssetData is not valid. Unable to set TempCanStack value"), *GetFName().ToString());
			SetSlotAmountSuccessDelegate.Broadcast(false, Slot, bIsEquipment);
			bIsProcessing = false;
			return;
		}

		const FName AssetRegistrySearchablePropertyName = GET_MEMBER_NAME_CHECKED(UItemDataAsset, bCanStack);
		AssetData.GetTagValue(AssetRegistrySearchablePropertyName, TempCanStack);

		if (!TempCanStack && Amount > 1)
		{
			UE_LOG(InventorySystem, Log, TEXT("[UItemContainerComponent|%s][SetSlotAmount]: Amount was set to 1 as item is not stackable!"), *GetFName().ToString());
			InventoryAmounts[AmountIndex] = 1;
		}
		else
		{
			InventoryAmounts[AmountIndex] = Amount;
		}

		SetSlotAmountSuccessDelegate.Broadcast(true, Slot, bIsEquipment);
		ChangedInventorySlotsDelegate.Broadcast({Slot});
		bIsProcessing = false;
		return;
	}

	UE_LOG(InventorySystem, Log, TEXT("[UItemContainerComponent|%s][SetSlotAmount]: Amount of item could not be set: %d"), *GetFName().ToString(), Slot);
	SetSlotAmountSuccessDelegate.Broadcast(false, Slot, bIsEquipment);
	bIsProcessing = false;
}

bool UItemContainerComponent::SetSlotItemProperty_Validate(const int Slot, const FName Name, const FText& DisplayName, const FText& Value, const bool bIsEquipment)
{
	return true;
}

void UItemContainerComponent::SetSlotItemProperty_Implementation(const int Slot, const FName Name, const FText& DisplayName, const FText& Value, const bool bIsEquipment)
{
	if (bIsProcessing)
	{
		UE_LOG(InventorySystem, Warning, TEXT("[UItemContainerComponent|%s][SetSlotItemProperty]: Component is still processing previous request"), *GetFName().ToString());
		SetSlotItemPropertySuccessDelegate.Broadcast(false, Slot, bIsEquipment);
		return;
	}

	bIsProcessing = true;

	const int InventoryDynamicStatsIndex = InventoryDynamicStatsIndices.Find(Slot);
	if (const int InventoryIndex = InventoryIndices.Find(Slot); InventoryIndex == INDEX_NONE || Name.IsNone() || bIsEquipment)
	{
		UE_LOG(InventorySystem, Error, TEXT("[UItemContainerComponent|%s][SetSlotItemProperty]: Data invalid for slot %d"), *GetFName().ToString(), Slot);
		SetSlotItemPropertySuccessDelegate.Broadcast(false, Slot, bIsEquipment);
		bIsProcessing = false;
		return;
	}

	if (InventoryDynamicStatsIndex == INDEX_NONE)
	{
		TArray<FItemProperty> NewItemProperties;
		NewItemProperties.Add(FItemProperty{Name, DisplayName, Value});
		// Something is wrong! Should not be filled without index
		if (const int NewInventoryDynamicStatsIndex = InventoryDynamicStatsIndices.AddUnique(Slot); InventoryDynamicStats.IsValidIndex(NewInventoryDynamicStatsIndex))
		{
			UE_LOG(InventorySystem, Error, TEXT("[UItemContainerComponent|%s][SetSlotItemProperty]: InventoryDynamicStats should not be filled. Index was just created"), *GetFName().ToString());
			// Revert back
			InventoryDynamicStatsIndices.RemoveAt(NewInventoryDynamicStatsIndex);
			SetSlotItemPropertySuccessDelegate.Broadcast(false, Slot, bIsEquipment);
			bIsProcessing = false;
			return;
		}
		InventoryDynamicStats.Add(FItemProperties{NewItemProperties});
		SetSlotItemPropertySuccessDelegate.Broadcast(true, Slot, bIsEquipment);
		ChangedInventorySlotsDelegate.Broadcast({Slot});
		bIsProcessing = false;
		return;
	}

	// Something is wrong! Should be filled with stats
	if (!InventoryDynamicStats.IsValidIndex(InventoryDynamicStatsIndex))
	{
		UE_LOG(InventorySystem, Error, TEXT("[UItemContainerComponent|%s][SetSlotItemProperty]: InventoryDynamicStats is not filled but has an InventoryDynamicStatsIndices entry"), *GetFName().ToString());
		SetSlotItemPropertySuccessDelegate.Broadcast(false, Slot, bIsEquipment);
		bIsProcessing = false;
		return;
	}

	FItemProperty DeleteItemProperty; 
	for (FItemProperty& ItemProperty : InventoryDynamicStats[InventoryDynamicStatsIndex].ItemProperties)
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
			ChangedInventorySlotsDelegate.Broadcast({Slot});
			bIsProcessing = false;
			return;
		}
	}

	if (!DeleteItemProperty.Name.IsNone())
	{
		InventoryDynamicStats[InventoryDynamicStatsIndex].ItemProperties.Remove(DeleteItemProperty);
		if (InventoryDynamicStats[InventoryDynamicStatsIndex].ItemProperties.IsEmpty())
		{
			InventoryDynamicStatsIndices.RemoveAt(InventoryDynamicStatsIndex);
			InventoryDynamicStats.RemoveAt(InventoryDynamicStatsIndex);
		}
		SetSlotItemPropertySuccessDelegate.Broadcast(true, Slot, bIsEquipment);
		ChangedInventorySlotsDelegate.Broadcast({Slot});
		bIsProcessing = false;
		return;	
	}

	InventoryDynamicStats[InventoryDynamicStatsIndex].ItemProperties.Add(FItemProperty{Name, DisplayName, Value});
	SetSlotItemPropertySuccessDelegate.Broadcast(true, Slot, bIsEquipment);
	ChangedInventorySlotsDelegate.Broadcast({Slot});
	bIsProcessing = false;
}

void UItemContainerComponent::FindItemStack(const FPrimaryAssetId& InventoryAsset, int& Index, int& Amount, bool& bSuccess, const FItemProperties DynamicStats, const int& ItemAmount, const bool bReturnFullStack, const TArray<int> IgnoreInventorySlots)
{
	Index = INDEX_NONE;
	bSuccess = false;
	Amount = INDEX_NONE;

	if (!IgnoreInventorySlots.IsEmpty())
	{
		int MaxArrayValue = INDEX_NONE;
		int IndexMaxValue = INDEX_NONE;
		UKismetMathLibrary::MaxOfIntArray(IgnoreInventorySlots, IndexMaxValue, MaxArrayValue);
		if (GetInventorySizeConfig() < MaxArrayValue)
		{
			UE_LOG(InventorySystem, Error, TEXT("[UItemContainerComponent|%s][FindItemStack]: One or more slots in the IgnoreInventorySlot array could not be found"), *GetFName().ToString());
			return;
		}
	}

	UAssetManager& Manager = UAssetManager::Get();
	if (!Manager.IsValid() || !InventoryAsset.IsValid() || InventoryAsset == FPrimaryAssetId() || ItemAmount < INDEX_NONE || ItemAmount == 0)
	{
		UE_LOG(InventorySystem, Error, TEXT("[UItemContainerComponent|%s][FindItemStack]: AssetManager is not initialized or item data is invalid"), *GetFName().ToString());
		return;
	}

	FAssetData AssetData;
	Manager.GetPrimaryAssetData(InventoryAsset, AssetData);

	if (!AssetData.IsValid())
	{
		UE_LOG(InventorySystem, Error, TEXT("[UItemContainerComponent|%s][FindItemStack]: AssetData is not valid. Unable to set TempCanStack value"), *GetFName().ToString());
		return;
	}

	bool TempCanStack = false;
	const FName AssetRegistrySearchablePropertyName = GET_MEMBER_NAME_CHECKED(UItemDataAsset, bCanStack);
	AssetData.GetTagValue(AssetRegistrySearchablePropertyName, TempCanStack);
	
	if (TempCanStack && InventoryIndices.Num())
	{
		TArray<int> TempInventoryIndices = InventoryIndices;
		TempInventoryIndices.Sort([](const int& Slot, const int& OtherSlot) {
			return Slot < OtherSlot;
		});

		for (const int Slot : TempInventoryIndices)
		{
			if (!IgnoreInventorySlots.Contains(Slot))
			{
				if (const int FoundIndex = InventoryIndices.Find(Slot); FoundIndex != INDEX_NONE && InventoryAssets[FoundIndex] == InventoryAsset)
				{
					const int NewAmount = ItemAmount > 0 ? InventoryAmounts[FoundIndex] + ItemAmount : InventoryAmounts[FoundIndex];
					if (bReturnFullStack)
					{
						if ((ItemAmount == INDEX_NONE && InventoryAmounts[FoundIndex] > GetStackSizeConfig()) || (ItemAmount != INDEX_NONE && NewAmount > GetStackSizeConfig()))
						{
							continue;
						}
					}
					else
					{
						if ((ItemAmount == INDEX_NONE && InventoryAmounts[FoundIndex] >= GetStackSizeConfig()) || (ItemAmount != INDEX_NONE && NewAmount >= GetStackSizeConfig()))
						{
							continue;
						}
					}

					const int DynamicStatsIndex = InventoryDynamicStatsIndices.Find(Slot);
					if (DynamicStatsIndex != INDEX_NONE && !DynamicStats.ItemProperties.IsEmpty())
					{
						if (!InventoryDynamicStats.IsValidIndex(DynamicStatsIndex))
						{
							UE_LOG(InventorySystem, Error, TEXT("[UItemContainerComponent|%s][FindItemStack]: InventoryDynamicStats is not filled but has an InventoryDynamicStatsIndices entry"), *GetFName().ToString());
							return;
						}

						if (InventoryDynamicStats[DynamicStatsIndex] != DynamicStats)
						{
							continue;
						}
					}
					else if (DynamicStatsIndex != INDEX_NONE || !DynamicStats.ItemProperties.IsEmpty())
					{
						continue;
					}
					
					Index = FoundIndex;
					Amount = NewAmount;
					bSuccess = true;
					return;
				}
			}
		}
	}
	
	bSuccess = false;
}

void UItemContainerComponent::FindNextEmptySlot(int& Slot, bool& bSuccess, const TArray<int> IgnoreInventorySlots) const
{
	Slot = INDEX_NONE;
	bSuccess = false;

	if (!IgnoreInventorySlots.IsEmpty())
	{
		int MaxArrayValue = INDEX_NONE;
		int IndexMaxValue = INDEX_NONE;
		UKismetMathLibrary::MaxOfIntArray(IgnoreInventorySlots, IndexMaxValue, MaxArrayValue);
		if (GetInventorySizeConfig() < MaxArrayValue)
		{
			UE_LOG(InventorySystem, Error, TEXT("[UItemContainerComponent|%s][FindNextEmptySlot]: One or more slots in the IgnoreInventorySlot array could not be found"), *GetFName().ToString());
			return;
		}
	}

	if (InventoryIndices.Num() >= GetInventorySizeConfig())
	{
		UE_LOG(InventorySystem, Warning, TEXT("[UItemContainerComponent|%s][FindNextEmptySlot]: No empty slot available"), *GetFName().ToString());
		return;
	}

	for (int I = 1; I <= GetInventorySizeConfig(); I++)
	{
		if (!InventoryIndices.Contains(I) && !IgnoreInventorySlots.Contains(I))
		{
			Slot = I;
			bSuccess = true;
			break;
		}
	}
}

bool UItemContainerComponent::AddItemToComponent_Validate(const int Slot, UItemContainerComponent* ItemContainerComponent, const int Amount, const bool bCanStack, const bool bRevertWhenFull)
{
	return true;
}

void UItemContainerComponent::AddItemToComponent_Implementation(const int Slot, UItemContainerComponent* ItemContainerComponent, const int Amount, const bool bCanStack, const bool bRevertWhenFull)
{
	if (bIsProcessing)
	{
		UE_LOG(InventorySystem, Warning, TEXT("[UItemContainerComponent|%s][AddItemToComponent]: Component is still processing previous request"), *GetFName().ToString());
		AddItemToComponentSuccessDelegate.Broadcast(false, Slot, true, nullptr);
		return;
	}

	bIsProcessing = true;

	if (!IsValid(ItemContainerComponent) || ItemContainerComponent->bIsProcessing)
	{
		UE_LOG(InventorySystem, Error, TEXT("[UItemContainerComponent|%s][AddItemToComponent]: Other component is invalid"), *GetFName().ToString());
		AddItemToComponentSuccessDelegate.Broadcast(false, Slot, true, nullptr);
		bIsProcessing = false;
		return;
	}

	ItemContainerComponent->bIsProcessing = true;
	ItemContainerComponent->AddItemToComponentOtherComponentStartDelegate.Broadcast();

	// Call internal function
	int ItemsLeft = Amount;
	if (const TArray<int> ChangedSlotsOtherComponent = AddItemToComponentInternal(Slot, ItemContainerComponent, ItemsLeft, bCanStack, false, bRevertWhenFull); !ChangedSlotsOtherComponent.IsEmpty())
	{
		AddItemToComponentSuccessDelegate.Broadcast(true, Slot, ItemsLeft, ItemContainerComponent);
		ItemContainerComponent->AddItemToComponentOtherComponentSuccessDelegate.Broadcast(true, Slot, ItemsLeft, this);
		ChangedInventorySlotsDelegate.Broadcast({Slot});
		ItemContainerComponent->ChangedInventorySlotsDelegate.Broadcast(ChangedSlotsOtherComponent);
		bIsProcessing = false;
		ItemContainerComponent->bIsProcessing = false;
		return;
	}
	
	AddItemToComponentSuccessDelegate.Broadcast(false, Slot, Amount, ItemContainerComponent);
	ItemContainerComponent->AddItemToComponentOtherComponentSuccessDelegate.Broadcast(false, Slot, Amount, this);
	bIsProcessing = false;
	ItemContainerComponent->bIsProcessing = false;
}

TArray<int> UItemContainerComponent::AddItemToComponentInternal(const int Slot, UItemContainerComponent* ItemContainerComponent, int& Amount, const bool bCanStack, const bool bIsEquipment, const bool bRevertWhenFull)
{
	const int Index = InventoryIndices.Find(Slot);
	TArray<int> ChangedSlots;
	if (bIsEquipment || Amount <= 0 || Index == INDEX_NONE || !InventoryAssets.IsValidIndex(Index) || !InventoryAssets[Index].IsValid() || InventoryAssets[Index] == FPrimaryAssetId() || !InventoryAmounts.IsValidIndex(Index) || InventoryAmounts[Index] <= 0 || Amount > InventoryAmounts[Index])
	{
		UE_LOG(InventorySystem, Error, TEXT("[UItemContainerComponent|%s][AddItemToComponentInternal]: Data invalid for slot %d"), *GetFName().ToString(), Slot);
		return ChangedSlots;;
	}

	if (!IsValid(ItemContainerComponent))
	{
		UE_LOG(InventorySystem, Error, TEXT("[UItemContainerComponent|%s][AddItemToComponentInternal]: Other component is invalid"), *GetFName().ToString());
		return ChangedSlots;
	}

	FItemProperties DynamicStats{};
	const int RealInventoryDynamicStatsIndicesIndex = InventoryDynamicStatsIndices.Find(Slot);
	if (RealInventoryDynamicStatsIndicesIndex != INDEX_NONE)
	{
		if (!InventoryDynamicStats.IsValidIndex(RealInventoryDynamicStatsIndicesIndex))
		{
			UE_LOG(InventorySystem, Error, TEXT("[UItemContainerComponent|%s][AddItemToComponentInternal]: InventoryDynamicStats is not filled but has an InventoryDynamicStatsIndices entry"), *GetFName().ToString());
			return ChangedSlots;
		}

		DynamicStats = InventoryDynamicStats[RealInventoryDynamicStatsIndicesIndex];
	}
	
	int ItemsLeft = Amount;
	ChangedSlots = ItemContainerComponent->AddItemInternal(InventoryAssets[Index], DynamicStats, ItemsLeft, bCanStack, bRevertWhenFull);
	if (!ChangedSlots.IsEmpty())
	{
		InventoryAmounts[Index] -= Amount - ItemsLeft;
		Amount = ItemsLeft;
		if (InventoryAmounts[Index] == 0)
		{
			InventoryIndices.RemoveAt(Index);
			InventoryAssets.RemoveAt(Index);
			InventoryAmounts.RemoveAt(Index);

			if (RealInventoryDynamicStatsIndicesIndex != INDEX_NONE)
			{
				InventoryDynamicStatsIndices.RemoveAt(RealInventoryDynamicStatsIndicesIndex);
				InventoryDynamicStats.RemoveAt(RealInventoryDynamicStatsIndicesIndex);
			}
			
			return ChangedSlots;
		}
		
		return ChangedSlots;
	}

	UE_LOG(InventorySystem, Log, TEXT("[UItemContainerComponent|%s][AddItemToComponentInternal]: Item could not be added to other component"), *GetFName().ToString());
	return ChangedSlots;
}

bool UItemContainerComponent::AddItem_Validate(const FPrimaryAssetId InventoryAsset, const FItemProperties DynamicStats, const int Amount, const bool bCanStack, const bool bRevertWhenFull)
{
	return true;
}

void UItemContainerComponent::AddItem_Implementation(const FPrimaryAssetId InventoryAsset, const FItemProperties DynamicStats, const int Amount, const bool bCanStack, const bool bRevertWhenFull)
{
	if (bIsProcessing)
	{
		UE_LOG(InventorySystem, Warning, TEXT("[UItemContainerComponent|%s][AddItem]: Component is still processing previous request"), *GetFName().ToString());
		AddItemFailureDelegate.Broadcast(InventoryAsset, DynamicStats, Amount);
		return;
	}

	bIsProcessing = true;
	int ItemAmount = Amount;
	const TArray<int> ChangedSlots = AddItemInternal(InventoryAsset, DynamicStats, ItemAmount, bCanStack, bRevertWhenFull);

	if (ChangedSlots.IsEmpty())
	{
		AddItemFailureDelegate.Broadcast(InventoryAsset, DynamicStats, bRevertWhenFull ? Amount : ItemAmount);
		bIsProcessing = false;
		return;
	}

	AddItemSuccessDelegate.Broadcast(ItemAmount, ChangedSlots);
	ChangedInventorySlotsDelegate.Broadcast(ChangedSlots);
	
	bIsProcessing = false;
}

TArray<int> UItemContainerComponent::AddItemInternal(const FPrimaryAssetId& InventoryAsset, const FItemProperties& DynamicStats, int& Amount, const bool bCanStack, const bool bRevertWhenFull)
{
	if (Amount <= 0 || !InventoryAsset.IsValid() || InventoryAsset == FPrimaryAssetId())
	{
		UE_LOG(InventorySystem, Error, TEXT("[UItemContainerComponent|%s][AddItem]: InventoryAsset data invalid"), *GetFName().ToString());
		return {};
	}

	TArray<int> ChangedSlots{};
	const TArray<int> TempInventorySlots = InventoryIndices;
	const TArray<int> TempInventoryAmounts = InventoryAmounts;
	const TArray<FPrimaryAssetId> TempInventoryAssets = InventoryAssets;
	const TArray<int> TempInventoryDynamicStatsIndices = InventoryDynamicStatsIndices;
	const TArray<FItemProperties> TempInventoryDynamicStats = InventoryDynamicStats;

	bool TempCanStack = false;
	UAssetManager& Manager = UAssetManager::Get();
	if (!Manager.IsValid())
	{
		UE_LOG(InventorySystem, Error, TEXT("[UInventorySystemComponent|%s][AddItem]: AssetManager is not initialized. Unable to set TempCanStack value"), *GetFName().ToString());
		return {};
	}

	FAssetData AssetData;
	Manager.GetPrimaryAssetData(InventoryAsset, AssetData);
	if (!AssetData.IsValid())
	{
		UE_LOG(InventorySystem, Error, TEXT("[UInventorySystemComponent|%s][AddItem]: AssetData is not valid. Unable to set TempCanStack value"), *GetFName().ToString());
		return {};
	}

	const FName AssetRegistrySearchablePropertyName = GET_MEMBER_NAME_CHECKED(UItemDataAsset, bCanStack);
	AssetData.GetTagValue(AssetRegistrySearchablePropertyName, TempCanStack);
	std::function<bool()> AddNewItem = [&]()
	{
		bool bSuccess = false;
		int Index = INDEX_NONE;

		if (bCanStack && TempCanStack)
		{
			int FoundAmount = INDEX_NONE;
			FindItemStack(InventoryAsset, Index, FoundAmount, bSuccess, DynamicStats);
			if (bSuccess && FoundAmount > 0)
			{
				if (FoundAmount + Amount <= GetStackSizeConfig())
				{
					ChangedSlots.Add(InventoryIndices[Index]);
					InventoryAmounts[Index] = FoundAmount + Amount;
					Amount = 0;
					return true;
				}
				
				InventoryAmounts[Index] = GetStackSizeConfig();
				Amount = FoundAmount + Amount - GetStackSizeConfig();
				ChangedSlots.Add(InventoryIndices[Index]);
				if (Amount > 0)
				{
					return AddNewItem();
				}
				return true;
			}
		}

		if (Amount > GetStackSizeConfig() || (!TempCanStack && Amount > 1))
		{
			const int SlotsToBeFilled = TempCanStack ? FMath::CeilToInt(static_cast<float>(Amount) / static_cast<float>(GetStackSizeConfig())) : Amount;
			bool bFailedOnce = false;
			for (int I = 1; I <= SlotsToBeFilled; I++)
			{
				if (I > GetInventorySizeConfig())
				{
					bFailedOnce = true;
					break;
				}
				
				FindNextEmptySlot(Index, bSuccess);
				if (bSuccess)
				{
					if (!DynamicStats.ItemProperties.IsEmpty())
					{
						// Something is wrong! Should not be filled without index
						if (const int NewInventoryDynamicStatsIndex = InventoryDynamicStatsIndices.AddUnique(Index); InventoryDynamicStats.IsValidIndex(NewInventoryDynamicStatsIndex))
						{
							UE_LOG(InventorySystem, Error, TEXT("[UItemContainerComponent|%s][AddItem]: InventoryDynamicStats should not be filled. Index was just created"), *GetFName().ToString());
							// Revert back
							InventoryDynamicStatsIndices.RemoveAt(NewInventoryDynamicStatsIndex);
							return false;
						}
						InventoryDynamicStats.Add(DynamicStats);
					}

					InventoryIndices.AddUnique(Index);
					ChangedSlots.Add(Index);
					InventoryAssets.Add(InventoryAsset);
					if (Amount >= GetStackSizeConfig())
					{
						InventoryAmounts.Add(GetStackSizeConfig());
						Amount -= GetStackSizeConfig();
					}
					else
					{
						InventoryAmounts.Add(TempCanStack ? Amount: 1);
						Amount = TempCanStack ? 0: Amount - 1;
					}
					continue;
				}

				bFailedOnce = true;
				break;
			}
			
			if (bFailedOnce)
			{
				if (bRevertWhenFull)
				{
					ChangedSlots.Empty();
					InventoryIndices = TempInventorySlots;
					InventoryAmounts = TempInventoryAmounts;
					InventoryAssets = TempInventoryAssets;
					InventoryDynamicStatsIndices = TempInventoryDynamicStatsIndices;
					InventoryDynamicStats = TempInventoryDynamicStats;

					UE_LOG(InventorySystem, Error, TEXT("[UItemContainerComponent|%s][AddItem]: Item could not be added completely. Reverting already added items and aborting action"), *GetFName().ToString());
					return false;
				}

				UE_LOG(InventorySystem, Error, TEXT("[UItemContainerComponent|%s][AddItem]: Item could not be added completely"), *GetFName().ToString());
			}
			
			return true;
		}

		FindNextEmptySlot(Index, bSuccess);
		if (bSuccess)
		{
			if (!DynamicStats.ItemProperties.IsEmpty())
			{
				// Something is wrong! Should not be filled without index
				if (const int NewInventoryDynamicStatsIndex = InventoryDynamicStatsIndices.AddUnique(Index); InventoryDynamicStats.IsValidIndex(NewInventoryDynamicStatsIndex))
				{
					UE_LOG(InventorySystem, Error, TEXT("[UItemContainerComponent|%s][AddItem]: InventoryDynamicStats should not be filled. Index was just created"), *GetFName().ToString());
					// Revert back
					InventoryDynamicStatsIndices.RemoveAt(NewInventoryDynamicStatsIndex);
					return false;
				}
				InventoryDynamicStats.Add(DynamicStats);
			}

			InventoryIndices.AddUnique(Index);
			InventoryAssets.Add(InventoryAsset);
			InventoryAmounts.Add(Amount);
			Amount = 0;
			ChangedSlots.Add(Index);
			return true;
		}

		if (bRevertWhenFull)
		{
			ChangedSlots.Empty();
			InventoryIndices = TempInventorySlots;
			InventoryAmounts = TempInventoryAmounts;
			InventoryAssets = TempInventoryAssets;
			InventoryDynamicStatsIndices = TempInventoryDynamicStatsIndices;
			InventoryDynamicStats = TempInventoryDynamicStats;

			UE_LOG(InventorySystem, Error, TEXT("[UItemContainerComponent|%s][AddItem]: Item could not be added completely. Reverting already added items and aborting action"), *GetFName().ToString());
			return false;
		}

		UE_LOG(InventorySystem, Log, TEXT("[UItemContainerComponent|%s][AddItem]: Item could not be added"), *GetFName().ToString());
		return false;
	};
	
	AddNewItem();
	return ChangedSlots;
}

bool UItemContainerComponent::AddItemToSlot_Validate(const FPrimaryAssetId InventoryAsset, const int Slot, const FItemProperties DynamicStats, const int Amount, const bool bCanStack, const bool bEnableFallback)
{
	return true;
}

void UItemContainerComponent::AddItemToSlot_Implementation(const FPrimaryAssetId InventoryAsset, const int Slot, const FItemProperties DynamicStats, const int Amount, const bool bCanStack, const bool bEnableFallback)
{
	if (bIsProcessing)
	{
		UE_LOG(InventorySystem, Warning, TEXT("[UItemContainerComponent|%s][AddItemToSlot]: Component is still processing previous request"), *GetFName().ToString());
		AddItemToSlotFailureDelegate.Broadcast(InventoryAsset, Slot, DynamicStats, Amount, bEnableFallback);
		return;
	}

	bIsProcessing = true;

	UAssetManager& Manager = UAssetManager::Get();
	if (Amount <= 0 || !Manager.IsValid() || !InventoryAsset.IsValid() || InventoryAsset == FPrimaryAssetId())
	{
		UE_LOG(InventorySystem, Error, TEXT("[UItemContainerComponent|%s][AddItemToSlot]: AssetManager is not initialized or item data is invalid"), *GetFName().ToString());
		AddItemToSlotFailureDelegate.Broadcast(InventoryAsset, Slot, DynamicStats, Amount, bEnableFallback);
		bIsProcessing = false;
		return;
	}

	FAssetData AssetData;
	Manager.GetPrimaryAssetData(InventoryAsset, AssetData);

	if (!AssetData.IsValid())
	{
		UE_LOG(InventorySystem, Error, TEXT("[UItemContainerComponent|%s][AddItemToSlot]: AssetData is not valid. Unable to set TempCanStack value"), *GetFName().ToString());
		AddItemToSlotFailureDelegate.Broadcast(InventoryAsset, Slot, DynamicStats, Amount, bEnableFallback);
		bIsProcessing = false;
		return;
	}

	bool TempCanStack = false;
	const FName AssetRegistrySearchablePropertyName = GET_MEMBER_NAME_CHECKED(UItemDataAsset, bCanStack);
	AssetData.GetTagValue(AssetRegistrySearchablePropertyName, TempCanStack);

	if (const int RealIndex = InventoryIndices.Find(Slot); RealIndex != INDEX_NONE)
	{
		if (bCanStack && TempCanStack && InventoryAssets.IsValidIndex(RealIndex) && InventoryAsset == InventoryAssets[RealIndex])
		{
			const int TempAmount = InventoryAmounts[RealIndex];
			const int InventoryDynamicStatsIndex = InventoryDynamicStatsIndices.Find(Slot);
			if (!DynamicStats.ItemProperties.IsEmpty() && InventoryDynamicStatsIndex != INDEX_NONE)
			{
				// Something is wrong! Should be filled with stats
				if (!InventoryDynamicStats.IsValidIndex(InventoryDynamicStatsIndex))
				{
					UE_LOG(InventorySystem, Error, TEXT("[UItemContainerComponent|%s][AddItemToSlot]: InventoryDynamicStats is not filled but has an InventoryDynamicStatsIndices entry"), *GetFName().ToString());
					AddItemToSlotFailureDelegate.Broadcast(InventoryAsset, Slot, DynamicStats, Amount, bEnableFallback);
					bIsProcessing = false;
					return;
				}
				

				if (DynamicStats == InventoryDynamicStats[InventoryDynamicStatsIndex])
				{
					if (Amount + InventoryAmounts[RealIndex] <= GetStackSizeConfig())
					{
						InventoryAmounts[RealIndex] += Amount;
						AddItemToSlotSuccessDelegate.Broadcast(INDEX_NONE, Slot, bEnableFallback);
						ChangedInventorySlotsDelegate.Broadcast({Slot});
						bIsProcessing = false;
						return;
					}
					
					int NewAmount = InventoryAmounts[RealIndex] + Amount - GetStackSizeConfig();
					InventoryAmounts[RealIndex] = GetStackSizeConfig();
					if (NewAmount > 0)
					{
						if (bEnableFallback)
						{
							if (TArray<int> ChangedSlots = AddItemInternal(InventoryAsset, DynamicStats, NewAmount, bCanStack, false); !ChangedSlots.IsEmpty())
							{
								AddItemToSlotSuccessDelegate.Broadcast(NewAmount, Slot, bEnableFallback);
								ChangedSlots.Add(Slot);
								ChangedInventorySlotsDelegate.Broadcast(ChangedSlots);
								bIsProcessing = false;
								return;
							}
						}

						UE_LOG(InventorySystem, Log, TEXT("[UItemContainerComponent|%s][AddItemToSlot]: Item could not be added completely"), *GetFName().ToString());

						AddItemToSlotSuccessDelegate.Broadcast(NewAmount, Slot, bEnableFallback);
						ChangedInventorySlotsDelegate.Broadcast({Slot});
						bIsProcessing = false;
						return;
					}

					InventoryAmounts[RealIndex] = TempAmount;
					AddItemToSlotFailureDelegate.Broadcast(InventoryAsset, Slot, DynamicStats, NewAmount, bEnableFallback);
					bIsProcessing = false;
					return;
				}
			}
			else if (DynamicStats.ItemProperties.IsEmpty() && InventoryDynamicStatsIndex == INDEX_NONE)
			{
				if (Amount + InventoryAmounts[RealIndex] <= GetStackSizeConfig())
				{
					InventoryAmounts[RealIndex] += Amount;
					AddItemToSlotSuccessDelegate.Broadcast(INDEX_NONE, Slot, bEnableFallback);
					ChangedInventorySlotsDelegate.Broadcast({Slot});
					bIsProcessing = false;
					return;
				}
				
				int NewAmount = InventoryAmounts[RealIndex] + Amount - GetStackSizeConfig();
				InventoryAmounts[RealIndex] = GetStackSizeConfig();
				if (NewAmount > 0)
				{
					if (bEnableFallback)
					{
						if (TArray<int> ChangedSlots = AddItemInternal(InventoryAsset, DynamicStats, NewAmount, bCanStack, false); !ChangedSlots.IsEmpty())
						{
							AddItemToSlotSuccessDelegate.Broadcast(NewAmount, Slot, bEnableFallback);
							ChangedSlots.Add(Slot);
							ChangedInventorySlotsDelegate.Broadcast(ChangedSlots);
							bIsProcessing = false;
							return;
						}	
					}

					UE_LOG(InventorySystem, Log, TEXT("[UItemContainerComponent|%s][AddItemToSlot]: Item could not be added completely"), *GetFName().ToString());

					AddItemToSlotSuccessDelegate.Broadcast(NewAmount, Slot, bEnableFallback);
					ChangedInventorySlotsDelegate.Broadcast({Slot});
					bIsProcessing = false;
					return;
				}

				InventoryAmounts[RealIndex] = TempAmount;
				AddItemToSlotFailureDelegate.Broadcast(InventoryAsset, Slot, DynamicStats, NewAmount, bEnableFallback);
				bIsProcessing = false;
				return;
			}
		}
	}
	else
	{
		if (!DynamicStats.ItemProperties.IsEmpty())
		{
			// Something is wrong! Should not be filled without index
			if (const int NewInventoryDynamicStatsIndex = InventoryDynamicStatsIndices.AddUnique(Slot); InventoryDynamicStats.IsValidIndex(NewInventoryDynamicStatsIndex))
			{
				UE_LOG(InventorySystem, Error, TEXT("[UItemContainerComponent|%s][AddItemToSlot]: InventoryDynamicStats should not be filled. Index was just created"), *GetFName().ToString());
				// Revert back
				InventoryDynamicStatsIndices.RemoveAt(NewInventoryDynamicStatsIndex);
				AddItemToSlotFailureDelegate.Broadcast(InventoryAsset, Slot, DynamicStats, Amount, bEnableFallback);
				bIsProcessing = false;
				return;
			}
			InventoryDynamicStats.Add(DynamicStats);
		}

		int const RealNewIndex = InventoryIndices.AddUnique(Slot);
		InventoryAssets.Add(InventoryAsset);
		int NewAmount = 0;
		if (TempCanStack)
		{
			if (Amount <= GetStackSizeConfig())
			{
				InventoryAmounts.Add(Amount);	
			}
			else
			{
				InventoryAmounts.Add(GetStackSizeConfig());
			}

			NewAmount = Amount - GetStackSizeConfig();
		}
		else
		{
			InventoryAmounts.Add(1);
			NewAmount = Amount - 1;
		}
		
		if (NewAmount > 0)
		{
			if (bEnableFallback)
			{
				if (TArray<int> ChangedSlots = AddItemInternal(InventoryAsset, DynamicStats, NewAmount, bCanStack, false); !ChangedSlots.IsEmpty())
				{
					AddItemToSlotSuccessDelegate.Broadcast(NewAmount, Slot, bEnableFallback);
					ChangedSlots.Add(Slot);
					ChangedInventorySlotsDelegate.Broadcast(ChangedSlots);
					bIsProcessing = false;
					return;
				}
			}
			
			UE_LOG(InventorySystem, Log, TEXT("[UItemContainerComponent|%s][AddItemToSlot]: Item could not be added completely"), *GetFName().ToString());

			AddItemToSlotSuccessDelegate.Broadcast(NewAmount, Slot, bEnableFallback);
			ChangedInventorySlotsDelegate.Broadcast({Slot});
			bIsProcessing = false;
			return;
		}

		InventoryIndices.RemoveAt(RealNewIndex);
		InventoryAmounts.RemoveAt(RealNewIndex);
		InventoryAssets.RemoveAt(RealNewIndex);
		if (!DynamicStats.ItemProperties.IsEmpty())
		{
			const int NewInventoryDynamicStatsIndex = InventoryDynamicStatsIndices.Find(Slot);
			if (!InventoryDynamicStats.IsValidIndex(NewInventoryDynamicStatsIndex))
			{
				UE_LOG(InventorySystem, Error, TEXT("[UItemContainerComponent|%s][AddItemToSlot]: InventoryDynamicStats is not filled but has an InventoryDynamicStatsIndices entry"), *GetFName().ToString());
				AddItemToSlotFailureDelegate.Broadcast(InventoryAsset, Slot, DynamicStats, NewAmount, bEnableFallback);
				bIsProcessing = false;
				return;
			}

			InventoryDynamicStatsIndices.RemoveAt(NewInventoryDynamicStatsIndex);
			InventoryDynamicStats.RemoveAt(NewInventoryDynamicStatsIndex);
		}
		
		AddItemToSlotFailureDelegate.Broadcast(InventoryAsset, Slot, DynamicStats, NewAmount, bEnableFallback);
		bIsProcessing = false;
		return;
	}

	int NewAmount = Amount;
	if (bEnableFallback)
	{
		if (TArray<int> ChangedSlots = AddItemInternal(InventoryAsset, DynamicStats, NewAmount, bCanStack, false); !ChangedSlots.IsEmpty())
		{
			AddItemToSlotSuccessDelegate.Broadcast(NewAmount, Slot, bEnableFallback);
			ChangedInventorySlotsDelegate.Broadcast(ChangedSlots);
			bIsProcessing = false;
			return;
		}
	}

	UE_LOG(InventorySystem, Log, TEXT("[UItemContainerComponent|%s][AddItemToSlot]: Item could not be added"), *GetFName().ToString());
	AddItemToSlotFailureDelegate.Broadcast(InventoryAsset, Slot, DynamicStats, NewAmount, bEnableFallback);
	bIsProcessing = false;
}

bool UItemContainerComponent::SwapItems_Validate(const int First, const int Second, const bool bCanStack, const bool bIsEquipment)
{
	return true;
}

void UItemContainerComponent::SwapItems_Implementation(const int First, const int Second, const bool bCanStack, const bool bIsEquipment)
{
	if (bIsProcessing)
	{
		UE_LOG(InventorySystem, Warning, TEXT("[UItemContainerComponent|%s][SwapItems]: Component is still processing previous request"), *GetFName().ToString());
		SwapItemSuccessDelegate.Broadcast(false, First, Second, bIsEquipment);
		return;
	}

	bIsProcessing = true;

	if (bIsEquipment)
	{
		UE_LOG(InventorySystem, Warning, TEXT("[UItemContainerComponent|%s][SwapItems]: Tried to call equipment swap on item container"), *GetFName().ToString());
		SwapItemSuccessDelegate.Broadcast(false, First, Second, bIsEquipment);
		bIsProcessing = false;
		return;
	}

	const int FirstIndex = InventoryIndices.Find(First);
	const int SecondIndex = InventoryIndices.Find(Second);

	UAssetManager& Manager = UAssetManager::Get();
	if ((FirstIndex == INDEX_NONE && SecondIndex == INDEX_NONE) || !Manager.IsValid())
	{
		UE_LOG(InventorySystem, Error, TEXT("[UItemContainerComponent|%s][SwapItems]: AssetManager is not initialized or item data is invalid"), *GetFName().ToString());
		SwapItemSuccessDelegate.Broadcast(false, First, Second, bIsEquipment);
		bIsProcessing = false;
		return;
	}

	const int RealFirstInventoryStatsIndex = InventoryDynamicStatsIndices.Find(First);
	const int RealSecondInventoryStatsIndex = InventoryDynamicStatsIndices.Find(Second);

	// Both slots in use
	if (FirstIndex != INDEX_NONE && SecondIndex != INDEX_NONE)
	{
		bool DataInvalid = false;
		if (!InventoryAssets.IsValidIndex(FirstIndex) || !InventoryAmounts.IsValidIndex(FirstIndex))
		{
			UE_LOG(InventorySystem, Error, TEXT("[UItemContainerComponent|%s][SwapItems]: Data invalid for slot %d"), *GetFName().ToString(), First);
			DataInvalid = true;
		}

		if (!InventoryAssets.IsValidIndex(SecondIndex) || !InventoryAmounts.IsValidIndex(SecondIndex))
		{
			UE_LOG(InventorySystem, Error, TEXT("[UItemContainerComponent|%s][SwapItems]: Data invalid for slot %d"), *GetFName().ToString(), Second);
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
		Manager.GetPrimaryAssetData(InventoryAssets[FirstIndex], FirstAssetData);
		Manager.GetPrimaryAssetData(InventoryAssets[SecondIndex], SecondAssetData);

		if (!FirstAssetData.IsValid() || !SecondAssetData.IsValid())
		{
			UE_LOG(InventorySystem, Error, TEXT("[UItemContainerComponent|%s][SwapItems]: Asset data not valid"), *GetFName().ToString());
			SwapItemSuccessDelegate.Broadcast(false, First, Second, bIsEquipment);
			bIsProcessing = false;
			return;
		}

		const FName AssetRegistrySearchablePropertyName = GET_MEMBER_NAME_CHECKED(UItemDataAsset, bCanStack);
		bool FirstTempCanStack = false;
		FirstAssetData.GetTagValue(AssetRegistrySearchablePropertyName, FirstTempCanStack);

		bool SecondTempCanStack = false;
		SecondAssetData.GetTagValue(AssetRegistrySearchablePropertyName, SecondTempCanStack);
		
		// Same item and can stack
		if (bCanStack && SecondTempCanStack && InventoryAssets[FirstIndex] == InventoryAssets[SecondIndex])
		{
			if (RealFirstInventoryStatsIndex != INDEX_NONE && RealSecondInventoryStatsIndex != INDEX_NONE)
			{
				if (!InventoryDynamicStats.IsValidIndex(RealFirstInventoryStatsIndex) || !InventoryDynamicStats.IsValidIndex(RealSecondInventoryStatsIndex))
				{
					UE_LOG(InventorySystem, Error, TEXT("[UItemContainerComponent|%s][SwapItems]: InventoryDynamicStats is not filled but has an InventoryDynamicStatsIndices entry"), *GetFName().ToString());
					SwapItemSuccessDelegate.Broadcast(false, First, Second, bIsEquipment);
					bIsProcessing = false;
					return;
				}
				
				InventoryDynamicStats.RemoveAt(RealFirstInventoryStatsIndex);
				InventoryDynamicStatsIndices.RemoveAt(RealFirstInventoryStatsIndex);

				InventoryAmounts[SecondIndex] += InventoryAmounts[FirstIndex];
				InventoryIndices.RemoveAt(FirstIndex);
				InventoryAmounts.RemoveAt(FirstIndex);
				InventoryAssets.RemoveAt(FirstIndex);

				SwapItemSuccessDelegate.Broadcast(true, First, Second, bIsEquipment);
				ChangedInventorySlotsDelegate.Broadcast({First, Second});
				bIsProcessing = false;
				return;
			}

			if (!InventoryDynamicStats.IsValidIndex(RealFirstInventoryStatsIndex) && !InventoryDynamicStats.IsValidIndex(RealSecondInventoryStatsIndex))
			{
				InventoryAmounts[SecondIndex] += InventoryAmounts[FirstIndex];
				InventoryIndices.RemoveAt(FirstIndex);
				InventoryAmounts.RemoveAt(FirstIndex);
				InventoryAssets.RemoveAt(FirstIndex);

				SwapItemSuccessDelegate.Broadcast(true, First, Second, bIsEquipment);
				ChangedInventorySlotsDelegate.Broadcast({First, Second});
				bIsProcessing = false;
				return;
			}
		}

		if (RealFirstInventoryStatsIndex != INDEX_NONE && RealSecondInventoryStatsIndex != INDEX_NONE)
		{
			InventoryDynamicStats.Swap(FirstIndex, SecondIndex);
		}
		else if (RealFirstInventoryStatsIndex != INDEX_NONE && RealSecondInventoryStatsIndex == INDEX_NONE)
		{
			if (!InventoryDynamicStats.IsValidIndex(RealFirstInventoryStatsIndex))
			{
				UE_LOG(InventorySystem, Error, TEXT("[UItemContainerComponent|%s][SwapItems]: InventoryDynamicStats is not filled but has an InventoryDynamicStatsIndices entry"), *GetFName().ToString());
				SwapItemSuccessDelegate.Broadcast(false, First, Second, bIsEquipment);
				bIsProcessing = false;
				return;
			}
		
			InventoryDynamicStatsIndices.AddUnique(Second);
			FItemProperties TempStats = InventoryDynamicStats[RealFirstInventoryStatsIndex];
			InventoryDynamicStats.Add(TempStats);
			InventoryDynamicStatsIndices.RemoveAt(RealFirstInventoryStatsIndex);
			InventoryDynamicStats.RemoveAt(RealFirstInventoryStatsIndex);
		}
		else if (RealSecondInventoryStatsIndex != INDEX_NONE && RealFirstInventoryStatsIndex == INDEX_NONE)
		{
			if (!InventoryDynamicStats.IsValidIndex(RealSecondInventoryStatsIndex))
			{
				UE_LOG(InventorySystem, Error, TEXT("[UItemContainerComponent|%s][SwapItems]: InventoryDynamicStats is not filled but has an InventoryDynamicStatsIndices entry"), *GetFName().ToString());
				SwapItemSuccessDelegate.Broadcast(false, First, Second, bIsEquipment);
				bIsProcessing = false;
				return;
			}
			
			InventoryDynamicStatsIndices.AddUnique(First);
			FItemProperties TempStats = InventoryDynamicStats[RealSecondInventoryStatsIndex];
			InventoryDynamicStats.Add(TempStats);
			InventoryDynamicStatsIndices.RemoveAt(RealSecondInventoryStatsIndex);
			InventoryDynamicStats.RemoveAt(RealSecondInventoryStatsIndex);
		}
		
		InventoryAmounts.Swap(FirstIndex, SecondIndex);
		InventoryAssets.Swap(FirstIndex, SecondIndex);

		SwapItemSuccessDelegate.Broadcast(true, First, Second, bIsEquipment);
		ChangedInventorySlotsDelegate.Broadcast({First, Second});
		bIsProcessing = false;
		return;
	}

	// Only first slot valid
	if (FirstIndex != INDEX_NONE)
	{
		if (!InventoryAssets.IsValidIndex(FirstIndex) || !InventoryAmounts.IsValidIndex(FirstIndex))
		{
			UE_LOG(InventorySystem, Error, TEXT("[UItemContainerComponent|%s][SwapItems]: Data invalid for slot %d"), *GetFName().ToString(), First);
			SwapItemSuccessDelegate.Broadcast(false, First, Second, bIsEquipment);
			bIsProcessing = false;
			return;
		}

		if (RealFirstInventoryStatsIndex != INDEX_NONE)
		{
			if (!InventoryDynamicStats.IsValidIndex(RealFirstInventoryStatsIndex))
			{
				UE_LOG(InventorySystem, Error, TEXT("[UItemContainerComponent|%s][SwapItems]: InventoryDynamicStats is not filled but has an InventoryDynamicStatsIndices entry"), *GetFName().ToString());
				SwapItemSuccessDelegate.Broadcast(false, First, Second, bIsEquipment);
				bIsProcessing = false;
				return;
			}

			InventoryDynamicStatsIndices.AddUnique(Second);
			FItemProperties TempStats = InventoryDynamicStats[RealFirstInventoryStatsIndex];
			InventoryDynamicStats.Add(TempStats);
			InventoryDynamicStatsIndices.RemoveAt(RealFirstInventoryStatsIndex);
			InventoryDynamicStats.RemoveAt(RealFirstInventoryStatsIndex);
		}

		InventoryIndices.AddUnique(Second);
		InventoryIndices.RemoveAt(FirstIndex);
		int TempAmount = InventoryAmounts[FirstIndex];
		InventoryAmounts.Add(TempAmount);
		InventoryAmounts.RemoveAt(FirstIndex);
		FPrimaryAssetId TempAsset = InventoryAssets[FirstIndex];
		InventoryAssets.Add(TempAsset);
		InventoryAssets.RemoveAt(FirstIndex);

		SwapItemSuccessDelegate.Broadcast(true, First, Second, bIsEquipment);
		ChangedInventorySlotsDelegate.Broadcast({First, Second});
		bIsProcessing = false;
		return;
	}

	// Only second slot valid
	if (SecondIndex != INDEX_NONE)
	{
		if (!InventoryAssets.IsValidIndex(SecondIndex) || !InventoryAmounts.IsValidIndex(SecondIndex))
		{
			UE_LOG(InventorySystem, Error, TEXT("[UItemContainerComponent|%s][SwapItems]: Data invalid for slot %d"), *GetFName().ToString(), Second);
			SwapItemSuccessDelegate.Broadcast(false, First, Second, bIsEquipment);
			bIsProcessing = false;
			return;
		}

		if (RealSecondInventoryStatsIndex != INDEX_NONE)
		{
			if (!InventoryDynamicStats.IsValidIndex(RealSecondInventoryStatsIndex))
			{
				UE_LOG(InventorySystem, Error, TEXT("[UItemContainerComponent|%s][SwapItems]: InventoryDynamicStats is not filled but has an InventoryDynamicStatsIndices entry"), *GetFName().ToString());
				SwapItemSuccessDelegate.Broadcast(false, First, Second, bIsEquipment);
				bIsProcessing = false;
				return;
			}

			InventoryDynamicStatsIndices.AddUnique(First);
			FItemProperties TempStats = InventoryDynamicStats[RealSecondInventoryStatsIndex];
			InventoryDynamicStats.Add(TempStats);
			InventoryDynamicStatsIndices.RemoveAt(RealSecondInventoryStatsIndex);
			InventoryDynamicStats.RemoveAt(RealSecondInventoryStatsIndex);
		}

		InventoryIndices.AddUnique(First);
		InventoryIndices.RemoveAt(SecondIndex);
		int TempAmount = InventoryAmounts[SecondIndex];
		InventoryAmounts.Add(TempAmount);
		InventoryAmounts.RemoveAt(SecondIndex);
		FPrimaryAssetId TempAsset = InventoryAssets[SecondIndex];
		InventoryAssets.Add(TempAsset);
		InventoryAssets.RemoveAt(SecondIndex);

		SwapItemSuccessDelegate.Broadcast(true, First, Second, bIsEquipment);
		ChangedInventorySlotsDelegate.Broadcast({First, Second});
		bIsProcessing = false;
		return;
	}

	UE_LOG(InventorySystem, Error, TEXT("[UItemContainerComponent|%s][SwapItems]: Items could not be swapped"), *GetFName().ToString());
	SwapItemSuccessDelegate.Broadcast(false, First, Second, bIsEquipment);
	bIsProcessing = false;
}

bool UItemContainerComponent::RemoveAmountFromSlot_Validate(const int Slot, const int Amount)
{
	return true;
}

void UItemContainerComponent::RemoveAmountFromSlot_Implementation(const int Slot, const int Amount)
{
	if (bIsProcessing)
	{
		UE_LOG(InventorySystem, Warning, TEXT("[UItemContainerComponent|%s][RemoveAmountFromSlot]: Component is still processing previous request"), *GetFName().ToString());
		RemoveAmountFromSlotSuccessDelegate.Broadcast(false, FInventorySlot{Slot, FPrimaryAssetId{}, FItemProperties{}, -1}, Amount);
		return;
	}

	bIsProcessing = true;

	const int RealInventoryIndex = InventoryIndices.Find(Slot);
	if (Amount > GetStackSizeConfig() || Amount <= 0 || RealInventoryIndex == INDEX_NONE || !InventoryAmounts.IsValidIndex(RealInventoryIndex) || !InventoryAssets.IsValidIndex(RealInventoryIndex))
	{
		UE_LOG(InventorySystem, Error, TEXT("[UItemContainerComponent|%s][RemoveAmountFromSlot]: Data invalid for slot %d"), *GetFName().ToString(), Slot);
		RemoveAmountFromSlotSuccessDelegate.Broadcast(false, FInventorySlot{Slot, FPrimaryAssetId{}, FItemProperties{}, -1}, Amount);
		bIsProcessing = false;
		return;
	}

	bIsProcessing = true;

	const int NewAmount = InventoryAmounts[RealInventoryIndex] - Amount;

	if (NewAmount < 0)
	{
		UE_LOG(InventorySystem, Error, TEXT("[UItemContainerComponent|%s][RemoveAmountFromSlot]: New amount is smaller then 0. Aborting action"), *GetFName().ToString());
		RemoveAmountFromSlotSuccessDelegate.Broadcast(false, FInventorySlot{Slot, FPrimaryAssetId{}, FItemProperties{}, -1}, Amount);
		bIsProcessing = false;
		return;
	}

	const int TempAmount = InventoryAmounts[RealInventoryIndex];
	const FPrimaryAssetId TempAsset = InventoryAssets[RealInventoryIndex];
	int RealInventoryStatsIndex = INDEX_NONE;
	FItemProperties TempDynamicStats;
	if (RealInventoryStatsIndex = InventoryDynamicStatsIndices.Find(Slot); RealInventoryStatsIndex != INDEX_NONE)
	{
		if (!InventoryDynamicStats.IsValidIndex(RealInventoryStatsIndex))
		{
			UE_LOG(InventorySystem, Error, TEXT("[UItemContainerComponent|%s][RemoveAmountFromSlot]: InventoryDynamicStats is not filled but has an InventoryDynamicStatsIndices entry"), *GetFName().ToString());
			RemoveAmountFromSlotSuccessDelegate.Broadcast(false, FInventorySlot{Slot, FPrimaryAssetId{}, FItemProperties{}, -1}, Amount);
			bIsProcessing = false;
			return;
		}

		TempDynamicStats = InventoryDynamicStats[RealInventoryStatsIndex];
	}

	if (NewAmount == 0)
	{
		if (RealInventoryStatsIndex != INDEX_NONE)
		{
			InventoryDynamicStatsIndices.RemoveAt(RealInventoryStatsIndex);
			InventoryDynamicStats.RemoveAt(RealInventoryStatsIndex);
		}

		InventoryAmounts.RemoveAt(RealInventoryIndex);
		InventoryAssets.RemoveAt(RealInventoryIndex);
		InventoryIndices.Remove(Slot);
		RemoveAmountFromSlotSuccessDelegate.Broadcast(true, FInventorySlot{Slot, TempAsset, TempDynamicStats, TempAmount}, Amount);
		ChangedInventorySlotsDelegate.Broadcast({Slot});
		bIsProcessing = false;
		return;
	}

	InventoryAmounts[RealInventoryIndex] = NewAmount;

	RemoveAmountFromSlotSuccessDelegate.Broadcast(true, FInventorySlot{Slot, TempAsset, TempDynamicStats, TempAmount}, Amount);
	ChangedInventorySlotsDelegate.Broadcast({Slot});
	bIsProcessing = false;
}

bool UItemContainerComponent::SplitItemStack_Validate(const int Slot, const int SplitAmount)
{
	return true;
}

void UItemContainerComponent::SplitItemStack_Implementation(const int Slot, const int SplitAmount)
{
	if (bIsProcessing)
	{
		UE_LOG(InventorySystem, Warning, TEXT("[UItemContainerComponent|%s][SplitItemStack]: Component is still processing previous request"), *GetFName().ToString());
		SplitItemStackSuccessDelegate.Broadcast(false, Slot, INDEX_NONE);
		return;
	}

	bIsProcessing = true;

	const int RealInventoryIndex = InventoryIndices.Find(Slot);
	if (RealInventoryIndex == INDEX_NONE || SplitAmount == 0 || SplitAmount >= InventoryAmounts[RealInventoryIndex] || !InventoryAssets.IsValidIndex(RealInventoryIndex) || SplitAmount > GetStackSizeConfig())
	{
		UE_LOG(InventorySystem, Error, TEXT("[UItemContainerComponent|%s][SplitItemStack]: Data invalid for slot %d"), *GetFName().ToString(), Slot);
		SplitItemStackSuccessDelegate.Broadcast(false, Slot, INDEX_NONE);
		bIsProcessing = false;
		return;
	}

	bool bSuccess = false;
	int FoundSlot = INDEX_NONE;

	FindNextEmptySlot(FoundSlot, bSuccess);
	if (!bSuccess)
	{
		UE_LOG(InventorySystem, Warning, TEXT("[UItemContainerComponent|%s][SplitItemStack]: No empty slot available"), *GetFName().ToString());
		SplitItemStackSuccessDelegate.Broadcast(false, Slot, INDEX_NONE);
		bIsProcessing = false;
		return;
	}

	InventoryIndices.AddUnique(FoundSlot);
	InventoryAmounts.Add(SplitAmount);
	const FPrimaryAssetId TempInventoryAsset = InventoryAssets[RealInventoryIndex];
	InventoryAssets.Add(TempInventoryAsset);
	if (int const FoundInventoryDynamicStatsIndex = InventoryDynamicStatsIndices.Find(Slot); FoundInventoryDynamicStatsIndex != INDEX_NONE && InventoryDynamicStats.IsValidIndex(FoundInventoryDynamicStatsIndex))
	{
		InventoryDynamicStatsIndices.AddUnique(FoundSlot);
		const FItemProperties TempInventoryDynamicStats = InventoryDynamicStats[FoundInventoryDynamicStatsIndex];
		InventoryDynamicStats.Add(TempInventoryDynamicStats);
	}
	InventoryAmounts[RealInventoryIndex] -= SplitAmount;


	SplitItemStackSuccessDelegate.Broadcast(true, Slot, FoundSlot);
	ChangedInventorySlotsDelegate.Broadcast({Slot, FoundSlot});
	bIsProcessing = false;
}

bool UItemContainerComponent::SwapItemWithComponent_Validate(const int First, const int Second, UItemContainerComponent* ItemContainerComponent, const bool bCanMergeStack)
{
	return true;
}

void UItemContainerComponent::SwapItemWithComponent_Implementation(const int First, const int Second, UItemContainerComponent* ItemContainerComponent, const bool bCanMergeStack)
{
	if (bIsProcessing)
	{
		UE_LOG(InventorySystem, Warning, TEXT("[UItemContainerComponent|%s][SwapItemWithComponent]: Component is still processing previous request"), *GetFName().ToString());
		SwapItemWithComponentSuccessDelegate.Broadcast(false, First, nullptr);
		return;
	}

	bIsProcessing = true;

	// Is valid component?
	if (!IsValid(ItemContainerComponent) || ItemContainerComponent->bIsProcessing)
	{
		UE_LOG(InventorySystem, Error, TEXT("[UItemContainerComponent|%s][SwapItemWithComponent]: Other component is invalid"), *GetFName().ToString());
		SwapItemWithComponentSuccessDelegate.Broadcast(false, First, nullptr);
		bIsProcessing = false;
		return;
	}

	ItemContainerComponent->bIsProcessing = true;
	ItemContainerComponent->SwapItemWithComponentOtherComponentStartDelegate.Broadcast();

	// Check if empty items are traded or we have an error
	const int RealFirstInventoryIndex = InventoryIndices.Find(First);
	UAssetManager& Manager = UAssetManager::Get();
	if (RealFirstInventoryIndex == INDEX_NONE || !Manager.IsValid() || !InventoryAssets.IsValidIndex(RealFirstInventoryIndex) || !InventoryAmounts.IsValidIndex(RealFirstInventoryIndex))
	{
		UE_LOG(InventorySystem, Error, TEXT("[UItemContainerComponent|%s][SwapItemWithComponent]: AssetManager is not initialized or item data is invalid for slot: %d"), *GetFName().ToString(), First);
		SwapItemWithComponentSuccessDelegate.Broadcast(false, First, ItemContainerComponent);
		ItemContainerComponent->SwapItemWithComponentOtherComponentSuccessDelegate.Broadcast(false, Second, this);
		ItemContainerComponent->bIsProcessing = false;
		bIsProcessing = false;
		return;
	}

	FAssetData FirstAssetData;
	Manager.GetPrimaryAssetData(InventoryAssets[RealFirstInventoryIndex], FirstAssetData);

	if (!FirstAssetData.IsValid())
	{
		UE_LOG(InventorySystem, Error, TEXT("[UItemContainerComponent|%s][SwapItemWithComponent]: AssetData is not valid. Unable to set FirstTempCanStack value"), *GetFName().ToString());
		SwapItemWithComponentSuccessDelegate.Broadcast(false, First, ItemContainerComponent);
		ItemContainerComponent->SwapItemWithComponentOtherComponentSuccessDelegate.Broadcast(false, Second, this);
		ItemContainerComponent->bIsProcessing = false;
		bIsProcessing = false;
		return;
	}

	bool FirstTempCanStack = false;
	const FName AssetRegistrySearchablePropertyName = GET_MEMBER_NAME_CHECKED(UItemDataAsset, bCanStack);
	FirstAssetData.GetTagValue(AssetRegistrySearchablePropertyName, FirstTempCanStack);

	const int RealSecondInventoryIndex = ItemContainerComponent->InventoryIndices.Find(Second);
	const int RealFirstInventoryDynamicStatsIndicesIndex = InventoryDynamicStatsIndices.Find(First);
	if (RealSecondInventoryIndex != INDEX_NONE)
	{
		if (!ItemContainerComponent->InventoryAmounts.IsValidIndex(RealSecondInventoryIndex) || !ItemContainerComponent->InventoryAssets.IsValidIndex(RealSecondInventoryIndex))
		{
			UE_LOG(InventorySystem, Error, TEXT("[UItemContainerComponent|%s][SwapItemWithComponent]: Data invalid for slot %d"), *ItemContainerComponent->GetFName().ToString(), Second);
			SwapItemWithComponentSuccessDelegate.Broadcast(false, First, ItemContainerComponent);
			ItemContainerComponent->SwapItemWithComponentOtherComponentSuccessDelegate.Broadcast(false, Second, this);
			ItemContainerComponent->bIsProcessing = false;
			bIsProcessing = false;
			return;
		}

		const int RealSecondInventoryDynamicStatsIndicesIndex = ItemContainerComponent->InventoryDynamicStatsIndices.Find(Second);

		// We need to check StackSize and stacks manually. Replicated functions will not allow a return
		if (bCanMergeStack && FirstTempCanStack && InventoryAssets[RealFirstInventoryIndex] == ItemContainerComponent->InventoryAssets[RealSecondInventoryIndex] && InventoryAmounts[RealFirstInventoryIndex] + ItemContainerComponent->InventoryAmounts[RealSecondInventoryIndex] <= ItemContainerComponent->GetStackSizeConfig())
		{
			bool bIsSameItem = false;
			if ((RealFirstInventoryDynamicStatsIndicesIndex != INDEX_NONE && !InventoryDynamicStats.IsValidIndex(RealFirstInventoryDynamicStatsIndicesIndex)) || (RealSecondInventoryDynamicStatsIndicesIndex != INDEX_NONE && !ItemContainerComponent->InventoryDynamicStats.IsValidIndex(RealSecondInventoryDynamicStatsIndicesIndex)))
			{
				UE_LOG(InventorySystem, Error, TEXT("[UItemContainerComponent|%s][SwapItemWithComponent]: InventoryDynamicStats is not filled but has an InventoryDynamicStatsIndices entry"), *GetFName().ToString());
				SwapItemWithComponentSuccessDelegate.Broadcast(false, First, ItemContainerComponent);
				ItemContainerComponent->SwapItemWithComponentOtherComponentSuccessDelegate.Broadcast(false, Second, this);
				bIsProcessing = false;
				ItemContainerComponent->bIsProcessing = false;
				return;
			}

			if ((RealFirstInventoryDynamicStatsIndicesIndex != INDEX_NONE && RealSecondInventoryDynamicStatsIndicesIndex != INDEX_NONE && InventoryDynamicStats[RealFirstInventoryDynamicStatsIndicesIndex] == ItemContainerComponent->InventoryDynamicStats[RealSecondInventoryDynamicStatsIndicesIndex]) || (RealFirstInventoryDynamicStatsIndicesIndex == INDEX_NONE && RealSecondInventoryDynamicStatsIndicesIndex == INDEX_NONE))
			{
				bIsSameItem = true;
			}

			if (bIsSameItem)
			{
				// Set new amount
				ItemContainerComponent->InventoryAmounts[RealSecondInventoryIndex] += InventoryAmounts[RealFirstInventoryIndex];

				// Delete old item
				InventoryIndices.RemoveAt(RealFirstInventoryIndex);
				InventoryAmounts.RemoveAt(RealFirstInventoryIndex);
				InventoryAssets.RemoveAt(RealFirstInventoryIndex);
				if (RealFirstInventoryDynamicStatsIndicesIndex != INDEX_NONE)
				{
					InventoryDynamicStatsIndices.RemoveAt(RealFirstInventoryDynamicStatsIndicesIndex);
					InventoryDynamicStats.RemoveAt(RealFirstInventoryDynamicStatsIndicesIndex);
				}

				SwapItemWithComponentSuccessDelegate.Broadcast(true, First, ItemContainerComponent);
				ItemContainerComponent->SwapItemWithComponentOtherComponentSuccessDelegate.Broadcast(true, Second, this);
				ChangedInventorySlotsDelegate.Broadcast({First});
				ItemContainerComponent->ChangedInventorySlotsDelegate.Broadcast({Second});
				bIsProcessing = false;
				ItemContainerComponent->bIsProcessing = false;
				return;
			}
		}
		// Just swap
		if (RealFirstInventoryDynamicStatsIndicesIndex != INDEX_NONE && RealSecondInventoryDynamicStatsIndicesIndex != INDEX_NONE)
		{
			Swap(InventoryDynamicStats[RealFirstInventoryIndex], ItemContainerComponent->InventoryDynamicStats[RealSecondInventoryIndex]);
		}
		else if (RealFirstInventoryDynamicStatsIndicesIndex != INDEX_NONE)
		{
			if (const int NewInventoryDynamicStatsIndex = ItemContainerComponent->InventoryDynamicStatsIndices.AddUnique(Second); ItemContainerComponent->InventoryDynamicStats.IsValidIndex(NewInventoryDynamicStatsIndex))
			{
				UE_LOG(InventorySystem, Error, TEXT("[UItemContainerComponent|%s][SwapItemWithComponent]: InventoryDynamicStats should not be filled. Index was just created"), *GetFName().ToString());
				// Revert back
				ItemContainerComponent->InventoryDynamicStatsIndices.RemoveAt(NewInventoryDynamicStatsIndex);
				SwapItemWithComponentSuccessDelegate.Broadcast(false, First, ItemContainerComponent);
				ItemContainerComponent->SwapItemWithComponentOtherComponentSuccessDelegate.Broadcast(false, Second, this);
				bIsProcessing = false;
				ItemContainerComponent->bIsProcessing = false;
				return;
			}

			ItemContainerComponent->InventoryDynamicStats.Add(InventoryDynamicStats[RealFirstInventoryDynamicStatsIndicesIndex]);

			// Remove old dynamic
			InventoryDynamicStats.RemoveAt(RealFirstInventoryDynamicStatsIndicesIndex);
			InventoryDynamicStatsIndices.RemoveAt(RealFirstInventoryDynamicStatsIndicesIndex);
		}
		else if (RealSecondInventoryDynamicStatsIndicesIndex != INDEX_NONE)
		{
			if (const int NewInventoryDynamicStatsIndex = InventoryDynamicStatsIndices.AddUnique(First); InventoryDynamicStats.IsValidIndex(NewInventoryDynamicStatsIndex))
			{
				UE_LOG(InventorySystem, Error, TEXT("[UItemContainerComponent|%s][SwapItemWithComponent]: InventoryDynamicStats should not be filled. Index was just created"), *GetFName().ToString());
				// Revert back
				InventoryDynamicStatsIndices.RemoveAt(NewInventoryDynamicStatsIndex);
				SwapItemWithComponentSuccessDelegate.Broadcast(false, First, ItemContainerComponent);
				ItemContainerComponent->SwapItemWithComponentOtherComponentSuccessDelegate.Broadcast(false, Second, this);
				bIsProcessing = false;
				ItemContainerComponent->bIsProcessing = false;
				return;
			}

			InventoryDynamicStats.Add(ItemContainerComponent->InventoryDynamicStats[RealSecondInventoryDynamicStatsIndicesIndex]);

			// Remove old dynamic
			ItemContainerComponent->InventoryDynamicStats.RemoveAt(RealSecondInventoryDynamicStatsIndicesIndex);
			ItemContainerComponent->InventoryDynamicStatsIndices.RemoveAt(RealSecondInventoryDynamicStatsIndicesIndex);
		}

		Swap(InventoryAmounts[RealFirstInventoryIndex], ItemContainerComponent->InventoryAmounts[RealSecondInventoryIndex]);
		Swap(InventoryAssets[RealFirstInventoryIndex], ItemContainerComponent->InventoryAssets[RealSecondInventoryIndex]);

		SwapItemWithComponentSuccessDelegate.Broadcast(true, First, ItemContainerComponent);
		ItemContainerComponent->SwapItemWithComponentOtherComponentSuccessDelegate.Broadcast(true, Second, this);
		ChangedInventorySlotsDelegate.Broadcast({First});
		ItemContainerComponent->ChangedInventorySlotsDelegate.Broadcast({Second});
		bIsProcessing = false;
		ItemContainerComponent->bIsProcessing = false;
		return;
	}

	// Just add to other empty slot (Create)
	if (RealFirstInventoryDynamicStatsIndicesIndex != INDEX_NONE)
	{
		if (const int NewInventoryDynamicStatsIndex = ItemContainerComponent->InventoryDynamicStatsIndices.AddUnique(Second); ItemContainerComponent->InventoryDynamicStats.IsValidIndex(NewInventoryDynamicStatsIndex))
		{
			UE_LOG(InventorySystem, Error, TEXT("[UItemContainerComponent|%s][SwapItemWithComponent]: InventoryDynamicStats should not be filled. Index was just created"), *GetFName().ToString());
			// Revert back
			ItemContainerComponent->InventoryDynamicStatsIndices.RemoveAt(NewInventoryDynamicStatsIndex);
			SwapItemWithComponentSuccessDelegate.Broadcast(false, First, ItemContainerComponent);
			ItemContainerComponent->SwapItemWithComponentOtherComponentSuccessDelegate.Broadcast(false, Second, this);
			bIsProcessing = false;
			ItemContainerComponent->bIsProcessing = false;
			return;
		}

		ItemContainerComponent->InventoryDynamicStats.Add(InventoryDynamicStats[RealFirstInventoryDynamicStatsIndicesIndex]);

		// Remove stats from this inventory
		InventoryDynamicStatsIndices.RemoveAt(RealFirstInventoryDynamicStatsIndicesIndex);
		InventoryDynamicStats.RemoveAt(RealFirstInventoryDynamicStatsIndicesIndex);
	}

	ItemContainerComponent->InventoryIndices.AddUnique(Second);
	ItemContainerComponent->InventoryAssets.Add(InventoryAssets[RealFirstInventoryIndex]);
	ItemContainerComponent->InventoryAmounts.Add(InventoryAmounts[RealFirstInventoryIndex]);

	// Remove the rest of the data from this inventory
	InventoryIndices.RemoveAt(RealFirstInventoryIndex);
	InventoryAssets.RemoveAt(RealFirstInventoryIndex);
	InventoryAmounts.RemoveAt(RealFirstInventoryIndex);

	SwapItemWithComponentSuccessDelegate.Broadcast(true, First, ItemContainerComponent);
	ItemContainerComponent->SwapItemWithComponentOtherComponentSuccessDelegate.Broadcast(true, Second, this);
	ChangedInventorySlotsDelegate.Broadcast({First});
	ItemContainerComponent->ChangedInventorySlotsDelegate.Broadcast({Second});
	bIsProcessing = false;
	ItemContainerComponent->bIsProcessing = false;
}

bool UItemContainerComponent::CollectAllItems_Validate(UItemContainerComponent* ItemContainerComponent, const bool bCanStack)
{
	return true;
}

void UItemContainerComponent::CollectAllItems_Implementation(UItemContainerComponent* ItemContainerComponent, const bool bCanStack)
{
	if (bIsProcessing)
	{
		UE_LOG(InventorySystem, Warning, TEXT("[UItemContainerComponent|%s][CollectAllItems]: Component is still processing previous request"), *GetFName().ToString());
		CollectAllItemsSuccessDelegate.Broadcast(false, true, nullptr);
		return;
	}

	bIsProcessing = true;

	// Is valid component?
	if (!IsValid(ItemContainerComponent) || ItemContainerComponent->bIsProcessing)
	{
		UE_LOG(InventorySystem, Error, TEXT("[UItemContainerComponent|%s][CollectAllItems]: Other component is invalid or still processing"), *GetFName().ToString());
		CollectAllItemsSuccessDelegate.Broadcast(false, true, nullptr);
		return;
	}

	ItemContainerComponent->bIsProcessing = true;
	ItemContainerComponent->CollectAllItemsOtherComponentStartDelegate.Broadcast();

	// Start transferring all items, skip items that are not in other container if full and bCanStack
	bool bAddedOnce = false;
	bool bItemsLeft = false;
	TArray<int> ChangedSlots;
	TArray<int> ChangedSlotsOtherComponent;
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
			UE_LOG(InventorySystem, Error, TEXT("[UItemContainerComponent|%s][CollectAllItems]: Data invalid for slot %d"), *GetFName().ToString(), InventoryIndices[I]);
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
		UE_LOG(InventorySystem, Warning, TEXT("[UItemContainerComponent|%s][CollectAllItems]: Could not collect any item"), *GetFName().ToString());
	}

	CollectAllItemsSuccessDelegate.Broadcast(bAddedOnce, bItemsLeft, ItemContainerComponent);
	ItemContainerComponent->CollectAllItemsOtherComponentSuccessDelegate.Broadcast(bAddedOnce, bItemsLeft, this);
	ChangedInventorySlotsDelegate.Broadcast(ChangedSlots);
	ItemContainerComponent->ChangedInventorySlotsDelegate.Broadcast(ChangedSlotsOtherComponent);
	bIsProcessing = false;
	ItemContainerComponent->bIsProcessing = false;
}

int UItemContainerComponent::GetStackSizeConfig() const
{
	const UInventorySystemSettings* InventorySettings = GetMutableDefault<UInventorySystemSettings>();
	return MaxStackSize > 1 ? MaxStackSize : InventorySettings->MaxItemContainerStackSize;
}

bool UItemContainerComponent::SetStackSizeConfig_Validate(const int NewMaxStackSize, const bool bForce)
{
	return true;
}

void UItemContainerComponent::SetStackSizeConfig_Implementation(const int NewMaxStackSize, const bool bForce)
{
	if (bIsProcessing)
	{
		UE_LOG(InventorySystem, Warning, TEXT("[UItemContainerComponent|%s][SetStackSizeConfig]: Component is still processing previous request"), *GetFName().ToString());
		SetMaxStackSizeSuccessDelegate.Broadcast(false);
		return;
	}
	
	bIsProcessing = true;
	
	if (bForce)
	{
		MaxStackSize = NewMaxStackSize;
		InternalChecks();
		SetMaxStackSizeSuccessDelegate.Broadcast(true);
		ChangedInventorySlotsDelegate.Broadcast(InventoryIndices);
		bIsProcessing = false;
		return;
	}
	
	if (!InventoryAmounts.IsEmpty())
	{
		for (const int Amount : InventoryAmounts)
		{
			if (Amount > NewMaxStackSize)
			{
				UE_LOG(InventorySystem, Warning, TEXT("[UItemContainerComponent|%s][SetStackSizeConfig]: Aborted action! Item overflow detected"), *GetFName().ToString());
				SetMaxStackSizeSuccessDelegate.Broadcast(false);
				bIsProcessing = false;
				return;
			}
		}
	}

	MaxStackSize = NewMaxStackSize;
	SetMaxStackSizeSuccessDelegate.Broadcast(true);
	ChangedInventorySlotsDelegate.Broadcast(InventoryIndices);
	bIsProcessing = false;
}

int UItemContainerComponent::GetInventorySizeConfig() const
{
	const UInventorySystemSettings* InventorySettings = GetMutableDefault<UInventorySystemSettings>();
	return InventorySize > 0 ? InventorySize : InventorySettings->MaxItemContainerSize;
}

bool UItemContainerComponent::SetInventorySizeConfig_Validate(const int NewInventorySize, const bool bForce)
{
	return true;
}

void UItemContainerComponent::SetInventorySizeConfig_Implementation(const int NewInventorySize, const bool bForce)
{
	if (bIsProcessing)
	{
		UE_LOG(InventorySystem, Warning, TEXT("[UItemContainerComponent|%s][SetInventorySizeConfig]: Component is still processing previous request"), *GetFName().ToString());
		SetInventorySizeSuccessDelegate.Broadcast(false);
		return;
	}

	bIsProcessing = true;
	
	if (bForce)
	{
		InventorySize = NewInventorySize;
		InternalChecks();
		SetInventorySizeSuccessDelegate.Broadcast(true);
		ChangedInventorySlotsDelegate.Broadcast(InventoryIndices);
		bIsProcessing = false;
		return;
	}
	
	if (!InventoryIndices.IsEmpty() && InventoryIndices.Num() > NewInventorySize)
	{
		UE_LOG(InventorySystem, Warning, TEXT("[UItemContainerComponent|%s][SetInventorySizeConfig]: Aborted action! Item overflow detected"), *GetFName().ToString());
		SetInventorySizeSuccessDelegate.Broadcast(false);
		bIsProcessing = false;
		return;
	}

	InventorySize = NewInventorySize;
	SetInventorySizeSuccessDelegate.Broadcast(true);
	ChangedInventorySlotsDelegate.Broadcast(InventoryIndices);
	bIsProcessing = false;
}

#undef LOCTEXT_NAMESPACE
