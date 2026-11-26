<<<<<<< HEAD
# omp-vox
Ergonomic Pawn roleplay voice wrapper for SampVoice on open.mp &amp; SA-MP (local proximity with custom range, handy talky with frequency control, phone calls, and global admin voice).
=======
# omp-3dtext

[![open.mp Compatible](https://img.shields.io/badge/open.mp-compatible-blue.svg)](https://open.mp/)
[![SA-MP](https://img.shields.io/badge/SA--MP-0.3.7-orange.svg)](https://sa-mp.mp/)
[![Streamer Plugin](https://img.shields.io/badge/streamer-v2.9+-green.svg)](https://github.com/samp-incognito/samp-streamer-plugin)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)

A modern, type-safe, and ergonomic Pawn include wrapper for Incognito's Streamer Plugin Dynamic 3D TextLabels (`STREAMER_TYPE_3D_TEXT_LABEL`). Designed for open.mp and SA-MP developers to reduce boilerplate, avoid raw data ID manipulation, and eliminate manual lifecycle tracking.

---

## Prerequisites & Compatibility

- **Incognito's Streamer Plugin** (`samp-streamer-plugin` v2.9+): Mandatory dependency. Must be included before `<omp_3dtext>`.
- **open.mp** (`<open.mp>`) or **SA-MP** (`<a_samp>`): Fully compatible with both standard library environments.
- **Pawn Compiler**: Compiles cleanly with zero warnings under standard and strict warning compiler flags (`-d3 -;+ -(+ -Z+`).

---

## Installation

### Via `sampctl`

Add `RaihanPrnwd/omp-3dtext` to your `pawn.json` dependencies:

```json
{
  "dependencies": [
    "openmultiplayer/omp-stdlib",
    "samp-incognito/samp-streamer-plugin",
    "RaihanPrnwd/omp-3dtext"
  ]
}
```

Or install directly via CLI:

```bash
sampctl package install RaihanPrnwd/omp-3dtext
```

### Manual Installation

1. Download [`include/omp_3dtext.inc`](include/omp_3dtext.inc).
2. Copy it into your server's `pawno/include/`, `qawno/include/`, or custom include directory.
3. Include `<streamer>` before `<omp_3dtext>` in your script:

```pawn
#include <open.mp> // or <a_samp>
#include <streamer>
#include <omp_3dtext>
```

---

## Configuration Macros

Define these macros **before** including `<omp_3dtext>` to customize library behavior:

| Macro | Default | Description |
| :--- | :--- | :--- |
| `MAX_O3D_TEXT_LENGTH` | `1024` | Maximum buffer length in cells for formatted text and text retrieval buffers. |
| `O3D_DISABLE_CALLBACKS` | *undefined* | Define to disable invoking custom lifecycle callbacks (`OnO3DLabel*`). |
| `O3D_DISABLE_AUTOCLEANUP` | *undefined* | Define to disable automatic disconnect cleanup and ALS hooking on `OnPlayerDisconnect`. |

### Built-in Constants

| Constant | Value | Description |
| :--- | :--- | :--- |
| `O3D_ATTACH_TYPE_NONE` | `0` | Attachment type representing no attachment / detached state. |
| `O3D_ATTACH_TYPE_PLAYER` | `1` | Attachment type representing a player attachment. |
| `O3D_ATTACH_TYPE_VEHICLE` | `2` | Attachment type representing a vehicle attachment. |

---

## Quick Start & Usage Examples

### 1. Creating Labels with Defaults

```pawn
// Standard creation with clean default parameters
new STREAMER_TAG_3D_TEXT_LABEL:label = O3D_Create(
    "Welcome to the Server!", 
    0x00FF00FF, 
    2000.0, 1500.0, 15.0, 
    25.0 // draw distance
);

// Player-scoped label
new STREAMER_TAG_3D_TEXT_LABEL:pLabel = O3D_CreateForPlayer(
    playerid, 
    "Personal Checkpoint Marker", 
    0xFFFF00FF, 
    2010.0, 1500.0, 15.0
);
```

### 2. Zero-Boilerplate Formatted Text

Create and update text labels using inline format strings without declaring temporary arrays:

```pawn
// Create with formatted text
new STREAMER_TAG_3D_TEXT_LABEL:bankLabel = O3D_Createf(
    20.0, 
    0x00FFFFFF, 
    1450.0, -1020.0, 24.0, 
    "Bank Vault\nStatus: %s\nReserves: $%d", 
    "Locked", 
    250000
);

// Update existing label with formatted text
O3D_UpdateTextf(bankLabel, 0xFF0000FF, "Bank Vault\nStatus: %s\nAlarm: %d seconds", "BREACHED", 30);
```

### 3. Attachment & Detach with Position Sync

```pawn
// Attach label above a vehicle with an offset
O3D_AttachToVehicle(label, vehicleid, 0.0, 0.0, 1.2);

// Detach and automatically synchronize world coordinates to vehicle's current position
O3D_DetachEx(label, true);
```

### 4. Visibility Control & Distance Calculation

```pawn
// Hide and show for specific players
O3D_HideForPlayer(playerid, label);
O3D_ShowForPlayer(playerid, label);

// Calculate 3D Euclidean distance to a world point
new Float:distance;
if (O3D_GetDistanceFromPoint(label, 1400.0, -1000.0, 20.0, distance))
{
    printf("Distance to label: %.2f units", distance);
}
```

### 5. Custom Lifecycle Callbacks

```pawn
public OnO3DLabelCreated(STREAMER_TAG_3D_TEXT_LABEL:labelid, forplayerid)
{
    printf("[3DText] Label ID %d created for player %d", _:labelid, forplayerid);
    return 1;
}

public OnO3DLabelDestroyed(STREAMER_TAG_3D_TEXT_LABEL:labelid)
{
    printf("[3DText] Label ID %d destroyed", _:labelid);
    return 1;
}

public OnO3DLabelTextUpdated(STREAMER_TAG_3D_TEXT_LABEL:labelid, color, const text[])
{
    printf("[3DText] Label ID %d text updated: %s", _:labelid, text);
    return 1;
}

public OnO3DLabelAttached(STREAMER_TAG_3D_TEXT_LABEL:labelid, attachedtype, targetid)
{
    printf("[3DText] Label ID %d attached (type: %d, target: %d)", _:labelid, attachedtype, targetid);
    return 1;
}
```

---

## API Reference

### Creation & Destruction

| Function | Signature / Parameters | Returns | Description |
| :--- | :--- | :--- | :--- |
| `O3D_Create` | `(const text[], color, Float:x, Float:y, Float:z, Float:drawdistance = 20.0, attachedplayer = INVALID_PLAYER_ID, attachedvehicle = INVALID_VEHICLE_ID, testlos = 1, worldid = -1, interiorid = -1, playerid = -1, Float:streamdistance = STREAMER_3D_TEXT_LABEL_SD, STREAMER_TAG_AREA:areaid = STREAMER_TAG_AREA:-1, priority = 0)` | `STREAMER_TAG_3D_TEXT_LABEL:` | Creates a dynamic 3D text label with streamlined defaults. |
| `O3D_CreateEx` | `(const text[], color, Float:x, Float:y, Float:z, Float:drawdistance = 20.0, attachedplayer = INVALID_PLAYER_ID, attachedvehicle = INVALID_VEHICLE_ID, testlos = 1, Float:streamdistance = STREAMER_3D_TEXT_LABEL_SD, const worlds[] = { -1 }, const interiors[] = { -1 }, const players[] = { -1 }, const STREAMER_TAG_AREA:areas[] = { STREAMER_TAG_AREA:-1 }, priority = 0, maxworlds = sizeof worlds, maxinteriors = sizeof interiors, maxplayers = sizeof players, maxareas = sizeof areas)` | `STREAMER_TAG_3D_TEXT_LABEL:` | Creates an extended dynamic 3D text label spanning multiple worlds, interiors, or players. |
| `O3D_CreateForPlayer` | `(playerid, const text[], color, Float:x, Float:y, Float:z, Float:drawdistance = 20.0, attachedplayer = INVALID_PLAYER_ID, attachedvehicle = INVALID_VEHICLE_ID, testlos = 1, worldid = -1, interiorid = -1, Float:streamdistance = STREAMER_3D_TEXT_LABEL_SD, STREAMER_TAG_AREA:areaid = STREAMER_TAG_AREA:-1, priority = 0)` | `STREAMER_TAG_3D_TEXT_LABEL:` | Creates a dynamic 3D text label visible strictly to a single player. |
| `O3D_Createf` | `(Float:drawdistance, color, Float:x, Float:y, Float:z, const formatText[], {Float,_}:...)` | `STREAMER_TAG_3D_TEXT_LABEL:` | Creates a dynamic 3D text label using a formatted string. |
| `O3D_CreateForPlayerf` | `(playerid, Float:drawdistance, color, Float:x, Float:y, Float:z, const formatText[], {Float,_}:...)` | `STREAMER_TAG_3D_TEXT_LABEL:` | Creates a player-scoped dynamic 3D text label using a formatted string. |
| `O3D_Destroy` | `(STREAMER_TAG_3D_TEXT_LABEL:id)` | `1` / `0` | Destroys a dynamic 3D text label safely. |
| `O3D_DestroyAll` | `(serverwide = 1)` | `1` / `0` | Destroys all dynamic 3D text labels created in the streamer. |
| `O3D_IsValid` | `(STREAMER_TAG_3D_TEXT_LABEL:id)` | `1` / `0` | Checks whether a dynamic 3D text label ID is valid. |
| `O3D_Count` | `()` | `cell` | Returns total number of registered dynamic 3D text labels. |

### Text & Color Manipulation

| Function | Signature / Parameters | Returns | Description |
| :--- | :--- | :--- | :--- |
| `O3D_GetText` | `(STREAMER_TAG_3D_TEXT_LABEL:id, dest[], maxlength = sizeof dest)` | `1` / `0` | Retrieves the text of a dynamic 3D text label. |
| `O3D_UpdateText` | `(STREAMER_TAG_3D_TEXT_LABEL:id, color, const text[])` | `1` / `0` | Updates both text and color of a dynamic 3D text label. |
| `O3D_SetText` | `(STREAMER_TAG_3D_TEXT_LABEL:id, const text[])` | `1` / `0` | Updates text while preserving the existing color. |
| `O3D_SetColor` | `(STREAMER_TAG_3D_TEXT_LABEL:id, color)` | `1` / `0` | Updates color while preserving the existing text. |
| `O3D_GetColor` | `(STREAMER_TAG_3D_TEXT_LABEL:id)` | `cell` | Retrieves the color integer of a dynamic 3D text label. |
| `O3D_UpdateTextf` | `(STREAMER_TAG_3D_TEXT_LABEL:id, color, const formatText[], {Float,_}:...)` | `1` / `0` | Updates text of a label using a formatted string. |

### Position & Streamer Properties

| Function | Signature / Parameters | Returns | Description |
| :--- | :--- | :--- | :--- |
| `O3D_GetPos` | `(STREAMER_TAG_3D_TEXT_LABEL:id, &Float:x, &Float:y, &Float:z)` | `1` / `0` | Retrieves 3D position coordinates of a label. |
| `O3D_SetPos` | `(STREAMER_TAG_3D_TEXT_LABEL:id, Float:x, Float:y, Float:z)` | `1` / `0` | Updates 3D position coordinates of a label. |
| `O3D_GetDistanceFromPoint` | `(STREAMER_TAG_3D_TEXT_LABEL:id, Float:x, Float:y, Float:z, &Float:distance)` | `1` / `0` | Calculates 3D Euclidean distance between label and given point. |
| `O3D_GetDrawDistance` | `(STREAMER_TAG_3D_TEXT_LABEL:id)` | `Float:` | Retrieves the draw distance of a label. |
| `O3D_SetDrawDistance` | `(STREAMER_TAG_3D_TEXT_LABEL:id, Float:distance)` | `1` / `0` | Sets the draw distance of a label. |
| `O3D_GetStreamDistance` | `(STREAMER_TAG_3D_TEXT_LABEL:id)` | `Float:` | Retrieves the streaming distance of a label. |
| `O3D_SetStreamDistance` | `(STREAMER_TAG_3D_TEXT_LABEL:id, Float:distance)` | `1` / `0` | Sets the streaming distance of a label. |
| `O3D_GetWorld` | `(STREAMER_TAG_3D_TEXT_LABEL:id)` | `cell` | Retrieves the virtual world ID of a label. |
| `O3D_SetWorld` | `(STREAMER_TAG_3D_TEXT_LABEL:id, worldid)` | `1` / `0` | Sets the virtual world ID of a label. |
| `O3D_GetInterior` | `(STREAMER_TAG_3D_TEXT_LABEL:id)` | `cell` | Retrieves the interior ID of a label. |
| `O3D_SetInterior` | `(STREAMER_TAG_3D_TEXT_LABEL:id, interiorid)` | `1` / `0` | Sets the interior ID of a label. |
| `O3D_GetArea` | `(STREAMER_TAG_3D_TEXT_LABEL:id)` | `STREAMER_TAG_AREA:` | Retrieves the area constraint ID of a label. |
| `O3D_SetArea` | `(STREAMER_TAG_3D_TEXT_LABEL:id, STREAMER_TAG_AREA:areaid)` | `1` / `0` | Sets the area constraint ID of a label. |
| `O3D_GetTestLOS` | `(STREAMER_TAG_3D_TEXT_LABEL:id)` | `1` / `0` | Retrieves whether line-of-sight testing is enabled. |
| `O3D_SetTestLOS` | `(STREAMER_TAG_3D_TEXT_LABEL:id, testlos)` | `1` / `0` | Enables or disables line-of-sight testing. |
| `O3D_GetPriority` | `(STREAMER_TAG_3D_TEXT_LABEL:id)` | `cell` | Retrieves streamer priority of a label. |
| `O3D_SetPriority` | `(STREAMER_TAG_3D_TEXT_LABEL:id, priority)` | `1` / `0` | Sets streamer priority of a label. |
| `O3D_GetExtraID` | `(STREAMER_TAG_3D_TEXT_LABEL:id)` | `cell` | Retrieves custom extra ID attached to a label. |
| `O3D_SetExtraID` | `(STREAMER_TAG_3D_TEXT_LABEL:id, extraid)` | `1` / `0` | Sets custom extra ID attached to a label. |

### Attachments & Offsets

| Function | Signature / Parameters | Returns | Description |
| :--- | :--- | :--- | :--- |
| `O3D_AttachToPlayer` | `(STREAMER_TAG_3D_TEXT_LABEL:id, playerid, Float:offsetX = 0.0, Float:offsetY = 0.0, Float:offsetZ = 0.0)` | `1` / `0` | Attaches a label to a player with relative offsets. |
| `O3D_AttachToVehicle` | `(STREAMER_TAG_3D_TEXT_LABEL:id, vehicleid, Float:offsetX = 0.0, Float:offsetY = 0.0, Float:offsetZ = 0.0)` | `1` / `0` | Attaches a label to a vehicle with relative offsets. |
| `O3D_Detach` | `(STREAMER_TAG_3D_TEXT_LABEL:id)` | `1` / `0` | Detaches a label from any player or vehicle. |
| `O3D_DetachEx` | `(STREAMER_TAG_3D_TEXT_LABEL:id, bool:syncPos = true)` | `1` / `0` | Detaches a label and optionally synchronizes coordinates with the entity. |
| `O3D_GetAttachedPlayer` | `(STREAMER_TAG_3D_TEXT_LABEL:id)` | `cell` | Retrieves the player ID the label is attached to. |
| `O3D_GetAttachedVehicle` | `(STREAMER_TAG_3D_TEXT_LABEL:id)` | `cell` | Retrieves the vehicle ID the label is attached to. |
| `O3D_GetAttachOffsets` | `(STREAMER_TAG_3D_TEXT_LABEL:id, &Float:offsetX, &Float:offsetY, &Float:offsetZ)` | `1` / `0` | Retrieves the attachment offset values of a label. |

### Streaming & Per-Player Visibility

| Function | Signature / Parameters | Returns | Description |
| :--- | :--- | :--- | :--- |
| `O3D_ToggleForPlayer` | `(playerid, STREAMER_TAG_3D_TEXT_LABEL:id, toggle)` | `1` / `0` | Toggles label streaming for a specific player. |
| `O3D_ShowForPlayer` | `(playerid, STREAMER_TAG_3D_TEXT_LABEL:id)` | `1` / `0` | Shows a dynamic label for a specific player. |
| `O3D_HideForPlayer` | `(playerid, STREAMER_TAG_3D_TEXT_LABEL:id)` | `1` / `0` | Hides a dynamic label for a specific player. |
| `O3D_IsToggleForPlayer` | `(playerid, STREAMER_TAG_3D_TEXT_LABEL:id)` | `1` / `0` | Checks whether label streaming is toggled for a player. |
| `O3D_UpdateForPlayer` | `(playerid, Float:x, Float:y, Float:z, worldid = -1, interiorid = -1, compensatedtime = -1, freezeplayer = 1)` | `1` / `0` | Forces an immediate streamer update for a player. |
| `O3D_CleanupPlayer` | `(playerid)` | `1` | Destroys player-scoped dynamic labels and detaches labels attached to `playerid`. |

### Custom Lifecycle Callbacks

| Callback | Signature | Description |
| :--- | :--- | :--- |
| `OnO3DLabelCreated` | `(STREAMER_TAG_3D_TEXT_LABEL:labelid, forplayerid)` | Called when a label is created via `O3D_Create*`. |
| `OnO3DLabelDestroyed` | `(STREAMER_TAG_3D_TEXT_LABEL:labelid)` | Called when a label is destroyed via `O3D_Destroy`. |
| `OnO3DLabelTextUpdated` | `(STREAMER_TAG_3D_TEXT_LABEL:labelid, color, const text[])` | Called when label text or color is updated. |
| `OnO3DLabelAttached` | `(STREAMER_TAG_3D_TEXT_LABEL:labelid, attachedtype, targetid)` | Called when a label is attached or detached (`0` = none, `1` = player, `2` = vehicle). |

---

## Testing & Contributing

A full test suite is provided in [`test.pwn`](test.pwn) covering every function, default argument, boundary condition, invalid ID guard, disconnect cleanup, and custom callback.

To compile and run tests locally with `qawno` or `pawncc`:

```bash
qawno\pawncc.exe test.pwn -iqawno\include -d3 -;+ -(+ -Z+
```

Contributions, bug reports, and pull requests are welcome on GitHub at [RaihanPrnwd/omp-3dtext](https://github.com/RaihanPrnwd/omp-3dtext).

---

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.  
Copyright (c) 2026 RaihanPrnwd
>>>>>>> b3c7de5 (feat: initial release of omp-vox roleplay wrapper for SampVoice)
