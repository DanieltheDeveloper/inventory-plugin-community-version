// © 2024 Daniel Münch. All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"
#include "Styling/SlateStyle.h"
#include "Templates/SharedPointer.h"

DECLARE_LOG_CATEGORY_EXTERN(InventorySystem, Log, All);

/**
 * FInventorySystemEditorModule is a module class that provides editor-specific functionality for the Inventory System plugin.
 * 
 * This class implements the IModuleInterface interface and is responsible for initializing and shutting down the plugin's editor functionality.
 */
class INVENTORYSYSTEMEDITOR_API FInventorySystemEditorModule final : public IModuleInterface
{
public:

	/**
	 * Determines whether the module supports dynamic reloading.
	 * 
	 * @return True if dynamic reloading is supported; otherwise, false.
	 */
	virtual bool SupportsDynamicReloading() override;

	/**
	 * Called when the module is started during the editor initialization process.
	 */
	virtual void StartupModule() override;

	/**
	 * Callback for when the world is initialized.
	 * 
	 * @param World - The initialized world.
	 * @param Ivs - The initialization values for the world.
	 */
	void OnWorldInitialized(UWorld* World, UWorld::InitializationValues Ivs);

	/**
	 * Called when the module is being shut down.
	 */
	virtual void ShutdownModule() override;

private:
	/**
	 *  A handle for managing timer-related functionality.
	 */
	FTimerHandle TimerHandle;

	/**
	 * A handle for setting changes.
	 */
	FDelegateHandle EnableSettingsDelegateHandle;

	/**
	 *  A handle for setting changes.
	 */
	FDelegateHandle DisableSettingsDelegateHandle;

	/**
	 * An atomic boolean variable used for managing asynchronous task execution.
	 */
	std::atomic<bool> bAsyncTaskShouldRun;

	/** Holds the plug-in's style set. */
	TSharedPtr<ISlateStyle> Style;
};
