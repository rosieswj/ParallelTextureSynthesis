#include "texture.h"
#include "lib/cycletimer.h"
#include "lib/instrument.h"

int main()
{
    //user defined parameters
    int WINDOW = 10;
    int RADIUS = 100;
    const string INPUT = "./src/sample1.ttr";
    const string OUTPUT = "./out/sample1-out-";

    bool instrument = true;
    track_activity(instrument);

    //read sample image
    START_ACTIVITY(ACVIVITY_IMAGE);
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
    printf("\n############## read sample done ##################\n");
    printf("[sample] %d x %d, size = %d\n", sw, sh, sw * sh);
    printf("[result] %d x %d, size = %d\n", rsize, rsize, rsize * rsize);
    FINISH_ACTIVITY(ACVIVITY_IMAGE);

    synthesize(sample, res, RADIUS, WINDOW, sw, sh);
    printf("\n############## synthesize done #####################\n");

    //write result image
    START_ACTIVITY(ACVIVITY_IMAGE);
    RGBtoImage(res, rsize, rsize, OUTPUT);
    FINISH_ACTIVITY(ACVIVITY_IMAGE);

    deleteRGB(res, rsize, rsize);
    deleteRGB(sample, sw, sh);
    SHOW_ACTIVITY(stderr, instrument);
    return 0;
}
