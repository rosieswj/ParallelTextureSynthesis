#include "texture.h"
#include "lib/cycletimer.h"
#include "lib/instrument.h"

/* return pairwise weighted square distance by iterate through all pixel values in
result and sample windows */
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
    int BLOCK_SIZE = 16;
    for (int i = 0; i < w; i+=BLOCK_SIZE) {
        int leni = (w-i) >= BLOCK_SIZE ? BLOCK_SIZE : (w-i);
        for (int j = 0; j < w; j+=BLOCK_SIZE) {
            int lenj = (w-j) >= BLOCK_SIZE ? BLOCK_SIZE : (w-j);
            for (int ii = 0; ii < leni; ii++) {
                for (int jj = 0; jj < lenj; jj++) {
                    if (flag[tx + i + ii][ty + j + jj]) {
                        validcnt++;
                        sum += getSquareDist(info->sample[(sx + i + ii) * sh + sy + j + jj],
                            res[(tx + i + ii) * rh + ty + j + jj]) * kernel[i + ii][j + jj];
                    }
                }
            }

        }
    }
    if (!validcnt)
        return 1e6;
    else
        return sum / validcnt;
}

/* return all pixels to be processed in the batch with current radius */
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

/* main function that performs texture synthesis */
void synthesize(state_t *s, info_t *info)
{

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
    double* dis = new double[xEnd * yEnd];
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
            int ri = ID(cx + i, cy + j, rh);        //res[width][height]
            for (int c = 0; c < 3; c++)
            {
                res[ri][c] = sample[si][c];
            }
            total++;
            flag[cx + i][cy + j] = true;
        }
    }
    FINISH_ACTIVITY(ACTIVITY_STARTUP);

    int currR = 2;
    while (currR <= info->r)
    {
        START_ACTIVITY(ACTIVITY_BATCH);
        int traverseSize = 2 * currR * 4;
        int **ts = new int *[traverseSize];
        for (int i = 0; i < traverseSize; i++)
        {
            ts[i] = new int[2];
        }
        getTraversalSequence(ts, currR, cx, cy);
        FINISH_ACTIVITY(ACTIVITY_BATCH);

        for (int i = 0; i < traverseSize; i++)
        {
            START_ACTIVITY(ACTIVITY_DIST);
            int tx = ts[i][0];
            int ty = ts[i][1];

            if (flag[tx][ty]) continue;
            double minDis = 1e6;
            #pragma omp parallel default(none) shared(dis, s, info, tx, ty, minDis)
            {
                int tcount = omp_get_num_threads();
                int tid = omp_get_thread_num();
                /* use separate-accumulate to eliminate synchronization
                   across threads to write to min */
                int cornerx = tx - (info->w-1)/2;
                int cornery = ty - (info->w-1)/2;
                double *scratch_vector = new double[tcount];
                for (int i = 0; i < tcount; i++){
                    scratch_vector[i] = 1e6;
                }
                int xEnd = info->xEnd;
                int yEnd = info->yEnd;
                #pragma omp for schedule(static) nowait
                for (int i=0; i< xEnd * yEnd; i++) {
                    int x = i / yEnd;
                    int y = i % yEnd;
                    double dist = getDistanceOfBatch(s, info, x, y, cornerx, cornery);
                    if (dist < scratch_vector[tid]) {
                        scratch_vector[tid] = dist;
                    }
                    dis[i] = dist;
                }
                #pragma omp for reduction(min:minDis)
                for (int i = 0; i < tcount; i++) {
                    minDis = fmin(minDis,scratch_vector[i]);
                }
                /**
                #pragma omp for schedule(static)
                for (int i = 0; i < tcount; i++) {
                    #pragma omp critical
                    minDis = min(minDis,scratch_vector[i]);
                }**/
            }
            FINISH_ACTIVITY(ACTIVITY_DIST);

            START_ACTIVITY(ACTIVITY_NEXT);
            vector<int> canPixel;
            canPixel.clear();
            double upperBound = minDis * (1 + EPSILON);
            for (int i=0; i<xEnd * yEnd; i++) {
                if (dis[i] <= upperBound) {
                    canPixel.push_back(i);
                }
            }
            int pixelCnt = canPixel.size();
            int choice = canPixel[randint(pixelCnt)];
            int choiceX = choice / yEnd + halfw;
            int choiceY = choice % yEnd + halfw;

            int r_id = ID(tx, ty, rh);
            int s_id = ID(choiceX, choiceY, sh);
            for (int c = 0; c < 3; c++)
            {
                res[r_id][c] = sample[s_id][c];
            }
            total++;
            flag[tx][ty] = true;
            FINISH_ACTIVITY(ACTIVITY_NEXT);
        }
        currR++;
    }

    printf("processed pixel total = %d\n", total);
    delete[] dis;
}
