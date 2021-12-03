import json
import os
import time

import pandas

def create_file_size(size, name):
    # Need to write in chunks for very large numbers!
    write_block_default_size = 100000
    num_writes = int(size/write_block_default_size)
    os.system(f"./run {name} -w {write_block_default_size} {num_writes}")

def time_run(name, block_size, size, mode):
    block_count = int(size/block_size)
    ofname = name.split('.')[0] + 'out.txt'
    t1 = time.time()
    resp = os.system(f"dd if={name} of={ofname} bs={block_size} count={block_count}")
    t2 = time.time()
    print(t2-t1)
    return t2-t1


def main():
    test_cases = "test_cases_part_two_dd.json"
    with open(test_cases, 'r') as test_cases:
        runs = json.loads(test_cases.read())


    for run in runs:
        ofname = run['name'].split('.')[0] + 'out.txt'
        if run['mode'] == 'r':
            if os.path.exists(run['name']):
                os.remove(run['name'])
            if os.path.exists(ofname):  
                os.remove(ofname)
            create_file_size(run['size'], run['name'])

        run['time_no_cache'] = time_run(run['name'], run['block_size'], run['size'], run['mode'])
        #run['time_cache'] = time_run(run['name'], run['block_size'], run['block_count'], run['mode'])

        if run['mode'] == 'r':
            os.remove(run['name'])
            os.remove(ofname)

    df = pandas.DataFrame(runs)
    print(df)
    df.to_csv('part_two_dd_analysis.csv')

if __name__ == "__main__":
    main()
