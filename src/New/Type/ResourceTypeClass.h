#pragma once

#include <Utilities/Enumerable.h>
#include <Utilities/Template.h>
#include <Utilities/GeneralUtils.h>
#include <Utilities/Constructs.h>

class ResourceTypeClass final : public Enumerable<ResourceTypeClass>
{
public:
	Nullable<CSFText> UIName;

	ResourceTypeClass(const char* const pTitle) : Enumerable<ResourceTypeClass>(pTitle)
		, UIName { }
	{ }

	virtual ~ResourceTypeClass() override = default;

	virtual void LoadFromINI(CCINIClass* pINI) override;
	virtual void LoadFromStream(PhobosStreamReader& Stm);
	virtual void SaveToStream(PhobosStreamWriter& Stm);

private:
	template <typename T>
	void Serialize(T& Stm);
};
