// © 2024 Daniel Münch. All Rights Reserved

#include "InventorySystemEditor.h"

#include "EngineUtils.h"
#include "ItemDrop.h"
#include "Editor.h"
#include "Engine/AssetManager.h"
#include "Styles/InventorySystemStyles.h"
#include "ISettingsModule.h"
#include "Settings/InventorySystemSettings.h"

DEFINE_LOG_CATEGORY(InventorySystem);

#define LOCTEXT_NAMESPACE "InventorySystem"

bool FInventorySystemEditorModule::SupportsDynamicReloading()
{
	return true;
}

void FInventorySystemEditorModule::StartupModule()
{
	Style = MakeShareable(new FInventorySystemStyles());
	
	if (ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings"))
	{
		//const TSharedPtr<ISettingsContainer> SettingsContainer = SettingsModule->GetContainer("Project");
		const auto SettingsContainer = SettingsModule->RegisterSettings("Project", "Plugins", "InventorySystem",LOCTEXT("InventorySystem", "Inventory System"), LOCTEXT("InventorySystemDescription", "Inventory System Plugin Settings"),GetMutableDefault<UInventorySystemSettings>());
	}
	
	FWorldDelegates::OnPostWorldInitialization.AddRaw(this, &FInventorySystemEditorModule::OnWorldInitialized);

	// Listen for PIE start
	DisableSettingsDelegateHandle = FEditorDelegates::PostPIEStarted.AddLambda([](bool bIsSimulating)
	{
		UInventorySystemSettings* Settings = GetMutableDefault<UInventorySystemSettings>();
		if (Settings)
		{
			Settings->bHasBegunPlayEditor = 1;
		}
	});

	// Listen for PIE end
	EnableSettingsDelegateHandle = FEditorDelegates::EndPIE.AddLambda([](bool bIsSimulating)
	{
		UInventorySystemSettings* Settings = GetMutableDefault<UInventorySystemSettings>();
		if (Settings)
		{
			Settings->bHasBegunPlayEditor = 0;
		}
	});
}

void FInventorySystemEditorModule::OnWorldInitialized(UWorld* World, const UWorld::InitializationValues Ivs)
{
	// Set a flag to indicate the task should run
	bAsyncTaskShouldRun = true;

	// Set up a timer to periodically execute the task
	World->GetTimerManager().SetTimer(TimerHandle, [this, World]()
	{
		if (!bAsyncTaskShouldRun)
		{
			World->GetTimerManager().ClearTimer(TimerHandle);
			return;
		}

		if (World && World->WorldType == EWorldType::Editor)
		{
			if (UAssetManager& Manager = UAssetManager::Get(); Manager.IsValid() && Manager.HasInitialScanCompleted())
			{
				for (TActorIterator<AItemDrop> It(World); It; ++It)
				{
					AItemDrop* Actor = *It;
					Manager.LoadPrimaryAsset(Actor->InventoryAsset, TArray<FName>{}, FStreamableDelegate::CreateLambda([this, Actor, World]()
					{
						Actor->RerunConstructionScripts();
						bAsyncTaskShouldRun = false;
					}));

					Actor->RerunConstructionScripts();
				}
				
				// Clear the timer as we no longer need it
				World->GetTimerManager().ClearTimer(TimerHandle);
			}
		}
	}, 0.25f, true); // 0.5 seconds interval, repeating
}

void FInventorySystemEditorModule::ShutdownModule()
{
	if (ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings"))
	{
		SettingsModule->UnregisterSettings("Project", "Plugins", "InventorySystem");
	}

	// Listen for PIE start
	FEditorDelegates::PostPIEStarted.Remove(DisableSettingsDelegateHandle);

	// Listen for PIE end
	FEditorDelegates::EndPIE.Remove(EnableSettingsDelegateHandle);
}

IMPLEMENT_MODULE(FInventorySystemEditorModule, InventorySystemEditor)

#undef LOCTEXT_NAMESPACE