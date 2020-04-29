#include "texture.h"
#include "all.h"

double getSquareDist(double *A, double *B)
{
    return (A[0] - B[0]) * (A[0] - B[0]) + (A[1] - B[1]) * (A[1] - B[1]) + (A[2] - B[2]) * (A[2] - B[2]);
}

int ID(int x, int y, int w)
{
    return x * w + y;
}

info_t * init_info(double **sample, int sw, int sh, int w, int r) {
    info_t* info = (info_t *) malloc(sizeof(info_t));
    if (info == NULL) {
        outmsg("Couldn't allocate storage for info\n");
        return NULL;
    }
    info->sw = sw;
    info->sh = sh;
    info->w = w;
    info->r = r;
    info->rw = 2 * r + w + 10;
    info->rh = 2 * r + w + 10;
    info->xEnd = sw - w + 1;
    info->yEnd = sh - w + 1;
    info->sample=  sample;
    double **kernel = (double **)malloc(sizeof(double *) * w);
    for (int i = 0; i < w; i++)
    {
        kernel[i] = double_alloc(w);
    }
    getGaussianKernel(SIGMA, w, kernel);
    info->kernel = kernel;
    return info;
}

void freeInfo(info_t *info) {
    for (int i = 0; i < info->w; i++)
    {
        free(info->kernel[i]);
    }
    free(info->kernel);
    free(info);
}


state_t *init_state(info_t *info) {
    state_t *s = (state_t *) malloc(sizeof(state_t));
    if (info == NULL) {
        outmsg("Couldn't allocate storage state\n");
        return NULL;
    }
    int rw = info->rw;
    int rh = info->rh;

    int rsize = 2 * info->r + info->w + 10;

    double **res = (double **)malloc(sizeof(double *) * rsize * rsize);
    for (int i = 0; i < rsize * rsize; i++)
    {
        res[i] = new double[3]; //RGB value
    }
    s->res = res;


    bool **flag = (bool **)malloc(sizeof(bool *) * rw);
    for (int i = 0; i < rw; i++)
    {
        flag[i] = new bool[rh];
        for (int j = 0; j < rh; j++)
        {
            flag[i][j] = false;
        }
    }
    s->flag = flag;
    return s;
}

void freeState(state_t *s, info_t *info) {
    for (int i = 0; i < info->rw; i++)
    {
        free(s->flag[i]);
    }
    free(s->flag);

    int rsize = 2 * info->r + info->w + 10;
    for (int i = 0; i < rsize * rsize; i++)
    {
        free(s->res[i]); //RGB value
    }
    free(s->res);
    free(s);
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
    double **pixels = (double **) malloc(sizeof(double) * img.width * img.height);
    for (int i = 0; i < img.width * img.height; ++i)
    {
        pixels[i] = double_alloc(3);
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
    ofstream out(filename + "_" + int2str(w) + "x" + int2str(h) + ".ppm");
    out << "P3" << endl;
    out << w << ' ' << h << endl;
    out << 255 << endl;
    for (int i = 0; i < w * h; i++)
    {
        out << int(pixels[i][0]) << ' ' <<  int(pixels[i][1])  << ' ' <<  int(pixels[i][2]) << endl;
    }
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

void outmsg(const char *fmt, ...)
{
    va_list ap;
    bool got_newline = fmt[strlen(fmt) - 1] == '\n';
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    va_end(ap);
    if (!got_newline)
        fprintf(stderr, "\n");
}
void set_zero(void *buf, size_t len)
{
    if (buf == NULL)
        return;
    memset(buf, 0, len);
}

double *double_alloc(size_t n)
{
    size_t len = n * sizeof(double);
    double *result = (double *)malloc(len);
    set_zero(result, len);
    return result;
}




