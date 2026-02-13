// © 2024 Daniel Münch. All Rights Reserved

#include "ItemDrop.h"

#include "ImageUtils.h"
#include "InventorySystem.h"
#include "InventorySystemComponent.h"
#include "Components/BillboardComponent.h"
#include "Engine/AssetManager.h"
#include "AssetRegistry/AssetData.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "Interfaces/IPluginManager.h"
#include "Math/UnrealMathUtility.h"
#include "Net/UnrealNetwork.h"
#include "Settings/InventorySystemSettings.h"
#include "UObject/ObjectSaveContext.h"
#include "UObject/SavePackage.h"

#define LOCTEXT_NAMESPACE "InventorySystem"

AItemDrop::AItemDrop()
{
	USceneComponent* SceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent0"));
	SetRootComponent(SceneComponent);
	
#if WITH_EDITORONLY_DATA
	AllowItemAssetEdit = HasActorBegunPlay();
	AllowItemEdit = InventoryAsset.IsValid();

	if (SpriteComponent = CreateEditorOnlyDefaultSubobject<UBillboardComponent>(TEXT("Sprite")); IsValid(SpriteComponent))
	{
		SpriteComponent->SetRelativeScale3D(FVector(1.0f, 1.0f, 1.0f));
		SpriteComponent->SetupAttachment(GetRootComponent());
		SpriteComponent->bHiddenInGame = true;
		SpriteComponent->SetIsVisualizationComponent(true);
		SpriteComponent->SpriteInfo.Category = TEXT("Inventory System");
		SpriteComponent->SpriteInfo.DisplayName = LOCTEXT("Item Drop", "Item Drop");
		SpriteComponent->bIsScreenSizeScaled = true;
		SpriteComponent->SetDepthPriorityGroup(SDPG_Foreground);
	}
#endif
}

#if WITH_EDITORONLY_DATA
void AItemDrop::PreSave(FObjectPreSaveContext ObjectSaveContext)
{
	if (IsRunningCommandlet())
	{
		Super::PreSave(ObjectSaveContext);
		return;
	}

	InternalChecks(false, true);
	
	Super::PreSave(ObjectSaveContext);

	if (!IsTemplate(RF_ClassDefaultObject | RF_ArchetypeObject))
	{
		RerunConstructionScripts();
	}
}
#endif

void AItemDrop::GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(AItemDrop, DynamicStats);
	DOREPLIFETIME(AItemDrop, InventoryAsset);
	DOREPLIFETIME(AItemDrop, Amount);
	DOREPLIFETIME(AItemDrop, MaxStackSize);
	DOREPLIFETIME(AItemDrop, bIsProcessing);
}

#if WITH_EDITORONLY_DATA
void AItemDrop::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	// Check if the changed property is InventoryDataAsset
	if (!HasActorBegunPlay() && !IsActorBeingDestroyed() && PropertyChangedEvent.Property && PropertyChangedEvent.Property->GetFName() == GET_MEMBER_NAME_CHECKED(AItemDrop, InventoryDataAsset))
	{
		AddToRoot();
		Modify();
		if (IsValid(InventoryDataAsset))
		{
			InventoryAsset = InventoryDataAsset->GetPrimaryAssetId();
			InternalCanStack = InventoryDataAsset->bCanStack;
		}
		else
		{
			InventoryAsset = FPrimaryAssetId();
			InternalCanStack = false;
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
					UE_LOG(InventorySystem, Warning, TEXT("[AItemDrop|%s][PostEditChangeProperty]: DataAsset data was changed. %s"), *GetFName().ToString(), *InventoryAsset.ToString());
				}
			}	
		}
		RemoveFromRoot();
	}

	// Check if the changed property is MinRandomAmount, Amount, MaxRandomAmount or InventoryAsset
	if (PropertyChangedEvent.Property && (PropertyChangedEvent.Property->GetFName() == GET_MEMBER_NAME_CHECKED(AItemDrop, MinRandomAmount) || PropertyChangedEvent.Property->GetFName() == GET_MEMBER_NAME_CHECKED(AItemDrop, Amount) || PropertyChangedEvent.Property->GetFName() == GET_MEMBER_NAME_CHECKED(AItemDrop, MaxRandomAmount) || PropertyChangedEvent.Property->GetFName() == GET_MEMBER_NAME_CHECKED(AItemDrop, InventoryAsset) || PropertyChangedEvent.Property->GetFName() == GET_MEMBER_NAME_CHECKED(AItemDrop, InventoryAsset) || PropertyChangedEvent.Property->GetFName() == GET_MEMBER_NAME_CHECKED(AItemDrop, MaxStackSize)))
	{
		InternalChecks();
	}
	
	Super::PostEditChangeProperty(PropertyChangedEvent);

	if (!IsTemplate(RF_ClassDefaultObject | RF_ArchetypeObject))
	{
		RerunConstructionScripts();
	}
}
#endif

void AItemDrop::InternalChecks(const bool bPreventExecution, const bool bIsSavePackageEvent)
{
	AddToRoot();

	if (bIsSavePackageEvent)
	{
		if (Amount > GetStackSizeConfig())
		{
			Amount = GetStackSizeConfig();
			UE_LOG(InventorySystem, Warning, TEXT("[AItemDrop|%s][InternalChecks]: Amount is bigger then MaxStackSize. Changing value to biggest possible"), *GetFName().ToString());
		}
	}
	
	bool InternalPreventExecution = false;
	bool IsDataChanged = false;
	
	if (!InventoryAsset.IsValid() || InventoryAsset == FPrimaryAssetId())
	{
		InternalPreventExecution = true;
#if WITH_EDITORONLY_DATA
		AllowItemEdit = false;
		AllowItemAssetEdit = false;
#endif
		if (!IsTemplate(RF_ClassDefaultObject | RF_ArchetypeObject))
		{
			UE_LOG(InventorySystem, Warning, TEXT("[AItemDrop|%s][InternalChecks]: InventoryAsset is invalid"), *GetFName().ToString());
		}
	}

#if WITH_EDITORONLY_DATA
	AllowItemEdit = true;
	AllowItemAssetEdit = HasActorBegunPlay();
#endif

	// Check and set InternalCanStack. This value is important for any checks related with amounts. This only applies to runtime
	if (const UAssetManager* AssetManager = UAssetManager::GetIfInitialized(); AssetManager && AssetManager->IsInitialized())
	{
		FAssetData AssetData;
		AssetManager->GetPrimaryAssetData(InventoryAsset,  AssetData);
		if (AssetData.IsValid())
		{
			const FName AssetRegistrySearchablePropertyName = GET_MEMBER_NAME_CHECKED(UItemDataAsset, bCanStack);
			AssetData.GetTagValue(AssetRegistrySearchablePropertyName, InternalCanStack);
		}
		else
		{
			UE_LOG(InventorySystem, Warning, TEXT("[AItemDrop|%s][InternalChecks]: AssetData is not valid. Unable to set InternalCanStack value"), *GetFName().ToString());
			InternalPreventExecution = true;	
		}
	}
	else
	{
		UE_LOG(InventorySystem, Warning, TEXT("[AItemDrop|%s][InternalChecks]: AssetManager is not initialized. Unable to set InternalCanStack value"), *GetFName().ToString());
		InternalPreventExecution = true;
	}

	if (!IsActorBeingDestroyed() && Amount <= 0)
	{
		Amount = 1;
		IsDataChanged = true;
		UE_LOG(InventorySystem, Warning, TEXT("[AItemDrop|%s][InternalChecks]: Amount was reset! Should not be a negativ value"), *GetFName().ToString());
	}
	
	if (InternalCanStack)
	{
		if (Amount > GetStackSizeConfig())
		{
			Amount = GetStackSizeConfig();
			IsDataChanged = true;
			UE_LOG(InventorySystem, Warning, TEXT("[AItemDrop|%s][InternalChecks]: Amount is bigger then MaxStackSize. Changing value to biggest possible"), *GetFName().ToString());
		}
		
		if (MaxRandomAmount < 0 || MinRandomAmount < 0)
		{
			MaxRandomAmount = 0;
			MinRandomAmount = 0;
			InternalPreventExecution = true;
			IsDataChanged = true;
			UE_LOG(InventorySystem, Warning, TEXT("[AItemDrop|%s][InternalChecks]: MaxRandomAmount and MinRandomAmount was reset! Should not be a negativ value"), *GetFName().ToString());
		}
		
		if (MaxRandomAmount > 0)
		{
			if (MaxRandomAmount > GetStackSizeConfig())
			{
				UE_LOG(InventorySystem, Warning, TEXT("[AItemDrop|%s][InternalChecks]: MaxRandomAmount was reset! Should not be over MaxStackSize"), *GetFName().ToString());
				MaxRandomAmount = GetStackSizeConfig();
				InternalPreventExecution = true;
				IsDataChanged = true;
			}
		}

		if (MaxRandomAmount > 0 && MaxRandomAmount <= MinRandomAmount)
		{
			UE_LOG(InventorySystem, Warning, TEXT("[AItemDrop|%s][InternalChecks]: MaxRandomAmount is out of range! MaxRandomAmount should always be bigger then MinRandomAmount"), *GetFName().ToString());
			MaxRandomAmount = MinRandomAmount + 1;
			IsDataChanged = true;
		}
		
		if (MinRandomAmount >= 0 && MaxRandomAmount != 0 && MinRandomAmount >= MaxRandomAmount)
		{
			UE_LOG(InventorySystem, Warning, TEXT("[AItemDrop|%s][InternalChecks]: MinRandomAmount is out of range! MinRandomAmount should always be smaller then MaxRandomAmount"), *GetFName().ToString());
			MinRandomAmount = MaxRandomAmount - 1;
			IsDataChanged = true;
		}

#if WITH_EDITORONLY_DATA
		if (!HasActorBegunPlay() && !IsActorBeingDestroyed() && !bIsSavePackageEvent && IsDataChanged)
		{
			// Only save changes when in editor otherwise it could lead to problems and unexpected behavior
			if (!HasAnyFlags(RF_Transient | RF_TagGarbageTemp) && !HasAnyMarks(OBJECTMARK_EditorOnly))
			{
				if (UPackage* Package = GetOutermost(); IsValid(Package) && !Package->HasAnyFlags(RF_Transient | RF_TagGarbageTemp) && !Package->HasAnyMarks(OBJECTMARK_EditorOnly))
				{
					const FString PackageFileName = FPackageName::LongPackageNameToFilename(Package->GetName(), FPackageName::GetAssetPackageExtension());
					FSavePackageArgs Args = FSavePackageArgs();
					Args.TopLevelFlags = GetFlags();
			
					if (UPackage::SavePackage(Package, this, *PackageFileName, Args))
					{
						UE_LOG(InventorySystem, Warning, TEXT("[AItemDrop|%s][InternalChecks]: A mistake in setup resulted in data being altered... saving"), *GetFName().ToString());
					}
				}
			}	
		}
#endif

		if (bPreventExecution && InternalPreventExecution)
		{
			UE_LOG(InventorySystem, Error, TEXT("[AItemDrop|%s][InternalChecks]: Not set up properly. AItemDrop was destroyed"), *GetFName().ToString());
			RemoveFromRoot();
			Destroy();
			return;
		}

		RemoveFromRoot();
		return;
	}

	if (Amount > 1)
	{
		UE_LOG(InventorySystem, Warning, TEXT("[AItemDrop|%s][InternalChecks]: Amount is out of range! Item is not stackable but amount was set to %d. Amount was reseted"), *GetFName().ToString(), Amount);
		Amount = 1;
		IsDataChanged = true;
	}

#if WITH_EDITORONLY_DATA
	if (!HasActorBegunPlay() && !IsActorBeingDestroyed() && !bIsSavePackageEvent && IsDataChanged)
	{
		// Only save changes when in editor otherwise it could lead to problems and unexpected behavior
		if (!HasAnyFlags(RF_Transient | RF_TagGarbageTemp) && !HasAnyMarks(OBJECTMARK_EditorOnly))
		{
			if (UPackage* Package = GetOutermost(); IsValid(Package) && !Package->HasAnyFlags(RF_Transient | RF_TagGarbageTemp) && !Package->HasAnyMarks(OBJECTMARK_EditorOnly))
			{
				const FString PackageFileName = FPackageName::LongPackageNameToFilename(Package->GetName(), FPackageName::GetAssetPackageExtension());
				FSavePackageArgs Args = FSavePackageArgs();
				Args.TopLevelFlags = GetFlags();
			
				if (UPackage::SavePackage(Package, this, *PackageFileName, Args))
				{
					UE_LOG(InventorySystem, Warning, TEXT("[AItemDrop|%s][InternalChecks]: A mistake in setup resulted in data being altered... saving"), *GetFName().ToString());
				}
			}
		}	
	}
#endif

	if (bPreventExecution && InternalPreventExecution)
	{
		UE_LOG(InventorySystem, Error, TEXT("[AItemDrop|%s][InternalChecks]: Not correctly setup. AItemDrop destroyed"), *GetFName().ToString());
		RemoveFromRoot();
		Destroy();
	}

	RemoveFromRoot();
}

bool AItemDrop::PickUp_Validate(UInventorySystemComponent* InventorySystemComponent, const bool bCanStack)
{
	return true;
}

void AItemDrop::PickUp_Implementation(UInventorySystemComponent* InventorySystemComponent, const bool bCanStack)
{
	if (HasAuthority())
	{
		if (bIsProcessing)
		{
			UE_LOG(InventorySystem, Error, TEXT("[AItemDrop|%s][PickUp]: AItemDrop is still processing previous request"), *GetFName().ToString());
			return;
		}
	
		bIsProcessing = true;
		if (IsValid(InventorySystemComponent) && !InventorySystemComponent->bIsProcessing)
		{
			InventorySystemComponent->PickUpItemDrop(this, bCanStack);
			return;
		}
		UE_LOG(InventorySystem, Warning, TEXT("[AItemDrop|%s][InternalChecks]: Invalid InventorySystemComponent or still processing"), *GetFName().ToString());
		bIsProcessing = false;
	}
}

void AItemDrop::AfterPickUpEvent_Implementation(const bool bSuccess)
{
	if (bSuccess)
	{
		if (bDestroyAfterPickUp && Amount <= 0)
		{
			Destroy();	
		}
	}

	bIsProcessing = false;
}

void AItemDrop::OnConstruction(const FTransform& Transform)
{
#if WITH_EDITORONLY_DATA
	if (IsValid(InventoryDataAsset))
	{
		InternalCanStack = InventoryDataAsset->bCanStack;
	}
	
	if (IsValid(SpriteComponent))
	{
		const FString BaseDir = IPluginManager::Get().FindPlugin("InventorySystem")->GetBaseDir();
		const FString TexturePath = FPaths::Combine(*BaseDir, TEXT("Resources/ItemDrop128.png"));

		UTexture2D* NewTexture = FImageUtils::ImportFileAsTexture2D(TexturePath);
		
		SpriteComponent->SetSprite(NewTexture);
	}
#endif
	
	Super::OnConstruction(Transform);
}

void AItemDrop::BeginPlay()
{
	Super::BeginPlay();
	
	InternalChecks(true);
	
	if (MaxRandomAmount > 1 && MinRandomAmount > 0 && MaxRandomAmount > MinRandomAmount) {
		Amount = FMath::RandRange(MinRandomAmount, MaxRandomAmount);
	}

#if WITH_EDITORONLY_DATA
	InventoryDataAsset = nullptr;
	bHasBegunPlayEditor = true;
#endif
}

#if WITH_EDITORONLY_DATA
void AItemDrop::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	bHasBegunPlayEditor = false;

	InternalChecks();
}
#endif

int AItemDrop::GetStackSizeConfig() const
{
	const UInventorySystemSettings* InventorySettings = GetMutableDefault<UInventorySystemSettings>();
	return MaxStackSize > 1 ? MaxStackSize : InventorySettings->MaxItemDropStackSize;
}

#undef LOCTEXT_NAMESPACE