import random

n = 50000           # Number of cities (change to 50000 if you want bigger)
penalty = 1000

with open("really_big_test_input.txt", "w") as f:
    f.write(f"{penalty}\n")
    for city_id in range(1, n + 1):
        x = random.randint(0, 99999)
        y = random.randint(0, 99999)
        f.write(f"{city_id} {x} {y}\n")
