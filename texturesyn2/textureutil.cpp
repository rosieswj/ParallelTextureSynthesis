#include "texture.h"

double getSquareDist(double *A, double *B)
{
    return (A[0] - B[0]) * (A[0] - B[0]) + (A[1] - B[1]) * (A[1] - B[1]) + (A[2] - B[2]) * (A[2] - B[2]);
}

int ID(int x, int y, int w)
{
    return x * w + y;
}

void getGaussianKernel(double sigma, int w, double **kernel)
{
    double p1 = -1 / (2 * sigma * sigma);
    double p2 = 1 / (2 * PI * sigma * sigma);
    int center = (w - 1) / 2;

    for (int i = 0; i < w; i++)
    {
        for (int j = 0; j < w; j++)
        {
            kernel[i][j] = p2 * exp(p1 * ((i - center) * (i - center) + (j - center) * (j - center)));
        }
    }
}

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
    if (pixels == NULL)
        return;
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
    res.save(filename + int2str(w) + "x" + int2str(h) + ".ppm");
}

void printRGB(double **val, int idx)
{
    printf("%.f,%.f, %.f\n", val[idx][0], val[idx][1], val[idx][2]);
}

void printAll(double **val, int w, int h)
{
    for (int i = 0; i < w; i++)
    {
        for (int j = 0; j < h; j++)
        {
            printRGB(val, i * h + j);
        }
    }
}

int randint(int N)
{
    return floor(rand() * 1.0 / RAND_MAX * N);
}

string int2str(int x)
{
    stringstream stream;
    stream << x;
    return stream.str();
}
