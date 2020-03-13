#!/bin/bash -e

readonly SCRIPT_DIR="$(dirname $(which $0))"
readonly PROJECT_DIR="${SCRIPT_DIR}/.."

function configure_and_build {
    local mode=$1
    local build_dir=$mode

    echo "Configuring and building project in $mode mode"

    echo "Cleaning old build directory $build_dir"
    rm -rf $build_dir
    echo 'Cleaned old build directory'

    mkdir $build_dir

    pushd $build_dir >/dev/null

        echo 'configuring project'
        cmake .. -DCMAKE_BUILD_TYPE=$mode -DCMAKE_CXX_FLAGS="-std=c++11"
        echo 'configured project'

        echo 'building project'
        make -j
        make test
        echo 'built project'

    popd >/dev/null


    echo "Configured and built project"
}

function main {
    pushd $PROJECT_DIR >/dev/null

        configure_and_build 'Debug'
        configure_and_build 'RelWithDebInfo'

    popd >/dev/null
}

main "$@"


