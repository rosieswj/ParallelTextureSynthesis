#include "api.h"
#include <getopt.h>
#include "lib/cycletimer.h"
#include "lib/instrument.h"

static void usage(char *name)
{
    const char *use_string = "-s SFILE -o OFILE -w WINDOW -r RADIUS [-I]";
    outmsg("Usage: %s %s\n", name, use_string);
    outmsg("   -s SFILE   Sample file\n");
    outmsg("   -o OFILE   Output file\n");
    outmsg("   -w WINDOW  Window size\n");
    outmsg("   -r RADIUS  Output radius\n");
    outmsg("   -I         Instrument simulation activities\n");
    outmsg("   -h         Print this message\n");
    exit(0);
}

int main(int argc, char *argv[])
{
    //user defined parameters
    int WINDOW = -1;
    int RADIUS = -1;

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
    if (WINDOW == -1 || RADIUS == -1)
    {
        outmsg("Missing paramters window/radius\n");
        usage(argv[0]);
    }

    printf("w=%d, r=%d\n", WINDOW, RADIUS);
    Image sample(INPUT);
    int sw = sample.width;
    int sh = sample.height;
    if (WINDOW > sw || WINDOW > sh) {
        outmsg("Window size cannot be greater than sample dimension\n");
        exit(1);
    }

    TextureSynthesis(sample, OUTPUT, RADIUS, WINDOW, instrument);

    return 0;
}
