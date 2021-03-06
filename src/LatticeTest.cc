#include <iostream>
#include <iomanip>
#include <vector>
#include <algorithm>
#include <cmath>
#include <typeinfo>
#include <list>
#include "NTL/ZZ.h"
#include "NTL/LLL.h"
#include "NTL/vec_ZZ.h"
#include "NTL/mat_ZZ.h"

#include "latmrg/IntLattice.h"
#include "latmrg/Merit.h"
#include "latticetester/Util.h"
#include "latmrg/PolyPE.h"
#include "latmrg/LatticeTest.h"
#include "latticetester/Reducer.h"


using namespace std;
using namespace NTL;
using namespace LatticeTester;


///**************************************************************************

namespace LatMRG
{

LatticeTest::LatticeTest (LatMRG::IntLattice * lat): m_merit()
{
   m_lat = lat;
   m_dualF = true;
   m_invertF = false;
   m_maxAllDimFlag = true;
   m_detailF = 0;
   Reducer::maxNodesBB = m_maxNodesBB = 10000000;
   timer.init ();
}


//===========================================================================

LatticeTest::~LatticeTest ()
{
};


//===========================================================================

bool LatticeTest::test (int minDim, int maxDim, double minVal[],
                        const double* weights)
{
   throw "Not implemented with weights";
   // remark: compiler warnings
   //minDim = maxDim = -1;
   //minVal[0] =  weights[0];
}


//===========================================================================

void LatticeTest::resetFromDim(int order, int & fromDim)
{
   if (fromDim <= order) {
      // fromDim = order + 1;
     // cout << "********* WARNING LatticeTest:  fromDim is <= order.\n";
      //  cout << " It is now reset to: fromDim = order + 1\n" << endl;
   }
}


//===========================================================================

void LatticeTest::setDualFlag (bool dualF)
{
   m_dualF = dualF;
   m_lat->fixLatticeNormalization (dualF);
}


//===========================================================================

void LatticeTest::setMaxAllDimFlag (bool maxAllDimF)
{
   m_maxAllDimFlag = maxAllDimF;
}


//===========================================================================

void LatticeTest::setInvertFlag (bool invertF)
{
   m_invertF = invertF;
}


//===========================================================================

void LatticeTest::setDetailFlag (int d)
{
   m_detailF = d;
}


//===========================================================================

void LatticeTest::setMaxNodesBB (long maxNodesBB)
{
   Reducer::maxNodesBB = m_maxNodesBB = maxNodesBB;
}


//===========================================================================

void LatticeTest::dispatchLatUpdate(IntLattice & lat)
{
   ob_list::const_iterator it;
   LatticeTestObserver *ob;

   for (it = m_observers.begin(); it != m_observers.end(); ++it) {
      ob = *it;
      ob->latUpdate(lat);
   }
}


void LatticeTest::dispatchLatUpdate(IntLattice & lat, int i)
{
   ob_list::const_iterator it;
   LatticeTestObserver *ob;

   for (it = m_observers.begin(); it != m_observers.end(); ++it) {
      ob = *it;
      ob->latUpdate(lat, i);
   }
}


void LatticeTest::dispatchResultUpdate(double results[], int n)
{
   ob_list::const_iterator it;
   LatticeTestObserver *ob;

   for (it = m_observers.begin(); it != m_observers.end(); ++it) {
      ob = *it;
      ob->resultUpdate(results, n);
   }
}


void LatticeTest::dispatchTestInit(const string & s, string headers[], int n)
{
   ob_list::const_iterator it;
   LatticeTestObserver *ob;

   for (it = m_observers.begin(); it != m_observers.end(); ++it) {
      ob = *it;
      ob->testInit(s, headers, n);
   }
}


void LatticeTest::dispatchTestCompleted()
{
   ob_list::const_iterator it;
   LatticeTestObserver *ob;

   for (it = m_observers.begin(); it != m_observers.end(); ++it) {
      ob = *it;
      ob->testCompleted();
   }
}


void LatticeTest::dispatchTestFailed(int dim)
{
   ob_list::const_iterator it;
   LatticeTestObserver *ob;

   for (it = m_observers.begin(); it != m_observers.end(); ++it) {
      ob = *it;
      ob->testFailed(dim);
   }
}

}
