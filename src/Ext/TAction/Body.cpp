#include "Body.h"

#include <SessionClass.h>
#include <MessageListClass.h>
#include <HouseClass.h>
#include <CRT.h>
#include <SuperWeaponTypeClass.h>
#include <SuperClass.h>
#include <Ext/SWType/Body.h>
#include <Utilities/SavegameDef.h>

#include <Ext/Scenario/Body.h>
#include <Ext/House/Body.h>

//Static init
template<> const DWORD Extension<TActionClass>::Canary = 0x91919191;
TActionExt::ExtContainer TActionExt::ExtMap;

// =============================
// load / save

template <typename T>
void TActionExt::ExtData::Serialize(T& Stm)
{
	//Stm;
}

void TActionExt::ExtData::LoadFromStream(PhobosStreamReader& Stm)
{
	Extension<TActionClass>::LoadFromStream(Stm);
	this->Serialize(Stm);
}

void TActionExt::ExtData::SaveToStream(PhobosStreamWriter& Stm)
{
	Extension<TActionClass>::SaveToStream(Stm);
	this->Serialize(Stm);
}

bool TActionExt::Execute(TActionClass* pThis, HouseClass* pHouse, ObjectClass* pObject,
	TriggerClass* pTrigger, CellStruct const& location, bool& bHandled)
{
	bHandled = true;

	// Vanilla
	switch (pThis->ActionKind)
	{
	case TriggerAction::PlaySoundEffectRandom:
		return TActionExt::PlayAudioAtRandomWP(pThis, pHouse, pObject, pTrigger, location);
	default:
		break;
	};

	// Phobos
	switch (static_cast<PhobosTriggerAction>(pThis->ActionKind))
	{
	case PhobosTriggerAction::SaveGame:
		return TActionExt::SaveGame(pThis, pHouse, pObject, pTrigger, location);
	case PhobosTriggerAction::EditVariable:
		return TActionExt::EditVariable(pThis, pHouse, pObject, pTrigger, location);
	case PhobosTriggerAction::GenerateRandomNumber:
		return TActionExt::GenerateRandomNumber(pThis, pHouse, pObject, pTrigger, location);
	case PhobosTriggerAction::PrintVariableValue:
		return TActionExt::PrintVariableValue(pThis, pHouse, pObject, pTrigger, location);
	case PhobosTriggerAction::BinaryOperation:
		return TActionExt::BinaryOperation(pThis, pHouse, pObject, pTrigger, location);
	case PhobosTriggerAction::RunSuperWeaponAtLocation:
		return TActionExt::RunSuperWeaponAtLocation(pThis, pHouse, pObject, pTrigger, location);
	case PhobosTriggerAction::RunSuperWeaponAtWaypoint:
		return TActionExt::RunSuperWeaponAtWaypoint(pThis, pHouse, pObject, pTrigger, location);

	case PhobosTriggerAction::ToggleMCVRedeploy:
		return TActionExt::ToggleMCVRedeploy(pThis, pHouse, pObject, pTrigger, location);

	case PhobosTriggerAction::EditResource:
		return TActionExt::EditResource(pThis, pHouse, pObject, pTrigger, location);
	case PhobosTriggerAction::GenerateRandomResource:
		return TActionExt::GenerateRandomResource(pThis, pHouse, pObject, pTrigger, location);
	case PhobosTriggerAction::PrintResourceValue:
		return TActionExt::PrintResourceValue(pThis, pHouse, pObject, pTrigger, location);
	case PhobosTriggerAction::BinaryResourceOperation:
		return TActionExt::BinaryResourceOperation(pThis, pHouse, pObject, pTrigger, location);

	default:
		bHandled = false;
		return true;
	}
}

bool TActionExt::PlayAudioAtRandomWP(TActionClass* pThis, HouseClass* pHouse, ObjectClass* pObject, TriggerClass* pTrigger, CellStruct const& location)
{
	std::vector<int> waypoints;
	waypoints.reserve(ScenarioExt::Global()->Waypoints.size());

	auto const pScen = ScenarioClass::Instance();

	for (auto pair : ScenarioExt::Global()->Waypoints)
		if (pScen->IsDefinedWaypoint(pair.first))
			waypoints.push_back(pair.first);

	if (waypoints.size() > 0)
	{
		auto const index = pScen->Random.RandomRanged(0, waypoints.size() - 1);
		auto const luckyWP = waypoints[index];
		auto const cell = pScen->GetWaypointCoords(luckyWP);
		auto const coords = CellClass::Cell2Coord(cell);
		VocClass::PlayIndexAtPos(pThis->Value, coords);
	}

	return true;
}

bool TActionExt::SaveGame(TActionClass* pThis, HouseClass* pHouse, ObjectClass* pObject, TriggerClass* pTrigger, CellStruct const& location)
{
	if (SessionClass::IsSingleplayer())
	{
		auto PrintMessage = [](const wchar_t* pMessage)
		{
			MessageListClass::Instance->PrintMessage(
				pMessage,
				RulesClass::Instance->MessageDelay,
				HouseClass::CurrentPlayer->ColorSchemeIndex,
				true
			);
		};

		char fName[0x80];

		SYSTEMTIME time;
		GetLocalTime(&time);

		_snprintf_s(fName, 0x7F, "Map.%04u%02u%02u-%02u%02u%02u-%05u.sav",
			time.wYear, time.wMonth, time.wDay, time.wHour, time.wMinute, time.wSecond, time.wMilliseconds);

		PrintMessage(StringTable::LoadString("TXT_SAVING_GAME"));

		wchar_t fDescription[0x80] = { 0 };
		wcscpy_s(fDescription, ScenarioClass::Instance->UINameLoaded);
		wcscat_s(fDescription, L" - ");
		wcscat_s(fDescription, StringTable::LoadString(pThis->Text));

		if (ScenarioClass::Instance->SaveGame(fName, fDescription))
			PrintMessage(StringTable::LoadString("TXT_GAME_WAS_SAVED"));
		else
			PrintMessage(StringTable::LoadString("TXT_ERROR_SAVING_GAME"));
	}

	return true;
}

bool TActionExt::EditVariable(TActionClass* pThis, HouseClass* pHouse, ObjectClass* pObject, TriggerClass* pTrigger, CellStruct const& location)
{
	// Variable Index
	// holds by pThis->Value

	// Operations:
	// 0 : set value - operator=
	// 1 : add value - operator+
	// 2 : minus value - operator-
	// 3 : multiply value - operator*
	// 4 : divide value - operator/
	// 5 : mod value - operator%
	// 6 : <<
	// 7 : >>
	// 8 : ~ (no second param being used)
	// 9 : ^
	// 10 : |
	// 11 : &
	// holds by pThis->Param3

	// Params:
	// The second value
	// holds by pThis->Param4

	// Global Variable or Local
	// 0 for local, 1 for global
	// holds by pThis->Param5

	auto modifyVariable = [pThis](int& nCurrentValue)
	{
		// variable being found
		switch (pThis->Param3)
		{
		case 0: { nCurrentValue = pThis->Param4; break; }
		case 1: { nCurrentValue += pThis->Param4; break; }
		case 2: { nCurrentValue -= pThis->Param4; break; }
		case 3: { nCurrentValue *= pThis->Param4; break; }
		case 4: { nCurrentValue /= pThis->Param4; break; }
		case 5: { nCurrentValue %= pThis->Param4; break; }
		case 6: { nCurrentValue <<= pThis->Param4; break; }
		case 7: { nCurrentValue >>= pThis->Param4; break; }
		case 8: { nCurrentValue = ~nCurrentValue; break; }
		case 9: { nCurrentValue ^= pThis->Param4; break; }
		case 10: { nCurrentValue |= pThis->Param4; break; }
		case 11: { nCurrentValue &= pThis->Param4; break; }
		default:
			break;
		}

		if (!pThis->Param5)
			TagClass::NotifyLocalChanged(pThis->Value);
		else
			TagClass::NotifyGlobalChanged(pThis->Value);
	};

	// uses !pThis->Param5 to ensure Param5 is 0 or 1
	auto& variables = ScenarioExt::Global()->Variables[pThis->Param5 != 0];
	auto itr = variables.find(pThis->Value);
	if (itr != variables.end())
		modifyVariable(itr->second.Value);

	return true;
}

bool TActionExt::GenerateRandomNumber(TActionClass* pThis, HouseClass* pHouse, ObjectClass* pObject, TriggerClass* pTrigger, CellStruct const& location)
{
	auto randomizeVariable = [pThis](int& nCurrentValue)
	{
		nCurrentValue = ScenarioClass::Instance->Random.RandomRanged(pThis->Param3, pThis->Param4);
		if (!pThis->Param5)
			TagClass::NotifyLocalChanged(pThis->Value);
		else
			TagClass::NotifyGlobalChanged(pThis->Value);
	};

	auto& variables = ScenarioExt::Global()->Variables[pThis->Param5 != 0];
	auto itr = variables.find(pThis->Value);
	if (itr != variables.end())
		randomizeVariable(itr->second.Value);

	return true;
}

bool TActionExt::PrintVariableValue(TActionClass* pThis, HouseClass* pHouse, ObjectClass* pObject, TriggerClass* pTrigger, CellStruct const& location)
{
	auto randomizeVariable = [pThis](int& nCurrentValue)
	{
		CRT::swprintf(Phobos::wideBuffer, L"%d", nCurrentValue);
		MessageListClass::Instance->PrintMessage(Phobos::wideBuffer);
		if (!pThis->Param3)
			TagClass::NotifyLocalChanged(pThis->Value);
		else
			TagClass::NotifyGlobalChanged(pThis->Value);
	};

	auto& variables = ScenarioExt::Global()->Variables[pThis->Param3 != 0];
	auto itr = variables.find(pThis->Value);
	if (itr != variables.end())
		randomizeVariable(itr->second.Value);

	return true;
}

bool TActionExt::BinaryOperation(TActionClass* pThis, HouseClass* pHouse, ObjectClass* pObject, TriggerClass* pTrigger, CellStruct const& location)
{
	auto modifyVariable = [pThis](int& nCurrentValue, int& nOptValue)
	{
		switch (pThis->Param3)
		{
		case 0: { nCurrentValue = nOptValue; break; }
		case 1: { nCurrentValue += nOptValue; break; }
		case 2: { nCurrentValue -= nOptValue; break; }
		case 3: { nCurrentValue *= nOptValue; break; }
		case 4: { nCurrentValue /= nOptValue; break; }
		case 5: { nCurrentValue %= nOptValue; break; }
		case 6: { nCurrentValue <<= nOptValue; break; }
		case 7: { nCurrentValue >>= nOptValue; break; }
		case 8: { nCurrentValue = nOptValue; break; }
		case 9: { nCurrentValue ^= nOptValue; break; }
		case 10: { nCurrentValue |= nOptValue; break; }
		case 11: { nCurrentValue &= nOptValue; break; }
		default:
			break;
		}

		if (!pThis->Param5)
			TagClass::NotifyLocalChanged(pThis->Value);
		else
			TagClass::NotifyGlobalChanged(pThis->Value);
	};

	auto& variables1 = ScenarioExt::Global()->Variables[pThis->Param5 != 0];
	auto itr1 = variables1.find(pThis->Value);
	auto& variables2 = ScenarioExt::Global()->Variables[pThis->Param6 != 0];
	auto itr2 = variables2.find(pThis->Param4);
	if (itr1 != variables1.end() && itr2 != variables2.end())
		modifyVariable(itr1->second.Value, itr2->second.Value);

	return true;
}

bool TActionExt::RunSuperWeaponAtLocation(TActionClass* pThis, HouseClass* pHouse, ObjectClass* pObject, TriggerClass* pTrigger, CellStruct const& location)
{
	if (!pThis)
		return true;

	TActionExt::RunSuperWeaponAt(pThis, pThis->Param5, pThis->Param6);

	return true;
}

bool TActionExt::RunSuperWeaponAtWaypoint(TActionClass* pThis, HouseClass* pHouse, ObjectClass* pObject, TriggerClass* pTrigger, CellStruct const& location)
{
	if (!pThis)
		return true;

	auto& waypoints = ScenarioExt::Global()->Waypoints;
	int nWaypoint = pThis->Param5;

	// Check if is a valid Waypoint
	if (nWaypoint >= 0 && waypoints.find(nWaypoint) != waypoints.end() && waypoints[nWaypoint].X && waypoints[nWaypoint].Y)
	{
		auto const selectedWP = waypoints[nWaypoint];
		TActionExt::RunSuperWeaponAt(pThis, selectedWP.X, selectedWP.Y);
	}

	return true;
}

bool TActionExt::RunSuperWeaponAt(TActionClass* pThis, int X, int Y)
{
	if (SuperWeaponTypeClass::Array->Count > 0)
	{
		int swIdx = pThis->Param3;
		int houseIdx = -1;
		std::vector<int> housesListIdx;
		CellStruct targetLocation = { (short)X, (short)Y };

		do
		{
			if (X < 0)
				targetLocation.X = (short)ScenarioClass::Instance->Random.RandomRanged(0, MapClass::Instance->MapCoordBounds.Right);

			if (Y < 0)
				targetLocation.Y = (short)ScenarioClass::Instance->Random.RandomRanged(0, MapClass::Instance->MapCoordBounds.Bottom);
		}
		while (!MapClass::Instance->IsWithinUsableArea(targetLocation, false));

		// Only valid House indexes
		if ((pThis->Param4 >= HouseClass::Array->Count
			&& pThis->Param4 < HouseClass::PlayerAtA)
			|| pThis->Param4 > (HouseClass::PlayerAtA + HouseClass::Array->Count - 3))
		{
			return true;
		}

		switch (pThis->Param4)
		{
		case HouseClass::PlayerAtA:
			houseIdx = 0;
			break;

		case HouseClass::PlayerAtB:
			houseIdx = 1;
			break;

		case HouseClass::PlayerAtC:
			houseIdx = 2;
			break;

		case HouseClass::PlayerAtD:
			houseIdx = 3;
			break;

		case HouseClass::PlayerAtE:
			houseIdx = 4;
			break;

		case HouseClass::PlayerAtF:
			houseIdx = 5;
			break;

		case HouseClass::PlayerAtG:
			houseIdx = 6;
			break;

		case HouseClass::PlayerAtH:
			houseIdx = 7;
			break;

		case -1:
			// Random non-neutral
			for (auto pHouse : *HouseClass::Array)
			{
				if (!pHouse->Defeated
					&& !pHouse->IsObserver()
					&& !pHouse->Type->MultiplayPassive)
				{
					housesListIdx.push_back(pHouse->ArrayIndex);
				}
			}

			if (housesListIdx.size() > 0)
				houseIdx = housesListIdx.at(ScenarioClass::Instance->Random.RandomRanged(0, housesListIdx.size() - 1));
			else
				return true;

			break;

		case -2:
			// Find first Neutral
			for (auto pHouseNeutral : *HouseClass::Array)
			{
				if (pHouseNeutral->IsNeutral())
				{
					houseIdx = pHouseNeutral->ArrayIndex;
					break;
				}
			}

			if (houseIdx < 0)
				return true;

			break;

		case -3:
			// Random Human Player
			for (auto pHouse : *HouseClass::Array)
			{
				if (pHouse->IsControlledByHuman()
					&& !pHouse->Defeated
					&& !pHouse->IsObserver())
				{
					housesListIdx.push_back(pHouse->ArrayIndex);
				}
			}

			if (housesListIdx.size() > 0)
				houseIdx = housesListIdx.at(ScenarioClass::Instance->Random.RandomRanged(0, housesListIdx.size() - 1));
			else
				return true;

			break;

		default:
			if (pThis->Param4 >= 0)
				houseIdx = pThis->Param4;
			else
				return true;

			break;
		}

		if (HouseClass* pHouse = HouseClass::Array->GetItem(houseIdx))
		{
			if (auto const pSuper = pHouse->Supers.GetItem(swIdx))
			{
				int oldstart = pSuper->RechargeTimer.StartTime;
				int oldleft = pSuper->RechargeTimer.TimeLeft;
				pSuper->SetReadiness(true);
				pSuper->Launch(targetLocation, false);
				pSuper->Reset();
				pSuper->RechargeTimer.StartTime = oldstart;
				pSuper->RechargeTimer.TimeLeft = oldleft;
			}
		}
	}

	return true;
}

bool TActionExt::ToggleMCVRedeploy(TActionClass* pThis, HouseClass* pHouse, ObjectClass* pObject, TriggerClass* pTrigger, CellStruct const& location)
{
	GameModeOptionsClass::Instance->MCVRedeploy = pThis->Param3 != 0;
	return true;
}

void NotifyVariableChanged(int resourceIdx, int houseIdx)
{
	// TODO
}

bool TActionExt::EditResource(TActionClass* pThis, HouseClass* pHouse, ObjectClass* pObject, TriggerClass* pTrigger, CellStruct const& location)
{
	// Value - resource index
	// Param3 - House index
	// Param4 - operator
	// Param5 - value

	auto modifyVariable = [pThis](int& nCurrentValue)
	{
		// variable being found
		switch (pThis->Param4)
		{
		case 0: { nCurrentValue = pThis->Param5; break; }
		case 1: { nCurrentValue += pThis->Param5; break; }
		case 2: { nCurrentValue -= pThis->Param5; break; }
		case 3: { nCurrentValue *= pThis->Param5; break; }
		case 4: { nCurrentValue /= pThis->Param5; break; }
		case 5: { nCurrentValue %= pThis->Param5; break; }
		case 6: { nCurrentValue <<= pThis->Param5; break; }
		case 7: { nCurrentValue >>= pThis->Param5; break; }
		case 8: { nCurrentValue = ~nCurrentValue; break; }
		case 9: { nCurrentValue ^= pThis->Param5; break; }
		case 10: { nCurrentValue |= pThis->Param5; break; }
		case 11: { nCurrentValue &= pThis->Param5; break; }
		default:
			break;
		}

		NotifyVariableChanged(pThis->Value, pThis->Param3);
	};

	auto pHouse1 = HouseClass::FindByIndex(pThis->Param3);
	if (!pHouse1)
		return false;

	auto pExt = HouseExt::ExtMap.Find(pHouse1);
	for (size_t i = 0; i < pExt->Resource_Types.size(); i++)
	{
		if (pExt->Resource_Types[i] == pThis->Value)
		{
			modifyVariable(pExt->Resource_Values[i]);
			break;
		}
	}

	return true;
}

bool TActionExt::GenerateRandomResource(TActionClass* pThis, HouseClass* pHouse, ObjectClass* pObject, TriggerClass* pTrigger, CellStruct const& location)
{
	// Value - resource index
	// Param3 - House index
	// Param4 - min
	// Param5 - max

	auto randomizeVariable = [pThis](int& nCurrentValue)
	{
		nCurrentValue = ScenarioClass::Instance->Random.RandomRanged(pThis->Param4, pThis->Param5);
		NotifyVariableChanged(pThis->Value, pThis->Param3);
	};

	auto pHouse1 = HouseClass::FindByIndex(pThis->Param3);
	if (!pHouse1)
		return false;

	auto pExt = HouseExt::ExtMap.Find(pHouse1);
	for (size_t i = 0; i < pExt->Resource_Types.size(); i++)
	{
		if (pExt->Resource_Types[i] == pThis->Value)
		{
			randomizeVariable(pExt->Resource_Values[i]);
			break;
		}
	}

	return true;
}

bool TActionExt::PrintResourceValue(TActionClass* pThis, HouseClass* pHouse, ObjectClass* pObject, TriggerClass* pTrigger, CellStruct const& location)
{
	// Value - resource index
	// Param3 - House index

	auto randomizeVariable = [pThis](int& nCurrentValue)
	{
		CRT::swprintf(Phobos::wideBuffer, L"%d", nCurrentValue);
		MessageListClass::Instance->PrintMessage(Phobos::wideBuffer);
		NotifyVariableChanged(pThis->Value, pThis->Param3);
	};

	auto pHouse1 = HouseClass::FindByIndex(pThis->Param3);
	if (!pHouse1)
		return false;

	auto pExt = HouseExt::ExtMap.Find(pHouse1);
	for (size_t i = 0; i < pExt->Resource_Types.size(); i++)
	{
		if (pExt->Resource_Types[i] == pThis->Value)
		{
			randomizeVariable(pExt->Resource_Values[i]);
			break;
		}
	}

	return true;
}

bool TActionExt::BinaryResourceOperation(TActionClass* pThis, HouseClass* pHouse, ObjectClass* pObject, TriggerClass* pTrigger, CellStruct const& location)
{
	// Value - resource index
	// Param3 - House index
	// Param4 - 2nd resource index
	// Param5 - 2nd House index
	// Param6 - operator

	auto modifyVariable = [pThis](int& nCurrentValue, int& nOptValue)
	{
		switch (pThis->Param6)
		{
		case 0: { nCurrentValue = nOptValue; break; }
		case 1: { nCurrentValue += nOptValue; break; }
		case 2: { nCurrentValue -= nOptValue; break; }
		case 3: { nCurrentValue *= nOptValue; break; }
		case 4: { nCurrentValue /= nOptValue; break; }
		case 5: { nCurrentValue %= nOptValue; break; }
		case 6: { nCurrentValue <<= nOptValue; break; }
		case 7: { nCurrentValue >>= nOptValue; break; }
		case 8: { nCurrentValue = nOptValue; break; }
		case 9: { nCurrentValue ^= nOptValue; break; }
		case 10: { nCurrentValue |= nOptValue; break; }
		case 11: { nCurrentValue &= nOptValue; break; }
		default:
			break;
		}

		NotifyVariableChanged(pThis->Value, pThis->Param3);
	};

	auto pHouse1 = HouseClass::FindByIndex(pThis->Param3);
	auto pHouse2 = HouseClass::FindByIndex(pThis->Param4);
	if (!pHouse1 || !pHouse2)
		return false;

	auto pExt = HouseExt::ExtMap.Find(pHouse1);
	auto pExt2 = HouseExt::ExtMap.Find(pHouse2);
	int idx1 = -1;
	int idx2 = -1;
	for (size_t i = 0; i < pExt->Resource_Types.size(); i++)
	{
		if (pExt->Resource_Types[i] == pThis->Value)
		{
			idx1 = i;
			break;
		}
	}
	for (size_t i = 0; i < pExt->Resource_Types.size(); i++)
	{
		if (pExt->Resource_Types[i] == pThis->Value)
		{
			idx2 = i;
			break;
		}
	}

	if (idx1 == -1 || idx2 == -1)
		return false;

	modifyVariable(pExt->Resource_Values[idx1], pExt2->Resource_Values[idx2]);

	return true;
}

// =============================
// container

TActionExt::ExtContainer::ExtContainer() : Container("TActionClass") { }

TActionExt::ExtContainer::~ExtContainer() = default;

// =============================
// container hooks

#ifdef MAKE_GAME_SLOWER_FOR_NO_REASON
DEFINE_HOOK(0x6DD176, TActionClass_CTOR, 0x5)
{
	GET(TActionClass*, pItem, ESI);

	TActionExt::ExtMap.FindOrAllocate(pItem);
	return 0;
}

DEFINE_HOOK(0x6E4761, TActionClass_SDDTOR, 0x6)
{
	GET(TActionClass*, pItem, ESI);

	TActionExt::ExtMap.Remove(pItem);
	return 0;
}

DEFINE_HOOK_AGAIN(0x6E3E30, TActionClass_SaveLoad_Prefix, 0x8)
DEFINE_HOOK(0x6E3DB0, TActionClass_SaveLoad_Prefix, 0x5)
{
	GET_STACK(TActionClass*, pItem, 0x4);
	GET_STACK(IStream*, pStm, 0x8);

	TActionExt::ExtMap.PrepareStream(pItem, pStm);

	return 0;
}

DEFINE_HOOK(0x6E3E29, TActionClass_Load_Suffix, 0x4)
{
	TActionExt::ExtMap.LoadStatic();
	return 0;
}

DEFINE_HOOK(0x6E3E4A, TActionClass_Save_Suffix, 0x3)
{
	TActionExt::ExtMap.SaveStatic();
	return 0;
}
#endif
