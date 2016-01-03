using namespace NUnit::Framework;

#define _AFXDLL
#include <afxwin.h>
#undef _AFXDLL

#include "Base/DynLayout/GlobExport/area.h"
#include "Base/DynLayout/GlobExport/control.h"
#include "Base/DynLayout/GlobExport/manager.h"
#include "Base/DynLayout/GlobExport/alignment.h"

// Asserts sollen bei diesen testfällen nicht angezeigt werden.
// U.a. sind mehrere Asserts gewollt.
int FakeReportHook(int reportType, char *message, int *returnValue)
{
	return TRUE;
}

class LayoutTest
{
public:
	static Layout::Control* CreateControl()
	{
		return new Layout::Control(NULL, NULL, Layout::Align::TopLeft(), Layout::Align::TopLeft());
	}
	
	static void ControlSetOrigRect(Layout::Control* control, const CRect& rect)
	{
		control->m_rctOrig = rect;
	}
	
	static Layout::Area* CreateArea(const CRect& frame)
	{
		return new Layout::Area((const Layout::Manager *)NULL, frame, CSize(0, 0), CSize(0, 0));
	}
	
	static void AreaSetOrigRect(Layout::Area* area, const CRect& rect)
	{
		area->m_rctOrigClientShape = rect;
	}
	
	static void AreaSetClientRect(Layout::Area* area, const CRect& rect)
	{
		area->m_rctCurrentClientShape = rect;
	}
};

[TestFixture]
ref class AlignmentTest
{
public:
	[SetUp]
	void Setup()
	{
		_CrtSetReportHook(&FakeReportHook);
	}
	
	[TearDown]
	void TearDown()
	{}
	
	[Test]
	void topLeft()
	{
		Layout::Control* control = LayoutTest::CreateControl();
		Layout::Area* area = LayoutTest::CreateArea(CRect(0, 0, 100, 100));
		
		LayoutTest::ControlSetOrigRect(control, CRect(25, 25, 75, 75));
		LayoutTest::AreaSetOrigRect(area, CRect(0, 0, 100, 100));
		Layout::Align::TopLeft alignment;
		CRect outRect(25, 25, 75, 75);
		
		control->setAlignmentArea(area);
		
		LayoutTest::AreaSetClientRect(area, CRect(0, 0, 100, 100));
		
		alignment.update(control, Layout::Align::Horizontal, outRect);
		Assert::IsTrue(outRect == CRect(25, 25, 75, 75));
		
		LayoutTest::AreaSetClientRect(area, CRect(0, 0, 200, 200));
		
		alignment.update(control, Layout::Align::Horizontal, outRect);
		Assert::IsTrue(outRect == CRect(25, 25, 75, 75));
	}
	
	[Test]
	void bottomRight()
	{
		Layout::Control* control = LayoutTest::CreateControl();
		Layout::Area* area = LayoutTest::CreateArea(CRect(0, 0, 100, 100));
		
		LayoutTest::ControlSetOrigRect(control, CRect(25, 25, 75, 75));
		LayoutTest::AreaSetOrigRect(area, CRect(0, 0, 100, 100));
		Layout::Align::BottomRight alignment;
		CRect outRect(25, 25, 75, 75);
		
		control->setAlignmentArea(area);
		
		LayoutTest::AreaSetClientRect(area, CRect(0, 0, 100, 100));
		
		alignment.update(control, Layout::Align::Horizontal, outRect);
		Assert::IsTrue(outRect == CRect(25, 25, 75, 75));
		
		LayoutTest::AreaSetClientRect(area, CRect(0, 0, 200, 200));
		
		alignment.update(control, Layout::Align::Horizontal, outRect);
		Assert::IsTrue(outRect == CRect(125, 25, 175, 75));
	}
	
	[Test]
	void resize()
	{
		Layout::Control* control = LayoutTest::CreateControl();
		Layout::Area* area = LayoutTest::CreateArea(CRect(0, 0, 100, 100));
		
		LayoutTest::ControlSetOrigRect(control, CRect(25, 25, 75, 75));
		LayoutTest::AreaSetOrigRect(area, CRect(0, 0, 100, 100));
		Layout::Align::Resize alignment;
		CRect outRect(25, 25, 75, 75);
		
		control->setAlignmentArea(area);
		
		LayoutTest::AreaSetClientRect(area, CRect(0, 0, 100, 100));
		
		alignment.update(control, Layout::Align::Horizontal, outRect);
		Assert::IsTrue(outRect == CRect(25, 25, 75, 75));
		
		LayoutTest::AreaSetClientRect(area, CRect(0, 0, 200, 200));
		
		alignment.update(control, Layout::Align::Horizontal, outRect);
		Assert::IsTrue(outRect == CRect(25, 25, 175, 75));
	}
};