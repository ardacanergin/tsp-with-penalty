def check_ids_in_tsp_files(bigger_file, lesser_file):
    # Extract only the first integer (ID) from each line in the bigger file
    with open(bigger_file, 'r', encoding='utf-8') as bf:
        big_ids = set(line.strip().split()[0] for line in bf if line.strip())

    # Read all IDs from the lesser file (assumes each line is just the ID)
    with open(lesser_file, 'r', encoding='utf-8') as lf:
        less_ids = [line.strip() for line in lf if line.strip()]

    # Check for missing IDs
    missing = [id_ for id_ in less_ids if id_ not in big_ids]

    if missing:
        print(f"The following {len(missing)} ID(s) from '{lesser_file}' do NOT exist in '{bigger_file}':")
        for id_ in missing:
            print(id_)
    else:
        print(f"All IDs in '{lesser_file}' exist in '{bigger_file}'.")

# Example usage
check_ids_in_tsp_files('test-input-4.txt', 'output-test.txt')

