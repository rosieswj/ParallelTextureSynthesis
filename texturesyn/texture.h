
#include "image.h"
#include <omp.h>

#define CHUNK_SIZE 64
#define EPSILON 0.1
#define SIGMA 1.0
#define BORDER_WIDTH 10

typedef struct {
    double **kernel;
    double **sample;
    int sw;     //sample width
    int sh;     //sample height
    int rw;     //result width
    int rh;     //result height
    int w;      //window
    int r;      //radius
    int xEnd;   //window count in sample (horizontal)
    int yEnd;   //window count in sample (vertical)
} info_t;

typedef struct {
    double **res;   //result array, SIZE = [rw * rh][3]
    bool **flag;    //pixel processed status, SIZE = [rw][rh]
} state_t;

/* computes and stores the gaussian kernel of each of the neighborhood
centered at each position of the output texture*/
void getGaussianKernel(double sigma, int w, double **kernel);

/* return all pixels to be processed in the batch with current radius */
void getTraversalSequence(int *ts, int radius, int cx, int cy);

/* arithmatic operation helpers */
int ID(int x, int y, int w);    //return (x * w + y)
double getSquareDist(double *A, double *B);
string int2str(int x);
int randint(int N);

/* alloc and free data structures */
double *double_alloc(size_t n);

info_t * init_info(double **sample, int sw, int sh, int w, int r);
void freeInfo(info_t *info);

state_t *init_state(info_t *info);
void freeState(state_t *s, info_t *info);

double **getRGB(const Image &img);  //size = (w*h)*3
void deleteRGB(double **pixels, int w, int h);

/* write RGB values to ppm image */
void RGBtoImage(double **pixels, int w, int h, const string &filename);

/* main function that performs texture synthesis */
void synthesize(state_t *s, info_t *info);

/* debug */
void printRGB(double **val, int idx);      //print single RGB value
void printAll(double **val, int w, int h); //print all rgb values in image
void outmsg(const char *fmt, ...);
