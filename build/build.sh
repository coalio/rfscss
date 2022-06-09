
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
            elif [ -x "$(command -v yum)" ]
            then
                echo "[build.sh] Install CMake with 'sudo yum install cmake'"
                exit 1
            elif [ -x "$(command -v dnf)" ]
            then
                echo "[build.sh] Install CMake with 'sudo dnf install cmake'"
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

if [[ $1 = "--install" || $1 = "--build-install" || $1 = "-i" ]]
then
    # Install rfscss
    if [ -x "$(command -v make)" ]
    then
        sudo make install
        echo "[build.sh] finished installing"
    else
        echo "[build.sh] GNU Make is not installed on your system."
        exit 1
    fi
elif [ $# -eq 1 ]
then
    echo "Unknown option"
fi