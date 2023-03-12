#include "ResourceTypeClass.h"

#include <Utilities/TemplateDef.h>

Enumerable<ResourceTypeClass>::container_t Enumerable<ResourceTypeClass>::Array;

// pretty nice, eh
const char* Enumerable<ResourceTypeClass>::GetMainSection()
{
	return "ResourceTypes";
}

void ResourceTypeClass::LoadFromINI(CCINIClass* pINI)
{
	const char* section = this->Name;

	INI_EX exINI(pINI);

	this->UIName.Read(exINI, section, "UIName");
}

template <typename T>
void ResourceTypeClass::Serialize(T& Stm)
{
	Stm
		.Process(this->UIName)
		;
};

void ResourceTypeClass::LoadFromStream(PhobosStreamReader& Stm)
{
	this->Serialize(Stm);
}

void ResourceTypeClass::SaveToStream(PhobosStreamWriter& Stm)
{
	this->Serialize(Stm);
}
