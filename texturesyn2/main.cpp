#include "api.h"
#include "texture.h"

int main()
{
    //user defined parameters
    int WINDOW = 10;
    int RADIUS = 100;
    const string INPUT = "./src/sample1.ttr";
    const string OUTPUT = "./out/sample1-out-";

    Image sampleImg(INPUT);
    int sw = sampleImg.width;
    int sh = sampleImg.height;
    double **sample = getRGB(sampleImg);
    int rsize = 2 * RADIUS + WINDOW + 10;
    double **res = new double *[rsize * rsize];
    for (int i = 0; i < rsize * rsize; i++)
    {
        res[i] = new double[3]; //RGB value
    }
    printf("############## read sample done ##################\n");
    printf("[sample] %d x %d, size = %d\n", sw, sh, sw * sh);
    printf("[result] %d x %d, size = %d\n", rsize, rsize, rsize * rsize);

    synthesize(sample, res, RADIUS, WINDOW, sw, sh);
    printf("############## synthesize done #####################\n");

    RGBtoImage(res, rsize, rsize, OUTPUT);
    printf("############## output to image done #####################\n");

    deleteRGB(res, rsize, rsize);
    deleteRGB(sample, sw, sh);
    printf("############## free malloc done #####################\n");
    return 0;
}
