#include "ResourceInfo.h"

#include <Utilities/GeneralUtils.h>
#include <HouseClass.h>
#include <Helpers/Enumerators.h>
#include <CRT.h>

#include <Ext/House/Body.h>

const char* ResourceInfoCommandClass::GetName() const
{
	return "Dump ResourceInfo";
}

const wchar_t* ResourceInfoCommandClass::GetUIName() const
{
	return GeneralUtils::LoadStringUnlessMissing("TXT_DUMP_RESOURCE_INFO", L"Dump Resource Info");
}

const wchar_t* ResourceInfoCommandClass::GetUICategory() const
{
	return CATEGORY_DEVELOPMENT;
}

const wchar_t* ResourceInfoCommandClass::GetUIDescription() const
{
	return GeneralUtils::LoadStringUnlessMissing("TXT_DUMP_RESOURCE_INFO_DESC", L"Dump ResourceInfo to log file and display it.");
}

void ResourceInfoCommandClass::Execute(WWKey eInput) const
{
	if (this->CheckDebugDeactivated())
		return;

	char buffer[0x800] = { 0 };

	auto append = [&buffer](const char* pFormat, ...)
	{
		va_list args;
		va_start(args, pFormat);
		vsprintf_s(Phobos::readBuffer, pFormat, args);
		va_end(args);
		strcat_s(buffer, Phobos::readBuffer);
	};

	auto display = [&buffer]()
	{
		memset(Phobos::wideBuffer, 0, sizeof Phobos::wideBuffer);
		CRT::mbstowcs(Phobos::wideBuffer, buffer, strlen(buffer));
		MessageListClass::Instance->PrintMessage(Phobos::wideBuffer, 600, 5, true);
		Debug::Log("%s\n", buffer);
		buffer[0] = 0;
	};

	auto printHouse = [&append, &display](HouseClass* pHouse)
	{
		append("[Phobos] Dump ResourceInfo runs.\n");
		auto pType = pHouse->Type;
		auto pExt = HouseExt::ExtMap.Find(pHouse);
		append("ID = %s, ", pType->ID);
		append("Money = %d, ", pHouse->Balance);
		append("Power = %d/%d, ", pHouse->PowerOutput, pHouse->PowerDrain);
		append("Score = %d\n", pHouse->SiloMoney);
		for (size_t i = 0; i < pExt->Resource_Types.size(); i++)
		{
			auto resourceName = ResourceTypeClass::Array[pExt->Resource_Types[i]].get()->UIName;
			append("%S = %d", resourceName.Get().Text, pExt->Resource_Values[i]);
		}
		append("\n");
		display();
	};

	for (auto pHouse : *HouseClass::Array)
		if (pHouse->IsInPlayerControl)
			printHouse(pHouse);
}
