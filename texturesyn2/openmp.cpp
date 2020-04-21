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

double compute_dist(state_t *s,info_t *info, double **my_dis,int tx, int ty) {

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
            my_dis[x][y] = dist;
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
    // bool ** flag = s->flag;
    // double **sample = info->sample;


    double **dis = new double *[sw - w + 1];
    for (int i = 0; i < sw - w + 1; i++)
    {
        dis[i] = new double[sh - w + 1];
    }

    //============================init window
    int cx = rw / 2;
    int cy = rh / 2;
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
                s->res[ri][c] = info->sample[si][c];
            }
            total++;
            s->flag[cx + i][cy + j] = true;
        }
    }
    FINISH_ACTIVITY(ACTIVITY_STARTUP);

    //=================================
    int currR = 2;
    while (currR <= info->r)
    {
        int traverseSize = 2 * currR * 4;
        int **ts = (int **)malloc(sizeof(int *) * traverseSize);
        for (int i = 0; i < traverseSize; i++)
        {
            ts[i] =(int *)malloc(sizeof(int) * 2);
        }
        getTraversalSequence(ts, currR, cx, cy);
        // printf("r=%d: %d\n", currR, traverseSize);

        #pragma omp parallel default(none) shared(s, info, ts, traverseSize)
        {
            int tid = omp_get_thread_num();
            int tcnt = omp_get_num_threads();
            int halfw = (info->w - 1) / 2;
            int yEnd = info->yEnd;

            int chunk_size =  traverseSize / (tcnt) * 2;
            chunk_size = chunk_size > 1 ? chunk_size : 1;
            // printf("[%d] total: %d, chunk_size: %d\n", tid, traverseSize, chunk_size);

            double **dis = new double *[info->xEnd];
            for (int i = 0; i < info->xEnd; i++)
            {
                dis[i] = new double[yEnd];
            }
            int *pixel = new int[info->xEnd * info->yEnd];

            double minDis = 1e6;
            #pragma omp for schedule(static, chunk_size)
            for (int i = 0; i < traverseSize; i++) {
                int tx = ts[i][0];
                int ty = ts[i][1];
                START_ACTIVITY(ACTIVITY_DIST);
                minDis = compute_dist(s, info, dis, tx, ty);
                FINISH_ACTIVITY(ACTIVITY_DIST);


                START_ACTIVITY(ACTIVITY_NEXT);
                // vector<int> canPixel;
                // canPixel.clear();
                for (int i=0; i<info->yEnd * info->xEnd; i++) {
                    pixel[i] = 0.0;
                }
                double upperBound = minDis * (1 + EPSILON);
                int myCanPixelCnt = 0;
                for (int x = 0; x < info->xEnd; x++)
                {
                    for (int y = 0; y < yEnd; y++)
                    {
                        if (dis[x][y] <= upperBound)
                        {
                            pixel[myCanPixelCnt++] =  x * yEnd + y;
                            // canPixel.push_back(x * yEnd + y);
                        }
                    }
                }
                // int pixelCnt = canPixel.size();
                // printf("r=%d, count=%d, canPixel=%d\n", currR, traverseSize, pixelCnt);
                // int choice = canPixel[randint(pixelCnt)];

                int choice = pixel[randint(myCanPixelCnt)];
                int choiceX = choice / yEnd + halfw;
                int choiceY = choice % yEnd + halfw;

                int r_id = ID(tx, ty, info->rh);
                int s_id = ID(choiceX, choiceY, info->sh);
                for (int c = 0; c < 3; c++)
                {
                    s->res[r_id][c] = info->sample[s_id][c];
                }
                s->flag[tx][ty] = true;
                FINISH_ACTIVITY(ACTIVITY_NEXT);

                for (int i = 0; i < info->xEnd; i++)
                {
                    delete[] dis[i];
                }
                delete[] dis;
                delete[] pixel;
            }

        }
        // printf("######################### r ###################\n");
        for (int i = 0; i < traverseSize; i++) {
            free(ts[i]);
        }
        free(ts);

        total += traverseSize;
        currR++;
    }

    printf("processed pixel total = %d\n", total);

    for (int i = 0; i < sw - w + 1; i++)
    {
        delete[] dis[i];
    }
    delete[] dis;


    for (int t=0; t<tcnt ; t++) {

        for (int i = 0; i < xEnd; i++)
        {
            free(thread_dis[t][i]);
        }
        free(thread_dis[t]);
        free(thread_canpixel[t]);
    }
    free(thread_canpixel);
    free(thread_dis);


}
