// sdk/schemas.hpp — CS2 schema offsets, build 14160 (2026-05-07)
// Update alongside offsets.hpp when CS2 patches.
#pragma once
#include <cstddef>

namespace schemas {

    // C_CSPlayerPawn
    namespace pawn {
        constexpr ptrdiff_t m_iHealth           = 0x344;
        constexpr ptrdiff_t m_lifeState         = 0x348;
        constexpr ptrdiff_t m_iTeamNum          = 0x3E3;
        constexpr ptrdiff_t m_pGameSceneNode    = 0x200;
        constexpr ptrdiff_t m_pCameraServices   = 0x14B8;
        constexpr ptrdiff_t m_flFlashMaxAlpha   = 0x13FC;
        constexpr ptrdiff_t m_flFlashDuration   = 0x1400;
        constexpr ptrdiff_t m_pClippingWeapon   = 0xBB8;   // was 0x9C8 (stale) — verify if weapon ammo still blank
    }

    // CCSPlayerController
    namespace controller {
        constexpr ptrdiff_t m_hPlayerPawn       = 0x608;
        constexpr ptrdiff_t m_iszPlayerName     = 0x640;   // inline char[128], NOT char*
    }

    // CGameSceneNode
    namespace node {
        constexpr ptrdiff_t m_vRenderOrigin     = 0x274;
        constexpr ptrdiff_t m_bDormant          = 0x377;
    }

    // CPlayer_CameraServices
    namespace camera {
        constexpr ptrdiff_t m_flFOVDesired      = 0x2A8;
    }

    // CBasePlayerWeapon
    namespace weapon {
        constexpr ptrdiff_t m_iClipAmmo                 = 0x330;
        constexpr ptrdiff_t m_iPrimaryReserveAmmoCount  = 0x33C;
    }
}
