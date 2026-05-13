// sdk/schemas.hpp — CS2 schema offsets, build 14160 (2026-05-11)
// Regenerated to match offsets.hpp after build 14160 cs2-dumper pass.
#pragma once
#include <cstddef>

namespace schemas {

    // C_CSPlayerPawn (extends C_BasePlayerPawn)
    namespace pawn {
        constexpr ptrdiff_t m_iHealth           = 0x34C;   // int32 — was 0x344
        constexpr ptrdiff_t m_lifeState         = 0x354;   // uint8 — was 0x348; 0=alive
        constexpr ptrdiff_t m_iTeamNum          = 0x3EB;   // uint8 — was 0x3E3; 2=T 3=CT
        constexpr ptrdiff_t m_pGameSceneNode    = 0x330;   // CGameSceneNode* — was 0x200
        constexpr ptrdiff_t m_pCameraServices   = 0x1218;  // CPlayer_CameraServices* — was 0x14B8
        constexpr ptrdiff_t m_flFlashMaxAlpha   = 0x13FC;  // float — unchanged
        constexpr ptrdiff_t m_flFlashDuration   = 0x1400;  // float — unchanged
        // m_pClippingWeapon REMOVED build 14160 — use m_pWeaponServices path
        constexpr ptrdiff_t m_pWeaponServices   = 0x11E0;  // CPlayer_WeaponServices*
        constexpr ptrdiff_t m_ArmorValue        = 0x1C7C;  // int32 — 0-100
        constexpr ptrdiff_t m_pMovementServices = 0x1220;  // CPlayer_MovementServices*
    }

    // CCSPlayerController
    namespace controller {
        constexpr ptrdiff_t m_hPlayerPawn       = 0x90C;   // CHandle uint32 — was 0x608
        constexpr ptrdiff_t m_iszPlayerName     = 0x6F4;   // inline char[128] — was 0x640
        constexpr ptrdiff_t m_bPawnHasDefuser   = 0x920;   // bool
        constexpr ptrdiff_t m_bPawnHasHelmet    = 0x921;   // bool
    }

    // CGameSceneNode
    namespace node {
        constexpr ptrdiff_t m_vRenderOrigin     = 0x128;   // Vector — was 0x274
        constexpr ptrdiff_t m_bDormant          = 0x103;   // bool   — was 0x377
    }

    // CPlayer_CameraServices
    namespace camera {
        constexpr ptrdiff_t m_flFOVDesired      = 0x2A8;   // float — unchanged
    }

    // CPlayer_WeaponServices
    namespace weapon_svc {
        constexpr ptrdiff_t m_hActiveWeapon     = 0x60;    // CHandle uint32
    }

    // CBasePlayerWeapon
    namespace weapon {
        constexpr ptrdiff_t m_iClip1               = 0x16D8; // int32 — clip ammo
        constexpr ptrdiff_t m_pReserveAmmo          = 0x16E0; // int32[2] — [0] = primary reserve
        // Combined: m_AttributeManager(0x1180) + m_Item(0x50) + m_iItemDefinitionIndex(0x1BA)
        constexpr ptrdiff_t m_iItemDefinitionIndex  = 0x138A; // uint16
    }

    // CPlayer_MovementServices internal layout (pointer from pawn::m_pMovementServices)
    namespace movement_svc {
        constexpr ptrdiff_t m_bOnGround         = 0x11A4;  // bool — unchanged
    }
}
