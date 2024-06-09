#include <iostream>
#include <vector>
#include <algorithm>
#include <cmath>
#include <limits>

using namespace std;

// Structure to represent a cell
struct Cell
{
    double x; // x-coordinate of cell center
    double y; // y-coordinate of cell center
    double width;
    double height;
};

// Structure to represent a row
struct Row
{
    double y; // y-coordinate of the row
    double width;
    double height;
    double x_min;
    double x_max;
};

// Function to legalize cell placement using Abacus algorithm
void abacusLegalization(vector<Cell> &cells, vector<Row> &rows)
{
    // Sort cells by their x-coordinates
    sort(cells.begin(), cells.end(), [](const Cell &a, const Cell &b)
         { return a.x < b.x; });

    for (auto &cell : cells)
    {
        double best_x = cell.x;
        double best_y = cell.y;
        double min_displacement = numeric_limits<double>::max();

        // Find the best row for the current cell
        Row *best_row = nullptr;
        for (auto &row : rows)
        {
            double row_x_min = row.x_min;
            double row_x_max = row.x_max - cell.width;

            // Ensure the cell fits within the row's horizontal bounds
            if (cell.x < row_x_min)
                cell.x = row_x_min;
            if (cell.x > row_x_max)
                cell.x = row_x_max;

            // Calculate displacement for this row
            double displacement = abs(cell.x - best_x) + abs(row.y - best_y);

            // Check if this row is better
            if (displacement < min_displacement)
            {
                min_displacement = displacement;
                best_row = &row;
            }
        }

        // Place the cell in the best row
        if (best_row)
        {
            cell.y = best_row->y;

            // Ensure cells in the same row do not overlap
            double current_x = best_row->x_min;
            for (auto &other_cell : cells)
            {
                if (other_cell.y == best_row->y)
                {
                    other_cell.x = max(current_x, other_cell.x);
                    current_x = other_cell.x + other_cell.width;
                }
            }
        }
    }
}

int main()
{
    // Example cells
    vector<Cell> cells = {
        {10.0, 15.0, 5.0, 2.0},
        {20.0, 25.0, 5.0, 2.0},
        {30.0, 35.0, 5.0, 2.0},
        {40.0, 45.0, 5.0, 2.0}};

    // Example rows
    vector<Row> rows = {
        {10.0, 100.0, 2.0, 0.0, 100.0},
        {20.0, 100.0, 2.0, 0.0, 100.0},
        {30.0, 100.0, 2.0, 0.0, 100.0},
        {40.0, 100.0, 2.0, 0.0, 100.0}};

    // Perform Abacus legalization
    abacusLegalization(cells, rows);

    // Output legalized cell positions
    for (const auto &cell : cells)
    {
        cout << "Cell at (" << cell.x << ", " << cell.y << ")\n";
    }

    return 0;
}