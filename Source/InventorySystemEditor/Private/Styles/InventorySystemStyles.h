// © 2024 Daniel Münch. All Rights Reserved

#pragma once

#include "Interfaces/IPluginManager.h"
#include "Brushes/SlateImageBrush.h"
#include "Styling/SlateStyle.h"
#include "Styling/SlateStyleRegistry.h"

#define IMAGE_BRUSH(RelativePath, ...) FSlateImageBrush(RootToContentDir(RelativePath, TEXT(".png")), __VA_ARGS__)
#define BOX_BRUSH(RelativePath, ...) FSlateBoxBrush(RootToContentDir(RelativePath, TEXT(".png")), __VA_ARGS__)
#define BORDER_BRUSH(RelativePath, ...) FSlateBorderBrush(RootToContentDir(RelativePath, TEXT(".png")), __VA_ARGS__)
#define TTF_FONT(RelativePath, ...) FSlateFontInfo(RootToContentDir(RelativePath, TEXT(".ttf")), __VA_ARGS__)
#define OTF_FONT(RelativePath, ...) FSlateFontInfo(RootToContentDir(RelativePath, TEXT(".otf")), __VA_ARGS__)


/**
 * FInventorySystemStyles is a class that defines the visual style of the text asset editor UI.
 * 
 * This class provides styles for various UI elements, including icons and thumbnails for different asset types.
 */
class INVENTORYSYSTEMEDITOR_API FInventorySystemStyles final
	: public FSlateStyleSet
{
public:

	/** Default constructor. */
	FInventorySystemStyles()
		: FSlateStyleSet("InventorySystemStyles")
	{
		const FVector2D Icon16x16(16.0f, 16.0f);
		const FVector2D Icon20x20(20.0f, 20.0f);
		const FVector2D Icon40x40(40.0f, 40.0f);
		const FVector2D Icon128x128(128.f, 128.f);

		const FString BaseDir = IPluginManager::Get().FindPlugin("InventorySystem")->GetBaseDir();
		FSlateStyleSet::SetContentRoot(BaseDir / TEXT("Resources"));

		// Set new styles here, for example...
		Set("ClassThumbnail.ItemEquipmentTypeDataAsset", new IMAGE_BRUSH("EquipmentType128", Icon128x128));
		Set("ClassIcon.ItemEquipmentTypeDataAsset", new IMAGE_BRUSH("EquipmentTypeClassList128", Icon16x16));
		Set("ClassThumbnail.ItemDataAsset", new IMAGE_BRUSH("ItemDataAsset128", Icon128x128));
		Set("ClassIcon.ItemDataAsset", new IMAGE_BRUSH("ItemDataAssetClassList128", Icon16x16));
		Set("ClassThumbnail.ItemEquipmentDataAsset", new IMAGE_BRUSH("ItemEquipmentDataAsset128", Icon128x128));
		Set("ClassIcon.ItemEquipmentDataAsset", new IMAGE_BRUSH("ItemEquipmentDataAssetClassList128", Icon16x16));
		Set("ClassThumbnail.InventorySystemComponent", new IMAGE_BRUSH("InventorySystemClassList128", Icon128x128));
		Set("ClassIcon.InventorySystemComponent", new IMAGE_BRUSH("InventorySystem128", Icon16x16));
		Set("ClassThumbnail.ItemContainerComponent", new IMAGE_BRUSH("ItemContainerClassList128", Icon128x128));
		Set("ClassIcon.ItemContainerComponent", new IMAGE_BRUSH("ItemContainer128", Icon16x16));
		Set("ClassThumbnail.ItemDrop", new IMAGE_BRUSH("ItemDrop128", Icon128x128));
		Set("ClassIcon.ItemDrop", new IMAGE_BRUSH("ItemDropClassList128", Icon16x16));

		FSlateStyleRegistry::RegisterSlateStyle(*this);
	}

	/** Destructor. */
	virtual ~FInventorySystemStyles() override
	{
		FSlateStyleRegistry::UnRegisterSlateStyle(*this);
	}
};


#undef IMAGE_BRUSH
#undef BOX_BRUSH
#undef BORDER_BRUSH
#undef TTF_FONT
#undef OTF_FONT