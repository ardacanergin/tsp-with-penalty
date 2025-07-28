#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "city.h"
#include <math.h>
#include <stdint.h>

#define MAX_LINE 100
#define MAX_CITIES 5000

int read_input(const char *filename, int *penalty, City *cities);
int compare_morton(const void *a, const void *b);
uint64_t morton_code(int x, int y);
int distance(const City *a, const City *b);

int main(void)
{

    int penalty;
    City cities[MAX_CITIES];

    int n = read_input("test_input.txt", &penalty, cities);

    if (n < 0)
    {
        fprintf(stderr, "Input reading failed.\n");
        return 1;
    }   

    // Compute Morton code for each city
    for (int i = 0; i < n; i++)
    {
        cities[i].morton = morton_code(cities[i].x, cities[i].y);
    }

    qsort(cities, n, sizeof(City), compare_morton);

    // === Build and evaluate tour ===
    int tour_length = 0;

    for (int i = 0; i < n; i++)
    {
        City *curr = &cities[i];
        City *next = &cities[(i+1)%n]; // wrap-around for the cycle
        tour_length += distance(curr, next);
    }
    
    printf("Initial tour length (all cities, Morton order): %d\n", tour_length);

    printf("Tour order (city IDs):\n");
    for (int i = 0; i < n; i++)
    {
        printf("%d\n", cities[i].id);
    }

    return 0;

}