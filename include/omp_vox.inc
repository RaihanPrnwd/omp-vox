#if defined _omp_vox_included
    #endinput
#endif
#define _omp_vox_included

#tryinclude <sampvoice>
#if !defined _sampvoice_included
    #error "<omp_vox> requires <sampvoice> to be included first. Please install and include sampvoice."
#endif

// =============================================================================
// Constants & Configuration Defaults
// =============================================================================

#if !defined VOX_DEFAULT_LOCAL_DISTANCE
    #define VOX_DEFAULT_LOCAL_DISTANCE          (15.0)
#endif

#if !defined VOX_DEFAULT_LOCAL_KEY
    #define VOX_DEFAULT_LOCAL_KEY               (0x42) // 'B' key
#endif

#if !defined VOX_DEFAULT_HT_KEY
    #define VOX_DEFAULT_HT_KEY                  (0x5A) // 'Z' key
#endif

#if !defined VOX_MAX_HT_CHANNELS
    #define VOX_MAX_HT_CHANNELS                 (128)
#endif

#if !defined VOX_HT_FREQ_MULTIPLIER
    #define VOX_HT_FREQ_MULTIPLIER              (10)
#endif

#if !defined VOX_PHONE_STREAM_DISTANCE
    #define VOX_PHONE_STREAM_DISTANCE           (100000.0)
#endif

#if !defined VOX_LOCAL_STREAM_MAXPLAYERS
    #define VOX_LOCAL_STREAM_MAXPLAYERS         (SV_NULL)
#endif

#if !defined VOX_COLOR_LOCAL
    #define VOX_COLOR_LOCAL                     (0x00FF00AA)
#endif

#if !defined VOX_COLOR_HT
    #define VOX_COLOR_HT                        (0xFF9900AA)
#endif

#if !defined VOX_COLOR_PHONE
    #define VOX_COLOR_PHONE                     (0x0099FFAA)
#endif

#if !defined VOX_COLOR_ADMIN
    #define VOX_COLOR_ADMIN                     (0xFF0000AA)
#endif

// Global internal stream tags
static stock SV_DLSTREAM:g_VoxLocalStream[MAX_PLAYERS] = { SV_NULL, ... };
static stock Float:g_VoxLocalDistance[MAX_PLAYERS] = { VOX_DEFAULT_LOCAL_DISTANCE, ... };
static stock bool:g_VoxLocalEnabled[MAX_PLAYERS] = { false, ... };
static stock bool:g_VoxLocalMuted[MAX_PLAYERS] = { false, ... };
static stock bool:g_VoxLocalTalking[MAX_PLAYERS] = { false, ... };

static stock Float:g_VoxDefaultLocalDistance = VOX_DEFAULT_LOCAL_DISTANCE;
static stock g_VoxLocalKey = VOX_DEFAULT_LOCAL_KEY;
static stock g_VoxHTKey = VOX_DEFAULT_HT_KEY;

// Handy Talky state
enum E_VOX_HT_CHANNEL
{
    bool:e_VoxHTUsed,
    e_VoxHTFreqKey,
    SV_GSTREAM:e_VoxHTStream,
    bool:e_VoxHTLocked,
    e_VoxHTListeners
}
static stock g_VoxHTChannels[VOX_MAX_HT_CHANNELS][E_VOX_HT_CHANNEL];

static stock bool:g_VoxHTEnabled[MAX_PLAYERS] = { false, ... };
static stock g_VoxHTPlayerFreqKey[MAX_PLAYERS] = { 0, ... };
static stock bool:g_VoxHTTransmitting[MAX_PLAYERS] = { false, ... };

// Phone Call state
static stock g_VoxPhonePartner[MAX_PLAYERS] = { INVALID_PLAYER_ID, ... };
static stock SV_DLSTREAM:g_VoxPhoneStream[MAX_PLAYERS] = { SV_NULL, ... };
static stock bool:g_VoxPhoneKeepLocal[MAX_PLAYERS] = { true, ... };
static stock bool:g_VoxPhoneMuted[MAX_PLAYERS] = { false, ... };

// Admin Voice state
static stock SV_GSTREAM:g_VoxAdminStream = SV_NULL;
static stock bool:g_VoxAdminEnabled[MAX_PLAYERS] = { false, ... };
static stock bool:g_VoxAdminBroadcasting[MAX_PLAYERS] = { false, ... };
static stock bool:g_VoxGlobalMuted[MAX_PLAYERS] = { false, ... };

// =============================================================================
// Forward Declarations (Avoid warning 208)
// =============================================================================

forward bool:Vox_SetHTTransmitting(playerid, bool:transmitting);
forward bool:Vox_EndPhoneCall(playerid);
forward bool:Vox_SetAdminVoiceMode(playerid, bool:broadcastActive);

// =============================================================================
// Roleplay Callback Forwards
// =============================================================================

#if !defined VOX_DISABLE_CALLBACKS
    forward OnVoxPlayerLocalTalkStart(playerid);
    forward OnVoxPlayerLocalTalkStop(playerid);
    forward OnVoxLocalDistanceChanged(playerid, Float:oldDistance, Float:newDistance);
    forward OnVoxHTFrequencyChanged(playerid, Float:oldFreq, Float:newFreq);
    forward OnVoxHTTransmitStart(playerid, Float:frequency);
    forward OnVoxHTTransmitStop(playerid, Float:frequency);
    forward OnVoxPhoneCallStarted(callerid, targetid);
    forward OnVoxPhoneCallEnded(playerid, partnerid);
    forward OnVoxAdminVoiceStart(playerid);
    forward OnVoxAdminVoiceStop(playerid);
#endif

// =============================================================================
// Helper Internal Utilities
// =============================================================================

static stock Vox_EncodeFrequency(Float:frequency)
{
    if (frequency <= 0.0) return 0;
    return floatround(frequency * float(VOX_HT_FREQ_MULTIPLIER), floatround_round);
}

static stock Float:Vox_DecodeFrequency(freqKey)
{
    if (freqKey <= 0) return 0.0;
    return float(freqKey) / float(VOX_HT_FREQ_MULTIPLIER);
}

static stock Vox_FindHTChannel(freqKey)
{
    if (freqKey <= 0) return -1;
    for (new i = 0; i < VOX_MAX_HT_CHANNELS; i++)
    {
        if (g_VoxHTChannels[i][e_VoxHTUsed] && g_VoxHTChannels[i][e_VoxHTFreqKey] == freqKey)
        {
            return i;
        }
    }
    return -1;
}

static stock Vox_GetOrCreateHTChannel(freqKey)
{
    if (freqKey <= 0) return -1;
    new slot = Vox_FindHTChannel(freqKey);
    if (slot != -1) return slot;

    // Find free channel slot
    for (new i = 0; i < VOX_MAX_HT_CHANNELS; i++)
    {
        if (!g_VoxHTChannels[i][e_VoxHTUsed])
        {
            new streamName[16];
            format(streamName, sizeof(streamName), "VoxHT_%d", freqKey);

            g_VoxHTChannels[i][e_VoxHTUsed] = true;
            g_VoxHTChannels[i][e_VoxHTFreqKey] = freqKey;
            g_VoxHTChannels[i][e_VoxHTLocked] = false;
            g_VoxHTChannels[i][e_VoxHTListeners] = 0;
            g_VoxHTChannels[i][e_VoxHTStream] = SvCreateGStream(VOX_COLOR_HT, streamName);
            return i;
        }
    }
    return -1;
}

static stock Vox_CleanupHTChannelIfEmpty(slot)
{
    if (slot < 0 || slot >= VOX_MAX_HT_CHANNELS) return;
    if (!g_VoxHTChannels[slot][e_VoxHTUsed]) return;

    if (g_VoxHTChannels[slot][e_VoxHTListeners] <= 0 && !g_VoxHTChannels[slot][e_VoxHTLocked])
    {
        if (g_VoxHTChannels[slot][e_VoxHTStream] != SV_NULL)
        {
            SvDeleteStream(g_VoxHTChannels[slot][e_VoxHTStream]);
            g_VoxHTChannels[slot][e_VoxHTStream] = SV_NULL;
        }
        g_VoxHTChannels[slot][e_VoxHTUsed] = false;
        g_VoxHTChannels[slot][e_VoxHTFreqKey] = 0;
        g_VoxHTChannels[slot][e_VoxHTLocked] = false;
        g_VoxHTChannels[slot][e_VoxHTListeners] = 0;
    }
}

static stock Vox_EnsureAdminStream()
{
    if (g_VoxAdminStream == SV_NULL)
    {
        new streamName[16];
        format(streamName, sizeof(streamName), "VoxAdmin");
        g_VoxAdminStream = SvCreateGStream(VOX_COLOR_ADMIN, streamName);
    }
    return 1;
}

// =============================================================================
// Local Proximity Voice API
// =============================================================================

stock bool:Vox_SetLocalVoiceEnabled(playerid, bool:toggle)
{
    if (!IsPlayerConnected(playerid)) return false;

    if (toggle)
    {
        if (g_VoxLocalStream[playerid] == SV_NULL)
        {
            new Float:dist = (g_VoxLocalDistance[playerid] > 0.0) ? g_VoxLocalDistance[playerid] : g_VoxDefaultLocalDistance;
            new streamName[16];
            format(streamName, sizeof(streamName), "VoxLocal_%d", playerid);
            g_VoxLocalStream[playerid] = SvCreateDLStreamAtPlayer(dist, VOX_LOCAL_STREAM_MAXPLAYERS, playerid, VOX_COLOR_LOCAL, streamName);
        }
        g_VoxLocalEnabled[playerid] = true;
        if (!g_VoxLocalMuted[playerid])
        {
            SvAddKey(playerid, g_VoxLocalKey);
        }
    }
    else
    {
        g_VoxLocalEnabled[playerid] = false;
        SvRemoveKey(playerid, g_VoxLocalKey);
        if (g_VoxLocalTalking[playerid])
        {
            if (g_VoxLocalStream[playerid] != SV_NULL)
            {
                SvDetachSpeakerFromStream(g_VoxLocalStream[playerid], playerid);
            }
            SvStopRecord(playerid);
            g_VoxLocalTalking[playerid] = false;
            #if !defined VOX_DISABLE_CALLBACKS
                CallLocalFunction("OnVoxPlayerLocalTalkStop", "i", playerid);
            #endif
        }
    }
    return true;
}

stock bool:Vox_IsLocalVoiceEnabled(playerid)
{
    if (!IsPlayerConnected(playerid)) return false;
    return g_VoxLocalEnabled[playerid];
}

stock bool:Vox_SetLocalVoiceDistance(playerid, Float:distance)
{
    if (!IsPlayerConnected(playerid) || distance <= 0.0) return false;

    new Float:oldDistance = g_VoxLocalDistance[playerid];
    g_VoxLocalDistance[playerid] = distance;

    if (g_VoxLocalStream[playerid] != SV_NULL)
    {
        SvUpdateDistanceForLStream(g_VoxLocalStream[playerid], distance);
    }

    #if !defined VOX_DISABLE_CALLBACKS
        CallLocalFunction("OnVoxLocalDistanceChanged", "iff", playerid, oldDistance, distance);
    #endif
    return true;
}

stock Float:Vox_GetLocalVoiceDistance(playerid)
{
    if (!IsPlayerConnected(playerid)) return 0.0;
    return g_VoxLocalDistance[playerid];
}

stock bool:Vox_SetDefaultLocalDistance(Float:distance)
{
    if (distance <= 0.0) return false;
    g_VoxDefaultLocalDistance = distance;
    return true;
}

stock Float:Vox_GetDefaultLocalDistance()
{
    return g_VoxDefaultLocalDistance;
}

stock bool:Vox_MutePlayerLocal(playerid, bool:mute)
{
    if (!IsPlayerConnected(playerid)) return false;

    g_VoxLocalMuted[playerid] = mute;
    if (mute)
    {
        SvRemoveKey(playerid, g_VoxLocalKey);
        if (g_VoxLocalTalking[playerid])
        {
            if (g_VoxLocalStream[playerid] != SV_NULL)
            {
                SvDetachSpeakerFromStream(g_VoxLocalStream[playerid], playerid);
            }
            SvStopRecord(playerid);
            g_VoxLocalTalking[playerid] = false;
            #if !defined VOX_DISABLE_CALLBACKS
                CallLocalFunction("OnVoxPlayerLocalTalkStop", "i", playerid);
            #endif
        }
    }
    else
    {
        if (g_VoxLocalEnabled[playerid])
        {
            SvAddKey(playerid, g_VoxLocalKey);
        }
    }
    return true;
}

stock bool:Vox_IsPlayerLocalMuted(playerid)
{
    if (!IsPlayerConnected(playerid)) return false;
    return g_VoxLocalMuted[playerid];
}

stock bool:Vox_IsPlayerLocalTalking(playerid)
{
    if (!IsPlayerConnected(playerid)) return false;
    return g_VoxLocalTalking[playerid];
}

// =============================================================================
// Handy Talky (Radio) API
// =============================================================================

stock bool:Vox_SetHTKey(key)
{
    g_VoxHTKey = key;
    return true;
}

stock Vox_GetHTKey()
{
    return g_VoxHTKey;
}

stock bool:Vox_SetLocalKey(key)
{
    g_VoxLocalKey = key;
    return true;
}

stock Vox_GetLocalKey()
{
    return g_VoxLocalKey;
}

stock bool:Vox_SetHTEnabled(playerid, bool:toggle)
{
    if (!IsPlayerConnected(playerid)) return false;

    if (toggle)
    {
        g_VoxHTEnabled[playerid] = true;
        SvAddKey(playerid, g_VoxHTKey);

        new freqKey = g_VoxHTPlayerFreqKey[playerid];
        if (freqKey > 0)
        {
            new slot = Vox_GetOrCreateHTChannel(freqKey);
            if (slot != -1 && g_VoxHTChannels[slot][e_VoxHTStream] != SV_NULL)
            {
                if (!SvHasListenerInStream(g_VoxHTChannels[slot][e_VoxHTStream], playerid))
                {
                    SvAttachListenerToStream(g_VoxHTChannels[slot][e_VoxHTStream], playerid);
                    g_VoxHTChannels[slot][e_VoxHTListeners]++;
                }
            }
        }
    }
    else
    {
        g_VoxHTEnabled[playerid] = false;
        SvRemoveKey(playerid, g_VoxHTKey);

        if (g_VoxHTTransmitting[playerid])
        {
            Vox_SetHTTransmitting(playerid, false);
        }

        new freqKey = g_VoxHTPlayerFreqKey[playerid];
        if (freqKey > 0)
        {
            new slot = Vox_FindHTChannel(freqKey);
            if (slot != -1 && g_VoxHTChannels[slot][e_VoxHTStream] != SV_NULL)
            {
                if (SvHasListenerInStream(g_VoxHTChannels[slot][e_VoxHTStream], playerid))
                {
                    SvDetachListenerFromStream(g_VoxHTChannels[slot][e_VoxHTStream], playerid);
                    g_VoxHTChannels[slot][e_VoxHTListeners]--;
                    Vox_CleanupHTChannelIfEmpty(slot);
                }
            }
        }
    }
    return true;
}

stock bool:Vox_IsHTEnabled(playerid)
{
    if (!IsPlayerConnected(playerid)) return false;
    return g_VoxHTEnabled[playerid];
}

stock bool:Vox_SetHTFrequency(playerid, Float:frequency)
{
    if (!IsPlayerConnected(playerid)) return false;

    new oldFreqKey = g_VoxHTPlayerFreqKey[playerid];
    new Float:oldFreq = Vox_DecodeFrequency(oldFreqKey);
    new newFreqKey = Vox_EncodeFrequency(frequency);

    if (oldFreqKey == newFreqKey) return true;

    // Detach from old channel
    if (oldFreqKey > 0)
    {
        new oldSlot = Vox_FindHTChannel(oldFreqKey);
        if (oldSlot != -1 && g_VoxHTChannels[oldSlot][e_VoxHTStream] != SV_NULL)
        {
            if (g_VoxHTTransmitting[playerid])
            {
                SvDetachSpeakerFromStream(g_VoxHTChannels[oldSlot][e_VoxHTStream], playerid);
            }
            if (SvHasListenerInStream(g_VoxHTChannels[oldSlot][e_VoxHTStream], playerid))
            {
                SvDetachListenerFromStream(g_VoxHTChannels[oldSlot][e_VoxHTStream], playerid);
                g_VoxHTChannels[oldSlot][e_VoxHTListeners]--;
                Vox_CleanupHTChannelIfEmpty(oldSlot);
            }
        }
    }

    g_VoxHTPlayerFreqKey[playerid] = newFreqKey;

    // Attach to new channel if HT is enabled
    if (g_VoxHTEnabled[playerid] && newFreqKey > 0)
    {
        new newSlot = Vox_GetOrCreateHTChannel(newFreqKey);
        if (newSlot != -1 && g_VoxHTChannels[newSlot][e_VoxHTStream] != SV_NULL)
        {
            if (!SvHasListenerInStream(g_VoxHTChannels[newSlot][e_VoxHTStream], playerid))
            {
                SvAttachListenerToStream(g_VoxHTChannels[newSlot][e_VoxHTStream], playerid);
                g_VoxHTChannels[newSlot][e_VoxHTListeners]++;
            }
            if (g_VoxHTTransmitting[playerid])
            {
                SvAttachSpeakerToStream(g_VoxHTChannels[newSlot][e_VoxHTStream], playerid);
            }
        }
    }

    #if !defined VOX_DISABLE_CALLBACKS
        CallLocalFunction("OnVoxHTFrequencyChanged", "iff", playerid, oldFreq, frequency);
    #endif
    return true;
}

stock Float:Vox_GetHTFrequency(playerid)
{
    if (!IsPlayerConnected(playerid)) return 0.0;
    return Vox_DecodeFrequency(g_VoxHTPlayerFreqKey[playerid]);
}

stock bool:Vox_SetHTTransmitting(playerid, bool:transmitting)
{
    if (!IsPlayerConnected(playerid)) return false;
    if (!g_VoxHTEnabled[playerid] && transmitting) return false;

    new freqKey = g_VoxHTPlayerFreqKey[playerid];
    if (freqKey <= 0 && transmitting) return false;

    new slot = Vox_FindHTChannel(freqKey);
    if (transmitting)
    {
        if (slot == -1)
        {
            slot = Vox_GetOrCreateHTChannel(freqKey);
        }
        if (slot == -1 || g_VoxHTChannels[slot][e_VoxHTStream] == SV_NULL) return false;

        g_VoxHTTransmitting[playerid] = true;
        SvAttachSpeakerToStream(g_VoxHTChannels[slot][e_VoxHTStream], playerid);
        SvStartRecord(playerid);

        #if !defined VOX_DISABLE_CALLBACKS
            CallLocalFunction("OnVoxHTTransmitStart", "if", playerid, Vox_DecodeFrequency(freqKey));
        #endif
    }
    else
    {
        if (g_VoxHTTransmitting[playerid])
        {
            g_VoxHTTransmitting[playerid] = false;
            if (slot != -1 && g_VoxHTChannels[slot][e_VoxHTStream] != SV_NULL)
            {
                SvDetachSpeakerFromStream(g_VoxHTChannels[slot][e_VoxHTStream], playerid);
            }
            if (!g_VoxLocalTalking[playerid] && !g_VoxAdminBroadcasting[playerid] && g_VoxPhonePartner[playerid] == INVALID_PLAYER_ID)
            {
                SvStopRecord(playerid);
            }

            #if !defined VOX_DISABLE_CALLBACKS
                CallLocalFunction("OnVoxHTTransmitStop", "if", playerid, Vox_DecodeFrequency(freqKey));
            #endif
        }
    }
    return true;
}

stock bool:Vox_IsHTTransmitting(playerid)
{
    if (!IsPlayerConnected(playerid)) return false;
    return g_VoxHTTransmitting[playerid];
}

stock bool:Vox_LockHTFrequency(Float:frequency, bool:locked)
{
    new freqKey = Vox_EncodeFrequency(frequency);
    if (freqKey <= 0) return false;

    new slot = Vox_GetOrCreateHTChannel(freqKey);
    if (slot == -1) return false;

    g_VoxHTChannels[slot][e_VoxHTLocked] = locked;
    if (!locked)
    {
        Vox_CleanupHTChannelIfEmpty(slot);
    }
    return true;
}

stock bool:Vox_IsHTFrequencyLocked(Float:frequency)
{
    new freqKey = Vox_EncodeFrequency(frequency);
    if (freqKey <= 0) return false;

    new slot = Vox_FindHTChannel(freqKey);
    if (slot == -1) return false;
    return g_VoxHTChannels[slot][e_VoxHTLocked];
}

stock Vox_GetHTFrequencyListeners(Float:frequency)
{
    new freqKey = Vox_EncodeFrequency(frequency);
    if (freqKey <= 0) return 0;

    new slot = Vox_FindHTChannel(freqKey);
    if (slot == -1) return 0;
    return g_VoxHTChannels[slot][e_VoxHTListeners];
}

// =============================================================================
// Phone Call Voice API (1-on-1)
// =============================================================================

stock bool:Vox_StartPhoneCall(callerid, targetid, bool:keepLocalVoice = true)
{
    if (!IsPlayerConnected(callerid) || !IsPlayerConnected(targetid) || callerid == targetid)
    {
        return false;
    }

    if (g_VoxPhonePartner[callerid] != INVALID_PLAYER_ID)
    {
        Vox_EndPhoneCall(callerid);
    }
    if (g_VoxPhonePartner[targetid] != INVALID_PLAYER_ID)
    {
        Vox_EndPhoneCall(targetid);
    }

    // Create bi-directional high-range DLStreams
    new streamCaller[20];
    new streamTarget[20];
    format(streamCaller, sizeof(streamCaller), "VoxPhone_%d", callerid);
    format(streamTarget, sizeof(streamTarget), "VoxPhone_%d", targetid);

    g_VoxPhoneStream[callerid] = SvCreateDLStreamAtPlayer(VOX_PHONE_STREAM_DISTANCE, 1, callerid, VOX_COLOR_PHONE, streamCaller);
    g_VoxPhoneStream[targetid] = SvCreateDLStreamAtPlayer(VOX_PHONE_STREAM_DISTANCE, 1, targetid, VOX_COLOR_PHONE, streamTarget);

    if (g_VoxPhoneStream[callerid] != SV_NULL)
    {
        SvAttachSpeakerToStream(g_VoxPhoneStream[callerid], callerid);
        SvAttachListenerToStream(g_VoxPhoneStream[callerid], targetid);
    }
    if (g_VoxPhoneStream[targetid] != SV_NULL)
    {
        SvAttachSpeakerToStream(g_VoxPhoneStream[targetid], targetid);
        SvAttachListenerToStream(g_VoxPhoneStream[targetid], callerid);
    }

    g_VoxPhonePartner[callerid] = targetid;
    g_VoxPhonePartner[targetid] = callerid;
    g_VoxPhoneKeepLocal[callerid] = keepLocalVoice;
    g_VoxPhoneKeepLocal[targetid] = keepLocalVoice;
    g_VoxPhoneMuted[callerid] = false;
    g_VoxPhoneMuted[targetid] = false;

    // Start recording for both players so they can speak continuously
    SvStartRecord(callerid);
    SvStartRecord(targetid);

    #if !defined VOX_DISABLE_CALLBACKS
        CallLocalFunction("OnVoxPhoneCallStarted", "ii", callerid, targetid);
    #endif
    return true;
}

stock bool:Vox_EndPhoneCall(playerid)
{
    if (!IsPlayerConnected(playerid)) return false;

    new partnerid = g_VoxPhonePartner[playerid];
    if (partnerid == INVALID_PLAYER_ID) return false;

    // Cleanup streams
    if (g_VoxPhoneStream[playerid] != SV_NULL)
    {
        SvDeleteStream(g_VoxPhoneStream[playerid]);
        g_VoxPhoneStream[playerid] = SV_NULL;
    }
    if (IsPlayerConnected(partnerid) && g_VoxPhoneStream[partnerid] != SV_NULL)
    {
        SvDeleteStream(g_VoxPhoneStream[partnerid]);
        g_VoxPhoneStream[partnerid] = SV_NULL;
    }

    // Stop recording unless transmitting on HT or talking local
    if (!g_VoxLocalTalking[playerid] && !g_VoxHTTransmitting[playerid] && !g_VoxAdminBroadcasting[playerid])
    {
        SvStopRecord(playerid);
    }
    if (IsPlayerConnected(partnerid))
    {
        if (!g_VoxLocalTalking[partnerid] && !g_VoxHTTransmitting[partnerid] && !g_VoxAdminBroadcasting[partnerid])
        {
            SvStopRecord(partnerid);
        }
    }

    g_VoxPhonePartner[playerid] = INVALID_PLAYER_ID;
    g_VoxPhoneKeepLocal[playerid] = true;
    g_VoxPhoneMuted[playerid] = false;

    if (IsPlayerConnected(partnerid))
    {
        g_VoxPhonePartner[partnerid] = INVALID_PLAYER_ID;
        g_VoxPhoneKeepLocal[partnerid] = true;
        g_VoxPhoneMuted[partnerid] = false;
    }

    #if !defined VOX_DISABLE_CALLBACKS
        CallLocalFunction("OnVoxPhoneCallEnded", "ii", playerid, partnerid);
    #endif
    return true;
}

stock bool:Vox_IsInPhoneCall(playerid)
{
    if (!IsPlayerConnected(playerid)) return false;
    return (g_VoxPhonePartner[playerid] != INVALID_PLAYER_ID);
}

stock Vox_GetPhoneCallPartner(playerid)
{
    if (!IsPlayerConnected(playerid)) return INVALID_PLAYER_ID;
    return g_VoxPhonePartner[playerid];
}

stock bool:Vox_SetPhoneCallMuted(playerid, bool:mute)
{
    if (!IsPlayerConnected(playerid) || g_VoxPhonePartner[playerid] == INVALID_PLAYER_ID)
    {
        return false;
    }

    g_VoxPhoneMuted[playerid] = mute;
    if (g_VoxPhoneStream[playerid] != SV_NULL)
    {
        if (mute)
        {
            SvDetachSpeakerFromStream(g_VoxPhoneStream[playerid], playerid);
        }
        else
        {
            SvAttachSpeakerToStream(g_VoxPhoneStream[playerid], playerid);
        }
    }
    return true;
}

stock bool:Vox_IsPhoneCallMuted(playerid)
{
    if (!IsPlayerConnected(playerid)) return false;
    return g_VoxPhoneMuted[playerid];
}

// =============================================================================
// Global Admin Voice API
// =============================================================================

stock bool:Vox_SetAdminVoiceEnabled(playerid, bool:toggle)
{
    if (!IsPlayerConnected(playerid)) return false;

    Vox_EnsureAdminStream();
    g_VoxAdminEnabled[playerid] = toggle;

    if (!toggle && g_VoxAdminBroadcasting[playerid])
    {
        Vox_SetAdminVoiceMode(playerid, false);
    }
    return true;
}

stock bool:Vox_IsAdminVoiceEnabled(playerid)
{
    if (!IsPlayerConnected(playerid)) return false;
    return g_VoxAdminEnabled[playerid];
}

stock bool:Vox_SetAdminVoiceMode(playerid, bool:broadcastActive)
{
    if (!IsPlayerConnected(playerid)) return false;
    if (broadcastActive && !g_VoxAdminEnabled[playerid]) return false;

    Vox_EnsureAdminStream();

    if (broadcastActive)
    {
        g_VoxAdminBroadcasting[playerid] = true;
        SvAttachSpeakerToStream(g_VoxAdminStream, playerid);
        SvStartRecord(playerid);

        #if !defined VOX_DISABLE_CALLBACKS
            CallLocalFunction("OnVoxAdminVoiceStart", "i", playerid);
        #endif
    }
    else
    {
        if (g_VoxAdminBroadcasting[playerid])
        {
            g_VoxAdminBroadcasting[playerid] = false;
            SvDetachSpeakerFromStream(g_VoxAdminStream, playerid);
            if (!g_VoxLocalTalking[playerid] && !g_VoxHTTransmitting[playerid] && g_VoxPhonePartner[playerid] == INVALID_PLAYER_ID)
            {
                SvStopRecord(playerid);
            }

            #if !defined VOX_DISABLE_CALLBACKS
                CallLocalFunction("OnVoxAdminVoiceStop", "i", playerid);
            #endif
        }
    }
    return true;
}

stock bool:Vox_IsAdminVoiceBroadcasting(playerid)
{
    if (!IsPlayerConnected(playerid)) return false;
    return g_VoxAdminBroadcasting[playerid];
}

stock bool:Vox_MutePlayerGlobal(playerid, bool:mute)
{
    if (!IsPlayerConnected(playerid)) return false;

    Vox_EnsureAdminStream();
    g_VoxGlobalMuted[playerid] = mute;

    if (mute)
    {
        SvDetachListenerFromStream(g_VoxAdminStream, playerid);
    }
    else
    {
        if (!SvHasListenerInStream(g_VoxAdminStream, playerid))
        {
            SvAttachListenerToStream(g_VoxAdminStream, playerid);
        }
    }
    return true;
}

stock bool:Vox_IsPlayerGlobalMuted(playerid)
{
    if (!IsPlayerConnected(playerid)) return false;
    return g_VoxGlobalMuted[playerid];
}

// =============================================================================
// Lifecycle & Player Cleanup
// =============================================================================

stock Vox_InitPlayer(playerid)
{
    g_VoxLocalDistance[playerid] = g_VoxDefaultLocalDistance;
    g_VoxLocalEnabled[playerid] = false;
    g_VoxLocalMuted[playerid] = false;
    g_VoxLocalTalking[playerid] = false;
    g_VoxLocalStream[playerid] = SV_NULL;

    g_VoxHTEnabled[playerid] = false;
    g_VoxHTPlayerFreqKey[playerid] = 0;
    g_VoxHTTransmitting[playerid] = false;

    g_VoxPhonePartner[playerid] = INVALID_PLAYER_ID;
    g_VoxPhoneStream[playerid] = SV_NULL;
    g_VoxPhoneKeepLocal[playerid] = true;
    g_VoxPhoneMuted[playerid] = false;

    g_VoxAdminEnabled[playerid] = false;
    g_VoxAdminBroadcasting[playerid] = false;
    g_VoxGlobalMuted[playerid] = false;

    // Attach to admin broadcast stream by default
    Vox_EnsureAdminStream();
    if (!SvHasListenerInStream(g_VoxAdminStream, playerid))
    {
        SvAttachListenerToStream(g_VoxAdminStream, playerid);
    }
    return 1;
}

stock Vox_CleanupPlayer(playerid)
{
    // 1. Phone cleanup
    if (g_VoxPhonePartner[playerid] != INVALID_PLAYER_ID)
    {
        Vox_EndPhoneCall(playerid);
    }
    if (g_VoxPhoneStream[playerid] != SV_NULL)
    {
        SvDeleteStream(g_VoxPhoneStream[playerid]);
        g_VoxPhoneStream[playerid] = SV_NULL;
    }

    // 2. HT cleanup
    if (g_VoxHTTransmitting[playerid])
    {
        Vox_SetHTTransmitting(playerid, false);
    }
    new freqKey = g_VoxHTPlayerFreqKey[playerid];
    if (freqKey > 0)
    {
        new slot = Vox_FindHTChannel(freqKey);
        if (slot != -1 && g_VoxHTChannels[slot][e_VoxHTStream] != SV_NULL)
        {
            if (SvHasListenerInStream(g_VoxHTChannels[slot][e_VoxHTStream], playerid))
            {
                SvDetachListenerFromStream(g_VoxHTChannels[slot][e_VoxHTStream], playerid);
                g_VoxHTChannels[slot][e_VoxHTListeners]--;
                Vox_CleanupHTChannelIfEmpty(slot);
            }
        }
    }
    g_VoxHTEnabled[playerid] = false;
    g_VoxHTPlayerFreqKey[playerid] = 0;

    // 3. Admin voice cleanup
    if (g_VoxAdminBroadcasting[playerid])
    {
        Vox_SetAdminVoiceMode(playerid, false);
    }
    if (g_VoxAdminStream != SV_NULL)
    {
        if (SvHasListenerInStream(g_VoxAdminStream, playerid))
        {
            SvDetachListenerFromStream(g_VoxAdminStream, playerid);
        }
    }
    g_VoxAdminEnabled[playerid] = false;
    g_VoxGlobalMuted[playerid] = false;

    // 4. Local proximity voice cleanup
    if (g_VoxLocalTalking[playerid])
    {
        g_VoxLocalTalking[playerid] = false;
        #if !defined VOX_DISABLE_CALLBACKS
            CallLocalFunction("OnVoxPlayerLocalTalkStop", "i", playerid);
        #endif
    }
    if (g_VoxLocalStream[playerid] != SV_NULL)
    {
        SvDeleteStream(g_VoxLocalStream[playerid]);
        g_VoxLocalStream[playerid] = SV_NULL;
    }

    g_VoxLocalEnabled[playerid] = false;
    g_VoxLocalMuted[playerid] = false;
    g_VoxLocalDistance[playerid] = g_VoxDefaultLocalDistance;

    SvRemoveAllKeys(playerid);
    SvStopRecord(playerid);
    return 1;
}

// =============================================================================
// ALS Hooking: OnPlayerConnect & OnPlayerDisconnect
// =============================================================================

#if !defined VOX_DISABLE_AUTOCLEANUP

public OnPlayerConnect(playerid)
{
    Vox_InitPlayer(playerid);

    #if defined Vox_OnPlayerConnect
        return Vox_OnPlayerConnect(playerid);
    #else
        return 1;
    #endif
}

#if defined _ALS_OnPlayerConnect
    #undef OnPlayerConnect
#else
    #define _ALS_OnPlayerConnect
#endif
#define OnPlayerConnect Vox_OnPlayerConnect
#if defined Vox_OnPlayerConnect
    forward Vox_OnPlayerConnect(playerid);
#endif


public OnPlayerDisconnect(playerid, reason)
{
    Vox_CleanupPlayer(playerid);

    #if defined Vox_OnPlayerDisconnect
        return Vox_OnPlayerDisconnect(playerid, reason);
    #else
        return 1;
    #endif
}

#if defined _ALS_OnPlayerDisconnect
    #undef OnPlayerDisconnect
#else
    #define _ALS_OnPlayerDisconnect
#endif
#define OnPlayerDisconnect Vox_OnPlayerDisconnect
#if defined Vox_OnPlayerDisconnect
    forward Vox_OnPlayerDisconnect(playerid, reason);
#endif

#endif // VOX_DISABLE_AUTOCLEANUP

// =============================================================================
// ALS Hooking: OnPlayerActivationKeyPress & OnPlayerActivationKeyRelease
// =============================================================================

#if !defined VOX_DISABLE_KEYHOOKS

public OnPlayerActivationKeyPress(SV_UINT:playerid, SV_UINT:keyid)
{
    new p = _:playerid;
    new k = _:keyid;

    if (k == g_VoxLocalKey)
    {
        if (g_VoxLocalEnabled[p] && !g_VoxLocalMuted[p])
        {
            if (g_VoxLocalStream[p] != SV_NULL)
            {
                SvAttachSpeakerToStream(g_VoxLocalStream[p], p);
                SvStartRecord(p);
                g_VoxLocalTalking[p] = true;
                #if !defined VOX_DISABLE_CALLBACKS
                    CallLocalFunction("OnVoxPlayerLocalTalkStart", "i", p);
                #endif
            }
        }
    }
    else if (k == g_VoxHTKey)
    {
        if (g_VoxHTEnabled[p])
        {
            Vox_SetHTTransmitting(p, true);
        }
    }

    #if defined Vox_OnPlayerActivationKeyPress
        return Vox_OnPlayerActivationKeyPress(playerid, keyid);
    #else
        return 1;
    #endif
}

#if defined _ALS_OnPlayerActivationKeyPress
    #undef OnPlayerActivationKeyPress
#else
    #define _ALS_OnPlayerActivationKeyPress
#endif
#define OnPlayerActivationKeyPress Vox_OnPlayerActivationKeyPress
#if defined Vox_OnPlayerActivationKeyPress
    forward SV_VOID:Vox_OnPlayerActivationKeyPress(SV_UINT:playerid, SV_UINT:keyid);
#endif


public OnPlayerActivationKeyRelease(SV_UINT:playerid, SV_UINT:keyid)
{
    new p = _:playerid;
    new k = _:keyid;

    if (k == g_VoxLocalKey)
    {
        if (g_VoxLocalTalking[p])
        {
            if (g_VoxLocalStream[p] != SV_NULL)
            {
                SvDetachSpeakerFromStream(g_VoxLocalStream[p], p);
            }
            g_VoxLocalTalking[p] = false;

            // Stop recording only if not in HT transmit, admin broadcast, or active phone call
            if (!g_VoxHTTransmitting[p] && !g_VoxAdminBroadcasting[p] && g_VoxPhonePartner[p] == INVALID_PLAYER_ID)
            {
                SvStopRecord(p);
            }

            #if !defined VOX_DISABLE_CALLBACKS
                CallLocalFunction("OnVoxPlayerLocalTalkStop", "i", p);
            #endif
        }
    }
    else if (k == g_VoxHTKey)
    {
        if (g_VoxHTTransmitting[p])
        {
            Vox_SetHTTransmitting(p, false);
        }
    }

    #if defined Vox_OnPlayerActivationKeyRelease
        return Vox_OnPlayerActivationKeyRelease(playerid, keyid);
    #else
        return 1;
    #endif
}

#if defined _ALS_OnPlayerActivationKeyRelease
    #undef OnPlayerActivationKeyRelease
#else
    #define _ALS_OnPlayerActivationKeyRelease
#endif
#define OnPlayerActivationKeyRelease Vox_OnPlayerActivationKeyRelease
#if defined Vox_OnPlayerActivationKeyRelease
    forward SV_VOID:Vox_OnPlayerActivationKeyRelease(SV_UINT:playerid, SV_UINT:keyid);
#endif

#endif // VOX_DISABLE_KEYHOOKS
