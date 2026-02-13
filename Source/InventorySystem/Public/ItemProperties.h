// © 2024 Daniel Münch. All Rights Reserved

#pragma once

#include "ItemProperties.generated.h"

#define LOCTEXT_NAMESPACE "InventorySystem"

/**
 * @class FItemProperty
 * @brief Represents a dynamic item property for in-game items, such as stats or characteristics.
 *
 * This struct provides a flexible way to define and use properties for items within the game, supporting properties like
 * damage, speed, or custom attributes. Properties can be defined with a name, display name, and a value, which can be a simple
 * number, a string, or even a JSON string for complex data structures.
 *
 * General Usage:
 * - Define item properties in item data assets.
 * - Use these properties in game logic for effects, item comparisons, and UI display.
 *
 * Example Use Case:
 * @code
 * FItemProperty DamageProperty(FText::FromString(TEXT("Damage")), FText::FromString(TEXT("Damage")), FText::FromString(TEXT("30")));
 * @endcode
 */
USTRUCT(BlueprintType, Category = "Inventory System")
struct INVENTORYSYSTEM_API FItemProperty
{
	GENERATED_BODY()

	FItemProperty() {};

	/**
	 * Construct an FItemProperty with the specified values.
	 *
	 * @param Name The index of the property for faster lookup.
	 * @param DisplayName The default display name used for this property. Use localization tables to insert the preferred property name faster.
	 * @param Value Give it a simple value like 30, Evil and Broken, or convert it to a string and use JSON to allow this value to hold objects and more complex data.
	 */
	FItemProperty(const FName Name, const FText& DisplayName, const FText& Value)
	{
		this->Name = Name;
		this->DisplayName = DisplayName;
		this->Value = Value;
	}

	// Index of the property. For faster lookup.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Inventory System")
	FName Name;

	// Default display name used for this property. Use localization tables to insert the preferred property name faster.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Inventory System")
	FText DisplayName;

	// The Value of the property. Give it a simple value like 30, Evil and Broken, or convert it to a string and use JSON to allow this value to hold objects and more complex data.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Inventory System")
	FText Value;

	/**
	 * Compare two FItemProperty objects for equality.
	 *
	 * @param First The first FItemProperty to compare.
	 * @param Other The second FItemProperty to compare.
	 * @return True if both FItemProperty objects are equal, false otherwise.
	 */
	friend bool operator== (const FItemProperty& First, const FItemProperty& Other)
	{
		return ((First.Name == Other.Name) && (First.Value.ToString() == Other.Value.ToString()));
	}

	/**
	 * Compare two FItemProperty objects for inequality.
	 *
	 * @param First The first FItemProperty to compare.
	 * @param Other The second FItemProperty to compare.
	 * @return True if both FItemProperty objects are not equal, false otherwise.
	 */
	friend bool operator!= (const FItemProperty& First, const FItemProperty& Other)
	{
		return ((First.Name != Other.Name) || (First.Value.ToString() != Other.Value.ToString()));
	}

	/**
	 * Compare two FItemProperty objects for greater than.
	 *
	 * @param First The first FItemProperty to compare.
	 * @param Other The second FItemProperty to compare.
	 * @return True if the first FItemProperty is greater than the second, false otherwise.
	 */
	friend bool operator> (const FItemProperty& First, const FItemProperty& Other)
	{
		if (!First.Value.IsNumeric())
		{
			return false;
		}
		return First.Name == Other.Name && FCString::Atod(*First.Value.ToString()) > FCString::Atod(*Other.Value.ToString());
	}

	/**
	 * Compare two FItemProperty objects for less than.
	 *
	 * @param First The first FItemProperty to compare.
	 * @param Other The second FItemProperty to compare.
	 * @return True if the first FItemProperty is less than the second, false otherwise.
	 */
	friend bool operator< (const FItemProperty& First, const FItemProperty& Other)
	{
		if (!First.Value.IsNumeric())
		{
			return false;
		}
		return First.Name == Other.Name && FCString::Atod(*First.Value.ToString()) < FCString::Atod(*Other.Value.ToString());
	}

	/**
	 * Compare two FItemProperty objects for greater than or equal to.
	 *
	 * @param First The first FItemProperty to compare.
	 * @param Other The second FItemProperty to compare.
	 * @return True if the first FItemProperty is greater than or equal to the second, false otherwise.
	 */
	friend bool operator>= (const FItemProperty& First, const FItemProperty& Other)
	{
		if (!First.Value.IsNumeric())
		{
			return false;
		}
		return First.Name == Other.Name && FCString::Atod(*First.Value.ToString()) >= FCString::Atod(*Other.Value.ToString());
	}

	/**
	 * Compare two FItemProperty objects for less than or equal to.
	 *
	 * @param First The first FItemProperty to compare.
	 * @param Other The second FItemProperty to compare.
	 * @return True if the first FItemProperty is less than or equal to the second, false otherwise.
	 */
	friend bool operator<= (const FItemProperty& First, const FItemProperty& Other)
	{
		if (!First.Value.IsNumeric())
		{
			return false;
		}
		return First.Name == Other.Name && FCString::Atod(*First.Value.ToString()) <= FCString::Atod(*Other.Value.ToString());
	}
};

/**
 * @class FItemProperties
 * @brief A collection of FItemProperty objects, representing all properties of an item.
 *
 * This struct is used to aggregate multiple properties into a single, manageable object. It facilitates the management
 * of item attributes in a structured manner, allowing for easy addition, removal, and modification of item properties.
 *
 * General Usage:
 * - Attach to items to define their attributes.
 * - Access and modify during gameplay for dynamic item effects.
 *
 * Example Use Case:
 * @code
 * FItemProperties ItemAttributes;
 * ItemAttributes.ItemProperties.Add(DamageProperty);
 * @endcode
 */
USTRUCT(BlueprintType, Category = "Inventory System")
struct INVENTORYSYSTEM_API FItemProperties
{
	GENERATED_BODY()

	FItemProperties() {};

	/**
	 * Construct FItemProperties with the specified properties.
	 *
	 * @param NewItemProperties The array of item properties to store.
	 */
	FItemProperties(const TArray<FItemProperty>& NewItemProperties)
	{
		ItemProperties = NewItemProperties;
	};

	// Store properties for slots.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Inventory System")
	TArray<FItemProperty> ItemProperties;

	/**
	 * Compare two FItemProperties objects for equality.
	 *
	 * @param First The first FItemProperties to compare.
	 * @param Other The second FItemProperties to compare.
	 * @return True if both FItemProperties objects are equal, false otherwise.
	 */
	friend bool operator==(const FItemProperties& First, const FItemProperties& Other)
	{
		return First.ItemProperties == Other.ItemProperties;
	}

	/**
	 * Compare two FItemProperties objects for equality.
	 *
	 * @param First The first FItemProperties to compare.
	 * @param Other The second FItemProperties to compare.
	 * @return True if FItemProperties objects are not equal, true otherwise.
	 */
	friend bool operator!=(const FItemProperties& First, const FItemProperties& Other)
	{
		return First.ItemProperties != Other.ItemProperties;
	}

	/**
	 * Combine two sets of FItemProperties.
	 *
	 * @param Rhs The second set of FItemProperties.
	 * @return The combined FItemProperties.
	 */
	FItemProperties& operator^=(const FItemProperties& Rhs)
	{
		ItemProperties = Rhs.ItemProperties;
		return *this;
	}
};

#undef LOCTEXT_NAMESPACE
