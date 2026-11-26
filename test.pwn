#include <open.mp>
#include <streamer>
#include "include/omp_3dtext.inc"

static g_CbCreatedCount = 0;
static g_CbDestroyedCount = 0;
static g_CbUpdatedCount = 0;
static g_CbAttachedCount = 0;

public OnO3DLabelCreated(STREAMER_TAG_3D_TEXT_LABEL:labelid, forplayerid)
{
	#pragma unused labelid, forplayerid
	g_CbCreatedCount++;
	return 1;
}

public OnO3DLabelDestroyed(STREAMER_TAG_3D_TEXT_LABEL:labelid)
{
	#pragma unused labelid
	g_CbDestroyedCount++;
	return 1;
}

public OnO3DLabelTextUpdated(STREAMER_TAG_3D_TEXT_LABEL:labelid, color, const text[])
{
	#pragma unused labelid, color, text
	g_CbUpdatedCount++;
	return 1;
}

public OnO3DLabelAttached(STREAMER_TAG_3D_TEXT_LABEL:labelid, attachedtype, targetid)
{
	#pragma unused labelid, attachedtype, targetid
	g_CbAttachedCount++;
	return 1;
}

public OnPlayerDisconnect(playerid, reason)
{
	#pragma unused playerid, reason
	return 1;
}

stock AssertTrue(condition, const message[])
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
	print(" Running omp-3dtext Extended Test Suite ");
	print("========================================\n");

	// 1. Invalid ID Handling
	new STREAMER_TAG_3D_TEXT_LABEL:invalidLabel = STREAMER_TAG_3D_TEXT_LABEL:0;
	AssertTrue(!O3D_IsValid(invalidLabel), "invalid label not valid");
	AssertTrue(!O3D_Destroy(invalidLabel), "invalid label destroy fail");
	AssertTrue(O3D_GetWorld(invalidLabel) == -1, "invalid world == -1");
	AssertTrue(O3D_GetInterior(invalidLabel) == -1, "invalid interior == -1");
	AssertTrue(O3D_GetArea(invalidLabel) == STREAMER_TAG_AREA:-1, "invalid area == -1");
	AssertTrue(O3D_GetTestLOS(invalidLabel) == 0, "invalid los == 0");
	AssertTrue(O3D_GetPriority(invalidLabel) == 0, "invalid priority == 0");
	AssertTrue(O3D_GetExtraID(invalidLabel) == 0, "invalid extra id == 0");
	AssertTrue(O3D_GetAttachedPlayer(invalidLabel) == INVALID_PLAYER_ID, "invalid attached player");
	AssertTrue(O3D_GetAttachedVehicle(invalidLabel) == INVALID_VEHICLE_ID, "invalid attached vehicle");

	new Float:tx = 0.0;
	new Float:ty = 0.0;
	new Float:tz = 0.0;
	new Float:dist = 0.0;
	AssertTrue(!O3D_GetPos(invalidLabel, tx, ty, tz), "invalid get pos");
	AssertTrue(!O3D_SetPos(invalidLabel, 10.0, 20.0, 30.0), "invalid set pos");
	AssertTrue(O3D_GetDrawDistance(invalidLabel) == 0.0, "invalid draw dist");
	AssertTrue(!O3D_SetDrawDistance(invalidLabel, 50.0), "invalid set draw dist");
	AssertTrue(O3D_GetStreamDistance(invalidLabel) == 0.0, "invalid stream dist");
	AssertTrue(!O3D_SetStreamDistance(invalidLabel, 150.0), "invalid set stream dist");
	AssertTrue(!O3D_SetWorld(invalidLabel, 1), "invalid set world");
	AssertTrue(!O3D_SetInterior(invalidLabel, 2), "invalid set interior");
	AssertTrue(!O3D_SetArea(invalidLabel, STREAMER_TAG_AREA:10), "invalid set area");
	AssertTrue(!O3D_SetTestLOS(invalidLabel, 1), "invalid set los");
	AssertTrue(!O3D_SetPriority(invalidLabel, 5), "invalid set priority");
	AssertTrue(!O3D_SetExtraID(invalidLabel, 99), "invalid set extra id");
	AssertTrue(!O3D_GetDistanceFromPoint(invalidLabel, 0.0, 0.0, 0.0, dist), "invalid distance from point");

	new textBuf[MAX_O3D_TEXT_LENGTH];
	AssertTrue(!O3D_GetText(invalidLabel, textBuf), "invalid get text");
	AssertTrue(!O3D_UpdateText(invalidLabel, 0xFFFFFFFF, "test"), "invalid update text");
	AssertTrue(!O3D_SetText(invalidLabel, "test"), "invalid set text");
	AssertTrue(!O3D_SetColor(invalidLabel, 0xFF0000FF), "invalid set color");
	AssertTrue(O3D_GetColor(invalidLabel) == 0, "invalid get color");

	AssertTrue(!O3D_AttachToPlayer(invalidLabel, 0, 0.0, 0.0, 1.0), "invalid attach player");
	AssertTrue(!O3D_AttachToVehicle(invalidLabel, 1, 0.0, 0.0, 1.5), "invalid attach vehicle");
	AssertTrue(!O3D_Detach(invalidLabel), "invalid detach");
	AssertTrue(!O3D_DetachEx(invalidLabel, true), "invalid detach ex");
	AssertTrue(!O3D_GetAttachOffsets(invalidLabel, tx, ty, tz), "invalid attach offsets");
	AssertTrue(!O3D_ToggleForPlayer(0, invalidLabel, 1), "invalid toggle player");
	AssertTrue(!O3D_ShowForPlayer(0, invalidLabel), "invalid show player");
	AssertTrue(!O3D_HideForPlayer(0, invalidLabel), "invalid hide player");
	AssertTrue(!O3D_IsToggleForPlayer(0, invalidLabel), "invalid is toggle player");

	// 2. Standard Label Creation & Attribute Manipulation
	new createdCountBefore = g_CbCreatedCount;
	new STREAMER_TAG_3D_TEXT_LABEL:label1 = O3D_Create("Initial Text", 0xFFFFFFFF, 100.0, 200.0, 10.0, 25.0);
	AssertTrue(O3D_IsValid(label1), "label1 valid");
	AssertTrue(g_CbCreatedCount == createdCountBefore + 1, "callback created incremented");

	AssertTrue(bool:O3D_SetPos(label1, 150.0, 250.0, 15.0), "label1 set pos");
	AssertTrue(bool:O3D_GetPos(label1, tx, ty, tz), "label1 get pos");
	AssertTrue(tx == 150.0 && ty == 250.0 && tz == 15.0, "label1 pos coords match");

	// Test O3D_GetDistanceFromPoint
	AssertTrue(bool:O3D_GetDistanceFromPoint(label1, 150.0, 250.0, 10.0, dist), "calc distance");
	AssertTrue(dist == 5.0, "calc distance value match");

	AssertTrue(bool:O3D_SetDrawDistance(label1, 40.0), "label1 set draw dist");
	AssertTrue(O3D_GetDrawDistance(label1) == 40.0, "label1 draw dist match");

	AssertTrue(bool:O3D_SetStreamDistance(label1, 180.0), "label1 set stream dist");
	AssertTrue(O3D_GetStreamDistance(label1) == 180.0, "label1 stream dist match");

	AssertTrue(bool:O3D_SetWorld(label1, 3), "label1 set world");
	AssertTrue(O3D_GetWorld(label1) == 3, "label1 world match");

	AssertTrue(bool:O3D_SetInterior(label1, 5), "label1 set interior");
	AssertTrue(O3D_GetInterior(label1) == 5, "label1 interior match");

	AssertTrue(bool:O3D_SetArea(label1, STREAMER_TAG_AREA:12), "label1 set area");
	AssertTrue(O3D_GetArea(label1) == STREAMER_TAG_AREA:12, "label1 area match");

	AssertTrue(bool:O3D_SetTestLOS(label1, 0), "label1 set los");
	AssertTrue(O3D_GetTestLOS(label1) == 0, "label1 los match");

	AssertTrue(bool:O3D_SetPriority(label1, 2), "label1 set priority");
	AssertTrue(O3D_GetPriority(label1) == 2, "label1 priority match");

	AssertTrue(bool:O3D_SetExtraID(label1, 777), "label1 set extra id");
	AssertTrue(O3D_GetExtraID(label1) == 777, "label1 extra id match");

	// 3. Text and Color Operations with Callbacks
	new updatedCountBefore = g_CbUpdatedCount;
	AssertTrue(bool:O3D_GetText(label1, textBuf), "label1 get text");
	AssertTrue(!strcmp(textBuf, "Initial Text"), "label1 text match");

	AssertTrue(bool:O3D_SetText(label1, "Updated Text Only"), "label1 set text");
	AssertTrue(g_CbUpdatedCount == updatedCountBefore + 1, "callback text updated");
	AssertTrue(bool:O3D_GetText(label1, textBuf), "label1 get text updated");
	AssertTrue(!strcmp(textBuf, "Updated Text Only"), "label1 updated text match");

	AssertTrue(bool:O3D_SetColor(label1, 0xFF0000FF), "label1 set color");
	AssertTrue(g_CbUpdatedCount == updatedCountBefore + 2, "callback color updated");
	AssertTrue(O3D_GetColor(label1) == 0xFF0000FF, "label1 color match");

	AssertTrue(bool:O3D_UpdateText(label1, 0x00FF00FF, "New Text Green"), "label1 update text");
	AssertTrue(g_CbUpdatedCount == updatedCountBefore + 3, "callback full updated");
	AssertTrue(O3D_GetColor(label1) == 0x00FF00FF, "label1 green color match");
	AssertTrue(bool:O3D_GetText(label1, textBuf), "label1 get green text");
	AssertTrue(!strcmp(textBuf, "New Text Green"), "label1 green text match");

	// 4. Attachments & Detach & DetachEx with Callbacks
	new attachedCountBefore = g_CbAttachedCount;
	AssertTrue(bool:O3D_AttachToPlayer(label1, 0, 0.1, 0.2, 0.8), "label1 attach player");
	AssertTrue(g_CbAttachedCount == attachedCountBefore + 1, "callback attached player");
	AssertTrue(O3D_GetAttachedPlayer(label1) == 0, "label1 attached player match");
	AssertTrue(O3D_GetAttachedVehicle(label1) == INVALID_VEHICLE_ID, "label1 attached vehicle invalid");
	AssertTrue(bool:O3D_GetAttachOffsets(label1, tx, ty, tz), "label1 get offsets");
	AssertTrue(tx == 0.1 && ty == 0.2 && tz == 0.8, "label1 offsets match");

	AssertTrue(bool:O3D_AttachToVehicle(label1, 42, 0.0, 0.0, 1.2), "label1 attach vehicle");
	AssertTrue(g_CbAttachedCount == attachedCountBefore + 2, "callback attached vehicle");
	AssertTrue(O3D_GetAttachedVehicle(label1) == 42, "label1 attached vehicle 42");
	AssertTrue(O3D_GetAttachedPlayer(label1) == INVALID_PLAYER_ID, "label1 attached player invalid");
	AssertTrue(bool:O3D_GetAttachOffsets(label1, tx, ty, tz), "label1 vehicle offsets");
	AssertTrue(tx == 0.0 && ty == 0.0 && tz == 1.2, "label1 vehicle offsets match");

	AssertTrue(bool:O3D_DetachEx(label1, false), "label1 detach ex");
	AssertTrue(g_CbAttachedCount == attachedCountBefore + 3, "callback detached");
	AssertTrue(O3D_GetAttachedPlayer(label1) == INVALID_PLAYER_ID, "label1 detached player invalid");
	AssertTrue(O3D_GetAttachedVehicle(label1) == INVALID_VEHICLE_ID, "label1 detached vehicle invalid");

	// 5. Extended Creation (O3D_CreateEx)
	new worlds[] = { 1, 2 };
	new interiors[] = { 0 };
	new players[] = { -1 };
	new STREAMER_TAG_AREA:areas[] = { STREAMER_TAG_AREA:-1 };
	new STREAMER_TAG_3D_TEXT_LABEL:labelEx = O3D_CreateEx(
		"Ex Label",
		0x0000FFFF,
		0.0,
		0.0,
		5.0,
		15.0,
		INVALID_PLAYER_ID,
		INVALID_VEHICLE_ID,
		1,
		100.0,
		worlds,
		interiors,
		players,
		areas,
		1
	);
	AssertTrue(O3D_IsValid(labelEx), "labelEx valid");

	// 6. Player-Scoped Creation & Automatic Disconnect Cleanup
	new STREAMER_TAG_3D_TEXT_LABEL:labelPlayer = O3D_CreateForPlayer(
		5,
		"Player 5 Scoped Label",
		0xFFFF00FF,
		10.0,
		20.0,
		30.0,
		25.0
	);
	AssertTrue(O3D_IsValid(labelPlayer), "labelPlayer valid");

	// Attach label1 to player 5
	O3D_AttachToPlayer(label1, 5, 0.0, 0.0, 1.0);
	AssertTrue(O3D_GetAttachedPlayer(label1) == 5, "label1 attached to player 5");

	// Simulate player 5 disconnect cleanup
	O3D_CleanupPlayer(5);
	AssertTrue(!O3D_IsValid(labelPlayer), "labelPlayer destroyed after cleanup");
	AssertTrue(O3D_GetAttachedPlayer(label1) == INVALID_PLAYER_ID, "label1 detached after cleanup");

	// 7. Formatted Text Creation & Updating (O3D_Createf, O3D_CreateForPlayerf, O3D_UpdateTextf)
	new STREAMER_TAG_3D_TEXT_LABEL:labelFmtZero = O3D_Createf(20.0, 0xFFFFFFFF, 0.0, 0.0, 0.0, "Plain String Without Varargs");
	AssertTrue(O3D_IsValid(labelFmtZero), "labelFmtZero valid");
	AssertTrue(bool:O3D_GetText(labelFmtZero, textBuf), "labelFmtZero get text");
	AssertTrue(!strcmp(textBuf, "Plain String Without Varargs"), "labelFmtZero match");
	O3D_Destroy(labelFmtZero);

	new STREAMER_TAG_3D_TEXT_LABEL:labelFmt = O3D_Createf(
		30.0,
		0xFFFFFFFF,
		1.0,
		2.0,
		3.0,
		"Player: %s [ID: %d] Score: %d",
		"Raihan",
		1,
		500
	);
	AssertTrue(O3D_IsValid(labelFmt), "labelFmt valid");
	AssertTrue(bool:O3D_GetText(labelFmt, textBuf), "labelFmt get text");
	AssertTrue(!strcmp(textBuf, "Player: Raihan [ID: 1] Score: 500"), "labelFmt formatted text match");

	new STREAMER_TAG_3D_TEXT_LABEL:labelPlayerFmt = O3D_CreateForPlayerf(
		0,
		30.0,
		0x00FFFFFF,
		4.0,
		5.0,
		6.0,
		"Target Health: %.1f%%",
		98.5
	);
	AssertTrue(O3D_IsValid(labelPlayerFmt), "labelPlayerFmt valid");

	AssertTrue(bool:O3D_UpdateTextf(labelFmt, 0xFF00FFFF, "Score updated: %d", 1200), "labelFmt update textf");
	AssertTrue(bool:O3D_GetText(labelFmt, textBuf), "labelFmt get updated text");
	AssertTrue(!strcmp(textBuf, "Score updated: 1200"), "labelFmt updated text match");

	// 8. Streaming Toggles & Show/Hide Helpers
	AssertTrue(bool:O3D_HideForPlayer(0, label1), "label1 hide player");
	AssertTrue(!O3D_IsToggleForPlayer(0, label1), "label1 toggle off check");
	AssertTrue(bool:O3D_ShowForPlayer(0, label1), "label1 show player");
	AssertTrue(bool:O3D_IsToggleForPlayer(0, label1), "label1 toggle on check");
	AssertTrue(bool:O3D_UpdateForPlayer(0, 0.0, 0.0, 0.0), "label1 update player");

	// 9. Destruction & Cleanup
	new destroyedCountBefore = g_CbDestroyedCount;
	AssertTrue(bool:O3D_Destroy(label1), "label1 destroy");
	AssertTrue(!O3D_IsValid(label1), "label1 destroyed not valid");
	AssertTrue(g_CbDestroyedCount == destroyedCountBefore + 1, "callback destroyed incremented");

	AssertTrue(bool:O3D_Destroy(labelEx), "labelEx destroy");
	AssertTrue(!O3D_IsValid(labelEx), "labelEx destroyed not valid");
	AssertTrue(bool:O3D_Destroy(labelFmt), "labelFmt destroy");
	AssertTrue(!O3D_IsValid(labelFmt), "labelFmt destroyed not valid");
	AssertTrue(bool:O3D_Destroy(labelPlayerFmt), "labelPlayerFmt destroy");
	AssertTrue(!O3D_IsValid(labelPlayerFmt), "labelPlayerFmt destroyed not valid");

	new totalCount = O3D_Count();
	printf("Remaining dynamic labels count: %d", totalCount);
	AssertTrue(bool:O3D_DestroyAll(1), "destroy all items");

	print("All extended 3DText tests passed successfully!\n");
	return 1;
}
