#include "hooks.h"
#include <iostream>
#include "../utils/logging/log.h"

#include "../../../external/kiero/minhook/include/MinHook.h"

#include "../../templeware/utils/memory/Interface/Interface.h"
#include "../utils/memory/patternscan/patternscan.h"
#include "../utils/memory/gaa/gaa.h"

#include "../players/hook/playerHook.h"
#include "../features/visuals/visuals.h"
#include "../features/chams/chams.h"

#include "../../cs2/datatypes/cutlbuffer/cutlbuffer.h"
#include "../../cs2/datatypes/keyvalues/keyvalues.h"
#include "../../cs2/entity/C_Material/C_Material.h"

#include "../config/config.h"
#include "../interfaces/interfaces.h"
#include "../features/aim/aim.h"

void __fastcall H::hkFrameStageNotify(void* a1, int stage)
{
	FrameStageNotify.GetOriginal()(a1, stage);

	// frame_render_stage | 9
	if (stage == 9 && oGetLocalPlayer && oGetLocalPlayer(0)) {
		Esp::cache();

		Aimbot();
	}
}

void* __fastcall H::hkLevelInit(void* pClientModeShared, const char* szNewMap) {
	static void* g_pPVS = (void*)M::getAbsoluteAddress(M::patternScan("engine2", "48 8D 0D ? ? ? ? 33 D2 FF 50"), 0x3);

	M::vfunc<void*, 6U, void>(g_pPVS, false);

	return LevelInit.GetOriginal()(pClientModeShared, szNewMap);
}

void H::Hooks::init() {

	uintptr_t scanWeaponData = M::patternScan("client", ("48 8B 81 ? ? ? ? 85 D2 78 ? 48 83 FA ? 73 ? F3 0F 10 84 90 ? ? ? ? C3 F3 0F 10 80 ? ? ? ? C3 CC CC CC CC"));
	if (scanWeaponData) oGetWeaponData = *reinterpret_cast<int*>(scanWeaponData + 0x3);
	Logger::Logf(LogType::Info, "scanWeaponData: %s (%p)", scanWeaponData ? "OK" : "FAILED", (void*)scanWeaponData);

	uintptr_t scanBaseEntity = M::patternScan("client", ("4C 8D 49 10 81 FA FE 7F 00 00 77 ? 8B CA C1 F9 09 83 F9 3F 77"));
	if (scanBaseEntity) ogGetBaseEntity = reinterpret_cast<decltype(ogGetBaseEntity)>(scanBaseEntity);
	Logger::Logf(LogType::Info, "scanBaseEntity: %s (%p)", scanBaseEntity ? "OK" : "FAILED", (void*)scanBaseEntity);

	uintptr_t scanLocalPlayer = M::patternScan("client", ("48 83 EC 28 83 F9 FF 75 17 48 8B 0D ? ? ? ? 48 8D 54 24 30 48 8B 01 FF 90 ? ? ? ? 8B 08 48 63 C1 48 8D 0D ? ? ? ? 48 8B 04 C1"));
	if (scanLocalPlayer) oGetLocalPlayer = reinterpret_cast<decltype(oGetLocalPlayer)>(scanLocalPlayer);
	Logger::Logf(LogType::Info, "scanLocalPlayer: %s (%p)", scanLocalPlayer ? "OK" : "FAILED", (void*)scanLocalPlayer);

	// scenesystem hooks disabled until correct functions identified for current build
	void* pUpdateWallsObject = nullptr;
	Logger::Logf(LogType::Info, "pUpdateWallsObject: DISABLED (scenesystem sig needs reverification)");

	void* pFrameStageNotify = (void*)M::patternScan("client", ("48 89 5C 24 08 57 48 83 EC 20 8B 05 ? ? ? ?"));
	if (pFrameStageNotify) FrameStageNotify.Add(pFrameStageNotify, &hkFrameStageNotify);
	Logger::Logf(LogType::Info, "pFrameStageNotify: %s (%p)", pFrameStageNotify ? "OK" : "FAILED", pFrameStageNotify);

	void* pDrawArray = nullptr;
	Logger::Logf(LogType::Info, "pDrawArray: DISABLED (scenesystem sig needs reverification)");

	uintptr_t scanRenderFov = M::patternScan("client", "E8 ? ? ? ? F3 0F 11 45 ? 48 8D 55");
	if (scanRenderFov) GetRenderFov.Add((void*)M::getAbsoluteAddress(scanRenderFov, 1), &hkGetRenderFov);
	Logger::Logf(LogType::Info, "scanRenderFov: %s (%p)", scanRenderFov ? "OK" : "FAILED", (void*)scanRenderFov);

	uintptr_t scanLevelInit = M::patternScan("client", "E8 ? ? ? ? C6 83 ? ? ? ? ? C6 83");
	if (scanLevelInit) LevelInit.Add((void*)M::getAbsoluteAddress(scanLevelInit, 1), &hkLevelInit);
	Logger::Logf(LogType::Info, "scanLevelInit: %s (%p)", scanLevelInit ? "OK" : "FAILED", (void*)scanLevelInit);

	void* pRenderFlash = (void*)M::patternScan("client", ("85 D2 0F 88 ? ? ? ? 48 89 5C 24 10 48 89 74 24 18"));
	if (pRenderFlash) RenderFlashBangOverlay.Add(pRenderFlash, &hkRenderFlashbangOverlay);
	Logger::Logf(LogType::Info, "pRenderFlash: %s (%p)", pRenderFlash ? "OK" : "FAILED", pRenderFlash);

	MH_EnableHook(MH_ALL_HOOKS);
	Logger::Log("MH_EnableHook called", LogType::Info);
}
