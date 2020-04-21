#include "api.h"
int main()
{
    // printf("test\n");
    // Image sample("../src/1.ttr");

    // printf("%d, %d\n", sample.width, sample.height);
    bool instrument = true;
    track_activity(instrument);
    START_ACTIVITY(ACVIVITY_IMAGE);

    Image sample("./src/sample1.ttr");
    FINISH_ACTIVITY(ACVIVITY_IMAGE);

    TextureSynthesis(sample, ".", 100, 10);

    SHOW_ACTIVITY(stderr, instrument);

    return 0;
}
