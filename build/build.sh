
if [[ $# -eq 0 || $1 = "--build-install" ]]
then
    if [ -x "$(command -v cmake)" ]
    then
        cmake .. && make
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
    echo "[build.sh] finished bulding"
fi

# if $1 is --test then
if [[ $1 = "--test" ]]
then
    # Run tests
    # Check if luajit is a valid command
    if [ -x "$(command -v rfscss)" ]
    then
        if [ -x "$(command -v luajit)" ]
        then
            cd ../dev/unit_testing && luajit run-tests.lua
        elif [ -x "$(command -v lua)" ]
        then
            cd ../dev/unit_testing && lua run-tests.lua
        else
            echo "[build.sh] Unable to run tests: Lua is required."
            echo "[build.sh] Tests are meant for development, so you don't need to install Lua in order to use rfscss."
        fi
    else
        echo "[build.sh] Unable to run tests: can't find rfscss at PATH."
        echo "[build.sh] Install rfscss first using 'sudo make install' or './build.sh --build-install'"
    fi
elif [[ $1 = "--install" || $1 = "--build-install" || $1 = "-i" ]]
then
    # Install rfscss
    if [ -x "$(command -v make)" ]
    then
        sudo make install
        echo "[build.sh] finished installing"
    else
        echo "[build.sh] Make is not found."
        echo "[build.sh] Install make with 'sudo pacman -S make'"
        exit 1
    fi
elif [ $# -eq 1 ]
then
    echo "Unknown option"
fi