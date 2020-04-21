#include "api.h"
int main()
{
    // printf("test\n");
    // Image sample("../src/1.ttr");

    // printf("%d, %d\n", sample.width, sample.height);

    Image sample("./src/sample1.ttr");
    TextureSynthesis(sample, ".", 100, 10);

    return 0;
}
