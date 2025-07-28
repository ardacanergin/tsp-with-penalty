#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include <math.h>

// -- Copy City struct, distance, prune_tour from your main code here --

void swap(int *a, int *b) {
    int tmp = *a;
    *a = *b;
    *b = tmp;
}

int tsp_bruteforce(const City *cities, int n, int penalty, int prune_count,
                   int *best_tour, int *best_tour_size, int *best_cost) {
    int *perm = malloc(n * sizeof(int));
    for (int i = 0; i < n; i++) perm[i] = i;
    int min_cost = INT_MAX;
    int min_tour[n], min_tour_size = n;

    // Heap’s algorithm for permutations
    void permute(int k) {
        if (k == 1) {
            int tour[n], tour_size = n;
            memcpy(tour, perm, n * sizeof(int));
            // Prune X times
            for (int i = 0; i < prune_count; i++)
                prune_tour((City *)cities, tour, &tour_size, penalty);
            int cost = tour_length(cities, tour, tour_size) + (n - tour_size) * penalty;
            if (cost < min_cost) {
                min_cost = cost;
                memcpy(min_tour, tour, tour_size * sizeof(int));
                min_tour_size = tour_size;
            }
        } else {
            for (int i = 0; i < k; i++) {
                permute(k-1);
                swap(&perm[k % 2 == 1 ? 0 : i], &perm[k-1]);
            }
        }
    }

    permute(n);

    // Output best found
    if (best_tour && best_tour_size && best_cost) {
        memcpy(best_tour, min_tour, min_tour_size * sizeof(int));
        *best_tour_size = min_tour_size;
        *best_cost = min_cost;
    }

    free(perm);
    return min_cost;
}

// don't ever try this one it is not feasible and the reason is given in the file 