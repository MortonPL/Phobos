#include <Helpers/Macro.h>
#include <Utilities/Macro.h>
#include <Utilities/Debug.h>
#include <Phobos.h>

#include <GScreenClass.h>
#include <WWMouseClass.h>
#include <TacticalClass.h>
#include <GadgetClass.h>
#include <MessageListClass.h>
#include <CCToolTip.h>
#include <MouseClass.h>
#include <SessionClass.h>
#include <Unsorted.h>
#include <AlphaShapeClass.h>

namespace Rendering
{
	static constexpr reference<bool, 0xB0B519u> const BlitMouse {};
	static constexpr reference<bool, 0xA9FAB0u> const IonStormClass_ChronoScreenEffect_Status {};
	static constexpr reference<GadgetClass*, 0xA8EF54u> const Buttons {};
	static constexpr reference<bool, 0xA8ED6Bu> const DebugDebugDebug {};
	static constexpr reference<bool, 0xA8B8B4u> const EnableMultiplayerDebug {};

	void MultiplayerDebugPrint()
		{ JMP_STD(0x55F1E0); }

	// Faithful reproduction of GScreenClass::Render().
	void Render(GScreenClass* pThis);

	// Faithful reproduction of TacticalClass::Render_Shroud().
	void RenderShroud(TacticalClass* pThis, RectangleStruct* pRect1, RectangleStruct* pRect2, RectangleStruct* pRect3, bool force);

	void RenderShroud2();
}

void Rendering::Render(GScreenClass* pThis)
{
	auto pTempSurface = DSurface::Temp.get();
	DSurface::Temp = DSurface::Composite;
	WWMouseClass::Instance->func_40(DSurface::Composite, false);

	bool shouldDraw = pThis->Bitfield != 0;
	bool complete = pThis->Bitfield == 2;
	pThis->Bitfield = 0;

	if (!IonStormClass_ChronoScreenEffect_Status)
	{
		TacticalClass::Instance->Render(DSurface::Composite, shouldDraw, 0);
		TacticalClass::Instance->Render(DSurface::Composite, shouldDraw, 1);
		pThis->Draw(complete);
		TacticalClass::Instance->Render(DSurface::Composite, shouldDraw, 2);
	}

	if (BlitMouse.get() && !DebugDebugDebug.get())
	{
		WWMouseClass::Instance->func_40(DSurface::Sidebar, true);
		BlitMouse = false;
	}

	if (Buttons.get())
		Buttons->DrawAll(false);

	MessageListClass::Instance->Draw();

	if (CCToolTip::Instance.get())
		CCToolTip::Instance->Draw(false);

	if (EnableMultiplayerDebug.get())
		MultiplayerDebugPrint();

	WWMouseClass::Instance->func_3C(DSurface::Composite, false);
	pThis->vt_entry_44();

	DSurface::Temp = pTempSurface;
}

void Rendering::RenderShroud(TacticalClass* pThis, RectangleStruct* pRect1,
	RectangleStruct* pRect2, RectangleStruct* pRectForIntersect, bool force)
{
	if (force)
	{
		// Fully recalculate and redraw shroud from scratch.
		pThis->RenderShroud2(&DSurface::ViewBounds);
	}
	else
	{
		// No clue what it does yet.
		for (int i = 0; i < pThis->VisibleCellCount; i++)
		{
			CellClass* cell = pThis->VisibleCells[i];
			CoordStruct coords = cell->GetCellCoords();
			Point2D point;

			// World cell coords to screen space, centered.
			pThis->CoordsToScreen(&point, &coords);
			point.X = point.X - pThis->TacticalPos.X + DSurface::ViewBounds->X - 30;
			point.Y = point.Y - pThis->TacticalPos.Y + DSurface::ViewBounds->Y - 15;

			// Figure out shroud shape and draw shroud and fog for this cell (?)
			if (Game::hWnd)
				cell->CellShadowUpdate(&point, pRect2);

			AlphaShapeClass::Blit(&point, pRect2);
		}

		// Draw shroud over all undiscovered cells.
		if (pRect1->Width > 0 && pRect1->Height > 0)
			pThis->RenderShroud2(pRect1);
		if (pRect2->Width > 0 && pRect2->Height > 0)
			pThis->RenderShroud2(pRect2);
	}

	// Idk.
	for (auto& area : Drawing::DirtyAreas.get())
	{
		if (!area.alphabool10)
			continue;

		auto rect = area.Rect;
		rect.X += DSurface::ViewBounds->X;
		rect.Y += DSurface::ViewBounds->Y;

		auto rect2 = Drawing::Intersect(rect, *pRectForIntersect);
		ABuffer::Instance->BlitRect(rect2);
		pThis->RenderShroud2(&rect2);
	}
}

/*
DEFINE_HOOK(0x4F4480, GScreenClass_Render_Replace, 8)
{
	Rendering::Render(MouseClass::Instance);
	return 0x4F45A8;
}
*/

DEFINE_HOOK(0x6D3660, Tactical_RenderShroud_Replace, 5)
{
	GET_STACK(RectangleStruct*, pRect1, 0x4);
	GET_STACK(RectangleStruct*, pRect2, 0x8);
	GET_STACK(RectangleStruct*, pRectForIntersect, 0xC);
	GET_STACK(bool, force, 0x10);
	Rendering::RenderShroud(TacticalClass::Instance, pRect1, pRect2, pRectForIntersect, force);
	return 0x6D386D;
}
