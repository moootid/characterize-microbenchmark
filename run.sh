#!/bin/sh


# run 'make clean' and 'make'
make clean
make
# Check if python is installed
if command -v python3 &>/dev/null; then
    echo "Python 3 is installed"
else
    echo "Python 3 is not installed"
    echo "Installing Python 3..."
    # Update package list
    sudo apt-get update
    # Install Python 3
    sudo apt-get install -y python3
fi
clear
./build/blackscholes -i vec 
python3 ./sleep.py
./build/blackscholes -i vec -d test
python3 ./sleep.py
./build/blackscholes -i vec -d dev
python3 ./sleep.py
./build/blackscholes -i vec -d small
python3 ./sleep.py
./build/blackscholes -i vec -d medium
python3 ./sleep.py
./build/blackscholes -i vec -d large
python3 ./sleep.py
./build/blackscholes -i vec -d native
python3 ./sleep.py
./build/blackscholes -i scalar 
python3 ./sleep.py
./build/blackscholes -i scalar -d test
python3 ./sleep.py
./build/blackscholes -i scalar -d dev
python3 ./sleep.py
./build/blackscholes -i scalar -d small
python3 ./sleep.py
./build/blackscholes -i scalar -d medium
python3 ./sleep.py
./build/blackscholes -i scalar -d large
python3 ./sleep.py
./build/blackscholes -i scalar -d native
python3 ./analytics.py
#test, dev, small, medium, large, native
