#pragma once
#include "../../../templeware/utils/memory/memorycommon.h"
#include "../../../templeware/utils/math/vector/vector.h"
#include "../../../templeware/utils/schema/schema.h"
#include "../C_CSWeaponBase/C_CSWeaponBase.h"
#include "../C_BaseEntity/C_BaseEntity.h"

#include <cstdint>

class C_CSPlayerPawn : public C_BaseEntity {
public:
	SCHEMA_ADD_OFFSET(Vector_t, m_vOldOrigin, 0x1390);
	SCHEMA_ADD_OFFSET(Vector_t, m_vecViewOffset, 0xE70);
	SCHEMA_ADD_OFFSET(CCSPlayer_WeaponServices*, m_pWeaponServices, 0x11E0);
	C_CSPlayerPawn(uintptr_t address);

	C_CSWeaponBase* GetActiveWeapon()const;
	CCSPlayer_WeaponServices* GetWeaponServices()const;
	Vector_t getPosition() const;
	Vector_t getEyePosition() const;

	uintptr_t getAddress() const;
	int getHealth() const;
	uint8_t getTeam() const;
	Vector_t getViewOffset() const;
private:
	uintptr_t address;
};
