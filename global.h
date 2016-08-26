#ifdef FLIPPED
#define IDX(i,j,k) ((j) + Dy*( k + 9*(i) ) )
#else
#define IDX(i,j,k) ((k) + 9*( (j)+ Dy*(i) ) )
#endif

#define idx(i,j) (j+Dy*i)

#define min(a,b) ({ typeof (a) _a = (a); typeof (b) _b = (b); _a < _b ? _a : _b; })

extern int Dx;
extern int Dy;
extern int xmin1, xmax1, ymin1, ymax1;
extern int xmin2, xmax2, ymin2, ymax2;
extern double w[9];
extern int c[9][2];

