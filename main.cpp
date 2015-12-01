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
  int nbOfTimeSteps, nbOfChunks,Lx, Ly;
  int facquVtk, facqU, facquForce;
  double tau, beta;
  double *fin, *fout, *temp, *rho, *ux, *uy;
  double Ma;   //Mach number
  string folderName, inputPopsFileName;
  
  /*Reads input file*/
      ifstream input_file("input.datin");
      input_file >> nbOfChunks;
      input_file >> nbOfTimeSteps;
      input_file >> Lx; Ly = Lx;
      input_file >> tau;
      input_file >> Ma;
      input_file >> folderName;
      input_file >> inputPopsFileName;
      input_file >> facquVtk;
      input_file >> facqU;
      input_file >> facquForce;
      input_file.close();
      
  /* --- Compute or define other parameters --- */
      Dy = 4*Ly + 1, Dx = 2*(Dy-1) + 1;
      xmin = (Dx-1)/2; xmax = xmin + Lx;
      ymin = (Dy-1)/2 - Ly/2; ymax = ymin + Ly;
      double cs = 1./sqrt(3); double rho0 = 1.0;
      double u0 = cs*cs*Ma; double uxSum, uxMean;
      double nu = 1./3.*(tau-0.5);
      double omega = 1.0/tau;
      double F;
      beta = 8*nu*u0/((Dy-1)/2)/((Dy-1)/2);
  
  /* --- | Create folder for storing data | ---  */
      string instru = "mkdir " + folderName;
      system(instru.c_str());
      instru = "mkdir " + folderName + "/vtk_fluid/";
      system(instru.c_str());
      
  /* --- | Create parameters file | --- */
      string openParamFile = folderName + "/parameters.datout";
      ofstream param;
      param.open(openParamFile.c_str());
      param << "Number of timesteps : " << nbOfTimeSteps*nbOfChunks << endl;
      param << "L : "  << Lx << endl;
      param << "Dx : " << Dx << endl;
      param << "Dy : " << Dy << endl;
      param << "tau : " << tau << endl;
      param << "beta : " << beta << endl;
      param.close();

      //string openReFile = folderName + "/re_t.datout";
      string openForceFile = folderName + "/data_force.datout";
      //string openuxFile = folderName + "/ux_t.datout";
      //string openuyFile = folderName + "/uy_t.datout";
      //ofstream ReFile, uxFile, uyFile;
      ofstream forceFile;
      //ReFile.open(openReFile.c_str(), ios::binary);
      forceFile.open(openForceFile.c_str(), ios::binary);
      //uxFile.open(openuxFile.c_str(), ios::binary);
      //uyFile.open(openuyFile.c_str(), ios::binary);

  /* ---- | Allocate populations and fields | --- */

      fin = (double *) memalign(getpagesize(), Dx*Dy*9*sizeof(double));
      fout = (double *) memalign(getpagesize(), Dx*Dy*9*sizeof(double));
      rho = (double *) memalign(getpagesize(), Dx*Dy*sizeof(double));
      ux = (double *) memalign(getpagesize(), Dx*Dy*sizeof(double));
      uy = (double *) memalign(getpagesize(), Dx*Dy*sizeof(double));

      if(inputPopsFileName != "0")
	{
	  ifstream popFile(inputPopsFileName.c_str());
	  cout << "Initialized populations taken from " << inputPopsFileName << endl;
	  for(int x=0;x<Dx;x++)
	    {
	      for(int y=0;y<Dy;y++)
		{
		  for(int k=0;k<9;k++)
		    {
		      popFile >> fin[IDX(x,y,k)];
		    }
		}
	    }
	  popFile.close();
	}
      else
	{
   /* --- Initialize pops to equilibrium value --- */
      initializePopulations(fin, Dx, Dy);
      initializeFields(rho, ux, uy, Dx, Dy);
	}
      
      int dummy2 = 0;
      struct timeval start, end;

      //gettimeofday(&start,NULL);
  /* --- START LBM ---*/
      int tt=0;
      for (int chunkID=0;chunkID<nbOfChunks;chunkID++)
	{
	  for (int lbTimeStepCount=0; lbTimeStepCount<nbOfTimeSteps;lbTimeStepCount++)
	    {
	      if(lbTimeStepCount%(nbOfTimeSteps/100)==0)
		dummy2++; cout<<dummy2<<"%\r"; fflush(stdout);
	      if(lbTimeStepCount%facquVtk==0)
		{
		  write_fluid_vtk(tt, Dx, Dy, rho, ux, uy, folderName.c_str());
		  tt++;
		}
	      streamingAndCollision_POSIX(fin, fout, rho, ux, uy, beta, tau);
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
	      
	      // /*Compute Reynolds number*/
	      // if(lbTimeStepCount%facquRe==0)
	      // 	{      
	      // 	  for(int y=0;y<Dy;y++)
	      // 	    {
	      // 	      uxSum += ux[idx(Dx/4, y)];
	      // 	    }
	      // 	  uxMean = uxSum/Dy;
	      // 	  ReFile << lbTimeStepCount << " " << (uxMean*Ly)/nu << endl;
	      // 	  uxSum=0.0; 
	      // 	}
	      /*Write velocity at a given point*/
	      /*if(lbTimeStepCount%facqU==0)
		{
	      uxFile.write((char*)&ux[idx(Dx/4,Dy/4)], sizeof(double));
	      uyFile.write((char*)&uy[idx(Dx/4,Dy/4)], sizeof(double));
	      }*/
	    }
	}
      //gettimeofday(&end,NULL);
	   //double t = (end.tv_sec - start.tv_sec)*1e6 + (end.tv_usec - start.tv_usec);
	   //cout << t/(1e6)/60 << "min" << endl;
	   //uyFile.close();
	   //uxFile.close();
	   //ReFile.close();
	  forceFile.close();
	  /*End of run - Save populations on disk*/
	  /*and complete parameters file*/
	  string popsFileName = folderName + "/pops.datout";
	  ofstream pops_output_file(popsFileName.c_str());
	  for(int x=0;x<Dx;x++)
	    {
	      for(int y=0;y<Dy;y++)
		{
		  for(int k=0;k<9;k++)
		    {
		      pops_output_file << fin[IDX(x,y,k)] << endl;
		    }
		}
	    }
	  pops_output_file.close();
	  
}



