#include <iostream>
#include <vector>
#include <fstream>
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

int main()
{
    // flip flop list
    vector<Point> points;
    unsigned int k, t;
    //-----

    // Reading coordinates of points

    // Finding bounding box of points
    auto [min_x_it, max_x_it] = std::minmax_element(points.begin(), points.end(), compareX);
    auto [min_y_it, max_y_it] = std::minmax_element(points.begin(), points.end(), compareY);

    double min_x = min_x_it->x;
    double max_x = max_x_it->x;
    double min_y = min_y_it->y;
    double max_y = max_y_it->y;

    // Initializing centers with random coordinates within bounding box
    vector<Point> centers(k);
    random_device rd;
    mt19937 gen(rd());
    uniform_real_distribution<> dis_x(min_x, max_x);
    uniform_real_distribution<> dis_y(min_y, max_y);

    for (unsigned int i = 0; i < k; ++i)
    {
        centers[i].x = dis_x(gen);
        centers[i].y = dis_y(gen);
    }

    // Loop for the number of iterations
    for (unsigned int iter = 0; iter < t; ++iter)
    {
        // Assign points to the nearest center 歸類
        for (auto &point : points)
        {
            double min_distance = numeric_limits<double>::max(); // initialize to max value
            int closest_center = -1;                             // initialize the closest center
            for (unsigned int i = 0; i < k; ++i)
            {
                double distance = MD(point, centers[i]);
                if (distance < min_distance)
                {
                    min_distance = distance;
                    closest_center = i;
                }
            }
            point.cluster = closest_center;
        }

        // Calculate new centers for each cluster
        vector<double> sum_x(k, 0.0), sum_y(k, 0.0);
        vector<int> count(k, 0);

        for (const auto &point : points)
        {
            int cluster = point.cluster;
            sum_x[cluster] += point.x;
            sum_y[cluster] += point.y;
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

    return 0;
}
