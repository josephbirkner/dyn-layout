using namespace NUnit::Framework;

#define _AFXDLL
#include <afxwin.h>
#undef _AFXDLL

#include "Base/DynLayout/GlobExport/geometry.h"

[TestFixture]
ref class GeomertryTest
{
public:
	[SetUp]
	void Setup()
	{}
	
	[TearDown]
	void TearDown()
	{}
	
	// [0]      
	//    [F]   
	//          
	[TestCase(  0,   0,   0,   0, Result = false)]
	
	//          
	//    [0]   
	//          
	[TestCase(100, 100,   0,   0, Result = false)]
	
	// [V]      
	//    [F]   
	//          
	[TestCase(  0,   0, 100, 100, Result = false)]
	
	//    [V]   
	//    [F]   
	//          
	[TestCase(100,   0, 100, 100, Result = false)]
	
	//       [V]
	//    [F]   
	//          
	[TestCase(200,   0, 100, 100, Result = false)]
	
	//          
	// [V][F]   
	//          
	[TestCase(  0, 100, 100, 100, Result = false)]
	
	//          
	//    [F][V]
	//          
	[TestCase(200, 100, 100, 100, Result = false)]
	
	//          
	//    [F]   
	// [V]      
	[TestCase(  0, 200, 100, 100, Result = false)]
	
	//          
	//    [F]   
	//    [V]   
	[TestCase(100, 200, 100, 100, Result = false)]
	
	//          
	//    [F]   
	//       [V]
	[TestCase(200, 200, 100, 100, Result = false)]
	
	// +---+
	// |V/F|
	// +---+
	//      
	[TestCase(100, 100, 100, 100, Result = true)]
	
	// +---+
	// | V |-+
	// +---+ |
	//   +---+
	// 
	[TestCase( 50,  50, 100, 100, Result = true)]
	
	//   +---+
	// +-| V |
	// | +---+
	// +---+
	// 
	[TestCase(150,  50, 100, 100, Result = true)]
	
	//   +---+
	// +---+ |
	// | V |-+
	// +---+
	// 
	[TestCase( 50, 150, 100, 100, Result = true)]
	
	// +---+
	// | +---+
	// +-| V |
	//   +---+
	// 
	[TestCase(150, 150, 100, 100, Result = true)]
	bool rectIntersetsRect(int x, int y, int width, int height)
	{
		CRect fixedRect(100, 100, 200, 200);
		CRect varRect(x, y, x + width, y + height);
		
		return Layout::RectIntersectsRect(varRect, fixedRect);
	}
	
	// [0]      
	//    [F]   
	//          
	[TestCase(  0,   0,   0,   0,   0,   0,   0,   0)]
	
	//          
	//    [0]   
	//          
	[TestCase(100, 100,   0,   0,   0,   0,   0,   0)]
	
	// [V]      
	//    [F]   
	//          
	[TestCase(  0,   0, 100, 100,   0,   0,   0,   0)]
	
	//    [V]   
	//    [F]   
	//          
	[TestCase(100,   0, 100, 100,   0,   0,   0,   0)]
	
	//       [V]
	//    [F]   
	//          
	[TestCase(200,   0, 100, 100,   0,   0,   0,   0)]
	
	//          
	// [V][F]   
	//          
	[TestCase(  0, 100, 100, 100,   0,   0,   0,   0)]
	
	//          
	//    [F][V]
	//          
	[TestCase(200, 100, 100, 100,   0,   0,   0,   0)]
	
	//          
	//    [F]   
	// [V]      
	[TestCase(  0, 200, 100, 100,   0,   0,   0,   0)]
	
	//          
	//    [F]   
	//    [V]   
	[TestCase(100, 200, 100, 100,   0,   0,   0,   0)]
	
	//          
	//    [F]   
	//       [V]
	[TestCase(200, 200, 100, 100,   0,   0,   0,   0)]
	
	// +---+
	// |V/F|
	// +---+
	//      
	[TestCase(100, 100, 100, 100, 100, 100, 100, 100)]
	
	// +---+
	// | V |-+
	// +---+ |
	//   +---+
	// 
	[TestCase( 50,  50, 100, 100, 100, 100,  50,  50)]
	
	//   +---+
	// +-| V |
	// | +---+
	// +---+
	// 
	[TestCase(150,  50, 100, 100, 150, 100,  50,  50)]
	
	//   +---+
	// +---+ |
	// | V |-+
	// +---+
	// 
	[TestCase( 50, 150, 100, 100, 100, 150,  50,  50)]
	
	// +---+
	// | +---+
	// +-| V |
	//   +---+
	//
	[TestCase(150, 150, 100, 100, 150, 150,  50,  50)]
	void rectGetIntersectionRect(int x1, int y1, int width1, int height1, int xr, int yr, int widthr, int heightr)
	{
		CRect fixedRect(100, 100, 200, 200);
		CRect varRect(x1, y1, x1 + width1, y1 + height1);
		CRect resRect(xr, yr, xr + widthr, yr + heightr);
		
		Assert::IsTrue(resRect == Layout::RectGetIntersectionRect(varRect, fixedRect));
	}
};