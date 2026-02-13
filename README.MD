# Inventory System Plugin — Unreal Engine 5.5

A production-ready, multiplayer-compatible inventory and equipment plugin for Unreal Engine 5.5, written in C++ with full Blueprint support.

Originally created by **Daniel Münch (Daniel the Developer)**. This is the community-maintained version.

- **Version:** 3.0.2
- **Engine:** Unreal Engine 5.5+
- **License:** [MIT](LICENSE)
- **Documentation:** [inventory-docs.danielthedeveloper.net](https://inventory-docs.danielthedeveloper.net/)

---

## Features

- **Inventory containers** with configurable size and stack limits
- **Equipment system** with typed equipment slots (weapon, armor, helmet, etc.)
- **Dynamic item properties** — attach arbitrary named stats (damage, speed, durability) to individual item stacks
- **Multiplayer-ready** — all operations are Server RPCs with validation and full property replication
- **Event-driven architecture** — 26+ delegates for success/failure callbacks, enabling clean UI integration
- **Blueprint-accessible** — every system is exposed to Blueprints; no C++ required to use the plugin
- **Data-driven items** — define items as `UPrimaryDataAsset` subclasses, loaded via the Asset Manager
- **In-editor asset creation** — dedicated asset factories and custom asset definitions in the Content Browser
- **Configurable defaults** — all stack sizes and inventory limits are tunable in Project Settings

---

## Architecture Overview

```
InventorySystem (Plugin)
├── Runtime Module (InventorySystem)
│   ├── Components
│   │   ├── UItemContainerComponent     — Base container: add, remove, swap, split, transfer
│   │   └── UInventorySystemComponent   — Extends container with full equipment support
│   ├── Actors
│   │   └── AItemDrop                   — World actor that players can pick up
│   ├── Data Assets
│   │   ├── UItemDataAsset              — Base item definition (name, icon, stackable)
│   │   ├── UItemEquipmentDataAsset     — Item with equipment type constraints
│   │   └── UItemEquipmentTypeDataAsset — Defines a slot type (e.g. "Helmet", "Sword")
│   ├── Interfaces
│   │   ├── IItemAssetInterface
│   │   └── IItemEquipmentTypeAssetInterface
│   ├── Structs
│   │   ├── FInventorySlot
│   │   ├── FEquipmentSlot
│   │   ├── FItemProperty
│   │   └── FItemProperties
│   ├── UI
│   │   ├── UUI_InventoryItem           — Widget base for an inventory slot
│   │   └── UUI_EquipmentItem           — Widget base for an equipment slot
│   ├── UInventorySystemBlueprintLibrary
│   └── UInventorySystemSettings        — Project Settings configuration
└── Editor Module (InventorySystemEditor)
    ├── Asset Factories                 — Content Browser "New Asset" integration
    ├── Asset Definitions               — Custom asset display and behavior
    └── Editor Styles
```

---

## Installation

1. Clone or download this repository into your project's `Plugins/` folder:
   ```
   YourProject/
   └── Plugins/
       └── InventorySystem/
           ├── InventorySystem.uplugin
           └── Source/
   ```
2. Regenerate Visual Studio / Rider project files (right-click `.uproject` → *Generate Project Files*).
3. Build and launch the editor. The plugin will compile automatically.
4. Enable the plugin in **Edit → Plugins → Inventory System** if not already enabled.

---

## Quick Start

### 1. Create an Item

In the Content Browser, right-click and navigate to **Inventory System → Item Data Asset**. Select `UItemDataAsset` or a custom subclass. Fill in the item name, icon, and whether it can stack.

For equippable items, create an **Item Equipment Data Asset** instead and assign one or more **Equipment Types**.

### 2. Set Up the Inventory Component

Add `UInventorySystemComponent` to your `APlayerState`, `ACharacter`, or any `AActor`. Configure:

| Property | Description |
|---|---|
| `InventorySize` | Number of inventory slots |
| `MaxStackSize` | Per-slot item limit |
| `MaxEquipmentStackSize` | Per-equipment-slot item limit |

For a simple container without equipment (e.g. a chest), use `UItemContainerComponent` instead.

### 3. Configure Equipment Slots

Create **Item Equipment Type Data Assets** for each slot category your game needs (e.g. *Helmet*, *Chest*, *Main Hand*). Assign these to the `EquipmentType` array on `UInventorySystemComponent`.

### 4. Place Item Drops in the World

Add `AItemDrop` actors to your level. Set `InventoryAsset`, `Amount`, and optionally `MinRandomAmount`/`MaxRandomAmount` for a random quantity on spawn.

### 5. Build UI Widgets

Subclass `UUI_InventoryItem` and `UUI_EquipmentItem` in Blueprint or C++. Bind to the provided delegates to react to slot changes:

```cpp
// Example: bind to inventory slot changes
InventoryItemWidget->OnInventorySlotChanged.AddDynamic(this, &UMyWidget::OnSlotUpdated);
```

---

## Core Components

### UItemContainerComponent

The base inventory container. Handles all item storage logic.

**Key operations:**

| Method | Description |
|---|---|
| `AddItem` | Add an item (auto-finds best slot) |
| `AddItemToSlot` | Add an item to a specific slot |
| `RemoveAmountFromSlot` | Remove a quantity from a slot |
| `SwapItems` | Swap two slots within this container |
| `SplitItemStack` | Split a stack into an empty slot |
| `AddItemToComponent` | Move items to another container |
| `SwapItemWithComponent` | Swap a slot with a slot in another container |
| `CollectAllItems` | Pull all items into a target container |
| `SetSlotItemProperty` | Set a dynamic property on a specific slot |

All write operations are **Server RPCs with validation** for multiplayer safety.

---

### UInventorySystemComponent

Extends `UItemContainerComponent` with a full equipment system.

**Additional operations:**

| Method | Description |
|---|---|
| `SetEquipmentType` | Add or remove an equipment slot type |
| `AddItemToEquipmentSlot` | Place an item directly into an equipment slot |
| `RemoveEquipmentAmountFromSlot` | Remove an equipped item |
| `ItemEquipFromInventory` | Move an item from inventory to an equipment slot |
| `ItemUnequip` | Move an equipped item back to inventory |
| `GetEquipmentSlots` | Query all current equipment slots |
| `GetEquipmentSlot` | Query a single equipment slot |

---

### AItemDrop

A replicated world actor representing a pickable item.

| Property | Description |
|---|---|
| `InventoryAsset` | The item to grant on pickup |
| `Amount` | Fixed pickup quantity |
| `MinRandomAmount / MaxRandomAmount` | Random quantity range (overrides `Amount` when > 0) |
| `bDestroyAfterPickUp` | Destroy the actor after a successful pickup |
| `DynamicStats` | Optional pre-set item properties |

Call `PickUp(UInventorySystemComponent*)` from the server to attempt a pickup.

---

## Dynamic Item Properties

Items support arbitrary named properties per stack, allowing things like durability, enchant level, or custom modifiers without subclassing.

```cpp
// Build a properties struct when spawning an item
FItemProperties Props;
FItemProperty Durability;
Durability.Name = "Durability";
Durability.DisplayName = FText::FromString("Durability");
Durability.Value = "100";
Props.Properties.Add(Durability);

// Query a property from a slot
FItemProperty OutProp;
bool bFound = Component->GetItemProperty(SlotIndex, "Durability", OutProp);

// Modify a property on a slot (Server RPC)
Component->SetSlotItemProperty(SlotIndex, "Durability", "85");
```

Properties support comparison operators (`==`, `!=`, `<`, `>`, `<=`, `>=`) for numeric values, exposed to both C++ and Blueprints via `UInventorySystemBlueprintLibrary`.

---

## Project Settings

Settings are exposed under **Project Settings → Plugins → Inventory System**:

| Setting | Default | Description |
|---|---|---|
| `MaxInventoryStackSize` | 99 | Default per-slot stack limit for `UInventorySystemComponent` |
| `MaxInventorySize` | 200 | Default slot count for `UInventorySystemComponent` |
| `MaxItemEquipmentStackSize` | 99 | Default per-slot stack limit for equipment slots |
| `MaxItemContainerStackSize` | 99 | Default per-slot stack limit for `UItemContainerComponent` |
| `MaxItemContainerSize` | 20 | Default slot count for `UItemContainerComponent` |
| `MaxItemDropStackSize` | 99 | Max amount a single `AItemDrop` can hold |

These are global defaults. Individual components can override them at runtime via `SetStackSizeConfig()` and `SetInventorySizeConfig()`.

---

## Delegates Reference

Both components expose delegates for every operation. All delegates fire on the client after server replication completes.

**UItemContainerComponent:**

| Delegate | Fires when |
|---|---|
| `OnAddItemSuccess / Failure` | `AddItem` completes |
| `OnAddItemToSlotSuccess / Failure` | `AddItemToSlot` completes |
| `OnSwapItemSuccess` | `SwapItems` completes |
| `OnRemoveAmountFromSlotSuccess` | `RemoveAmountFromSlot` completes |
| `OnSplitItemStackSuccess` | `SplitItemStack` completes |
| `OnAddItemToComponentSuccess` | `AddItemToComponent` completes |
| `OnSwapItemWithComponentSuccess` | `SwapItemWithComponent` completes |
| `OnCollectAllItemsSuccess` | `CollectAllItems` completes |
| `OnChangedInventorySlots` | Any slot data changes (batch update for UI) |
| `OnSetInventorySizeSuccess` | Inventory size changed |
| `OnSetMaxStackSizeSuccess` | Stack size config changed |

**UInventorySystemComponent (additional):**

| Delegate | Fires when |
|---|---|
| `OnPickUpItemSuccess / Failure` | Item pickup from `AItemDrop` |
| `OnAddItemToEquipmentSlotSuccess / Failure` | Item added to equipment slot |
| `OnItemEquipFromInventorySuccess` | Item moved to equipment |
| `OnItemUnequipSuccess` | Item moved back to inventory |
| `OnRemoveEquipmentAmountFromSlotSuccess` | Equipment removed |
| `OnChangedEquipmentSlots` | Any equipment slot changes (batch update for UI) |
| `OnSetEquipmentTypeSuccess / Failure` | Equipment type assigned or removed |
| `OnSetMaxEquipmentStackSizeSuccess` | Equipment stack size config changed |

---

## Extending the Plugin

### Custom Item Types

Subclass `UItemDataAsset` in C++ or Blueprint:

```cpp
UCLASS(BlueprintType)
class MYGAME_API UWeaponDataAsset : public UItemDataAsset
{
    GENERATED_BODY()

public:
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    float BaseDamage;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    float AttackSpeed;
};
```

The editor asset factories support selecting any `UItemDataAsset` subclass via the built-in class picker.

### Custom Container Logic

Subclass `UItemContainerComponent` or `UInventorySystemComponent` and override the virtual methods to add custom validation or side effects before or after any operation.

---

## Multiplayer Notes

- All item modification methods are `UFUNCTION(Server, Reliable, WithValidation)`.
- Container data (indices, asset references, amounts, dynamic stats) is fully replicated.
- `bIsProcessing` flags prevent duplicate calls while a server round-trip is pending.
- UI delegates are driven by `OnRep` callbacks, so they fire correctly on all clients without additional plumbing.
- The plugin does **not** handle cheat prevention beyond basic server validation. Add game-specific anti-cheat logic in subclass overrides.

---

## Contributing

Contributions are welcome. Please open an issue before submitting large changes so the approach can be discussed first.

1. Fork the repository
2. Create a feature branch: `git checkout -b feature/my-feature`
3. Commit your changes following Unreal Engine coding standards
4. Open a Pull Request against `version here`

Guidelines:
- Use `UPROPERTY`/`UFUNCTION` specifiers on all exposed members
- All state-modifying operations must be Server RPCs
- Keep runtime and editor code in their respective modules

---

## License

[MIT License](LICENSE) — Copyright 2026 Daniel the Developer