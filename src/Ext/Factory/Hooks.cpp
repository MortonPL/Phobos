#include "Body.h"

DEFINE_HOOK(0x4CA04B, FactoryClass_Abandon, 0x6)
{
	GET(FactoryClass*, pThis, ESI);

	auto pExt = FactoryExt::ExtMap.Find(pThis);
	auto pOwnerExt = HouseExt::ExtMap.Find(pThis->Owner);
	pOwnerExt->GiveResources(pExt->AccumulatedResources);
	pExt->BalanceResources.clear();
	pExt->AccumulatedResources.clear();

	return 0;
}

DEFINE_HOOK(0x4C9DE7, FactoryClass_Set, 0x6)
{
	GET(FactoryClass*, pThis, ESI);

	auto pExt = FactoryExt::ExtMap.Find(pThis);
	auto pType = TechnoTypeExt::ExtMap.Find(pThis->Object->GetTechnoType());
	pExt->BalanceResources = pType->Cost_Extra;
	pExt->AccumulatedResources.resize(pExt->BalanceResources.size());

	return 0;
}

DEFINE_HOOK(0x4C9BD5, FactoryClass_AI_CostPerTick, 0x6)
{
	enum { CantBuild = 0x4C9BE5 };

	GET(FactoryClass*, pThis, ESI);

	auto pExt = FactoryExt::ExtMap.Find(pThis);
	auto pOwnerExt = HouseExt::ExtMap.Find(pThis->Owner);

	std::vector<int> costs = pExt->BalanceResources;

	// Calculate cost per tick
	int stage = pThis->Production.Value;
	if (stage != 54)
	{
		for (int& x : costs)
			x /= 54 - stage;
	}

	for (size_t i = 0; i < costs.size(); i++)
		costs[i] = std::min(costs[i], pExt->BalanceResources[i]);

	// Check if we can afford this tick
	int canBuild = true;
	std::vector<int>& resources = pOwnerExt->Resources;
	for (size_t i = 0; i < costs.size(); i++)
		canBuild &= costs[i] <= resources[i];

	if (canBuild)
	{
		for (size_t i = 0; i < costs.size(); i++)
		{
			pExt->BalanceResources[i] -= costs[i];
			pExt->AccumulatedResources[i] += costs[i];
		}

		pOwnerExt->TakeResources(costs);

		return 0;
	}

	return CantBuild;
}
