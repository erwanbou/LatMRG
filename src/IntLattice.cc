#include "latmrg/IntLattice.h"
#include "latticetester/Util.h"
#include "latticetester/NormaBestLat.h"
#include "latticetester/NormaLaminated.h"
#include "latticetester/NormaRogers.h"
#include "latticetester/NormaMinkL1.h"
#include "latticetester/NormaMinkowski.h"
#include "latticetester/NormaPalpha.h"

#ifdef WITH_NTL
#include "NTL/quad_float.h"
#include "NTL/RR.h"
using namespace NTL;
#endif
using namespace std;
using namespace LatticeTester;


namespace LatMRG
{


//=========================================================================

IntLattice::IntLattice ( MScal modulo, int k, int maxDim, NormType norm ):
   IntLatticeBasis(maxDim, norm)
   {
      m_dim = maxDim;
      m_withDual = true;
      m_modulo = modulo;
      m_order = k;
      init ();
   }

//=========================================================================

IntLattice::IntLattice (const IntLattice & Lat):
   IntLatticeBasis(Lat)
{
   //m_dualbasis.resize(m_dim,m_dim);
   m_withDual = true;
   //m_dualvecNorm.resize(m_dim);
   setDualNegativeNorm();
   m_order = Lat.m_order;
   init ();
}


//=========================================================================


void IntLattice::init ()
{
   int dim = getDim ();
   kill ();
   double temp;
   conv (temp, m_modulo);
   m_lgVolDual2 = new double[dim];
   m_lgm2 = 2.0 * Lg (temp);
   m_lgVolDual2[0] = m_lgm2;
   m_vSI.resize(dim, dim);
   m_wSI.resize(dim, dim);
}

//=========================================================================

void IntLattice::kill ()
{
   if (m_lgVolDual2 == 0)
      return;
   delete [] m_lgVolDual2;
   m_lgVolDual2 = 0;
   m_vSI.clear();

   IntLatticeBasis::kill();
   if (!comp.empty()) {
      for (int s = 0; s < (int) comp.size(); s++)
         delete comp[s];
      comp.clear();
   }
}


//=========================================================================

IntLattice::~IntLattice ()
{
   kill ();
}

//=========================================================================

void IntLattice::incDim ()
{
   IntLattice lattmp(*this);
   int dim = getDim();
   m_basis.resize(dim+1, dim+1);
   m_dualbasis.resize(dim+1, dim+1);
   m_vecNorm.resize(dim+1);
   m_dualvecNorm.resize(dim+1);
   m_vSI.resize(dim+1, dim+1);
   m_wSI.resize(dim+1, dim+1);

   for(int i = 0; i < dim; i++){
      for(int j = 0; j < dim; j++){
         m_basis(i,j) = lattmp.m_basis(i,j);
         m_dualbasis(i,j) = lattmp.m_dualbasis(i,j);
         m_vSI(i,j) = lattmp.m_vSI(i,j);
         m_wSI(i,j) = lattmp.m_wSI(i,j);
      }
      m_vecNorm(i) = lattmp.m_vecNorm(i);
      m_dualvecNorm(i) = lattmp.m_dualvecNorm(i);
   }
   setNegativeNorm(dim);
   setDualNegativeNorm(dim);
   setDim(dim+1);
}

//=========================================================================

void IntLattice::calcLgVolDual2 (double lgm2)
{
   int dim = getDim();
   int rmax = min(m_order, dim);

   m_lgVolDual2[0] = lgm2;
   for (int r = 1; r < rmax; r++)
      m_lgVolDual2[r] = m_lgVolDual2[r - 1] + lgm2;
   // WARNING [David]: one version had `m_order` instead of `rmax`.
   // I am not sure which is the fix and which is the bug.
   for (int r = rmax; r < dim; r++)
      m_lgVolDual2[r] = m_lgVolDual2[r - 1];
}

//=========================================================================

void IntLattice::dualize ()
{
   BMat tmp(m_dim, m_dim);

   tmp = m_basis;
   m_basis = m_dualbasis;
   m_dualbasis = tmp;
   setNegativeNorm ();
   setDualNegativeNorm ();
}

//=========================================================================

void IntLattice::fixLatticeNormalization(bool dualF)
{
   // Normalization factor: dual to primal : M^(k/dim) -> 1/M^(k/dim)
   if (( dualF && m_lgVolDual2[0] < 0.0) ||
       (!dualF && m_lgVolDual2[0] > 0.0)) {
      for (int i = 0; i < getDim(); i++)
         m_lgVolDual2[i] = -m_lgVolDual2[i];
   }
//   for (int i = 1; i <= getMaxDim(); i++)
//      cout << " fix  " << m_lgVolDual2[i] << endl;
}

//=========================================================================

void IntLattice::buildProjection (IntLattice* lattice, const Coordinates & proj)
{
   const int dim = getDim ();
//  cout << "      ESPION_2\n";  getPrimalBasis ().write();
   int i = 0;
   for (Coordinates::const_iterator iter = proj.begin();
        iter != proj.end(); ++iter) {
      for (int j = 0; j < dim; j++)
         lattice->m_dualbasis(j,i) = m_basis(j, *iter);
      ++i;
   }

   lattice->setDim (static_cast<int>(proj.size()));
   lattice->m_order = m_order;

   Triangularization<BMat> (lattice->m_dualbasis, lattice->m_basis, dim, static_cast<int>(proj.size()), m_modulo);
// lattice->trace("\nESPION_4");
/* cout << "  ***** build 2\n";
lattice->getPrimalBasis ().setNegativeNorm (true);
lattice->getPrimalBasis ().updateScalL2Norm (1,proj.size());
lattice->getPrimalBasis ().write();*/
   CalcDual<BMat> (lattice->m_basis, lattice->m_dualbasis, static_cast<int>(proj.size()), m_modulo);
/*
cout << "  ***** build 3\n";
lattice->getDualBasis ().setNegativeNorm (true);
lattice->getDualBasis ().updateScalL2Norm (1,proj.size());
lattice->getDualBasis ().write();
*/
   lattice->setNegativeNorm ();
   lattice->setNegativeNorm ();
}

void IntLattice::buildBasis (int d)
{
   MyExit(1, " buildBasis does nothing");
   d++;  // eliminates compiler warning
}

//=========================================================================

void IntLattice::copy (const IntLattice & lat)
{
   m_order = lat.getOrder();
   m_modulo = lat.m_modulo;
   //m_m2 = lat.m_m2;
   m_basis = lat.m_basis;
   m_dualbasis = lat.m_dualbasis;
   init ();
   for (int i = 0; i < lat.getDim (); i++)
      m_xx[i] = lat.getXX(i);
}

//=========================================================================

Normalizer * IntLattice::getNormalizer (NormaType norma, int alpha)
{
   int dim = getDim();
   Normalizer *normal;

   switch (norma) {
   case BESTLAT:
      normal = new NormaBestLat (m_modulo, m_order, dim);
      break;
   case LAMINATED:
      normal = new NormaLaminated (m_modulo, m_order, dim);
      break;
   case ROGERS:
      normal = new NormaRogers (m_modulo, m_order, dim);
      break;
   case MINKL1:
      normal = new NormaMinkL1 (m_modulo, m_order, dim);
      break;
   case MINKOWSKI:
      normal = new NormaMinkowski (m_modulo, m_order, dim);
      break;
   case NORMA_GENERIC:
      normal = new Normalizer (m_modulo, m_order, dim, "Norma_generic");
      break;
   case PALPHA_N:
      normal = new NormaPalpha (m_modulo, alpha, dim);
      break;
   default:
      cout << "normalizer:   no such case";
      exit (2);
   }
   return normal;
}



}
