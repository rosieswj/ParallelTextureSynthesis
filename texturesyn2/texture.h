
#include "image.h"
#include "api.h"

void getGaussianKernel(double sigma, int w, double **kernel);
void getTraversalSequence(int *ts, int radius, int cx, int cy);

int ID(int x, int y, int w);
double getSquareDist(double *A, double *B);

//get RGB values from image, size = (w*h)*3
double **getRGB(const Image &img);
//free pixel array, size = w*h*3
void deleteRGB(double **pixels, int w, int h);
//write RGB values to ppm image
void RGBtoImage(double **pixels, int w, int h, const string &filename);
void printRGB(double **val, int idx);
void printAll(double **val, int w, int h);

void synthesize(double **sample, double **res, int radius, int w, int sw, int sh);
