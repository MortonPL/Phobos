#pragma once

#include <Ext/House/Body.h>

#include <Helpers/Macro.h>
#include <Utilities/Container.h>
#include <Utilities/TemplateDef.h>


class FactoryExt
{
public:
	using base_type = FactoryClass;

	static constexpr DWORD Canary = 0xA99AA99A;

	class ExtData final : public Extension<FactoryClass>
	{
	public:
		std::vector<int> AccumulatedResources;
		std::vector<int> BalanceResources;

		ExtData(FactoryClass* OwnerObject) : Extension<FactoryClass>(OwnerObject)
			, AccumulatedResources { }
			, BalanceResources { }
		{ }

		virtual ~ExtData() = default;

		virtual void InvalidatePointer(void* ptr, bool bRemoved) override { }

		virtual void LoadFromStream(PhobosStreamReader& Stm) override;
		virtual void SaveToStream(PhobosStreamWriter& Stm) override;

	private:
		template <typename T>
		void Serialize(T& Stm);
	};

	class ExtContainer final : public Container<FactoryExt>
	{
	public:
		ExtContainer();
		~ExtContainer();
	};

	static ExtContainer ExtMap;

	static bool LoadGlobals(PhobosStreamReader& Stm);
	static bool SaveGlobals(PhobosStreamWriter& Stm);
};
