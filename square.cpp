#ifndef __global__
#define __global__
#include "global.h"
#endif

void computeSquareBounceBack_TEST(double *fout, double *fin)
{
    for(int x=xmin1+1;x<xmax1;x++)
    {
      fout[IDX(x,ymin1,7)] = fin[IDX(x,ymin1,5)];
      fout[IDX(x,ymin1,4)] = fin[IDX(x,ymin1,2)];
      fout[IDX(x,ymin1,8)] = fin[IDX(x,ymin1,6)];

      fout[IDX(x,ymax1,5)] = fin[IDX(x,ymax1,7)];
      fout[IDX(x,ymax1,2)] = fin[IDX(x,ymax1,4)];
      fout[IDX(x,ymax1,6)] = fin[IDX(x,ymax1,8)];
    }
  for(int y=ymin1+1;y<ymax1;y++)
    {
      fout[IDX(xmin1,y,6)] = fin[IDX(xmin1,y,8)];
      fout[IDX(xmin1,y,3)] = fin[IDX(xmin1,y,1)];
      fout[IDX(xmin1,y,7)] = fin[IDX(xmin1,y,5)];

      fout[IDX(xmax1,y,8)] = fin[IDX(xmax1,y,6)];
      fout[IDX(xmax1,y,1)] = fin[IDX(xmax1,y,3)];
      fout[IDX(xmax1,y,5)] = fin[IDX(xmax1,y,7)];
    }
  fout[IDX(xmin1,ymin1,7)] = fin[IDX(xmin1,ymin1,5)];
  fout[IDX(xmin1,ymax1,6)] = fin[IDX(xmin1,ymax1,8)];
  fout[IDX(xmax1,ymax1,5)] = fin[IDX(xmax1,ymax1,7)];
  fout[IDX(xmax1,ymin1,8)] = fin[IDX(xmax1,ymin1,6)];


  for(int x=xmin2+1;x<xmax2;x++)
    {
      fout[IDX(x,ymin2,7)] = fin[IDX(x,ymin2,5)];
      fout[IDX(x,ymin2,4)] = fin[IDX(x,ymin2,2)];
      fout[IDX(x,ymin2,8)] = fin[IDX(x,ymin2,6)];

      fout[IDX(x,ymax2,5)] = fin[IDX(x,ymax2,7)];
      fout[IDX(x,ymax2,2)] = fin[IDX(x,ymax2,4)];
      fout[IDX(x,ymax2,6)] = fin[IDX(x,ymax2,8)];
    }
  for(int y=ymin2+1;y<ymax2;y++)
    {
      fout[IDX(xmin2,y,6)] = fin[IDX(xmin2,y,8)];
      fout[IDX(xmin2,y,3)] = fin[IDX(xmin2,y,1)];
      fout[IDX(xmin2,y,7)] = fin[IDX(xmin2,y,5)];

      fout[IDX(xmax2,y,8)] = fin[IDX(xmax2,y,6)];
      fout[IDX(xmax2,y,1)] = fin[IDX(xmax2,y,3)];
      fout[IDX(xmax2,y,5)] = fin[IDX(xmax2,y,7)];
    }
  fout[IDX(xmin2,ymin2,7)] = fin[IDX(xmin2,ymin2,5)];
  fout[IDX(xmin2,ymax2,6)] = fin[IDX(xmin2,ymax2,8)];
  fout[IDX(xmax2,ymax2,5)] = fin[IDX(xmax2,ymax2,7)];
  fout[IDX(xmax2,ymin2,8)] = fin[IDX(xmax2,ymin2,6)];

}
