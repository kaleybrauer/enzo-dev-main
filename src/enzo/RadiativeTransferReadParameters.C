/***********************************************************************
/
/  READS RADIATIVE TRANSFER PARAMETERS FROM INPUT FILE
/
/  written by: Tom Abel
/  date:       April, 2004
/  modified1:
/
/  PURPOSE: 
/
/  NOTE: modeled after CosmologyReadParameters
/
************************************************************************/

#ifdef USE_GRACKLE
extern "C" {
#include <grackle.h>
} // USE_GRACKLE
#endif
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "ErrorExceptions.h"
#include "macros_and_parameters.h"
#include "typedefs.h"
#include "global_data.h"

int RadiativeTransferReadParameters(FILE *fptr)
{
  int i;

  char line[MAX_LINE_LENGTH], *dummy = new char[MAX_LINE_LENGTH];
  dummy[0] = 0;

  /* Set defaults. */

  RadiationPressure           = FALSE;             // off
  RadiationPressureScale      = 1.0;
  PhotonTime                  = 0; 
  dtPhoton                    = FLOAT_UNDEFINED;
  for (i = 0; i < 4; i++) {
    EscapedPhotonCount[i] = 0.0;
    TotalEscapedPhotonCount[i] = 0.0;
  }
  PhotonEscapeFilename   = NULL;
  GlobalRadiationSources = new RadiationSourceEntry;
  GlobalRadiationSources->NextSource = NULL;
  GlobalRadiationSources->PreviousSource = NULL;
  SourceClusteringTree = NULL;
  OldSourceClusteringTree = NULL;

  RadiativeTransferSourceRadius               = 0;
  RadiativeTransferPropagationSpeedFraction   = 1.0;
  RadiativeTransferPropagationDistance        = 0.1;
  RadiativeTransferCoupledRateSolver          = TRUE;
  RadiativeTransferOpticallyThinH2            = TRUE;
  RadiativeTransferOpticallyThinH2CharLength  = 0.25;
  RadiativeTransferFluxBackgroundLimit        = 0.01;
  RadiativeTransferSplitPhotonRadius          = FLOAT_UNDEFINED; // kpc
  RadiativeTransferRaysPerCell                = 5.1;
  RadiativeTransferInitialHEALPixLevel        = 3;
  RadiativeTransferPhotonEscapeRadius         = 0.0;   // kpc
  RadiativeTransferInterpolateField           = FALSE;
  RadiativeTransferSourceClustering           = FALSE;
  RadiativeTransferOpticallyThinSourceClustering = FALSE;
  RadiativeTransferPhotonMergeRadius          = 3.0;
  RadiativeTransferTimestepVelocityLimit      = 100.0; // km/s
  RadiativeTransferTimestepVelocityLevel      = INT_UNDEFINED;
  RadiativeTransferPeriodicBoundary           = FALSE;
  RadiativeTransferFLDCallOnLevel             = 0;
  RadiativeTransferHIIRestrictedTimestep      = FALSE;
  RadiativeTransferAdaptiveTimestep           = FALSE;
  RadiativeTransferHydrogenOnly               = FALSE;
  RadiativeTransferTraceSpectrum              = FALSE;
  RadiativeTransferTraceSpectrumTable         = (char*) "spectrum_table.dat";
  RadiativeTransferSourceBeamAngle            = 30.0;
  RadiativeTransferLoadBalance                = FALSE;
  RadiativeTransferRayMaximumLength           = 1.7320508; //sqrt(3.0)
  RadiativeTransferUseH2Shielding             = TRUE;
  RadiativeTransferH2ShieldType               = 0;
  RadiativeTransferH2IIDiss                   = TRUE;
  RadiativeTransferHubbleTimeFraction         = 0.1;
  
  if (MultiSpecies == 0)
    RadiativeTransferOpticallyThinH2 = FALSE;

  /* read input from file */

  while (fgets(line, MAX_LINE_LENGTH, fptr) != NULL) {

    int ret = 0;

    /* read parameters */
    
    ret += sscanf(line, "RadiativeTransferRadiationPressure = %"ISYM, 
		  &RadiationPressure);
    ret += sscanf(line, "RadiativeTransferRadiationPressureScale = %"FSYM, 
		  &RadiationPressureScale);
    ret += sscanf(line, "RadiativeTransferSourceRadius = %"FSYM, 
		  &RadiativeTransferSourceRadius);
    ret += sscanf(line, "RadiativeTransferPropagationSpeedFraction = %"FSYM, 
		  &RadiativeTransferPropagationSpeedFraction);
    ret += sscanf(line, "RadiativeTransferPropagationDistance = %"FSYM, 
		  &RadiativeTransferPropagationDistance);
    ret += sscanf(line, "RadiativeTransferCoupledRateSolver = %"ISYM, 
		  &RadiativeTransferCoupledRateSolver);
    ret += sscanf(line, "RadiativeTransferOpticallyThinH2 = %"ISYM, 
		  &RadiativeTransferOpticallyThinH2);
    ret += sscanf(line, "RadiativeTransferOpticallyThinH2CharLength = %"FSYM, 
		  &RadiativeTransferOpticallyThinH2CharLength);
    ret += sscanf(line, "RadiativeTransferPeriodicBoundary = %"ISYM, 
		  &RadiativeTransferPeriodicBoundary);
    ret += sscanf(line, "RadiativeTransferSplitPhotonRadius = %"FSYM, 
		  &RadiativeTransferSplitPhotonRadius);
    ret += sscanf(line, "RadiativeTransferFluxBackgroundLimit = %"FSYM, 
		  &RadiativeTransferFluxBackgroundLimit);
    ret += sscanf(line, "RadiativeTransferRaysPerCell = %"FSYM, 
		  &RadiativeTransferRaysPerCell);
    ret += sscanf(line, "RadiativeTransferTimestepVelocityLimit = %"FSYM, 
		  &RadiativeTransferTimestepVelocityLimit);
    ret += sscanf(line, "RadiativeTransferTimestepVelocityLevel = %"ISYM, 
		  &RadiativeTransferTimestepVelocityLevel);
    ret += sscanf(line, "RadiativeTransferInitialHEALPixLevel = %"ISYM, 
		  &RadiativeTransferInitialHEALPixLevel);
    ret += sscanf(line, "RadiativeTransferPhotonEscapeRadius = %"FSYM, 
		  &RadiativeTransferPhotonEscapeRadius);
    ret += sscanf(line, "RadiativeTransferInterpolateField = %"ISYM, 
		  &RadiativeTransferInterpolateField);
    ret += sscanf(line, "RadiativeTransferSourceClustering = %"ISYM, 
		  &RadiativeTransferSourceClustering);
    ret += sscanf(line, "RadiativeTransferOpticallyThinSourceClustering = %"ISYM,
                  &RadiativeTransferOpticallyThinSourceClustering);
    ret += sscanf(line, "RadiativeTransferPhotonMergeRadius = %"FSYM, 
		  &RadiativeTransferPhotonMergeRadius);
    ret += sscanf(line, "RadiativeTransferFLDCallOnLevel = %"ISYM, 
		  &RadiativeTransferFLDCallOnLevel);
    ret += sscanf(line, "RadiativeTransferSourceBeamAngle = %"FSYM, 
		  &RadiativeTransferSourceBeamAngle);
    ret += sscanf(line, "RadiativeTransferHIIRestrictedTimestep = %"ISYM, 
		  &RadiativeTransferHIIRestrictedTimestep);
    ret += sscanf(line, "RadiativeTransferAdaptiveTimestep = %"ISYM, 
		  &RadiativeTransferAdaptiveTimestep);
    ret += sscanf(line, "RadiativeTransferHydrogenOnly = %"ISYM, 
		  &RadiativeTransferHydrogenOnly);
    ret += sscanf(line, "RadiativeTransferH2ShieldType = %"ISYM, 
		  &RadiativeTransferH2ShieldType);
    ret += sscanf(line, "RadiativeTransferH2IIDiss = %"ISYM, 
		  &RadiativeTransferH2IIDiss);
    ret += sscanf(line, "RadiativeTransferUseH2Shielding = %"ISYM, 
		  &RadiativeTransferUseH2Shielding);
    ret += sscanf(line, "RadiativeTransferTraceSpectrum = %"ISYM, 
		  &RadiativeTransferTraceSpectrum);
    ret += sscanf(line, "RadiativeTransferLoadBalance = %"ISYM, 
		  &RadiativeTransferLoadBalance);
    ret += sscanf(line, "RadiativeTransferRayMaximumLength = %"FSYM, 
		  &RadiativeTransferRayMaximumLength);
    ret += sscanf(line, "RadiativeTransferHubbleTimeFraction = %"FSYM, 
		  &RadiativeTransferHubbleTimeFraction);
    if (sscanf(line, "RadiativeTransferTraceSpectrumTable = %s", dummy) == 1)
      RadiativeTransferTraceSpectrumTable = dummy;  
    ret += sscanf(line, "dtPhoton = %"FSYM, &dtPhoton);

    /* If the dummy char space was used, then make another. */
 
    if (*dummy != 0) {
      dummy = new char[MAX_LINE_LENGTH];
      dummy[0] = 0;
      ret++;
    }
 
    /* if the line is suspicious, issue a warning */

    if (ret == 0 && strstr(line, "=") != NULL && line[0] != '#' && 
	MyProcessorNumber == ROOT_PROCESSOR &&
	strstr(line, "RadiativeTransfer") && !strstr(line, "RadiativeTransfer ")
	&& !strstr(line, "RadiativeTransferFLD "))
      fprintf(stderr, "warning: the following parameter line was not "
	      "interpreted:\n%s\n", line);
  }

  /* Error check */

  /* Check if H2 cooling is turned on for Lyman-Werner radiation. */

  if (RadiativeTransferOpticallyThinH2 && MultiSpecies < 2) {
    if (MyProcessorNumber == ROOT_PROCESSOR)
      fprintf(stderr, "Warning: optically thin Lyman-Werner radiation turned on "
	      "without H2 cooling.  Setting LW radiation OFF.\n");
    RadiativeTransferOpticallyThinH2 = FALSE;
  }

  /* Check if we're simultaneously using FLD and 1/r^2 Lyman-Werner radiation */

  if (RadiativeTransferOpticallyThinH2 && RadiativeTransferFLD) {
    if (MyProcessorNumber == ROOT_PROCESSOR)
      fprintf(stderr, "Warning: optically thin Lyman-Werner radiation and FLD "
	      "turned on.  Turning the optically thin radiation OFF.\n");
    RadiativeTransferOpticallyThinH2 = FALSE;
  }

  if (RadiativeTransferFLDCallOnLevel < 0) {
    if (MyProcessorNumber == ROOT_PROCESSOR)
      fprintf(stderr, "Warning: RadiativeTransferFLDCallOnLevel = %"ISYM
	      " cannot be negative!  Setting to 0.\n");
    RadiativeTransferFLDCallOnLevel = 0;
  }


  // If RadiativeTransferFLD > 1, turn off RadiativeTransfer
  if (RadiativeTransferFLD > 1  &&  RadiativeTransfer) {
    if (MyProcessorNumber == ROOT_PROCESSOR)
      fprintf(stderr, "Warning: RadiativeTransferFLD > 1 cannot be used with "
	      "RadiativeTransfer.  Turning ray-tracing solver OFF.\n");
    RadiativeTransfer = FALSE;
  }


//   // If RadiativeTransferFLD > 1, turn off RadiativeCooling
//   if (RadiativeTransferFLD > 1  &&  RadiativeCooling) {
//     if (MyProcessorNumber == ROOT_PROCESSOR)
//       fprintf(stderr, "Warning: RadiativeTransferFLD > 1 cannot be used with "
// 	      "RadiativeCooling solver.  Turning RadiativeCooling OFF.\n");
//     RadiativeCooling = 0;
//   }

  // If RadiativeTransferFLD > 1, reset RadiationFieldType (if necessary)
  if (RadiativeTransferFLD > 1  &&  (RadiationFieldType != 0)) {
    if (MyProcessorNumber == ROOT_PROCESSOR)
      fprintf(stderr, "Warning: RadiativeTransferFLD > 1 cannot be used with "
	      "RadiationFieldType != 0.  Resetting RadiationFieldType.\n");
    RadiationFieldType = 0;
  }

  // If RadiativeTransferFLD > 1, ensure that ImplicitProblem > 0
  if (RadiativeTransferFLD > 1  &&  (ImplicitProblem == 0)) 
    ENZO_FAIL("Error: RadiativeTransferFLD > 1 requires ImplicitProblem > 0!");

  // Make sure source clustering is turned on if optically thin source clustering
  // is to be used. Throw an error just in case.
  if (!RadiativeTransferSourceClustering && RadiativeTransferOpticallyThinSourceClustering)
    ENZO_FAIL("Error: RadiativeTransferSourceClustering must be turned on to use optically thin source clustering");

  if (RadiativeTransferOpticallyThinSourceClustering &&
      (RadiativeTransferPhotonMergeRadius > 5.0)){
    fprintf(stderr, "Warning: Caution when using optically thin source clustering and "
                    "values for RadiativeTransferPhotonMergeRadius > 5. This may result in missing "
                    "radiation near sources.\n");
  }

#ifdef USE_GRACKLE
  // Set some radiative transfer grackle parameters.
  if (grackle_data->use_grackle == TRUE) {
    grackle_data->radiative_transfer_coupled_rate_solver = (Eint32) RadiativeTransferCoupledRateSolver;
    grackle_data->radiative_transfer_hydrogen_only       = (Eint32) RadiativeTransferHydrogenOnly;
  }
#endif // USE_GRACKLE

  delete [] dummy;

  return SUCCESS;
}
