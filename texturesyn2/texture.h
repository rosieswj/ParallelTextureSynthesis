
#include "image.h"
#include "api.h"

//get RGB values from image, size = (w*h)*3
double **getRGB(const Image &img);
//free pixel array, size = w*h*3
void deleteRGB(double **pixels, int w, int h);
//write RGB values to ppm image
void RGBtoImage(double **pixels, int w, int h, const string &filename);

void synthesize(double **sample, double **res, int radius, int w, int sw, int sh);