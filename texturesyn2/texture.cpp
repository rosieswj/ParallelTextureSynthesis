#include "texture.h"
#include "lib/cycletimer.h"
#include "lib/instrument.h"

double getDistanceOfBatch(double **res, double **sample,
                          double **kernel, bool **flag, int sx, int sy, int tx, int ty,
                          int w, int rh, int sh)
{
    double sum = 0.0;
    int validcnt = 0;
    for (int i = 0; i < w; i++)
    {
        for (int j = 0; j < w; j++)
        {
            if (flag[tx + i][ty + j])
            {
                validcnt++;
                sum += getSquareDist(sample[(sx + i) * sh + sy + j], res[(tx + i) * rh + ty + j]) * kernel[i][j];
            }
        }
    }
    if (!validcnt)
        return 1e6;
    else
        return sum / validcnt;
}

void getTraversalSequence(int **ts, int radius, int cx, int cy)
{
    int idx = 0;
    int px = cx + radius;
    int py = cy + radius;
    for (int i = 0; i < 2 * radius; i++)
    {
        ts[idx][0] = px;
        ts[idx][1] = py;
        idx++;
        py--;
    }
    for (int i = 0; i < 2 * radius; i++)
    {
        ts[idx][0] = px;
        ts[idx][1] = py;
        idx++;
        px--;
    }
    for (int i = 0; i < 2 * radius; i++)
    {
        ts[idx][0] = px;
        ts[idx][1] = py;
        idx++;
        py++;
    }
    for (int i = 0; i < 2 * radius; i++)
    {
        ts[idx][0] = px;
        ts[idx][1] = py;
        idx++;
        px++;
    }
}

//TODO
void synthesize(double **sample, double **res, int radius, int w, int sw, int sh)
{
    // printAll(sample, sw, sh);
    START_ACTIVITY(ACTIVITY_STARTUP);
    int total = 0;
    int rw = 2 * radius + w + 10;
    int rh = rw;
    double epsilon = 0.1;
    double sigma = 1;

    double **kernel = new double *[w];
    bool **flag = new bool *[rw];
    double **dis = new double *[sw - w + 1];
    for (int i = 0; i < rw; i++)
    {
        flag[i] = new bool[rh];
        for (int j = 0; j < rh; j++)
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

    getGaussianKernel(sigma, w, kernel);
    // printf("\n############## alloc data done #####################\n");

    //============================init window
    int cx = rw / 2;
    int cy = rh / 2;
    int halfw = (w - 1) / 2;
    int init_x = randint(sw - 2) + 1;
    int init_y = randint(sh - 2) + 1;
    for (int i = -1; i < 2; i++)
    {
        for (int j = -1; j < 2; j++)
        {
            // int si = ID(init_x + i, init_y + j, sh); //sample[sw][sh]
            int si = (init_x + i) * sh + (init_y + j);
            int ri = ID(cx + i, cy + j, rh); //res[width][height]
            // printf("si: %d, ri: %d\n", si, ri);
            // printRGB(res, ri);
            for (int c = 0; c < 3; c++)
            {
                res[ri][c] = sample[si][c];
            }
            total++;
            flag[cx + i][cy + j] = true;
        }
    }
    FINISH_ACTIVITY(ACTIVITY_STARTUP);

    //=================================
    int currR = 2;
    while (currR <= radius)
    {
        int traverseSize = 2 * currR * 4;
        int **ts = new int *[traverseSize];
        for (int i = 0; i < traverseSize; i++)
        {
            ts[i] = new int[2];
        }
        getTraversalSequence(ts, currR, cx, cy);
        for (int i = 0; i < traverseSize; i++)
        {
            int tx = ts[i][0];
            int ty = ts[i][1];
            if (flag[tx][ty])
                continue;
            double minDis = 1e6;
            int cornerx = tx - halfw;
            int cornery = ty - halfw;

            START_ACTIVITY(ACTIVITY_DIST);
	    #pragma omp for schedule(static)
            for (int x = 0; x < sw - w + 1; x++)
            {
                for (int y = 0; y < sh - w + 1; y++)
                {
                    double dist = getDistanceOfBatch(res, sample, kernel, flag, x, y, cornerx, cornery, w, rh, sh);
                    minDis = min(dist, minDis);
                    dis[x][y] = dist;
                }
            }
            FINISH_ACTIVITY(ACTIVITY_DIST);

            START_ACTIVITY(ACTIVITY_NEXT);
            vector<int> canPixel;
            canPixel.clear();
            int windowH = sh - w + 1;
            double upperBound = minDis * (1 + epsilon);
            for (int x = 0; x < sw - w + 1; x++)
            {
                for (int y = 0; y < windowH; y++)
                {
                    if (dis[x][y] <= upperBound)
                    {
                        canPixel.push_back(x * windowH + y);
                    }
                }
            }
            int pixelCnt = canPixel.size();
            // printf("r=%d, count=%d, canPixel=%d\n", currR, traverseSize, pixelCnt);
            int choice = canPixel[randint(pixelCnt)];
            int choiceX = choice / windowH + halfw;
            int choiceY = choice % windowH + halfw;

            int r_id = ID(tx, ty, rh);
            int s_id = ID(choiceX, choiceY, sh);
            for (int c = 0; c < 3; c++)
            {
                // printf("setting rid=%d => ", r_id);
                // printRGB(sample, s_id);
                res[r_id][c] = sample[s_id][c];
            }
            // printf("res, sample\n");
            // printRGB(res, r_id);
            // printRGB(sample, s_id);
            total++;
            flag[tx][ty] = true;
            FINISH_ACTIVITY(ACTIVITY_NEXT);
        }
        currR++;
    }

    printf("processed pixel total = %d\n", total);
    for (int i = 0; i < w; i++)
    {
        delete[] kernel[i];
    }
    for (int i = 0; i < sw - w + 1; i++)
    {
        delete[] dis[i];
    }
    for (int i = 0; i < rw; i++)
    {
        delete[] flag[i];
    }
    delete[] dis;
    delete[] kernel;
    delete[] flag;
    // int cnt = 0;
    // for (int i = 0; i < sw * sh; i++)
    // {
    //     cnt++;
    //     printf("%.f, %.f, %.f\n", sample[i][0], sample[i][1], sample[i][2]);
    // }
    // printf("%d, %d, %d\n", sw, sh, cnt);
}
