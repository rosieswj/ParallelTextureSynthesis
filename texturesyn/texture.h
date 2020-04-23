
#include "image.h"
#include <omp.h>

#define CHUNK_SIZE 64
#define EPSILON 0.1
#define SIGMA 1.0


typedef struct {
    // double **res;
    double **kernel;
    double **sample;
    // double **sample;
    int sw;
    int sh;
    int rw;
    int rh;
    int w;
    int r;
    int xEnd;
    int yEnd;
} info_t;

typedef struct {
    double **res;
    bool **flag;
} state_t;


//test sftp
void getGaussianKernel(double sigma, int w, double **kernel);
void getTraversalSequence(int *ts, int radius, int cx, int cy);

int ID(int x, int y, int w);
double getSquareDist(double *A, double *B);

string int2str(int x);
int randint(int N);
double *double_alloc(size_t n);

info_t * init_info(double **sample, int sw, int sh, int w, int r);
void freeInfo(info_t *info);

state_t *init_state(info_t *info);
void freeState(state_t *s, info_t *info);

//get RGB values from image, size = (w*h)*3
double **getRGB(const Image &img);
//free pixel array, size = w*h*3
void deleteRGB(double **pixels, int w, int h);
//write RGB values to ppm image
void RGBtoImage(double **pixels, int w, int h, const string &filename);


void synthesize(state_t *s, info_t *info);

//print helpers
void printRGB(double **val, int idx);      //print single RGB value
void printAll(double **val, int w, int h); //print all rgb values in image
void outmsg(const char *fmt, ...);
