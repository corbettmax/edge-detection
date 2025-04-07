# Edge detection

C++ implementation of edge detection algorithms. Forked from https://github.com/vaultah/edge-detection. 

Updated to Qt6 GUI
Parallelized with OpenMP
Added Sample Images

Currently available algorithms:

 - Canny edge detector
 - Sobel edge detector
 - Prewitt edge detector
 - Roberts cross
 - Scharr operator

The project includes GUI for viewing results.


Setup:
sudo apt update
sudo apt install cmake qt6-base-dev qtcreator build-essential
cd ./build
cmake ..
make
./EdgeDetection



