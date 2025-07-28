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

int main()
{
    int penalty;
    City cities[MAX_CITIES];
    int n = read_input("test_input.txt", &penalty, cities);

    if (n < 0)
    {
        fprintf(stderr, "Input reading failed.\n");
        return 1;
    }

    // print cities input order
    printf("Penalty: %d, Cities: %d\n", penalty, n);
    printf("By input order: \n");
    for (int i = 0; i < n; i++)
    {
        printf("%d: (%d, %d)\n", cities[i].id, cities[i].x, cities[i].y);
    }

    // compute the morton for each city
    for (int i = 0; i < n; i++)
    {
        cities[i].morton = morton_code(cities[i].x, cities[i].y);
    }

    // use qsort using the comparetor
    qsort(cities, n, sizeof(City), compare_morton);

    // print cities by morton order
    printf("By morton order: \n");
    for (int i = 0; i < n; i++)
    {
        printf("%d: (%d, %d)\n", cities[i].id, cities[i].x, cities[i].y);
    }

    return 0;
}