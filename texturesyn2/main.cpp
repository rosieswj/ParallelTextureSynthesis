#include "api.h"
#include "text.h"

int main()
{
    Image sample("./src/sample1.ttr");
    double **pixels = getRGB(sample);

    int width = sample.width;
    int height = sample.height;
    // for (int i = 0; i < 20; i++) printf("%.f, %.f, %.f\n", pixels[i][0], pixels[i][1], pixels[i][2]);

    RGBtoImage(pixels, width, height, "./out/test-sample1-");
    deleteRGB(pixels, width, height);

    return 0;
}
