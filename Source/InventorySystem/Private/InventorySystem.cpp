// © 2024 Daniel Münch. All Rights Reserved

#include "InventorySystem.h"

#include "EngineUtils.h"
#include "ItemDrop.h"
#include "Engine/AssetManager.h"

DEFINE_LOG_CATEGORY(InventorySystem);

#define LOCTEXT_NAMESPACE "InventorySystem"

void FInventorySystemModule::StartupModule()
{
	
}

void FInventorySystemModule::ShutdownModule()
{
	
}

bool FInventorySystemModule::SupportsDynamicReloading()
{
	return true;
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FInventorySystemModule, InventorySystem)