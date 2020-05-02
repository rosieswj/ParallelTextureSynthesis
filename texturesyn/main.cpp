#include "texture.h"
#include "lib/cycletimer.h"
#include "lib/instrument.h"
#include <getopt.h>

static void usage(char *name)
{
    const char *use_string = "-s SFILE -o OFILE -w WINDOW -r RADIUS [-t THD] [-I]";
    outmsg("Usage: %s %s\n", name, use_string);
    outmsg("   -s SFILE   Sample file\n");
    outmsg("   -o OFILE   Output file\n");
    outmsg("   -w WINDOW  Window size\n");
    outmsg("   -r RADIUS  Output radius\n");
    outmsg("   -t THD     Number of threads\n");
    outmsg("   -I         Instrument simulation activities\n");
    outmsg("   -h         Print this message\n");
    exit(0);
}

int main(int argc, char *argv[])
{
    //user defined parameters
    int WINDOW = -1;
    int RADIUS = -1;

    //file input path
    char *INPUT = NULL;
    //file output path, extension not needed (generate .ppm by default)
    char *OUTPUT = NULL;

    bool instrument = false;
    int thread_count = 1;
    int c;

    const char *optstring = "hs:o:w:r:t:I";
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
        case 't':
            thread_count = atoi(optarg);
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
        exit(1);
    }
    if (OUTPUT == NULL)
    {
        outmsg("Please give output path\n");
        usage(argv[0]);
        exit(1);
    }
    if (WINDOW == -1 || RADIUS == -1)
    {
        outmsg("Missing paramters window/radius\n");
        usage(argv[0]);
        exit(1);
    }
    if (thread_count > 1) {
        outmsg("Running with %d threads. Max possible is %d.\n",
           thread_count, omp_get_max_threads());
    }
    else {
        outmsg("Running sequential implementation.\n");
    }
    omp_set_num_threads(thread_count);
    Image sampleImg(INPUT);

    track_activity(instrument);
    START_ACTIVITY(ACTIVITY_STARTUP);

    int sw = sampleImg.width;
    int sh = sampleImg.height;
    if (WINDOW > sw || WINDOW > sh) {
        outmsg("Window size cannot be greater than sample dimension\n");
        exit(1);
    }

    srand(INITSEED);
    double **sample = getRGB(sampleImg);

    info_t *info = init_info(sample, sw, sh, WINDOW, RADIUS);
    if (info == NULL) {
        outmsg("Couldn't allocate info\n");
        exit(1);
    }

    state_t *s = init_state(info);
    if (info == NULL) {
        outmsg("Couldn't allocate state\n");
        exit(1);
    }

    printf("sample [%d x %d] with r=%d, w=%d\n", info->sw, info->sh, info->r, info->w);
    printf("result [%d x %d]\n", info->rw, info->rh);
    FINISH_ACTIVITY(ACTIVITY_STARTUP);

    synthesize(s, info);

    printf("Done\n");
    SHOW_ACTIVITY(stderr, instrument);

    RGBtoImage(s->res, WINDOW, RADIUS, OUTPUT);
    freeState(s, info);
    freeInfo(info);

    return 0;
}
