#include "api.h"
#include <getopt.h>
#include "lib/cycletimer.h"
#include "lib/instrument.h"

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
    // printf("test\n");
    // Image sample("../src/1.ttr");

    // printf("%d, %d\n", sample.width, sample.height);
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

    START_ACTIVITY(ACTIVITY_IMAGE);
    Image sample(INPUT);
    FINISH_ACTIVITY(ACTIVITY_IMAGE);

    TextureSynthesis(sample, OUTPUT, RADIUS, WINDOW);

    SHOW_ACTIVITY(stderr, instrument);

    return 0;
}
