from collections import Counter

def check_duplicates(filename):
    with open(filename, 'r', encoding='utf-8') as file:
        lines = [line.strip() for line in file if line.strip()]

    counts = Counter(lines)
    duplicates = {id_: count for id_, count in counts.items() if count > 1}

    if duplicates:
        print(f"Total duplicated IDs: {len(duplicates)}")
        print("Duplicated IDs and their counts:")
        for id_, count in duplicates.items():
            print(f"ID: {id_} - Count: {count}")
    else:
        print("No duplicates found.")

# Example usage
check_duplicates('output-test.txt')
