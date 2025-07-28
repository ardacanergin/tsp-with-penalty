#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "city.h"
#include <math.h>
#include <stdint.h>
#include <time.h>

#define MAX_LINE 100
#define DEFAULT_MAX_CITIES 5000

int read_input(const char *filename, int *penalty, City *cities);
int compare_morton(const void *a, const void *b);
uint64_t morton_code(int x, int y);
int distance(const City *a, const City *b);
void two_opt_random_regions(City *cities, int *tour, int n, int window, int K);
void two_opt_local(City *cities, int *tour, int n, int window);
void two_opt(City *cities, int *tour, int n);
int tour_length(const City *cities, const int *tour, int n);
int prune_tour(City *cities, int *tour, int *tour_size, int penalty);

int main(int argc, char *argv[])
{
    clock_t start = clock(); // <-- START HERE

    int penalty;
    int max_cities = DEFAULT_MAX_CITIES;
    char *input_file = NULL;

    // Parse arguments
    if (argc < 2)
    {
        fprintf(stderr, "Usage: %s <inputfile> [--maxCities N]\n", argv[0]);
        return 1;
    }
    input_file = argv[1];
    if (argc >= 4 && strcmp(argv[2], "--maxCities") == 0)
    {
        max_cities = atoi(argv[3]);
        if (max_cities <= 0)
        {
            fprintf(stderr, "Invalid value for --maxCities\n");
            return 1;
        }
    }

    // Now allocate city array using max_cities
    City cities[max_cities]; // if there are too many cities like 1 million the stack will be to small for this array and we will need to use malloc
    if (!cities)
    {
        fprintf(stderr, "Failed to allocate memory for %d cities.\n", max_cities);
        return 1;
    }

    int n = read_input(input_file, &penalty, cities);
    if (n > max_cities)
    {
        fprintf(stderr, "Input file has %d cities, but max allowed is %d.\n", n, max_cities);
        n = max_cities;
    }

    // Find morton codes and sort the cities for good estimate of initial tour
    // Step 1: Find min and max for x and y
    int min_x = cities[0].x, max_x = cities[0].x;
    int min_y = cities[0].y, max_y = cities[0].y;
    for (int i = 1; i < n; i++)
    {
        if (cities[i].x < min_x)
            min_x = cities[i].x;
        if (cities[i].x > max_x)
            max_x = cities[i].x;
        if (cities[i].y < min_y)
            min_y = cities[i].y;
        if (cities[i].y > max_y)
            max_y = cities[i].y;
    }

    // Step 2: Map all coordinates to [0,65535] and assign Morton codes
    for (int i = 0; i < n; i++)
    {
        int x_mapped = (max_x == min_x) ? 0 : (int)(((cities[i].x - min_x) * 65535.0) / (max_x - min_x));
        int y_mapped = (max_y == min_y) ? 0 : (int)(((cities[i].y - min_y) * 65535.0) / (max_y - min_y));
        cities[i].morton = morton_code(x_mapped, y_mapped);
    }

    // assume initalized tour holds the order of morton order, it has nothing do with ids of the cities
    int tour[max_cities];
    for (int i = 0; i < n; i++)
        tour[i] = i;

    printf("Initial tour length (Morton order): %d\n", tour_length(cities, tour, n));

    // Choose 2-opt version based on the input size: 2-opt might blow the execution time if not restricted
    // espicially for large input sizes, the choise of doing partial 2-opt thereof

    if (n <= 5000)
    {
        printf("Running full 2-opt...\n");
        two_opt(cities, tour, n);
    }
    else if (n <= 20000)
    {
        printf("Running local 2-opt with window 500...\n");
        two_opt_local(cities, tour, n, 500); // windows can change based on the further tests
    }
    else
    {
        printf("Running random-region 2-opt: 1000 regions, window 1000...\n");
        two_opt_random_regions(cities, tour, n, 1000, 1000); // randomness factor, region number and windows may be changed based on further tests
    }

    printf("Improved tour length (after 2-opt): %d\n", tour_length(cities, tour, n));

    printf("Tour order (city IDs):\n");
    for (int i = 0; i < n; i++)
        printf("%d ", cities[tour[i]].id);
    printf("\n");

    // try to prune the tour if possible
    // number of prunes could be changed after some tests to optimize exectuion time over corerctness

    int tour_size = n;

    if (n <= 5000)
    {
        printf("Pruning 3 times...\n");
        for (int i = 0; i < 3; i++)
            prune_tour(cities, tour, &tour_size, penalty);
        two_opt(cities, tour, n); // 2-opt again after pruning - this could also be changed since full 2-opt makes code slower
        // you may one to just make local 2-opt after pruning to cut down execution time or not do it at all
    }
    else if (n <= 20000)
    {
        printf("Pruning 3 times...\n");
        for (int i = 0; i < 3; i++)
            prune_tour(cities, tour, &tour_size, penalty);
        two_opt_local(cities, tour, n, 500); // this is 2-opt after pruning
        // 2-opt window number of times or method of 2-opt full, local, random may change to better utilize execution time
    }
    else
    {
        printf("Pruning 3 times...\n");
        for (int i = 0; i < 3; i++)
            prune_tour(cities, tour, &tour_size, penalty);
        two_opt_random_regions(cities, tour, n, 1000, 1000); // further tests needed for better number of pruning or option of 2-opt based on execution time
    }

    // Final cost calculations
    int final_tour_length = tour_length(cities, tour, tour_size);
    int skipped = n - tour_size;
    int penalty_cost = skipped * penalty;
    int total_cost = final_tour_length + penalty_cost;

    printf("Final tour after pruning and 2-opt:\n");
    printf("  Cities visited : %d\n", tour_size);
    printf("  Skipped cities : %d\n", skipped);
    printf("  Penalty cost   : %d\n", penalty_cost);
    printf("  Tour length    : %d\n", final_tour_length);
    printf("  Total cost     : %d\n", total_cost);

    printf("Tour order (city IDs):\n");
    for (int i = 0; i < tour_size; i++)
        printf("%d ", cities[tour[i]].id);
    printf("\n");

    // === WRITING TO OUTPUTFILE ===

    FILE *fout = fopen("output.txt", "w");
    if (!fout)
    {
        perror("Could not open output file");
        return 1;
    }

    fprintf(fout, "%d %d\n", total_cost, tour_size);
    for (int i = 0; i < tour_size; i++)
        fprintf(fout, "%d\n", cities[tour[i]].id);
    fprintf(fout, "\n");
    fclose(fout);

    // ==== ends here ===> execution time calculation
    clock_t end = clock();
    double elapsed_secs = (double)(end - start) / CLOCKS_PER_SEC;
    printf("Execution time: %.8f seconds\n", elapsed_secs);

    return 0;
}