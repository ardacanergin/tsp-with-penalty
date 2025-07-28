// tsp_bruteforce_standalone.c
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <limits.h>

#define MAX_N 10

typedef struct {
    int id;
    int x, y;
} City;

int distance(const City *a, const City *b) {
    double dx = a->x - b->x;
    double dy = a->y - b->y;
    return (int)(round(sqrt(dx*dx + dy*dy)));
}

void swap(int *a, int *b) {
    int tmp = *a;
    *a = *b;
    *b = tmp;
}

void tsp_bruteforce(City *cities, int n, int *perm, int l, long long *min_len, int *best_perm) {
    if (l == n) {
        long long total = 0;
        for (int i = 0; i < n; i++) {
            City *curr = &cities[perm[i]];
            City *next = &cities[perm[(i+1)%n]];
            total += distance(curr, next);
        }
        if (total < *min_len) {
            *min_len = total;
            for (int i = 0; i < n; i++) best_perm[i] = perm[i];
        }
        return;
    }
    for (int i = l; i < n; i++) {
        swap(&perm[l], &perm[i]);
        tsp_bruteforce(cities, n, perm, l+1, min_len, best_perm);
        swap(&perm[l], &perm[i]);
    }
}

int main() {
    char filename[100] = "test_input.txt"; // Change as needed
    FILE *f = fopen(filename, "r");
    if (!f) {
        perror("File open error");
        return 1;
    }

    int penalty;
    if (fscanf(f, "%d", &penalty) != 1) {
        fprintf(stderr, "Error reading penalty line.\n");
        fclose(f);
        return 1;
    }

    City cities[MAX_N];
    int n = 0;
    int id, x, y;
    // Read cities until blank line or EOF
    while (fscanf(f, "%d %d %d", &id, &x, &y) == 3 && n < MAX_N) {
        cities[n].id = id;
        cities[n].x = x;
        cities[n].y = y;
        n++;
    }
    fclose(f);

    if (n < 2) {
        fprintf(stderr, "Not enough cities.\n");
        return 1;
    }

    int perm[MAX_N], best_perm[MAX_N];
    for (int i = 0; i < n; i++) perm[i] = i;
    long long min_len = LLONG_MAX;

    tsp_bruteforce(cities, n, perm, 0, &min_len, best_perm);

    printf("Brute-force TSP result (n=%d):\n", n);
    printf("Minimum tour length: %lld\n", min_len);
    printf("Tour order (city IDs): ");
    for (int i = 0; i < n; i++) {
        printf("%d ", cities[best_perm[i]].id);
    }
    printf("\n");

    return 0;
}
