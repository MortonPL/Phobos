#include "Body.h"

template<> const DWORD Extension<HouseTypeClass>::Canary = 0xAFFEAFFE;
HouseTypeExt::ExtContainer HouseTypeExt::ExtMap;

void HouseTypeExt::ExtData::Initialize() { }

void HouseTypeExt::ExtData::LoadFromINIFile(CCINIClass* pINI)
{
	auto pThis = this->OwnerObject();
	const char* pSection = pThis->ID;

	INI_EX exINI(pINI);

	ReadVariables(pINI, pSection);
}

void HouseTypeExt::ExtData::ReadVariables(CCINIClass* const pINI, const char* pSection)
{
	char tempBuffer[13];

	for (int i = 0; i < INT_MAX; i++)
	{
		_snprintf_s(tempBuffer, sizeof(tempBuffer), "Variable%d", i);
		PhobosFixedString<0x100> temp;
		temp.Read(pINI, pSection, tempBuffer);
		if (!temp)
			break;
		auto& var = this->Variables[i];
		char* buffer;
		strcpy_s(var.Name, strtok_s(Phobos::readBuffer, ",", &buffer));
		if (auto pState = strtok_s(nullptr, ",", &buffer))
			var.Value = atoi(pState);
		else
			var.Value = 0;
	}
}

template <typename T>
void HouseTypeExt::ExtData::Serialize(T& Stm)
{
	Stm
		.Process(this->Variables)
		;
}

void HouseTypeExt::ExtData::LoadFromStream(PhobosStreamReader& Stm)
{
	Extension<HouseTypeClass>::LoadFromStream(Stm);
	this->Serialize(Stm);
}

void HouseTypeExt::ExtData::SaveToStream(PhobosStreamWriter& Stm)
{
	Extension<HouseTypeClass>::SaveToStream(Stm);
	this->Serialize(Stm);
}

bool HouseTypeExt::LoadGlobals(PhobosStreamReader& Stm)
{
	return Stm
		.Success();
}

bool HouseTypeExt::SaveGlobals(PhobosStreamWriter& Stm)
{
	return Stm
		.Success();
}

// =============================
// container

HouseTypeExt::ExtContainer::ExtContainer() : Container("HouseTypeClass") { }
HouseTypeExt::ExtContainer::~ExtContainer() = default;
void HouseTypeExt::ExtContainer::InvalidatePointer(void* ptr, bool bRemoved) { }

// =============================
// container hooks

DEFINE_HOOK(0x511635, HouseTypeClass_CTOR_1, 0x5)
{
	GET(HouseTypeClass*, pItem, EAX);

	HouseTypeExt::ExtMap.FindOrAllocate(pItem);
	return 0;
}

DEFINE_HOOK(0x511643, HouseTypeClass_CTOR_2, 0x5)
{
	GET(HouseTypeClass*, pItem, EAX);

	HouseTypeExt::ExtMap.FindOrAllocate(pItem);
	return 0;
}

DEFINE_HOOK(0x5127CF, HouseTypeClass_DTOR, 0x6)
{
	GET(HouseTypeClass*, pItem, ESI);

	HouseTypeExt::ExtMap.Remove(pItem);
	return 0;
}

DEFINE_HOOK_AGAIN(0x512480, HouseTypeClass_SaveLoad_Prefix, 0x5)
DEFINE_HOOK(0x512290, HouseTypeClass_SaveLoad_Prefix, 0x5)
{
	GET_STACK(HouseTypeClass*, pItem, 0x4);
	GET_STACK(IStream*, pStm, 0x8);

	HouseTypeExt::ExtMap.PrepareStream(pItem, pStm);

	return 0;
}

DEFINE_HOOK(0x51246D, HouseTypeClass_Load_Suffix, 0x5)
{

	HouseTypeExt::ExtMap.LoadStatic();
	return 0;
}

DEFINE_HOOK(0x51255C, HouseTypeClass_Save_Suffix, 0x5)
{
	HouseTypeExt::ExtMap.SaveStatic();
	return 0;
}

DEFINE_HOOK_AGAIN(0x51215A, HouseTypeClass_LoadFromINI, 0x5)
DEFINE_HOOK(0x51214F, HouseTypeClass_LoadFromINI, 0x5)
{
	GET(HouseTypeClass*, pItem, EBX);
	GET_BASE(CCINIClass*, pINI, 0x8);

	HouseTypeExt::ExtMap.LoadFromINI(pItem, pINI);
	return 0;
}
