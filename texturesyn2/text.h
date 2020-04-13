
#include "image.h"
#include "api.h"

double **getRGB(const Image &img);
void deleteRGB(double **pixels, int w, int h);
void RGBtoImage(double **pixels, int w, int h, const string &filename);