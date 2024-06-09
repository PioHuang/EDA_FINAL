#include <iostream>
#include <vector>
#include <algorithm>
#include <cmath>
#include <random>
#include <limits>
#include <memory>
using namespace std;

// Representation of a point
struct Point
{
    double x, y;
    int cluster = -1;
};

// Function to calculate Manhattan distance between two points
double MD(const Point &a, const Point &b)
{
    double dx = a.x - b.x;
    double dy = a.y - b.y;
    return abs(dx) + abs(dy);
}

// Comparison functions
bool compareX(const Point &a, const Point &b)
{
    return a.x < b.x;
}

bool compareY(const Point &a, const Point &b)
{
    return a.y < b.y;
}
// nth ff negative slack, need to sum all of them up
int SFFN(int &SFFN, int Q_delay_diff, int displacement_delay, int WLQ_diff, int WLD_diff)
{
    return (SFFN + Q_delay_diff + displacement_delay * WLQ_diff + displacement_delay * WLD_diff);
}

// 要針對不同clk的ff進行clustering，要先分類
// 決定換哪種ff要用 cost function 決定
// 最後解決overlapping 和 bin utilization 的問題

int main()
{
    // flip flop list
    vector<Point> points;
    unsigned int k = 20, t = 10; // k = 20 clusters, t = 10 iterations

    // Generating 100 random points
    random_device rd;
    mt19937 gen(rd());
    uniform_real_distribution<> dis_x(0, 1000);
    uniform_real_distribution<> dis_y(0, 1000);

    for (unsigned int i = 0; i < 100; ++i)
    {
        Point new_point;
        new_point.x = dis_x(gen);
        new_point.y = dis_y(gen);
        new_point.cluster = -1;
        points.push_back(new_point);
    }

    // Finding bounding box of points
    vector<Point>::iterator min_x_it = min_element(points.begin(), points.end(), compareX);
    vector<Point>::iterator max_x_it = max_element(points.begin(), points.end(), compareX);
    vector<Point>::iterator min_y_it = min_element(points.begin(), points.end(), compareY);
    vector<Point>::iterator max_y_it = max_element(points.begin(), points.end(), compareY);

    double min_x = min_x_it->x;
    double max_x = max_x_it->x;
    double min_y = min_y_it->y;
    double max_y = max_y_it->y;

    // Initializing centers with random coordinates within bounding box
    vector<Point> centers(k);
    uniform_real_distribution<> dis_center_x(min_x, max_x);
    uniform_real_distribution<> dis_center_y(min_y, max_y);

    for (unsigned int i = 0; i < k; ++i)
    {
        centers[i].x = dis_center_x(gen);
        centers[i].y = dis_center_y(gen);
    }

    // Loop for the number of iterations
    for (unsigned int iter = 0; iter < t; ++iter)
    {
        // Assign points to the nearest center
        for (unsigned int j = 0; j < points.size(); ++j)
        {
            double min_distance = numeric_limits<double>::max(); // initialize to max value
            int closest_center = -1;                             // initialize the closest center
            for (unsigned int i = 0; i < k; ++i)
            {
                double distance = MD(points[j], centers[i]);
                if (distance < min_distance)
                {
                    min_distance = distance;
                    closest_center = i;
                }
            }
            points[j].cluster = closest_center;
        }

        // Calculate new centers for each cluster
        vector<double> sum_x(k, 0.0), sum_y(k, 0.0);
        vector<int> count(k, 0);

        for (unsigned int j = 0; j < points.size(); ++j)
        {
            int cluster = points[j].cluster;
            sum_x[cluster] += points[j].x;
            sum_y[cluster] += points[j].y;
            count[cluster]++;
        }

        for (unsigned int i = 0; i < k; ++i)
        {
            if (count[i] > 0)
            {
                centers[i].x = sum_x[i] / count[i];
                centers[i].y = sum_y[i] / count[i];
            }
        }
    }

    // Access the coordinates of the centers and create them as new points
    vector<Point> center_points;
    for (unsigned int i = 0; i < centers.size(); ++i)
    {
        Point new_point;
        new_point.x = centers[i].x;
        new_point.y = centers[i].y;
        center_points.push_back(new_point);
    }

    // Output the centers for verification
    cout << "Centers:\n";
    for (unsigned int i = 0; i < center_points.size(); ++i)
    {
        cout << "Center at (" << center_points[i].x << ", " << center_points[i].y << ")\n";
    }

    // Output the clusters and their points for verification
    cout << "\nClusters:\n";
    for (unsigned int i = 0; i < points.size(); ++i)
    {
        cout << "Point " << i << " at (" << points[i].x << ", " << points[i].y << ") is in cluster " << points[i].cluster << "\n";
    }

    return 0;
}
