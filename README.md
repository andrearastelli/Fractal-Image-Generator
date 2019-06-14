# Fractal generator

Author: Andrea Rastelli
Email: andrearastelli@live.com

## Algorithm
Simple implementation of the Mandelbrot fractal algorithm

## Parallelised process
Given the very simplistic approach to the fractal generation and the image
generation, to parallelize the algorithm I've predetermined the pixel list
using a simple `std::vector` with a custom struct, defined as:
```c++
struct Pixel {
    int x;
    int y;
};
```
and populated with pixel coordinates for each pixel in the image, returning a vector of `WIDTH * HEIGHT` elements.
Then I've computed the number of core for the current machine and splitted the list such that each core can compute a sublist with the same number of elements.
In each thread I also store the pixel value in the bitmap image.

## Image Format
All images generated using this code are uncompressed BMP files.

