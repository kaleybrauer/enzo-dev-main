/***********************************************************************
/
/  COMMUNICATION ROUTINE: TRANSFER ACTIVE PARTICLES
/
/  written by: Greg Bryan
/  date:       December, 1997
/  modified:   May, 2009 by John Wise: optimized version to transfer
/                particles in one sweep with collective calls.
/  modified:   July, 2009 by John Wise: adapted for stars
/  modified:   February, 2012 by John Wise: adapted for active particles
/
/  PURPOSE:
/
************************************************************************/
#ifdef USE_MPI
#endif /* USE_MPI */
 
#include "preincludes.h"
 
#include "ErrorExceptions.h"
#include "macros_and_parameters.h"
#include "typedefs.h"
#include "global_data.h"
#include "Fluxes.h"
#include "GridList.h"
#include "ExternalBoundary.h"
#include "Grid.h"
#include "TopGridData.h"
#include "Hierarchy.h"
#include "LevelHierarchy.h"
#include "CommunicationUtilities.h"
#include "SortCompareFunctions.h"
#include "ActiveParticle.h"

void my_exit(int status);
 
// function prototypes
 
int Enzo_Dims_create(int nnodes, int ndims, int *dims);
int search_lower_bound(int *arr, int value, int low, int high, 
		       int total);

// Remove define.  This method will always be used.
//#define KEEP_PARTICLES_LOCAL
 
int CommunicationTransferActiveParticles
(grid *GridPointer[], int NumberOfGrids, int TopGridDims[])
{

  if (NumberOfGrids == 1)
    return SUCCESS;

  int i, j, jstart, jend, dim, grid, proc, DisplacementCount, ThisCount;
  float ExactDims, ExactCount;

  /* Assume that the grid was split by Enzo_Dims_create, and create a
     map from grid number to an index that is determined from (i,j,k)
     of the grid partitions. */

  int *GridMap = new int[NumberOfGrids];
  int *StartIndex[MAX_DIMENSION];
  int Layout[MAX_DIMENSION], LayoutTemp[MAX_DIMENSION];
  int Rank, grid_num, bin, CenterIndex;
  int *pbin;
  int GridPosition[MAX_DIMENSION], Dims[MAX_DIMENSION];
  FLOAT Left[MAX_DIMENSION], Right[MAX_DIMENSION];

  for (dim = 0; dim < MAX_DIMENSION; dim++) {
    Layout[dim] = 0;
    GridPosition[dim] = 0;
    StartIndex[dim] = NULL;
  }

  GridPointer[0]->ReturnGridInfo(&Rank, Dims, Left, Right); // need rank
  Enzo_Dims_create(NumberOfGrids, Rank, LayoutTemp);

  for (dim = 0; dim < Rank; dim++)
    Layout[Rank-1-dim] = LayoutTemp[dim];
  for (dim = Rank; dim < MAX_DIMENSION; dim++)
    Layout[Rank-1-dim] = 0;

  /* For unequal splits of the topgrid, we need the start indices of
     the partitions.  It's easier to recalculate than to search for
     them. */

  for (dim = 0; dim < Rank; dim++) {

    StartIndex[dim] = new int[Layout[dim]+1];
    ExactDims = float(TopGridDims[dim]) / float(Layout[dim]);
    ExactCount = 0.0;
    DisplacementCount = 0;

    for (i = 0; i < Layout[dim]; i++) {
      ExactCount += ExactDims;
      if (i < Layout[dim]-1)
	ThisCount = nint(0.5*ExactCount)*2 - DisplacementCount;
      else
	ThisCount = nint(ExactCount) - DisplacementCount;
      StartIndex[dim][i] = DisplacementCount;
      DisplacementCount += ThisCount;
    } // ENDFOR i    

    StartIndex[dim][Layout[dim]] = TopGridDims[dim];

  } // ENDFOR dim

  for (grid = 0; grid < NumberOfGrids; grid++) {
    GridPointer[grid]->ReturnGridInfo(&Rank, Dims, Left, Right);
    for (dim = 0; dim < Rank; dim++) {

      if (Layout[dim] == 1) {
	GridPosition[dim] = 0;
      } else {

	CenterIndex = 
	  int(TopGridDims[dim] *
	      (0.5*(Right[dim]+Left[dim]) - DomainLeftEdge[dim]) /
	      (DomainRightEdge[dim] - DomainLeftEdge[dim]));

	GridPosition[dim] = 
	  search_lower_bound(StartIndex[dim], CenterIndex, 0, Layout[dim],
			     Layout[dim]);

      } // ENDELSE

    } // ENDFOR dim
    grid_num = GridPosition[0] + 
      Layout[0] * (GridPosition[1] + Layout[1]*GridPosition[2]);
    GridMap[grid_num] = grid;
  } // ENDFOR grids
 
  int *NumberToMove = new int[NumberOfProcessors];
  ActiveParticleList<ActiveParticleType> SendList;
 
  for (i = 0; i < NumberOfProcessors; i++)
    NumberToMove[i] = 0;

#ifdef DEBUG_CTP
  if (MyProcessorNumber == ROOT_PROCESSOR) 
  for (j = 0; j < NumberOfGrids; j++)
    fprintf(stderr, "Pa(%"ISYM") CTP grid[%"ISYM"] = %"ISYM"\n",
	    MyProcessorNumber, j, GridPointer[j]->ReturnNumberOfStars());
#endif
 
  /* Generate the list of star moves. */

  int Zero = 0;
  for (grid = 0; grid < NumberOfGrids; grid++)
    GridPointer[grid]->CommunicationTransferActiveParticles
      (GridPointer, NumberOfGrids, grid, TopGridDims, NumberToMove, Zero, Zero, 
       SendList, Layout, StartIndex, GridMap, COPY_OUT);

  int TotalNumberToMove = 0;
  for (proc = 0; proc < NumberOfProcessors; proc++)
    TotalNumberToMove += NumberToMove[proc];

  int NumberOfReceives = 0;

  /* Sort by grid number.  We no longer share with other processors
     because the stars are stored locally until
     CommunicationCollectParticles(SIBLINGS_ONLY). */

  NumberOfReceives = TotalNumberToMove;
  SendList.sort_grid(0, NumberOfReceives);

  /* Copy stars back to grids */

  jstart = 0;
  jend = 0;

  // Copy shared stars to grids, if any
  if (NumberOfReceives > 0) {
    for (j = 0; j < NumberOfGrids && jend < NumberOfReceives; j++) {
      while (SendList[jend]->ReturnGridID() <= j) {
	jend++;
	if (jend == NumberOfReceives) break;
      }

      GridPointer[j]->CommunicationTransferActiveParticles
	(GridPointer, NumberOfGrids, j, TopGridDims, NumberToMove, 
	 jstart, jend, SendList, Layout, StartIndex, GridMap, COPY_IN);

      jstart = jend;
    } // ENDFOR grids
  } // ENDIF NumberOfRecieves > 0

  /* Cleanup. */

  delete [] NumberToMove;
  delete [] GridMap;
  for (dim = 0; dim < Rank; dim++)
    delete [] StartIndex[dim];

  CommunicationSumValues(&TotalNumberToMove, 1);
  if (debug)
    printf("CommunicationTransferActiveParticles: moved = %"ISYM"\n",
  	   TotalNumberToMove);

  return SUCCESS;
}

