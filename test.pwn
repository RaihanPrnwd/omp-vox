#include <open.mp>
#include <sampvoice>
#include "include/omp_vox.inc"

// Custom callback counters
static g_CbLocalTalkStart = 0;
static g_CbLocalTalkStop = 0;
static g_CbDistChange = 0;
static g_CbHTFreqChange = 0;
static g_CbHTTransStart = 0;
static g_CbHTTransStop = 0;
static g_CbPhoneStart = 0;
static g_CbPhoneEnd = 0;
static g_CbAdminStart = 0;
static g_CbAdminStop = 0;

public OnVoxPlayerLocalTalkStart(playerid)
{
    #pragma unused playerid
    g_CbLocalTalkStart++;
    return 1;
}

public OnVoxPlayerLocalTalkStop(playerid)
{
    #pragma unused playerid
    g_CbLocalTalkStop++;
    return 1;
}

public OnVoxLocalDistanceChanged(playerid, Float:oldDistance, Float:newDistance)
{
    #pragma unused playerid, oldDistance, newDistance
    g_CbDistChange++;
    return 1;
}

public OnVoxHTFrequencyChanged(playerid, Float:oldFreq, Float:newFreq)
{
    #pragma unused playerid, oldFreq, newFreq
    g_CbHTFreqChange++;
    return 1;
}

public OnVoxHTTransmitStart(playerid, Float:frequency)
{
    #pragma unused playerid, frequency
    g_CbHTTransStart++;
    return 1;
}

public OnVoxHTTransmitStop(playerid, Float:frequency)
{
    #pragma unused playerid, frequency
    g_CbHTTransStop++;
    return 1;
}

public OnVoxPhoneCallStarted(callerid, targetid)
{
    #pragma unused callerid, targetid
    g_CbPhoneStart++;
    return 1;
}

public OnVoxPhoneCallEnded(playerid, partnerid)
{
    #pragma unused playerid, partnerid
    g_CbPhoneEnd++;
    return 1;
}

public OnVoxAdminVoiceStart(playerid)
{
    #pragma unused playerid
    g_CbAdminStart++;
    return 1;
}

public OnVoxAdminVoiceStop(playerid)
{
    #pragma unused playerid
    g_CbAdminStop++;
    return 1;
}

stock AssertTrue(bool:condition, const message[])
{
    if (!condition)
    {
        printf("[ASSERTION FAILED] %s", message);
        return 0;
    }
    return 1;
}

main()
{
    print("\n========================================");
    print(" Running omp-vox Test Suite ");
    print("========================================\n");

    new player1 = 1;
    new player2 = 2;

    // Simulate connection
    Vox_InitPlayer(player1);
    Vox_InitPlayer(player2);

    // 1. Proximity Local Voice Defaults & Configuration
    AssertTrue(Vox_GetDefaultLocalDistance() == 15.0, "default local dist is 15.0");
    AssertTrue(Vox_SetDefaultLocalDistance(20.0), "set default local dist to 20.0");
    AssertTrue(Vox_GetDefaultLocalDistance() == 20.0, "default local dist is 20.0 now");

    // Player local voice toggling & distance
    AssertTrue(!Vox_IsLocalVoiceEnabled(player1), "player1 local voice initially disabled");
    AssertTrue(Vox_SetLocalVoiceEnabled(player1, true), "player1 local voice enable");
    AssertTrue(Vox_IsLocalVoiceEnabled(player1), "player1 local voice is enabled");

    new distCbBefore = g_CbDistChange;
    AssertTrue(Vox_SetLocalVoiceDistance(player1, 35.0), "set player1 local voice dist 35.0");
    AssertTrue(Vox_GetLocalVoiceDistance(player1) == 35.0, "player1 local dist matches 35.0");
    AssertTrue(g_CbDistChange == distCbBefore + 1, "callback dist change triggered");

    // Local mute
    AssertTrue(!Vox_IsPlayerLocalMuted(player1), "player1 not local muted initially");
    AssertTrue(Vox_MutePlayerLocal(player1, true), "mute player1 local");
    AssertTrue(Vox_IsPlayerLocalMuted(player1), "player1 is local muted");
    AssertTrue(Vox_MutePlayerLocal(player1, false), "unmute player1 local");
    AssertTrue(!Vox_IsPlayerLocalMuted(player1), "player1 is unmuted local");

    // Key configuration
    AssertTrue(Vox_GetLocalKey() == VOX_DEFAULT_LOCAL_KEY, "local key default match");
    AssertTrue(Vox_GetHTKey() == VOX_DEFAULT_HT_KEY, "ht key default match");
    Vox_SetLocalKey(0x43); // 'C'
    AssertTrue(Vox_GetLocalKey() == 0x43, "local key updated");
    Vox_SetLocalKey(VOX_DEFAULT_LOCAL_KEY);

    // 2. Handy Talky (Radio) Features
    AssertTrue(!Vox_IsHTEnabled(player1), "player1 HT initially disabled");
    AssertTrue(Vox_SetHTEnabled(player1, true), "enable player1 HT");
    AssertTrue(Vox_IsHTEnabled(player1), "player1 HT is enabled");

    new freqCbBefore = g_CbHTFreqChange;
    AssertTrue(Vox_SetHTFrequency(player1, 104.5), "set player1 HT freq 104.5");
    AssertTrue(Vox_GetHTFrequency(player1) == 104.5, "player1 HT freq is 104.5");
    AssertTrue(g_CbHTFreqChange == freqCbBefore + 1, "HT freq change callback triggered");
    AssertTrue(Vox_GetHTFrequencyListeners(104.5) == 1, "1 listener on 104.5");

    // Player 2 joins same frequency
    AssertTrue(Vox_SetHTEnabled(player2, true), "enable player2 HT");
    AssertTrue(Vox_SetHTFrequency(player2, 104.5), "set player2 HT freq 104.5");
    AssertTrue(Vox_GetHTFrequencyListeners(104.5) == 2, "2 listeners on 104.5");

    // Radio lock
    AssertTrue(!Vox_IsHTFrequencyLocked(104.5), "104.5 not locked initially");
    AssertTrue(Vox_LockHTFrequency(104.5, true), "lock 104.5");
    AssertTrue(Vox_IsHTFrequencyLocked(104.5), "104.5 is locked");

    // Radio transmission
    new transCbBefore = g_CbHTTransStart;
    AssertTrue(Vox_SetHTTransmitting(player1, true), "player1 transmit HT");
    AssertTrue(Vox_IsHTTransmitting(player1), "player1 transmitting HT check");
    AssertTrue(g_CbHTTransStart == transCbBefore + 1, "HT transmit start callback triggered");

    new transStopCbBefore = g_CbHTTransStop;
    AssertTrue(Vox_SetHTTransmitting(player1, false), "player1 stop transmit HT");
    AssertTrue(!Vox_IsHTTransmitting(player1), "player1 not transmitting check");
    AssertTrue(g_CbHTTransStop == transStopCbBefore + 1, "HT transmit stop callback triggered");

    // Change frequency for player2 to test channel cleanup
    Vox_SetHTFrequency(player2, 108.0);
    AssertTrue(Vox_GetHTFrequencyListeners(104.5) == 1, "1 listener left on 104.5");
    AssertTrue(Vox_GetHTFrequencyListeners(108.0) == 1, "1 listener on 108.0");

    // 3. Phone Call Features
    new phoneStartBefore = g_CbPhoneStart;
    AssertTrue(Vox_StartPhoneCall(player1, player2, true), "start phone call p1 to p2");
    AssertTrue(Vox_IsInPhoneCall(player1), "p1 in phone call");
    AssertTrue(Vox_IsInPhoneCall(player2), "p2 in phone call");
    AssertTrue(Vox_GetPhoneCallPartner(player1) == player2, "p1 partner is p2");
    AssertTrue(Vox_GetPhoneCallPartner(player2) == player1, "p2 partner is p1");
    AssertTrue(g_CbPhoneStart == phoneStartBefore + 1, "phone call start callback triggered");

    // Phone mute
    AssertTrue(!Vox_IsPhoneCallMuted(player1), "p1 phone not muted");
    AssertTrue(Vox_SetPhoneCallMuted(player1, true), "mute p1 phone");
    AssertTrue(Vox_IsPhoneCallMuted(player1), "p1 phone is muted");
    AssertTrue(Vox_SetPhoneCallMuted(player1, false), "unmute p1 phone");

    new phoneEndBefore = g_CbPhoneEnd;
    AssertTrue(Vox_EndPhoneCall(player1), "end phone call");
    AssertTrue(!Vox_IsInPhoneCall(player1), "p1 not in phone call");
    AssertTrue(!Vox_IsInPhoneCall(player2), "p2 not in phone call");
    AssertTrue(Vox_GetPhoneCallPartner(player1) == INVALID_PLAYER_ID, "p1 partner is invalid");
    AssertTrue(g_CbPhoneEnd == phoneEndBefore + 1, "phone call end callback triggered");

    // 4. Admin Global Broadcast Voice
    AssertTrue(!Vox_IsAdminVoiceEnabled(player1), "p1 admin voice disabled initially");
    AssertTrue(Vox_SetAdminVoiceEnabled(player1, true), "enable p1 admin voice");
    AssertTrue(Vox_IsAdminVoiceEnabled(player1), "p1 admin voice is enabled");

    new adminStartBefore = g_CbAdminStart;
    AssertTrue(Vox_SetAdminVoiceMode(player1, true), "start admin broadcasting");
    AssertTrue(Vox_IsAdminVoiceBroadcasting(player1), "p1 is broadcasting");
    AssertTrue(g_CbAdminStart == adminStartBefore + 1, "admin start callback triggered");

    new adminStopBefore = g_CbAdminStop;
    AssertTrue(Vox_SetAdminVoiceMode(player1, false), "stop admin broadcasting");
    AssertTrue(!Vox_IsAdminVoiceBroadcasting(player1), "p1 stopped broadcasting");
    AssertTrue(g_CbAdminStop == adminStopBefore + 1, "admin stop callback triggered");

    // Global mute
    AssertTrue(!Vox_IsPlayerGlobalMuted(player2), "p2 not global muted");
    AssertTrue(Vox_MutePlayerGlobal(player2, true), "mute p2 global");
    AssertTrue(Vox_IsPlayerGlobalMuted(player2), "p2 is global muted");
    AssertTrue(Vox_MutePlayerGlobal(player2, false), "unmute p2 global");
    AssertTrue(!Vox_IsPlayerGlobalMuted(player2), "p2 is unmuted global");

    // 5. Cleanup on Disconnect
    Vox_CleanupPlayer(player1);
    Vox_CleanupPlayer(player2);

    AssertTrue(!Vox_IsLocalVoiceEnabled(player1), "p1 local voice cleaned up");
    AssertTrue(!Vox_IsHTEnabled(player1), "p1 HT cleaned up");
    AssertTrue(!Vox_IsInPhoneCall(player1), "p1 phone cleaned up");
    AssertTrue(!Vox_IsAdminVoiceEnabled(player1), "p1 admin voice cleaned up");

    print("\nAll omp-vox tests passed successfully!\n");
    return 1;
}
