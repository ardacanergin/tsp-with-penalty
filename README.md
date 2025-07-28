# TSP with Penalties Solver

This C program heuristically solves the Traveling Salesman Problem (TSP) **with penalties** for skipped cities. It uses Morton code sorting, various 2-opt local search heuristics, and tour pruning to find a low-cost tour given an input file of cities and penalties.

## Features

- Reads cities and penalty info from a file
- Initializes the tour using Morton order (spatial locality)
- Applies:
  - Full 2-opt optimization (for small instances)
  - Local or random-region 2-opt (for larger instances)
- Prunes cities to reduce overall cost, applies penalty for each skipped city
- Writes the resulting tour and cost to `output.txt`

## Compilation

Requires a C compiler (e.g., `gcc`):

## Usage

./tsp_with_penalty <inputfile> [--maxCities N]

```bash
gcc -o tsp_with_penalty main.c -lm
