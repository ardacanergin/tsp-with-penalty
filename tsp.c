#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <math.h>
#include <time.h>

#define MAX_LINE 100
#define DEFAULT_MAX_CITIES 5000

// struct for cities
typedef struct
{
    int id;
    int x, y; // euclid coordinates
    uint64_t morton;
} City;

// read input
int read_input(const char *filename, int *penalty, City *cities)
{
    FILE *f = fopen(filename, "r");
    if (!f)
    {
        perror("File open error");
        return -1;
    }

    char line[MAX_LINE]; // input buffer
    int city_count = 0;

    // Read penalty
    if (fgets(line, sizeof(line), f) == NULL)
    {
        fprintf(stderr, "Error: Could not read penalty line\n");
        fclose(f);
        return -1;
    }

    *penalty = atoi(line); // first line always pentaly
    int id, x, y;

    while (fgets(line, sizeof(line), f))
    {
        // check for blank line (end of input)
        if (strlen(line) < 1)
            break;

        // input format is strict
        if (sscanf(line, "%d %d %d", &id, &x, &y) == 3)
        {
            cities[city_count].id = id;
            cities[city_count].x = x;
            cities[city_count].y = y;
            city_count++;
        }
    }
    fclose(f);
    return city_count; // Return the number of cities read
}

// get morton codes to sort every city in the 2D plane
// https://en.wikipedia.org/wiki/Space-filling_curve
// https://www.forceflow.be/2013/10/07/morton-encodingdecoding-through-bit-interleaving-implementations/

// if coordinates exceeds 2^16-1 use following mapping for morton
/*
x_mapped = (int)(((x - min_x) * 65535.0) / (max_x - min_x));
y_mapped = (int)(((y - min_y) * 65535.0) / (max_y - min_y));
*/

#include <stdint.h>

// Helper to "spread" bits (see note below)
uint32_t part1by1(uint32_t n)
{
    n &= 0x0000ffff; // Keep 16 bits
    n = (n | (n << 8)) & 0x00FF00FF;
    n = (n | (n << 4)) & 0x0F0F0F0F;
    n = (n | (n << 2)) & 0x33333333;
    n = (n | (n << 1)) & 0x55555555;
    return n;
}

uint64_t morton_code(int x, int y)
{
    // Assumes x and y are non-negative and <= 65535
    return ((uint64_t)part1by1(y) << 1) | part1by1(x);
}

int compare_morton(const void *a, const void *b)
{
    uint64_t ma = ((City *)a)->morton;
    uint64_t mb = ((City *)b)->morton;
    if (ma < mb)
        return -1;
    if (ma > mb)
        return 1;
    return 0;
}

int distance(const City *a, const City *b)
{
    double dx = a->x - b->x;
    double dy = a->y - b->y;
    return (int)(round(sqrt(dx * dx + dy * dy)));
}

// after finding a base tour with the Morton heuristic approach improve it by 2-opt
// 2 opt swap utility
void reverse(int *tour, int start, int end)
{
    while (start < end)
    {
        int tmp = tour[start];
        tour[start] = tour[end];
        tour[end] = tmp;
        start++;
        end--;
    }
}

// helper function
/* update so it can represent bigger numbers
int tour_length(const City *cities, const int *tour, int n)
{
    int len = 0;
    for (int i = 0; i < n; i++)
    {
        const City *a = &cities[tour[i]];
        const City *b = &cities[tour[(i + 1) % n]];
        len += distance(a, b);
    }
    return len;
} */

unsigned long long tour_length(const City *cities, const int *tour, int n)
{
    unsigned long long len = 0;
    for (int i = 0; i < n; i++)
    {
        const City *a = &cities[tour[i]];
        const City *b = &cities[tour[(i + 1) % n]];
        len += distance(a, b);
    }
    return len;
}


// Basic, full 2-opt
void two_opt(City *cities, int *tour, int n)
{
    int improved = 1;
    while (improved)
    {
        improved = 0;
        for (int i = 0; i < n - 1; i++)
        {
            for (int j = i + 2; j < n && (i != 0 || j != n - 1); j++)
            {
                int a = tour[i], b = tour[(i + 1) % n];
                int c = tour[j], d = tour[(j + 1) % n];
                int old_dist = distance(&cities[a], &cities[b]) + distance(&cities[c], &cities[d]);
                int new_dist = distance(&cities[a], &cities[c]) + distance(&cities[b], &cities[d]);
                if (new_dist < old_dist)
                {
                    reverse(tour, i + 1, j);
                    improved = 1;
                }
            }
        }
    }
}

// restirct how far aparat two cities can be, windows size parameter, to fasten the execution time for really large inputs
void two_opt_local(City *cities, int *tour, int n, int window)
{
    int loop_counter = 0;
    clock_t t_start_2opt = clock();
    double max_seconds = 200; // experimental can be changed but done get rid of stuccink improvement can also be deactivated
    int improved = 1, pass =0;;
    while (improved)
    {
        // Check time at the top of each major pass
        double elapsed = (double)(clock() - t_start_2opt) / CLOCKS_PER_SEC;
        if (elapsed > max_seconds ) {
            printf("2-opt local: Time limit of %.2f seconds reached, exiting early at pass %d\n", max_seconds, pass);
            break;
        }

        loop_counter = 0;
        improved = 0;
        for (int i = 0; i < n - 1; i++)
        {
            if (loop_counter++ % 100 == 0) // print every 100th iteration debugging reason
                printf("2-opt: loop_counter = %d / %d   : %d \n", loop_counter, n - 1, i);

            int j_start = i + 2;
            int j_end = (window > 0) ? (i + window) : n - 1;
            if (j_end >= n)
                j_end = n - 1;
            for (int j = j_start; j <= j_end && (i != 0 || j != n - 1); j++)
            {
                int a = tour[i], b = tour[(i + 1) % n];
                int c = tour[j], d = tour[(j + 1) % n];
                int old_dist = distance(&cities[a], &cities[b]) + distance(&cities[c], &cities[d]);
                int new_dist = distance(&cities[a], &cities[c]) + distance(&cities[b], &cities[d]);
                if (new_dist < old_dist)
                {
                    reverse(tour, i + 1, j);
                    improved = 1;
                }
            }
        }
        printf("out of the inner loop! passes done: %d\n", pass);
        pass++;
    }
}

// Run 2-opt on K random segments of size 'window'
void two_opt_random_regions(City *cities, int *tour, int n, int window, int K)
{
    srand(time(NULL));
    for (int k = 0; k < K; k++)
    {
        printf("value of k: %d \n",k);
        int start = rand() % n;
        int end = start + window;
        if (end >= n)
            end = n - 1;
        two_opt_local(cities, tour, n, end - start);
    }
}

// actual penalty logic: if connecting two cities directly each other + penalty costs less than original length skip the city
// simple greedy
int prune_tour(City *cities, int *tour, int *tour_size, int penalty)
{

    int removed = 0;
    int n = *tour_size;
    int *to_remove = malloc(n * sizeof(int));
    if (!to_remove)
    {
        fprintf(stderr, "Memory allocation failed in prune_tour.\n");
        exit(1);
    }

    // initialize
    for (int i = 0; i < n; i++)
        to_remove[i] = 0;

    // try removing each city (except endpoints for a cycle)
    for (int i = 0; i < n; i++)
    {
        if (n <= 3)
            break; // base case: must have at least 3 cities for a cycle

        int prev = (i - 1 + n) % n;
        int next = (i + 1) % n;

        int a = tour[prev], b = tour[i], c = tour[next]; // tour only contains indexes of the cities that is the same as array indexes but 1 less

        int orig = distance(&cities[a], &cities[b]) + distance(&cities[b], &cities[c]);
        int skip = distance(&cities[a], &cities[c]) + penalty;

        if (skip < orig)
        {
            // mark for removal
            to_remove[i] = 1;
            removed++;
        }
    }

    // remove marked city from tour

    if (removed > 0)
    {
        int m = 0;
        int *new_tour = malloc(n * sizeof(int));
        if (!new_tour)
        {
            fprintf(stderr, "Memory allocation failed in prune_tour.\n");
            exit(1);
        }
        for (int i = 0; i < n; i++)
        {
            if (!to_remove[i])
                new_tour[m++] = tour[i];
        }
        for (int i = 0; i < m; i++)
        {
            tour[i] = new_tour[i];
        }
        *tour_size = m;

        free(new_tour);
    }

    free(to_remove);

    return removed; // return the number of removed elements
}

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
    City *cities = malloc(max_cities * sizeof(City));
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
    int *tour = malloc(max_cities * sizeof(int));
    if (!cities || !tour)
    {
        fprintf(stderr, "Allocation failed!\n");
        return 1;
    }

    for (int i = 0; i < n; i++)
        tour[i] = i;

    printf("Initial tour length (Morton order): %llu\n", tour_length(cities, tour, n));

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
        two_opt_random_regions(cities, tour, n, 1000, 20); // randomness factor, region number and windows may be changed based on further tests
    }

    printf("Improved tour length (after 2-opt): %llu\n", tour_length(cities, tour, n));

    printf("Tour order (city IDs):\n");
    for (int i = 0; i < n; i++)
        printf("%d ", cities[tour[i]].id);
    printf("\n");

    // try to prune the tour if possible
    // number of prunes could be changed after some tests to optimize exectuion time over corerctness

    int tour_size = n;

    if (n <= 5000)
    {
        printf("Pruning 5 times...\n");
        for (int i = 0; i < 5; i++)
            prune_tour(cities, tour, &tour_size, penalty);
        two_opt(cities, tour, tour_size); // 2-opt again after pruning - this could also be changed since full 2-opt makes code slower
        // you may one to just make local 2-opt after pruning to cut down execution time or not do it at all
    }
    else if (n <= 20000)
    {
        printf("Pruning 5 times...\n");
        for (int i = 0; i < 5; i++)
            prune_tour(cities, tour, &tour_size, penalty);
        two_opt_local(cities, tour, tour_size, 500); // this is 2-opt after pruning
        // 2-opt window number of times or method of 2-opt full, local, random may change to better utilize execution time
    }
    else
    {
        printf("Pruning 5 times...\n");
        for (int i = 0; i < 5; i++)
            prune_tour(cities, tour, &tour_size, penalty);
        two_opt_random_regions(cities, tour, tour_size, 1000, 20); // further tests needed for better number of pruning or option of 2-opt based on execution time
        // with big N and big K, number of times random region two opts it takes a lot of time so test for moderity
    }

    // Final cost calculations
    unsigned long long final_tour_length = tour_length(cities, tour, tour_size);
    int skipped = n - tour_size;
    unsigned long long penalty_cost = (unsigned long long)skipped * (unsigned long long)penalty;
    unsigned long long total_cost = final_tour_length + penalty_cost;

    printf("Final tour after pruning and 2-opt:\n");
    printf("  Cities visited : %d\n", tour_size);
    printf("  Skipped cities : %d\n", skipped);
    printf("  Penalty cost   : %llu\n", penalty_cost);
    printf("  Tour length    : %llu\n", final_tour_length);
    printf("  Total cost     : %llu\n", total_cost);

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

    fprintf(fout, "%llu %d\n", total_cost, tour_size);
    for (int i = 0; i < tour_size; i++)
        fprintf(fout, "%d\n", cities[tour[i]].id);
    fprintf(fout, "\n");
    fclose(fout);

    free(cities);
    free(tour);

    // ==== ends here ===> execution time calculation
    clock_t end = clock();
    double elapsed_secs = (double)(end - start) / CLOCKS_PER_SEC;
    printf("Execution time: %.8f seconds\n", elapsed_secs);

    return 0;
}
