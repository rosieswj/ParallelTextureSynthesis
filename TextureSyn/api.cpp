#include "api.h"

int randint(int N)
{
    return floor(rand() * 1.0 / RAND_MAX * N);
}
//find 2d gaussian kernal G which is used to compute d - d_SSD * G
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

//get all points with radius r to the center
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

void TextureSynthesis(const Image &sample, const string &savefolder, int radius, int w)
{
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
    //all pixel unmarked
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
    //color 1 batch of 3x3 at the center of image to get started
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
    //process from center to edges, each time increment radius by 1
    vector<Pixel> ts;
    int currR = 2;
    while (currR <= radius)
    {
        cout << "Current Radius : " << currR << endl;
        //get all points within the same radius from center
        GetTraversalSequence(center, currR, ts);
        for (auto pos : ts)
        {
            if (flag[pos.x][pos.y])
            /*&& 0 <= p1.x && p1.x < sample.width &&
                0 <= p1.y && p1.y < sample.height &&
                0 <= p2.x && p2.x < res.width &&
                0 <= p2.y && p2.y < res.height*/
            { //if points have been processed, skip
                cout << "Done" << endl;
                continue;
            }
            double minDis = 1e6;
            //top left corner of w(p) = square image patch with width w centered at p
            Pixel corner = pos - Pixel(halfw, halfw);
            //for possible batch in sample
            for (int x = 0; x < sw - w + 1; x++)
            {
                for (int y = 0; y < sh - w + 1; y++)
                {
                    //compute dist between batch_sample and batch_current
                    double dist = GetDistanceOfBatch(Vector2(x, y), corner, sample, res, kernel, flag, w);
                    minDis = min(dist, minDis);
                    dis[x][y] = dist;
                }
            }
            vector<Pixel> canPixel;
            canPixel.clear();
            double upperBound = minDis * (1 + epsilon);
            for (int x = 0; x < sw - w + 1; x++)
            {
                for (int y = 0; y < sh - w + 1; y++)
                { //include only patches w with d(w(p), w) <= (1+epsilon) d_min
                    if (dis[x][y] <= upperBound)
                        canPixel.push_back(Vector2(x, y));
                }
            }

            int pixelCnt = canPixel.size();
            int index = randint(pixelCnt);
            /* random select 1 pixel from batch_sample and use its color
                - canPixel[index] = top left corner of sample batch 
                - need +Vector2(halfw, halfw) to get the center point of sample batch
            */
            Pixel choice = canPixel[index] + Vector2(halfw, halfw);
            res.SetColor(pos, sample.GetColor(choice));
            //market current pixel at (x,y) as processed to be used later
            flag[pos.x][pos.y] = true;
        }
        res.save(savefolder + "/" + int2str(w) + "_" + int2str(currR) + ".ppm");
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
}

string int2str(int x)
{
    stringstream stream;
    stream << x;
    return stream.str();
}

//compute sum of difference of all pixels between current batch (ro) and teh sample batch(so)
//window square size = w * w
double GetDistanceOfBatch(const Pixel &so, const Pixel &ro, const Image &sample, const Image &res, double **kernel, bool **flag, int w)
{
    int halfw = (w - 1) / 2;
    double sum = 0;
    int validcnt = 0;
    //for all pixel in window
    for (int i = 0; i < w; i++)
    {
        for (int j = 0; j < w; j++)
        {
            Pixel p1 = so + Vector2(i, j); //pixel in sample
            Pixel p2 = ro + Vector2(i, j); //pixel in current patch w(p)
            //check if the pixel (p2) has been previously synthesized in w(p)
            if (flag[p2.x][p2.y])
            {
                validcnt++;
                /*  SQRDISTANCE(A,B) = sum of square distance of RGB values for a given pixel 
                    ((A.x-B.x)*(A.x-B.x)+(A.y-B.y)*(A.y-B.y)+(A.z-B.z)*(A.z-B.z))

                    Use kernel here so the error of nearby pixel is greater than error for pixels
                    that are far away
                    */
                sum += SQRDISTANCE(sample.GetColor(p1), res.GetColor(p2)) * kernel[i][j];
            }
        }
    }
    if (!validcnt)
        return 1e6;
    return sum / validcnt;
}
