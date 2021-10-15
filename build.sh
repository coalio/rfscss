
if [[ $# -eq 0 || $1 = "--build-test" ]]
then
    if [ -x "$(command -v cmake)" ]
    then
        cmake . && cd src && cmake . && cd -
    else
        echo "[build.sh] CMake was not found."
        if [ -f /etc/arch-release ]
        then
            echo "[build.sh] Install CMake with 'sudo pacman -S cmake'"
            exit 1
        else
            if [ -x "$(command -v apt)" ]
            then
                echo "[build.sh] Install CMake with 'sudo apt install cmake'"
                exit 1
            else
                echo "[build.sh] Install CMake and run this script again"
                exit 1
            fi
        fi
    fi

    # make
    echo "[build.sh] finished"
fi

# if $1 is --test then
if [[ $1 = "--test" || $1 = "--build-test" ]]
then
    # Run tests
    # Check if luajit is a valid command
    if [ -x "$(command -v luajit)" ]
    then
        cd dev/unit_testing && luajit run-tests.lua
    elif [ -x "$(command -v lua)" ]
    then
        cd dev/unit_testing && lua run-tests.lua
    else
        echo "Unable to run tests: Lua is required."
        echo "Tests are meant for development, so you don't need to install Lua in order to use rfscss."
    fi
elif [ $# -eq 1 ]
then
    echo "Unknown option"
fi