
#include "image.h"
#include <omp.h>
void getGaussianKernel(double sigma, int w, double **kernel);
void getTraversalSequence(int *ts, int radius, int cx, int cy);

int ID(int x, int y, int w);
double getSquareDist(double *A, double *B);

string int2str(int x);
int randint(int N);

//get RGB values from image, size = (w*h)*3
double **getRGB(const Image &img);
//free pixel array, size = w*h*3
void deleteRGB(double **pixels, int w, int h);
//write RGB values to ppm image
void RGBtoImage(double **pixels, int w, int h, const string &filename);
//print helpers
void printRGB(double **val, int idx);      //print single RGB value
void printAll(double **val, int w, int h); //print all rgb values in image

void synthesize(double **sample, double **res, int radius, int w, int sw, int sh);

void outmsg(const char *fmt, ...);
