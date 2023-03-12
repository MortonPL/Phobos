#pragma once

#include <CCINIClass.h>
#include <HouseTypeClass.h>

#include <Helpers/Macro.h>
#include <Utilities/Container.h>
#include <Utilities/TemplateDef.h>
#include <Ext/Scenario/Body.h>
#include <New/Type/ResourceTypeClass.h>

class HouseTypeExt
{
public:
	using base_type = HouseTypeClass;
	class ExtData final : public Extension<HouseTypeClass>
	{
	public:
		ValueableIdxVector<ResourceTypeClass> Resource_Types;
		ValueableVector<int> Resource_Values;

		ExtData(HouseTypeClass* OwnerObject) : Extension<HouseTypeClass>(OwnerObject),
			Resource_Types {},
			Resource_Values {}
		{
		}

		virtual ~ExtData() = default;

		virtual void LoadFromINIFile(CCINIClass* pINI) override;
		virtual void Initialize() override;
		virtual void InvalidatePointer(void* ptr, bool bRemoved) override { }

		virtual void LoadFromStream(PhobosStreamReader& Stm) override;
		virtual void SaveToStream(PhobosStreamWriter& Stm) override;

	private:
		template <typename T>
		void Serialize(T& Stm);
	};

	class ExtContainer final : public Container<HouseTypeExt>
	{
	public:
		ExtContainer();
		~ExtContainer();
		virtual void InvalidatePointer(void* ptr, bool bRemoved) override;
	};

	static bool LoadGlobals(PhobosStreamReader& Stm);
	static bool SaveGlobals(PhobosStreamWriter& Stm);

	static ExtContainer ExtMap;
};
