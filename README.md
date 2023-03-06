# Multithreaded Raytracer

Build this with the following command:

`cmake --build ./cmake-build-debug --target raytracing -j 12`

Run it like:

`$ ./cmake-build-debug/raytracing > image.ppm`

BUG:
- Image is transposed when it shouldn't be and a cheap fix is employed