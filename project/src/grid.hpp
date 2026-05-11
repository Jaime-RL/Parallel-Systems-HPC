#ifndef GRID_HPP
#define GRID_HPP

#include <cstdlib>
#include <random>
#include <algorithm>

#ifndef LEVEL1_DCACHE_LINESIZE
#define LEVEL1_DCACHE_LINESIZE 64 //bytes

class Grid {
public:
    int width, height, stride;
    unsigned char* data;

    Grid(int width, int height, bool usePadding) : width(width), height(height) {
        if (usePadding) {
            // Align the stride to the cache line size
            stride = ((width + LEVEL1_DCACHE_LINESIZE - 1) / LEVEL1_DCACHE_LINESIZE) * LEVEL1_DCACHE_LINESIZE;
        } else {
            stride = width;
        }

        size_t total_size = stride * height;
        // aligned_alloc requires the size to be a multiple of the line size
        size_t alloc_size = ((total_size + LEVEL1_DCACHE_LINESIZE - 1) / LEVEL1_DCACHE_LINESIZE) * LEVEL1_DCACHE_LINESIZE;
        
        data = (unsigned char*)aligned_alloc(64, alloc_size);
        std::fill(data, data + total_size, 0);
    }

    ~Grid() {
        free(data);
    }

    inline unsigned char& at(int x, int y) {
        return data[y * stride + x];
    }

    inline const unsigned char& at(int x, int y) const {
        return data[y * stride + x];
    }

    // Generate random noise, with the option to set density and seed (for reproducibility)
    void randomize(float density = 0.3f, unsigned int seed = std::random_device{}()) {
        std::mt19937 gen(seed);
        std::uniform_real_distribution<float> dis(0.0f, 1.0f);

        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                at(x, y) = (dis(gen) < density) ? 1 : 0;
            }
        }
    }
};

#endif
#endif