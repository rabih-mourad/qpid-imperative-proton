#!/bin/bash -e

readonly SCRIPT_DIR="$(dirname $(which $0))"
readonly PROJECT_DIR=$SCRIPT_DIR/..

readonly PARALLEL_RUNS=1

REPEAT_COUNT=500000
RUN_ONLY='n'

function run_tests {
    local nb_times_to_run=$1
    local test_results_dir=$PROJECT_DIR/Debug/test/results

    rm -rf $test_results_dir
    mkdir $test_results_dir

    pushd $test_results_dir >/dev/null
        local nb_of_loops=$((nb_times_to_run/PARALLEL_RUNS))
        local remainder=$((nb_times_to_run%PARALLEL_RUNS))
        local batch_count=1

        while (($batch_count<=$nb_of_loops )); do
            echo "Running batch $batch_count/$nb_of_loops with $PARALLEL_RUNS instance(s) in parallel..."
            run_tests_in_parallel $PARALLEL_RUNS $batch_count
            let batch_count++
        done

        if [ "$remainder" != "0" ]; then
            echo "Running last batch with $remainder instance(s) in parallel..."
            run_tests_in_parallel $remainder %batch_count
        fi

        tests_in_error
    popd >/dev/null
}

function run_tests_in_parallel {
    local nb_times_to_run=$1
    local batch_nb=$2
    local test_count=1

    while (($test_count<=$nb_times_to_run)); do
        timeout --signal 9 60s $PROJECT_DIR/Debug/test/qpid-imperative-proton-test >test-$batch_nb-$test_count.log 2>&1 && rm test-$batch_nb-$test_count.log  &
        pids[${n}]=$!
        let test_count++
    done

    echo "Waiting for process(es) to finish..."
    for pid in ${pids[*]}; do
        wait $pid || true
    done
}

function tests_in_error {
    remove_port_related_failures

    local failing=$(ls -l | grep test- | wc -l)

    if [ "$failing" == "0" ]; then
        echo "All tests passed successfully"
    else
        echo "$failing run(s) failed, results are under $PWD"

        grep -r '\[  FAILED  \] .* ms)' test-*.log

        local core=$(ls -l | grep core | wc -l)
        echo $core

        if [ "$core" != "0" ]; then
            echo "List of cores:"
            ls core*
        fi

        exit 1
    fi
}

function remove_port_related_failures {
    local portFailures=$(find . -type f -print0 | xargs -0 grep -l "listener has no port")

    echo "$(echo $portFailures | wc -w) run(s) failed because of port clashes."
    if [ -n "$portFailures" ]; then
        echo "Deleting corresponding outputs..."
        rm $portFailures
    fi
}

function main {
    if [ $RUN_ONLY == 'n'  ]; then
        echo 'Building the project'
        $SCRIPT_DIR/build.sh 
        echo 'Built the project'
    else
        echo 'Skipping build'
    fi

    echo "Running tests $REPEAT_COUNT times"
    time run_tests $REPEAT_COUNT
    echo 'Finished running tests'
}

function usage {
    echo """Usage: $0 [OPTION]
Builds the project and launches the tests $REPEAT_COUNT if not specified otherwise

Example: $0 --count=10 --run

Options:
 -h, --help     prints the usage
 -c, --count    number of times that the unit tests will be run, $REPEAT_COUNT if not specified
 -r, --run      run without rebuilding the project
"""
}

while (( "$#" )); do
    case "$1" in
        -h|--help)
            usage
            exit 0
            ;;
        -c|--count)
            REPEAT_COUNT=$2
            shift 2
            ;;
        -r|--run)
            RUN_ONLY='y'
            shift
            ;;
        --)
            shift
            break
            ;;
        *)
            echo 'Invalid option or argument'
            usage
            exit 1
            ;;
    esac;
done

main

