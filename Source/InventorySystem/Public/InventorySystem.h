// © 2024 Daniel Münch. All Rights Reserved

#pragma once

#include "Modules/ModuleManager.h"

DECLARE_LOG_CATEGORY_EXTERN(InventorySystem, Log, All);

/**
 * @class FInventorySystemModule
 * @brief Module class for the Inventory System, providing initialization and shutdown functionalities.
 *
 * Implements IModuleInterface, managing the lifecycle of the Inventory System plugin within Unreal Engine projects. It's responsible
 * for setting up and tearing down resources associated with the inventory system, ensuring proper plugin behavior throughout the game's runtime.
 *
 * General Usage:
 * - Included as part of the Inventory System plugin architecture.
 * - Supports dynamic reloading, enhancing development workflow by allowing changes without restarting the engine.
 * 
 * Example Use Case:
 * - Automatically called by the Unreal Engine system during startup and shutdown sequences.
 * - Can be extended or overridden to customize initialization and shutdown processes for the inventory system.
 */
class FInventorySystemModule final : public IModuleInterface
{
public:
	/**
	 * Called when the module is started.
	 */
	virtual void StartupModule() override;

	/**
	 * Called when the module is being shut down.
	 */
	virtual void ShutdownModule() override;

	/**
	 * Determines whether the module supports dynamic reloading.
	 * 
	 * @return True if dynamic reloading is supported; otherwise, false.
	 */
	virtual bool SupportsDynamicReloading() override;
};