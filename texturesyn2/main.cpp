#include "texture.h"
#include "lib/cycletimer.h"
#include "lib/instrument.h"
#include <getopt.h>

static void usage(char *name)
{
    const char *use_string = "-s SFILE -o OFILE -w WINDOWS -r RADIUS [-I]";
    outmsg("Usage: %s %s\n", name, use_string);
    outmsg("   -h         Print this message\n");
    outmsg("   -s SFILE   Sample file\n");
    outmsg("   -o OFILE   Output file\n");
    outmsg("   -w WINDOW  Window size\n");
    outmsg("   -r RADIUS  Output radius\n");
    outmsg("   -I         Instrument simulation activities\n");
    exit(0);
}

int main(int argc, char *argv[])
{
    //user defined parameters
    int WINDOW;
    int RADIUS;

    char *INPUT = NULL;
    char *OUTPUT = NULL;

    bool instrument = false;
    int c;

    const char *optstring = "hs:o:w:r:I";
    while ((c = getopt(argc, argv, optstring)) != -1)
    {
        switch (c)
        {
        case 'h':
            usage(argv[0]);
            break;
        case 's':
            INPUT = optarg;
            break;
        case 'o':
            OUTPUT = optarg;
            break;
        case 'w':
            WINDOW = atoi(optarg);
            break;
        case 'r':
            RADIUS = atoi(optarg);
            break;
        case 'I':
            instrument = true;
            break;
        default:
            outmsg("Unknown option '%c'\n", c);
            usage(argv[0]);
            exit(1);
        }
    }
    if (INPUT == NULL)
    {
        outmsg("Please give sample path\n");
        usage(argv[0]);
    }
    if (OUTPUT == NULL)
    {
        outmsg("Please give output path\n");
        usage(argv[0]);
    }
    printf("w=%d, r=%d\n", WINDOW, RADIUS);

    track_activity(instrument);
    //read sample image
    START_ACTIVITY(ACVIVITY_IMAGE);
    Image sampleImg(INPUT);
    srand(INITSEED);

    int sw = sampleImg.width;
    int sh = sampleImg.height;
    double **sample = getRGB(sampleImg);

    int rsize = 2 * RADIUS + WINDOW + 10;
    double **res = new double *[rsize * rsize];
    for (int i = 0; i < rsize * rsize; i++)
    {
        res[i] = new double[3]; //RGB value
    }
    // printf("\n############## read sample done ##################\n");
    printf("[sample] %d x %d, size = %d\n", sw, sh, sw * sh);
    printf("[result] %d x %d, size = %d\n", rsize, rsize, rsize * rsize);
    FINISH_ACTIVITY(ACVIVITY_IMAGE);

    synthesize(sample, res, RADIUS, WINDOW, sw, sh);
    // printf("\n############## synthesize done #####################\n");

    // printAll(res, rsize, rsize);
    //write result image
    START_ACTIVITY(ACVIVITY_IMAGE);
    RGBtoImage(res, rsize, rsize, OUTPUT);
    // RGBtoImage(sample, sw, sh, OUTPUT);

    FINISH_ACTIVITY(ACVIVITY_IMAGE);

    deleteRGB(res, rsize, rsize);
    deleteRGB(sample, sw, sh);

    SHOW_ACTIVITY(stderr, instrument);
    printf("DONE\n");
    return 0;
}
