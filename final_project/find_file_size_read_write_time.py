import json
import os
import time

import pandas

def create_file_size(size, name):
    # Need to write in chunks for very large numbers!
    write_block_default_size = 100000
    num_writes = int(size/write_block_default_size)
    os.system(f"./part_one {name} -w {write_block_default_size} {num_writes}")

def time_run(name, block_size, block_count, mode, size):
    write_block_default_size = 100000
    num_writes = int(size/write_block_default_size)
    t1 = time.time()
    os.system(f"./part_one {name} -w {write_block_default_size} {num_writes}")
    resp = os.system(f"./part_one {name} -{mode} {block_size} {block_count}")
    t2 = time.time()
    print(t2-t1)
    return t2-t1


def main():
    test_cases = "test_cases_part_two_dd.json"
    with open(test_cases, 'r') as test_cases:
        runs = json.loads(test_cases.read())


    for run in runs:
        if run['mode'] == 'r':
            if os.path.exists(run['name']):
                os.remove(run['name'])
            #create_file_size(run['size'], run['name'])

        run['time_no_cache'] = time_run(run['name'], run['block_size'], run['block_count'], run['mode'], run['size'])
        #run['time_cache'] = time_run(run['name'], run['block_size'], run['block_count'], run['mode'])

        if run['mode'] == 'r':
            os.remove(run['name'])

    df = pandas.DataFrame(runs)
    print(df)
    df.to_csv('part_two_analysis_read_write_time.csv')

if __name__ == "__main__":
    main()

