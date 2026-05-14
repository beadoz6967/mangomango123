#include "schema.h"
#include <vector>
#include <algorithm>
#include <Shlobj.h>
#include <shlobj_core.h>
#include <cstdio>

#include "../logging/log.h"


#include "../fnv1a/fnv1a.h"

#include "../math/utlstring/utlstring.h"
#include "../memory/Interface/Interface.h"

struct SchemaDumpedData_t
{
	uint32_t hashedName = 0x0ULL;
	std::uint32_t uOffset = 0x0U;
};

static std::vector<SchemaDumpedData_t> dumped_data;

static bool IsModuleReady(const char* moduleName)
{
	return moduleName != nullptr && GetModuleHandleA(moduleName) != nullptr;
}

static CSchemaSystemTypeScope* SafeFindTypeScope(ISchemaSystem* schemaSystem, const char* moduleName)
{
	__try {
		return schemaSystem != nullptr ? schemaSystem->FindTypeScopeForModule(moduleName) : nullptr;
	}
	__except (EXCEPTION_EXECUTE_HANDLER) {
		return nullptr;
	}
}

static bool TryDumpSchemaData(CSchemaSystemTypeScope* pTypeScope)
{
	__try {
		if (pTypeScope == nullptr)
			return false;

		const int nTableSize = pTypeScope->hashClasses.Count();
		Logger::Logf(LogType::Info, "Schema hash table size: %d", nTableSize);

		UtlTSHashHandle_t* pElements = new UtlTSHashHandle_t[nTableSize + 1U];
		const auto nElements = pTypeScope->hashClasses.GetElements(0, nTableSize, pElements);
		Logger::Logf(LogType::Info, "Schema elements enumerated: %d", nElements);

		char szFieldClassBuffer[512]{};
		for (int i = 0; i < nElements; i++)
		{
			const UtlTSHashHandle_t hElement = pElements[i];

			if (hElement == 0)
				continue;

			CSchemaClassBinding* pClassBinding = pTypeScope->hashClasses[hElement];
			if (pClassBinding == nullptr)
				continue;

			SchemaClassInfoData_t* pDeclaredClassInfo = nullptr;
			pTypeScope->FindDeclaredClass(&pDeclaredClassInfo, pClassBinding->szBinaryName);

			if (pDeclaredClassInfo == nullptr)
				continue;

			if (pDeclaredClassInfo->pFields == nullptr)
				continue;

			if (pDeclaredClassInfo->nFieldSize == 0)
				continue;

			for (auto j = 0; j < pDeclaredClassInfo->nFieldSize; j++)
			{
				SchemaClassFieldData_t* pFields = pDeclaredClassInfo->pFields;
				const char* className = pDeclaredClassInfo->szName ? pDeclaredClassInfo->szName : "";
				const char* fieldName = pFields[j].szName ? pFields[j].szName : "";
				sprintf_s(szFieldClassBuffer, "%s->%s", className, fieldName);

				dumped_data.emplace_back(hash_32_fnv1a_const(szFieldClassBuffer), pFields[j].nSingleInheritanceOffset);
			}

			HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
			SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY);
			printf("[Schema] ");
			SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
			printf("Dumped Class: %s fields: %i \n", pDeclaredClassInfo->szName, pDeclaredClassInfo->nFieldSize);
		}

		delete[] pElements;
		return true;
	}
	__except (EXCEPTION_EXECUTE_HANDLER) {
		Logger::Log("Schema init crashed while resolving schema data; deferring startup.", LogType::Error);
		return false;
	}
}

bool Schema::init(const char* ModuleName, int module_type)
{
    Logger::Logf(LogType::Info, "Schema init started for module=%s type=%d", ModuleName ? ModuleName : "<null>", module_type);

    if (!IsModuleReady("schemasystem.dll") || !IsModuleReady(ModuleName))
    {
        Logger::Log("Schema init deferred: required modules are not loaded yet.", LogType::Warning);
        return false;
    }

    schema_system = I::Get<ISchemaSystem>("schemasystem.dll", "SchemaSystem_00");
	if (!schema_system)
	{
		Logger::Log("Schema init failed: schema_system interface not found.", LogType::Error);
		return false;
	}

	CSchemaSystemTypeScope* pTypeScope = SafeFindTypeScope(schema_system, ModuleName);
	if (pTypeScope == nullptr)
	{
		Logger::Log("Schema init failed: type scope not found for module.", LogType::Error);
		return false;
	}

	Logger::Logf(LogType::Info, "Schema type scope resolved: %p", pTypeScope);

	return TryDumpSchemaData(pTypeScope);
}

std::uint32_t SchemaFinder::Get(const uint32_t hashedName)
{
	if (const auto it = std::ranges::find_if(dumped_data, [hashedName](const SchemaDumpedData_t& data)
		{ return data.hashedName == hashedName; });
		it != dumped_data.end())
		return it->uOffset;

	return 0U;
}

