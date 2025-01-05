#include <Utilities/Macro.h>
#include <Utilities/Debug.h>
#include "CellClass.h"
#include "YRMath.h"

// Complete rewrite. Allow subcell spot 1.
DEFINE_HOOK(0x481130, CellClass_IsSpotFree, 0)
{
	enum { Return = 0x48115D };
	GET(const CellClass*, pThis, ECX);
	GET_STACK(const int, spot, 0x4);
	GET_STACK(const bool, useAltFlag, 0x8);

	if (spot == 0) // This part is changed. In vanilla, it's `spot == 0 || spot == 1`, so spot 1 is never free.
	{
		R->EAX(false);
		return Return;
	}

	const auto flag = useAltFlag ? pThis->AltOccupationFlags : pThis->OccupationFlags;
	const auto result = !(flag & (1 << spot));
	R->EAX(result);

	return Return;
}

// Complete rewrite. Allow subcell spot 1.
DEFINE_HOOK(0x4810A0, CellClass_SpotIndex, 0)
{
	enum { Return = 0x4810F9 };

	GET(const CoordStruct*, pCoord, ECX);

	const double x = pCoord->X - 128;
	const double y = pCoord->Y - 128;
	int spot = 0;

	// Spot 0 if we're close enough to the center of the cell.
	if (Math::sqrt(x * x + y * y) < 60)
	{
		R->EAX(0);
		return Return;
	}

	// Spot numbers go left to right, top to bottom (in world space).
	// Visually:
	//  1
	// 302
	//  4
	if (pCoord->X > 128)
		spot |= 1;
	if (pCoord->Y > 128)
		spot |= 2;

	R->EAX(++spot); // This part is changed. In vanilla, it's `if (spot > 0) spot += 1`, so spot 1 is treated as zero.
	return Return;
}

// Allow subcell spot 1.
DEFINE_HOOK(0x48136E, CellClass_ClosestFreeSpot_1, 0)
{
	enum { SkipSpotOneCheck = 0x481373 };
	return SkipSpotOneCheck;
}
DEFINE_HOOK(0x48131A, CellClass_ClosestFreeSpot_2, 0)
{
	enum { SkipSpotOneCheck = 0x48131F };
	return SkipSpotOneCheck;
}
DEFINE_HOOK(0x48120D, CellClass_ClosestFreeSpot_InlinedIsSpotFree, 0)
{
	enum { SkipSpotOneCheck = 0x481212 };

	R->EDI(R->EDI() + 1); // See CellClass_IsSpotFree rewrite above.

	return SkipSpotOneCheck;
}

// InfantryClass_ClearCellSpot. Allow subcell spot 1
DEFINE_PATCH(0x5218A0, { 0x1E }); // Original: 0x1C, so 0x00011100 -> 0x00011110
DEFINE_PATCH(0x5218C9, { 0x1E });
// AnimClass_ownerstuff_426300. Ditto
DEFINE_PATCH(0x426379, { 0x1E });
DEFINE_PATCH(0x426350, { 0x1E });
