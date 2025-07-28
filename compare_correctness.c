#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "city.h"
#include <math.h>
#include <stdint.h>
#include <limits.h>

// compile option -- gcc compare_correctness.c tsp.c tspw_brute_force.c -o compare_program -lm

#define MAX_LINE 100
#define MAX_CITIES 5000
#define MAX_N 5

int read_input(const char *filename, int *penalty, City *cities);
int compare_morton(const void *a, const void *b);
uint64_t morton_code(int x, int y);
int distance(const City *a, const City *b);
void tsp_bruteforce(City *cities, int n, int *perm, int l, long long *min_len, int *best_perm);

int main()
{
    int penalty;
    City cities[MAX_N]; // Use MAX_N (10) for brute-force safety
    int n = read_input("test_input.txt", &penalty, cities);

    if (n < 2)
    {
        fprintf(stderr, "Not enough cities for TSP.\n");
        return 1;
    }
    if (n > MAX_N)
    {
        fprintf(stderr, "Brute-force only supported for n <= %d. You have n = %d.\n", MAX_N, n);
        return 1;
    }

    // ------- Brute-force solution -------
    int perm[MAX_N];
    int best_perm[MAX_N];
    for (int i = 0; i < n; i++)
        perm[i] = i;
    for (int i = 0; i < n; i++)
        best_perm[i] = 0; // or leave uninitialized; not critical before use

    long long min_len = LLONG_MAX;

    tsp_bruteforce(cities, n, perm, 0, &min_len, best_perm);

    printf("\n=== Brute-force TSP ===\n");
    printf("Minimum tour length: %lld\nTour order (city IDs):\n", min_len);
    for (int i = 0; i < n; i++)
    {
        printf("%d ", cities[best_perm[i]].id);
    }
    printf("\n");

    // ------- Morton heuristic solution -------
    // Add morton code field for all cities
    for (int i = 0; i < n; i++)
    {
        cities[i].morton = morton_code(cities[i].x, cities[i].y);
    }
    qsort(cities, n, sizeof(City), compare_morton);

    int morton_len = 0;
    for (int i = 0; i < n; i++)
    {
        morton_len += distance(&cities[i], &cities[(i + 1) % n]);
    }

    printf("\n=== Morton Order Heuristic ===\n");
    printf("Tour length: %d\nTour order (city IDs):\n", morton_len);
    for (int i = 0; i < n; i++)
    {
        printf("%d ", cities[i].id);
    }
    printf("\n");

    // ------- Comparison -------
    printf("\nComparison: Morton tour is %d units %s than optimal.\n",
           abs(morton_len - min_len),
           (morton_len > min_len) ? "longer" : (morton_len < min_len ? "shorter" : "equal"));

    return 0;
}
