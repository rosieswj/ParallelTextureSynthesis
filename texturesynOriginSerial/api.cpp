#include "api.h"

int randint(int N)
{
    return floor(rand() * 1.0 / RAND_MAX * N);
}

void GetGaussianKernel(double sigma, int w, double **kernel)
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

void GetTraversalSequence(const Pixel &center, int radius, vector<Pixel> &ts)
{
    ts.clear();
    if (radius == 0)
    {
        ts.push_back(center);
        return;
    }
    Pixel p = center + Pixel(radius, radius);
    for (int i = 0; i < 2 * radius; i++)
    {
        ts.push_back(p);
        p.y--;
    }
    for (int i = 0; i < 2 * radius; i++)
    {
        ts.push_back(p);
        p.x--;
    }
    for (int i = 0; i < 2 * radius; i++)
    {
        ts.push_back(p);
        p.y++;
    }
    for (int i = 0; i < 2 * radius; i++)
    {
        ts.push_back(p);
        p.x++;
    }
    return;
}

void TextureSynthesis(const Image &sample, const string &savefolder, int radius, int w, bool instrument)
{

    track_activity(instrument);
    START_ACTIVITY(ACTIVITY_STARTUP);
    int sw = sample.width;
    int sh = sample.height;
    int width = 2 * radius + w + 10;
    int height = width;
    double epsilon = 0.1;
    double sigma = 1;
    Image res(width, height);

    double **kernel = new double *[w];
    bool **flag = new bool *[width];
    double **dis = new double *[sw - w + 1];
    for (int i = 0; i < width; i++)
    {
        flag[i] = new bool[height];
        for (int j = 0; j < height; j++)
        {
            flag[i][j] = false;
        }
    }
    for (int i = 0; i < w; i++)
    {
        kernel[i] = new double[w];
    }
    for (int i = 0; i < sw - w + 1; i++)
    {
        dis[i] = new double[sh - w + 1];
    }
    GetGaussianKernel(sigma, w, kernel);
    Pixel center = Pixel(width / 2, height / 2);
    int halfw = (w - 1) / 2;
    int init_x = randint(sw - 2) + 1;
    int init_y = randint(sh - 2) + 1;


    for (int i = -1; i < 2; i++)
    {
        for (int j = -1; j < 2; j++)
        {
            res.SetColor(center + Vector2(i, j), sample.GetColor(init_x + i, init_y + j));
            flag[center.x + i][center.y + j] = true;
        }
    }
    FINISH_ACTIVITY(ACTIVITY_STARTUP);

    vector<Pixel> ts;
    int currR = 2;
    while (currR <= radius)
    {
        START_ACTIVITY(ACTIVITY_BATCH);
        GetTraversalSequence(center, currR, ts);
        FINISH_ACTIVITY(ACTIVITY_BATCH);
        for (auto pos : ts)
        {
            START_ACTIVITY(ACTIVITY_DIST);
            if (flag[pos.x][pos.y])
            {
                continue;
            }
            double minDis = 1e6;
            Pixel corner = pos - Pixel(halfw, halfw);
            for (int x = 0; x < sw - w + 1; x++)
            {
                for (int y = 0; y < sh - w + 1; y++)
                {
                    double dist = GetDistanceOfBatch(Vector2(x, y), corner, sample, res, kernel, flag, w);

                    minDis = min(dist, minDis);
                    dis[x][y] = dist;
                }
            }
            FINISH_ACTIVITY(ACTIVITY_DIST);

            START_ACTIVITY(ACTIVITY_NEXT);
            vector<Pixel> canPixel;
            canPixel.clear();

            double upperBound = minDis * (1 + epsilon);
            for (int x = 0; x < sw - w + 1; x++)
                for (int y = 0; y < sh - w + 1; y++)
                {
                    if (dis[x][y] <= upperBound)
                        canPixel.push_back(Vector2(x, y));
                }
            int pixelCnt = canPixel.size();
            int index = randint(pixelCnt);
            Pixel choice = canPixel[index] + Vector2(halfw, halfw);
            res.SetColor(pos, sample.GetColor(choice));
            flag[pos.x][pos.y] = true;
            FINISH_ACTIVITY(ACTIVITY_NEXT);
        }
        currR++;
    }
    for (int i = 0; i < w; i++)
    {
        delete[] kernel[i];
    }
    for (int i = 0; i < sw - w + 1; i++)
    {
        delete[] dis[i];
    }
    for (int i = 0; i < width; i++)
    {
        delete[] flag[i];
    }
    delete[] dis;
    delete[] kernel;
    delete[] flag;

    SHOW_ACTIVITY(stderr, instrument);
    res.save(savefolder + "_" + int2str(w) + "_" + int2str(radius) + ".ppm");
}

string int2str(int x)
{
    stringstream stream;
    stream << x;
    return stream.str();
}
double GetDistanceOfBatch(const Pixel &so, const Pixel &ro, const Image &sample, const Image &res, double **kernel, bool **flag, int w)
{
    double sum = 0;
    int validcnt = 0;
    for (int i = 0; i < w; i++)
    {
        for (int j = 0; j < w; j++)
        {
            Pixel p1 = so + Vector2(i, j);
            Pixel p2 = ro + Vector2(i, j);
            if (flag[p2.x][p2.y] )
            {
                validcnt++;
                Color c1 = sample.GetColor(p1);
                Color c2 = res.GetColor(p2);
                sum += SQRDISTANCE(c1, c2) * kernel[i][j];
            }
        }
    }
    if (!validcnt)
        return 1e6;
    return sum / validcnt;
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
