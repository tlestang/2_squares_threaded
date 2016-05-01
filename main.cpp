#include <iostream>
#include <cmath>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <iostream>
#include <cmath>
#include <fstream>
#include <sstream>
#include <malloc.h>
#include <unistd.h>
#include <sys/time.h>

#ifndef __global__
#define __global__
#include "global.h"
#endif


#include "initialize_lattice_arrays.h"
#include "streamCollCompute.h"
#include "boundaryConditions.h"
#include "force.h"
#include "write_vtk.h"

using namespace std;

int c[9][2] = {{0,0}, {1,0}, {0,1}, {-1,0}, {0,-1}, {1,1}, {-1,1}, {-1,-1}, {1,-1}};
double w[9]={4.0/9.0, 1.0/9.0, 1.0/9.0, 1.0/9.0, 1.0/9.0, 1.0/36.0, 1.0/36.0, 1.0/36.0, 1.0/36.0};
int Dx, Dy, xmin, xmax, ymin, ymax;

int main()
{
  /*Parameters for LB simulation*/
  int Lx, Ly;
  double T;
  int facquVtk, facqU, facquForce, facquPops;
  double tau, beta0, U0;
  double *fin, *fout, *temp, *rho, *ux, *uy;
  double Ma;   //Mach number
  string folderName, inputPopsFileName;
  
  /*Reads input file*/
      ifstream input_file("input.datin");
      //input_file >> nbOfChunks;
      input_file >> T;
      input_file >> Lx; Ly = Lx;
      input_file >> tau;
      input_file >> U0;
      input_file >> folderName;
      input_file >> inputPopsFileName;
      input_file >> facquVtk;
      input_file >> facqU;
      input_file >> facquForce;
      input_file >> facquPops;
      input_file.close();
      
  /* --- Compute or define other parameters --- */
      Dy = 4*Ly + 1, Dx = Dy; //2*(Dy-1) + 1;
      xmin = (Dx-1)/2; xmax = xmin + Lx;
      ymin = (Dy-1)/2 - Ly/2; ymax = ymin + Ly;
      double cs = 1./sqrt(3); double rho0 = 1.0;
      double u0 = cs*cs*Ma; double uxSum, uxMean;
      double nu = 1./3.*(tau-0.5);
      double omega = 1.0/tau;

      double F, T0, F0;

      //COMPUTE CHARESTICTC VELOCITY AND TIME
      beta0 = (1./(Dx-1))*((double)Lx/(Dy-1))*U0*U0;
      T0 = Lx/U0;
      F0 = (U0*U0)*(Lx-1)*0.5;
   
      double delta_t = 1.0/T0; //LBM time steps in units of physical time T0
      int nbOfTimeSteps = floor(T*T0);
  
  /* --- | Create folder for storing data | ---  */
      string popsFileName;
      string instru = "mkdir " + folderName;
      ofstream pops_output_file;
      system(instru.c_str());
      instru = "mkdir " + folderName + "/vtk_fluid/";
      system(instru.c_str());
      instru = "mkdir " + folderName + "/populations/";
      system(instru.c_str());
      
  /* --- | Create parameters file | --- */
      string openParamFile = folderName + "/parameters.datout";
      ofstream param;
      param.open(openParamFile.c_str());
      param << "Total simulation time : " << T << endl;
      param << "Timesteps : " << nbOfTimeSteps << endl;
      param << "L : "  << Lx << endl;
      param << "Dx : " << Dx << endl;
      param << "Dy : " << Dy << endl;
      param << "tau : " << tau << endl;
      param << "U0 : " << U0 << endl;
      param << "beta0 : " << beta0 << endl;
      param.close();


      string openForceFile = folderName + "/data_force.datout";
      string openuxFile = folderName + "/ux_t.datout";
      string openrhoFile = folderName + "/rho_t.datout";
      ofstream forceFile, uxFile, rhoFile;
      forceFile.open(openForceFile.c_str(), ios::binary);
      uxFile.open(openuxFile.c_str(), ios::binary);
      rhoFile.open(openrhoFile.c_str(), ios::binary);

  /* ---- | Allocate populations and fields | --- */

      fin = (double *) memalign(getpagesize(), Dx*Dy*9*sizeof(double));
      fout = (double *) memalign(getpagesize(), Dx*Dy*9*sizeof(double));
      rho = (double *) memalign(getpagesize(), Dx*Dy*sizeof(double));
      ux = (double *) memalign(getpagesize(), Dx*Dy*sizeof(double));
      uy = (double *) memalign(getpagesize(), Dx*Dy*sizeof(double));

      if(inputPopsFileName != "0")
	{
	  ifstream popFile(inputPopsFileName.c_str(), ios::binary);
	  cout << "Initialized populations taken from " << inputPopsFileName << endl;
	  popFile.read((char*)&fin[0], Dx*Dy*9*sizeof(double));
	  popFile.close();
	}
      else
	{
   /* --- Initialize pops to equilibrium value --- */
      initializePopulations(fin, Dx, Dy);
      initializeFields(fin, rho, ux, uy, Dx, Dy);
	}
      
      int dummy2 = 0;

  /* --- START LBM ---*/
      int tt=0;
      // for (int chunkID=0;chunkID<nbOfChunks;chunkID++)
      // 	{
	  for (int lbTimeStepCount=0; lbTimeStepCount<nbOfTimeSteps;lbTimeStepCount++)
	    {
	      if(lbTimeStepCount%(nbOfTimeSteps/100)==0)
		{
		dummy2++; cout<< "Running : " << dummy2<<"%" << endl;
		}

	      streamingAndCollision_POSIX(fin, fout, rho, ux, uy, beta0, tau);
	      computeDomainNoSlipWalls_BB(fout, fin);
	      computeSquareBounceBack_TEST(fout, fin);
	      /*Reset square nodes to equilibrium*/
	      for(int x=xmin+1;x<xmax;x++)
		{
		  for(int y=ymin+1;y<ymax;y++)
		    {
		      for(int k=0;k<9;k++)
			{
			  fout[IDX(x,y,k)] = w[k];
			}
		    }
		}
	      /*Swap populations*/
	      temp = fin;
	      fin = fout;
	      fout = temp;

	      /* --- Compute and Write force on disk --- */
	      if(lbTimeStepCount%facquForce==0)
		{
		  F = computeForceOnSquare(fin, omega);
		  forceFile.write((char*)&F, sizeof(double));
		}
	      if(lbTimeStepCount%facquVtk==0)
		{
		  write_fluid_vtk(tt, Dx, Dy, rho, ux, uy, folderName.c_str());
		  tt++;
		}
	      
	      /*Write velocity at a given point*/
	      if(lbTimeStepCount%facqU==0)
		{
	      uxFile.write((char*)&ux[idx(Dx/4,Dy/4)], sizeof(double));
		}

	      /*Write velocity at a given point*/
	      if(lbTimeStepCount%facqU==0)
		{
		  rhoFile.write((char*)&ux[idx(xmin-2,Dy/2)], sizeof(double));
		}
	      
	      // if(lbTimeStepCount%facquPops==0)
	      // 	{
	      // 	  stringstream fileName;
	      // 	  fileName << "pops_" << lbTimeStepCount << ".datout";
	      // 	  popsFileName = folderName+"/populations/"+fileName.str();
	      // 	  pops_output_file.open(popsFileName.c_str(), ios::binary);
	      // 	  pops_output_file.write((char*)&fin[0], Dx*Dy*9*sizeof(double));
	      // 	  pops_output_file.close();
	      // 	}
	    }
	  //}
	  uxFile.close();
	  forceFile.close();
	  /*End of run - Save populations on disk*/
	  /*and complete parameters file*/
	  popsFileName = folderName + "/pops_final.datout";
	  pops_output_file.open(popsFileName.c_str(), ios::binary);
	  pops_output_file.write((char*)&fin[0], Dx*Dy*9*sizeof(double));
	  pops_output_file.close();
	  
}



