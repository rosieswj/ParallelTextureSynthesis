#include "api.h"
#include "texture.h"

int main()
{
    //user defined parameters
    int WINDOW = 0;
    int RADIUS = 0;
    const string INPUT = "./src/sample1.ttr";
    const string OUTPUT = "./out/sample1-out-";

    Image sampleImg(INPUT);
    int sw = sampleImg.width;
    int sh = sampleImg.height;
    double **sample = getRGB(sampleImg);
    double **res = NULL;

    synthesize(sample, res, RADIUS, WINDOW, sw, sh);

    RGBtoImage(sample, sw, sh, OUTPUT);
    deleteRGB(sample, sw, sh);

    return 0;
}
