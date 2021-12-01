# CS-GY 6233 Final Project

## Introduction
This directory consists of the C code to read and write to disk with various parameters (thread count, block size, block count, read/write flag, file name). In addition, there are several python scripts used to measure the performance of the C programs. These python scripts rely on test cases defined in .json files.

## Compiling
To compile, simply run:

    ./build.sh

This will create binaries called run and part_one. part_one can be used to measure Disk IO without multiple threads, and run can be used with threads as a parameter, and will output the xor of all 4-byte integers in the file.

## Running
Run the binaries using the below commands:

    ./part_one <filename> [-r|-w] <block_size> <block_count>

    ./run <filename> [-r|-w] <block_size> <block_count> <num_threads>


## Read Mode
When in Read mode, block_count does not matter. We will simply read with a given block_size until the file has been completely read.

## Write Mode
Write mode is not multithreaded. No matter how many threads you pass in for num_threads, we will perform the write single-threaded. We will write pre-defined repeated characters to a file of size block_size * block_count.

## Performance Analysis/Testing
To analyze performance, we used two python scripts.

    python measure_raw_performance.py
    python find_file_size.py

The python scripts make use of the pandas library. Please pip install this library. We recommend using a virtual environment to isolate python dependencies.

measure_raw_performance.py runs the run binary with various parameters, while find_file_size runs the part_one binary with various parameters. These parameters are defined in json files as follows:

    [
        {
            "size": 100000000,
            "name": "one_hundred_mil_input.txt",
            "block_size": 1,
            "block_count": 1,
            "mode": "r"
        },
        {
            "size": 1000000000,
            "name": "one_bil_input.txt",
            "block_size": 1,
            "block_count": 1,
            "mode": "r"
        },
        ...
    ]

The python script is hard-coded to look for certain json files, and hardcoded to write results to certain naming conventions. See the below code:

    def main():
        test_cases = "part_six_test_cases.json"
        with open(test_cases, 'r') as test_cases:
            runs = json.loads(test_cases.read())


        for run in runs:
            if run['mode'] == 'r':
                if os.path.exists(run['name']):
                    os.remove(run['name'])
                create_file_size(run['size'], run['name'])

            run['time_no_cache'] = time_run(run['name'], run['block_size'], run['block_count'], run['mode'], run['num_threads'])
            run['time_cache'] = time_run(run['name'], run['block_size'], run['block_count'], run['mode'], run['num_threads'])

            if run['mode'] == 'r':
                os.remove(run['name'])

        df = pandas.DataFrame(runs)
        print(df)
        df.to_csv('part_six_analysis.csv')

Feel free to change the names of the json file and output file if you so choose. You can implement your own test cases this way. The test cases we used throughout our performance analysis are included in this repo.

## Additonal Notes
The final report for this project, which detaisl all of the results and methods used, can be found in OS_Final_Project_Report.pdf . Any miscellaneous files that are present in the repo that aren't explicityly mentioned in this README, are outlined in the report. Some of these include extra credit experiments, which require additional setup, also outlined in the report. 
