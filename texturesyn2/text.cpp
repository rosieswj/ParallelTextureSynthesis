#include "text.h"

double **getRGB(const Image &img)
{

    double **pixels = new double *[img.width * img.height];
    for (int i = 0; i < img.width * img.height; ++i)
    {
        pixels[i] = new double[3];
    }
    img.getPixels(pixels);
    return pixels;
}

void deleteRGB(double **pixels, int w, int h)
{
    for (int i = 0; i < w * h; ++i)
    {
        delete[] pixels[i];
    }
    delete[] pixels;
}

void RGBtoImage(double **pixels, int w, int h, const string &filename)
{
    Image res(w, h);
    for (int i = 0; i < w * h; i++)
    {
        int x = i % w;
        int y = i / w;
        res.SetColor(Vector2(x, y), Vector3(pixels[i][0], pixels[i][1], pixels[i][2]));
    }
    res.save(filename + int2str(w) + "_" + int2str(h) + ".ppm");
}
