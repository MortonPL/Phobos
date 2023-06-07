#include "Utilities/AresFunctions.h"
#include <Utilities/Macro.h>
#include <Phobos.h>
#include "BuildingClass.h"
#include "HouseClass.h"

namespace AresRemiplemented
{
	std::pair<int, BuildingClass*>* HasFactory(std::pair<int, BuildingClass*>* buffer, HouseClass* pOwner, TechnoTypeClass* pType, bool skipAircraft, bool requirePower, bool checkCanBuild, bool unknown);
}

std::pair<int, BuildingClass*>* AresRemiplemented::HasFactory(std::pair<int, BuildingClass*>* buffer, HouseClass* pOwner, TechnoTypeClass* pType, bool skipAircraft, bool requirePower, bool checkCanBuild, bool unknown)
{
	if (checkCanBuild && pOwner->CanBuild(pType, true, true) != CanBuildResult::Buildable)
	{
		*buffer = { 0, nullptr };
		return buffer;
	}

	AbstractType technoRTTI = pType->What_Am_I();
	BuildingClass* pResult = nullptr;
	BuildingClass* pResultButOffline = nullptr;

	for (const auto& pBuilding : pOwner->Buildings)
	{
		if (pBuilding->InLimbo
			|| pBuilding->Type->Factory != technoRTTI
			|| pBuilding->CurrentMission == Mission::Selling
			|| pBuilding->QueuedMission == Mission::Selling
			|| (pBuilding->Type->GetOwners() & pType->GetOwners()) == 0)
			continue;

		if (!skipAircraft
			&& technoRTTI == AbstractType::AircraftType
			&& pBuilding->HasAnyLink()
			&& (!Phobos::Misc::NotAirportBoundLimitBreak || ((AircraftTypeClass*)(pType))->AirportBound) // <=== CHANGE IS HERE
			&& !pBuilding->HasFreeLink())
			continue;

		if (technoRTTI == AbstractType::UnitType && pType->Naval && !pBuilding->Type->Naval)
			continue;

		if (!AresFunctions::CanBeBuiltAt((AresTechnoTypeExt*)pType->align_2FC, pBuilding->Type))
			continue;

		if (requirePower && (!pBuilding->HasPower || pBuilding->Deactivated))
		{
			pResultButOffline = pBuilding;
		}
		else
		{
			pResult = pBuilding;
			if (pBuilding->IsPrimaryFactory)
			{
				*buffer = { 4, pResult };
				return buffer;
			}
			if (unknown)
				break;
		}
	}

	if (pResult)
		*buffer = { 3, pResult };
	else if (pResultButOffline)
		*buffer = { 2, pResultButOffline };
	else
		*buffer = { 1, nullptr };

	return buffer;
}

DEFINE_HOOK(0x5F7900, Who_Can_Build_Me_AircraftBoundChange, 0x5)
{
	if (!IS_ARES_FUN_AVAILABLE(CanBeBuiltAt))
		return 0;

	GET(TechnoTypeClass*, pThis, ECX);
	GET_STACK(bool, skipAircraft, STACK_OFFSET(0x0, 0x4));
	GET_STACK(bool, requirePower, STACK_OFFSET(0x0, 0x8));
	GET_STACK(bool, checkCanBuild, STACK_OFFSET(0x0, 0xC));
	GET_STACK(HouseClass*, pHouse, STACK_OFFSET(0x0, 0x10));

	// replace Ares HouseExt::HasFactory()
	std::pair<int, BuildingClass*> buffer;
	AresRemiplemented::HasFactory(&buffer, pHouse, pThis, skipAircraft, requirePower, checkCanBuild, false);
	if (buffer.first >= 3)
		R->EAX(buffer.second);
	else
		R->EAX(NULL);

	return 0x5F7A89;
}

DEFINE_HOOK(0x5F79F4, Who_Can_Build_Me_AircraftBoundChangeVanilla, 0x5)
{
	enum { Buildable = 0x5F7A03, NotBuildable = 0x5F7A57 };

	GET(ObjectTypeClass*, pAircraft, EDI);
	GET(BuildingClass*, pBuilding, ECX);

	const bool canBuild = (Phobos::Misc::NotAirportBoundLimitBreak && !((AircraftTypeClass*)(pAircraft))->AirportBound) || pBuilding->HasFreeLink();
	R->EAX(pBuilding->IsPrimaryFactory);

	return canBuild ? Buildable : NotBuildable;
}

DEFINE_HOOK(0x443E95, BuildingClass_ExitObject_AboveFactory, 0xB)
{
	GET(BuildingClass*, pBuilding, ESI)

	REF_STACK(int, posX, STACK_OFFSET(0x148, -0xD0));
	REF_STACK(int, posY, STACK_OFFSET(0x148, -0xCC));
	REF_STACK(int, posZ, STACK_OFFSET(0x148, -0xC8));

	const auto pos = pBuilding->GetCenterCoords();
	posX = pos.X;
	posY = pos.Y;
	posZ = pos.Z + 1024; // note: might be placebo? aircraft emerges from pad regardless of this value

	return 0x443EA0;
}
