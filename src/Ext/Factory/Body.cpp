#include "Body.h"

FactoryExt::ExtContainer FactoryExt::ExtMap;

// =============================
// load / save

template <typename T>
void FactoryExt::ExtData::Serialize(T& Stm)
{
	Stm
		.Process(this->AccumulatedResources)
		.Process(this->BalanceResources)
		;
}

void FactoryExt::ExtData::LoadFromStream(PhobosStreamReader& Stm)
{
	Extension<FactoryClass>::LoadFromStream(Stm);
	this->Serialize(Stm);
}

void FactoryExt::ExtData::SaveToStream(PhobosStreamWriter& Stm)
{
	Extension<FactoryClass>::SaveToStream(Stm);
	this->Serialize(Stm);
}

bool FactoryExt::LoadGlobals(PhobosStreamReader& Stm)
{
	return Stm
		.Success();
}

bool FactoryExt::SaveGlobals(PhobosStreamWriter& Stm)
{
	return Stm
		.Success();
}

// =============================
// container

FactoryExt::ExtContainer::ExtContainer() : Container("FactoryClass") { }

FactoryExt::ExtContainer::~ExtContainer() = default;

// =============================
// container hooks

DEFINE_HOOK(0x4C99A8, FactoryClass_CTOR, 0x5)
{
	GET(FactoryClass*, pItem, ESI);

	FactoryExt::ExtMap.TryAllocate(pItem);

	return 0;
}

DEFINE_HOOK(0x4C9B03, FactoryClass_DTOR, 0x7)
{
	GET(FactoryClass*, pItem, ESI);

	FactoryExt::ExtMap.Remove(pItem);

	return 0;
}

DEFINE_HOOK_AGAIN(0x4CA3C0, FactoryClass_SaveLoad_Prefix, 0x5)
DEFINE_HOOK(0x4CA270, FactoryClass_SaveLoad_Prefix, 0x5)
{
	GET_STACK(FactoryClass*, pItem, 0x4);
	GET_STACK(IStream*, pStm, 0x8);

	FactoryExt::ExtMap.PrepareStream(pItem, pStm);

	return 0;
}

DEFINE_HOOK(0x4CA3B0, FactoryClass_Load_Suffix, 0x5)
{
	FactoryExt::ExtMap.LoadStatic();

	return 0;
}

DEFINE_HOOK(0x4CA41F, FactoryClass_Save_Suffix, 0x4)
{
	FactoryExt::ExtMap.SaveStatic();

	return 0;
}
