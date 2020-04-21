#include "texture.h"
#include "lib/cycletimer.h"
#include "lib/instrument.h"

double getDistanceOfBatch(state_t *s, info_t *info, int sx, int sy, int tx, int ty)
{
    double sum = 0.0;
    int validcnt = 0;
    double **kernel = info->kernel;
    double **res = s->res;
    bool **flag = s->flag;
    int sh = info->sh;
    int rh = info->rh;
    int w = info->w;
    for (int i = 0; i < w; i++)
    {
        for (int j = 0; j < w; j++)
        {
            if (flag[tx + i][ty + j])
            {
                validcnt++;
                sum += getSquareDist(info->sample[(sx + i) * sh + sy + j], res[(tx + i) * rh + ty + j]) * kernel[i][j];
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

double find_dist(state_t *s,info_t *info, double ** dis,int tx, int ty) {

    double minDis = 1e6;
    int xEnd = info->xEnd;
    int yEnd = info->yEnd;
    int cornerx = tx - (info->w-1)/2;
    int cornery = ty - (info->w-1)/2;
    for (int x = 0; x < xEnd; x++)
    {
        for (int y = 0; y < yEnd; y++)
        {
            double dist = getDistanceOfBatch(s, info, x, y, cornerx, cornery);
            minDis = min(dist, minDis);
            dis[x][y] = dist;
        }
    }
    return minDis;
}

//TODO
void synthesize(state_t *s, info_t *info)
{

    // printAll(sample, sw, sh);
    START_ACTIVITY(ACTIVITY_STARTUP);
    int total = 0;
    int rw = info->rw;
    int rh = info->rh;
    int sw = info->sw;
    int sh = info->sh;
    int xEnd = info->xEnd;
    int yEnd = info->yEnd;
    int w = info->w;
    bool ** flag = s->flag;
    double **sample = info->sample;
    double **res = s->res;

    double **dis = new double *[sw - w + 1];
    for (int i = 0; i < sw - w + 1; i++)
    {
        dis[i] = new double[sh - w + 1];
    }

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
    while (currR <= info->r)
    {
        int traverseSize = 2 * currR * 4;
        int **ts = new int *[traverseSize];
        for (int i = 0; i < traverseSize; i++)
        {
            ts[i] = new int[2];
        }
        getTraversalSequence(ts, currR, cx, cy);


        // printf("r=%d: %d\n", currR, traverseSize);
        for (int i = 0; i < traverseSize; i++)
        {
            int tx = ts[i][0];
            int ty = ts[i][1];
            if (flag[tx][ty]) continue;


        // #pragma omp for schedule(static)
            START_ACTIVITY(ACTIVITY_DIST);
            double minDis = find_dist(s, info, dis, tx, ty);
            FINISH_ACTIVITY(ACTIVITY_DIST);

            START_ACTIVITY(ACTIVITY_NEXT);
            vector<int> canPixel;
            canPixel.clear();

            double upperBound = minDis * (1 + EPSILON);
            for (int x = 0; x < xEnd; x++)
            {
                for (int y = 0; y < yEnd; y++)
                {
                    if (dis[x][y] <= upperBound)
                    {
                        canPixel.push_back(x * yEnd + y);
                    }
                }
            }
            int pixelCnt = canPixel.size();
            // printf("r=%d, count=%d, canPixel=%d\n", currR, traverseSize, pixelCnt);
            int choice = canPixel[randint(pixelCnt)];
            int choiceX = choice / yEnd + halfw;
            int choiceY = choice % yEnd + halfw;

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

    for (int i = 0; i < sw - w + 1; i++)
    {
        delete[] dis[i];
    }
    delete[] dis;
}
