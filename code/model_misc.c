#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <gsl/gsl_rng.h>

#include "core_allvars.h"
#include "core_proto.h"



void init_galaxy(int p, int halonr)
{
  int j, outputbin;

  if(halonr != Halo[halonr].FirstHaloInFOFgroup)
  {
    printf("Hah?\n");
    ABORT(1);
  }

  Gal[p].Type = 0;

  Gal[p].GalaxyNr = GalaxyCounter;
  GalaxyCounter++;
  
  Gal[p].HaloNr = halonr;
  Gal[p].MostBoundID = Halo[halonr].MostBoundID;
  Gal[p].SnapNum = Halo[halonr].SnapNum - 1;

  for(j = 0; j < 3; j++)
  {
    Gal[p].Pos[j] = Halo[halonr].Pos[j];
    Gal[p].Vel[j] = Halo[halonr].Vel[j];
  }

  Gal[p].Len = Halo[halonr].Len;
  Gal[p].Vmax = Halo[halonr].Vmax;
  Gal[p].Vvir = get_virial_velocity(halonr);
  Gal[p].Mvir = get_virial_mass(halonr);
  Gal[p].Rvir = get_virial_radius(halonr);


  Gal[p].ColdGas = 0.0;
  Gal[p].StellarMass = 0.0;
  Gal[p].BulgeMass = 0.0;
  Gal[p].HotGas = 0.0;
  Gal[p].EjectedMass = 0.0;
  Gal[p].BlackHoleMass = 0.0;
  Gal[p].ICS = 0.0;

  Gal[p].MetalsColdGas = 0.0;
  Gal[p].MetalsStellarMass = 0.0;
  Gal[p].MetalsBulgeMass = 0.0;
  if(SFprescription == 1) Gal[p].MetalsHotGas = 2.0e-5; else Gal[p].MetalsHotGas = 0.0;
  Gal[p].MetalsEjectedMass = 0.0;
  Gal[p].MetalsICS = 0.0;

  for(outputbin = 0; outputbin < NOUT; outputbin++)
  {
    Gal[p].Sfr[outputbin] = 0.0;
    Gal[p].SfrBulge[outputbin] = 0.0;
    Gal[p].SfrICS[outputbin] = 0.0;
  }

  Gal[p].DiskScaleRadius = get_disk_radius(halonr, p);
  Gal[p].MergTime = 999.9;
  Gal[p].AlreadyMerged = 0;
  Gal[p].Cooling = 0.0;
  Gal[p].Heating = 0.0;

}



double get_disk_radius(int halonr, int p)
{
  double SpinParameter;

  // See Mo, Shude & White (1998) eq12, and using a Bullock style lambda.  Since this is the scale length
  // we take the typical star forming region as 3.0 times this using the Milky Way as an approximate guide

  SpinParameter =
    sqrt(Halo[halonr].Spin[0] * Halo[halonr].Spin[0] + Halo[halonr].Spin[1] * Halo[halonr].Spin[1] +
    Halo[halonr].Spin[2] * Halo[halonr].Spin[2]) / (1.414 * Gal[p].Vvir * Gal[p].Rvir);
    
  return (SpinParameter / 1.414) * Gal[p].Rvir;

}



double get_metallicity(double gas, double metals)
{
  double metallicity;

  if(gas > 0.0 && metals > 0.0)
  {
    metallicity = metals / gas;
    if(metallicity < 1.0)
      return metallicity;
    else
      return 1.0;
  }
  else
    return 0.0;

}



double dmax(double x, double y)
{
  if(x > y)
    return x;
  else
    return y;
}



double get_virial_mass(int halonr)
{
  if(halonr == Halo[halonr].FirstHaloInFOFgroup && Halo[halonr].Mvir)
    return Halo[halonr].Mvir;   /* take spherical overdensity mass estimate */ 
  else
    return Halo[halonr].Len * PartMass;
}



double get_virial_velocity(int halonr)
{
  return sqrt(G * get_virial_mass(halonr) / get_virial_radius(halonr));
}



double get_virial_radius(int halonr)
{
  // return Halo[halonr].Rvir;  // Used for Bolshoi

  double zplus1, hubble_of_z_sq, rhocrit, fac;
  
  zplus1 = 1 + ZZ[Halo[halonr].SnapNum];
  hubble_of_z_sq =
    Hubble * Hubble *(Omega * zplus1 * zplus1 * zplus1 + (1 - Omega - OmegaLambda) * zplus1 * zplus1 +
    OmegaLambda);
  
  rhocrit = 3 * hubble_of_z_sq / (8 * M_PI * G);
  fac = 1 / (200 * 4 * M_PI / 3.0 * rhocrit);
  
  return cbrt(get_virial_mass(halonr) * fac);
}


