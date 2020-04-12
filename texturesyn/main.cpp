#include "api.h"
int main()
{
    // printf("test\n");
    // Image sample("../src/1.ttr");

    // printf("%d, %d\n", sample.width, sample.height);

    Image sample("./src/1.ttr");
    TextureSynthesis(sample, ".", 200, 5);

    return 0;
}
