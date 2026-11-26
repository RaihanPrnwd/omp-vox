# omp-vox

[![open.mp](https://img.shields.io/badge/open.mp-compatible-blue.svg)](https://open.mp)
[![SA-MP](https://img.shields.io/badge/SA--MP-0.3.7-orange.svg)](https://sa-mp.mp)
[![SampVoice](https://img.shields.io/badge/SampVoice-v11-green.svg)](https://github.com/AmyrAhmady/sampvoice)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)

A clean, ergonomic, zero-boilerplate Pawn voice communication library for **open.mp** and **SA-MP** roleplay servers, built on top of [SampVoice](https://github.com/AmyrAhmady/sampvoice) by AmyrAhmady.

`omp-vox` eliminates manual stream creation, speaker/listener handle routing, and push-to-talk key management boilerplate. It provides high-level APIs for **local proximity voice chat**, **walkie-talkie / handy-talky (HT) radio channels**, **1-on-1 private phone calls**, and **global admin voice broadcasts**.

---

## Features

- **Local Proximity Voice:** Per-player dynamic local streams (`SV_DLSTREAM`) with configurable range and distance changes on the fly.
- **Handy Talky (HT) Radio:** Decimal frequency tuning (e.g. `104.5 MHz`), automatic dynamic channel creation and cleanup, frequency locking, and push-to-talk key binding.
- **1-on-1 Phone Calls:** Point-to-point bi-directional calling with optional local voice pass-through so nearby players can overhear conversations.
- **Global Admin Voice:** Global broadcast streams (`SV_GSTREAM`) for administrative announcements.
- **Zero-Boilerplate Lifecycle:** Standard ALS hooking (`_ALS_`) for automatic stream cleanup on player disconnect and keypress dispatch.
- **Roleplay Callbacks:** Granular custom callbacks (`OnVox*`) for UI notifications, walkie-talkie animation triggers, and call state management.

---

## Prerequisites

1. [SampVoice Server Plugin](https://github.com/AmyrAhmady/sampvoice) installed on your open.mp or SA-MP server.
2. `sampvoice.inc` placed in your compiler's include path.
3. Include `<sampvoice>` **before** `<omp_vox>`.

---

## Installation

### Via `sampctl`
```bash
sampctl package install RaihanPrnwd/omp-vox
```

### Manual Installation
Download `include/omp_vox.inc` from this repository and copy it to your server's `qawno/include` or `pawno/include` folder:
```pawn
#include <open.mp>
#include <sampvoice>
#include <omp_vox>
```

---

## Configuration Defines

Configure these macros before including `<omp_vox>` to override default behaviors:

| Macro | Default | Description |
|---|---|---|
| `VOX_DEFAULT_LOCAL_DISTANCE` | `15.0` | Default proximity hearing radius in units |
| `VOX_DEFAULT_LOCAL_KEY` | `0x42` | Default push-to-talk key for local chat (`B` key) |
| `VOX_DEFAULT_HT_KEY` | `0x5A` | Default push-to-talk key for HT radio (`Z` key) |
| `VOX_MAX_HT_CHANNELS` | `128` | Maximum concurrent active radio frequency channels |
| `VOX_HT_FREQ_MULTIPLIER` | `10` | Frequency integer hashing multiplier (1 decimal place) |
| `VOX_DISABLE_CALLBACKS` | *(undefined)* | Define to disable custom `OnVox*` callback dispatches |
| `VOX_DISABLE_AUTOCLEANUP` | *(undefined)* | Define to disable automatic `OnPlayerConnect` / `OnPlayerDisconnect` hooks |
| `VOX_DISABLE_KEYHOOKS` | *(undefined)* | Define to disable automatic push-to-talk key event hooks |

---

## Quick Start Example

```pawn
#include <open.mp>
#include <sampvoice>
#include <omp_vox>

public OnPlayerSpawn(playerid)
{
    // Enable proximity voice with 15.0m range
    Vox_SetLocalVoiceEnabled(playerid, true);
    Vox_SetLocalVoiceDistance(playerid, 15.0);

    // Equip Handy Talky tuned to 104.5 MHz
    Vox_SetHTFrequency(playerid, 104.5);
    Vox_SetHTEnabled(playerid, true);
    return 1;
}

// Trigger animations or UI indicators when talking
public OnVoxPlayerLocalTalkStart(playerid)
{
    ApplyAnimation(playerid, "PED", "IDLE_CHAT", 4.1, 1, 1, 1, 1, 1, 1);
    return 1;
}

public OnVoxPlayerLocalTalkStop(playerid)
{
    ClearAnimations(playerid);
    return 1;
}

// Start phone call via roleplay command
public OnPlayerCommandText(playerid, cmdtext[])
{
    if (!strcmp(cmdtext, "/call", true, 5))
    {
        new targetid = strval(cmdtext[6]);
        Vox_StartPhoneCall(playerid, targetid, true);
        SendClientMessage(playerid, 0x00FF00AA, "Call connected.");
        return 1;
    }
    if (!strcmp(cmdtext, "/hangup", true))
    {
        Vox_EndPhoneCall(playerid);
        SendClientMessage(playerid, 0xFF0000AA, "Call ended.");
        return 1;
    }
    return 0;
}
```

---

## API Reference

### Local Voice
| Function | Return | Description |
|---|---|---|
| `Vox_SetLocalVoiceEnabled(playerid, bool:toggle)` | `bool` | Enables or disables player's proximity voice stream |
| `Vox_IsLocalVoiceEnabled(playerid)` | `bool` | Checks if player's local voice is enabled |
| `Vox_SetLocalVoiceDistance(playerid, Float:distance)` | `bool` | Updates player's local voice hearing range |
| `Float:Vox_GetLocalVoiceDistance(playerid)` | `Float` | Returns current hearing distance of player |
| `Vox_SetDefaultLocalDistance(Float:distance)` | `bool` | Updates global default local distance |
| `Float:Vox_GetDefaultLocalDistance()` | `Float` | Returns global default local distance |
| `Vox_MutePlayerLocal(playerid, bool:mute)` | `bool` | Mutes or unmutes player from speaking locally |
| `Vox_IsPlayerLocalMuted(playerid)` | `bool` | Checks if player is muted locally |
| `Vox_IsPlayerLocalTalking(playerid)` | `bool` | Checks if player is currently holding local push-to-talk |
| `Vox_SetLocalKey(key)` | `bool` | Updates the push-to-talk key ID for local voice |
| `Vox_GetLocalKey()` | `int` | Returns current push-to-talk key ID for local voice |

### Handy Talky (Radio)
| Function | Return | Description |
|---|---|---|
| `Vox_SetHTEnabled(playerid, bool:toggle)` | `bool` | Enables or disables HT radio device |
| `Vox_IsHTEnabled(playerid)` | `bool` | Checks if HT radio is active |
| `Vox_SetHTFrequency(playerid, Float:frequency)` | `bool` | Sets player's active HT frequency (e.g. `104.5`) |
| `Float:Vox_GetHTFrequency(playerid)` | `Float` | Returns current active frequency |
| `Vox_SetHTTransmitting(playerid, bool:transmitting)` | `bool` | Starts or stops audio transmission on frequency |
| `Vox_IsHTTransmitting(playerid)` | `bool` | Checks if player is transmitting on radio |
| `Vox_LockHTFrequency(Float:frequency, bool:locked)` | `bool` | Locks channel to prevent garbage collection |
| `Vox_IsHTFrequencyLocked(Float:frequency)` | `bool` | Checks if frequency channel is locked |
| `Vox_GetHTFrequencyListeners(Float:frequency)` | `int` | Returns count of active listeners tuned to frequency |
| `Vox_SetHTKey(key)` | `bool` | Updates push-to-talk key ID for HT radio |
| `Vox_GetHTKey()` | `int` | Returns current push-to-talk key ID for HT radio |

### Phone Call (1-on-1)
| Function | Return | Description |
|---|---|---|
| `Vox_StartPhoneCall(callerid, targetid, bool:keepLocalVoice = true)` | `bool` | Establishes private voice call between two players |
| `Vox_EndPhoneCall(playerid)` | `bool` | Terminates active call for player and partner |
| `Vox_IsInPhoneCall(playerid)` | `bool` | Checks if player is currently in a call |
| `Vox_GetPhoneCallPartner(playerid)` | `int` | Returns partner player ID or `INVALID_PLAYER_ID` |
| `Vox_SetPhoneCallMuted(playerid, bool:mute)` | `bool` | Mutes or unmutes player's microphone in call |
| `Vox_IsPhoneCallMuted(playerid)` | `bool` | Checks if player's phone transmission is muted |

### Global Admin Voice
| Function | Return | Description |
|---|---|---|
| `Vox_SetAdminVoiceEnabled(playerid, bool:toggle)` | `bool` | Grants or revokes admin broadcast permissions |
| `Vox_IsAdminVoiceEnabled(playerid)` | `bool` | Checks if player has admin voice enabled |
| `Vox_SetAdminVoiceMode(playerid, bool:broadcastActive)` | `bool` | Starts or stops global voice transmission |
| `Vox_IsAdminVoiceBroadcasting(playerid)` | `bool` | Checks if player is currently broadcasting |
| `Vox_MutePlayerGlobal(playerid, bool:mute)` | `bool` | Mutes or unmutes player from hearing global broadcasts |
| `Vox_IsPlayerGlobalMuted(playerid)` | `bool` | Checks if player is muted from global broadcasts |

---

## Callbacks

| Callback | Description |
|---|---|
| `OnVoxPlayerLocalTalkStart(playerid)` | Called when player presses local push-to-talk key |
| `OnVoxPlayerLocalTalkStop(playerid)` | Called when player releases local push-to-talk key |
| `OnVoxLocalDistanceChanged(playerid, Float:oldDistance, Float:newDistance)` | Called when player's local voice distance changes |
| `OnVoxHTFrequencyChanged(playerid, Float:oldFreq, Float:newFreq)` | Called when player tunes to a different frequency |
| `OnVoxHTTransmitStart(playerid, Float:frequency)` | Called when player starts radio transmission |
| `OnVoxHTTransmitStop(playerid, Float:frequency)` | Called when player stops radio transmission |
| `OnVoxPhoneCallStarted(callerid, targetid)` | Called when phone call connects |
| `OnVoxPhoneCallEnded(playerid, partnerid)` | Called when phone call disconnects |
| `OnVoxAdminVoiceStart(playerid)` | Called when admin begins broadcasting |
| `OnVoxAdminVoiceStop(playerid)` | Called when admin ends broadcasting |

---

## License

This project is licensed under the [MIT License](LICENSE).
