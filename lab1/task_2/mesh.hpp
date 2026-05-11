#pragma once // Ensure this header is included only once

#include "common.hpp" // definitions of point, ball, etc.
#include <cassert>	  // for assert()
#include <cstdlib>	  // for malloc() and free()
#include <cmath>	  // for sqrtf()
#include <immintrin.h> // for SIMD intrinsics

// return square of x
static inline float sq(const float &x)
{
	return x * x;
}

// return squared distance between two d-dimensional points
template <int d>
static inline float sqdst(const point<d> &a, const point<d> &b)
{
	float sum = 0.0f;
	for (int i = 0; i < d; ++i)
		sum += sq(a.v[i] - b.v[i]);
	return sum;
}

// mesh of n d-dimensional points (AoS layout)
template <const int d>
struct mesh
{
	// constructor: allocate memory for n points
	mesh(const int n) : n(n)
	{
		assert(n > 0 && n % 16 == 0);					 // size must be positive and multiple of 16
		data = (float**)malloc(sizeof(float*) * d); // allocate memory for d rows
		assert(data);
		for (int i = 0; i < d; ++i){
			data[i]= (float*)malloc(sizeof(float) * n);// allocate memory for n columns in each rows
			assert(data[i]);
		}

	}

	// destructor: free allocated memory
	~mesh()
	{
		for (int i=0; i<d; i++){
			free(data[i]);
		}
		free(data);
	}

	// set the i-th point in the mesh
	void set(const point<d> p, int i)
	{
		assert(i >= 0 && i < n);
		for(int j=0; j<d ; j++){
			data[j][i]=p.v[j];
		}
	}

	// compute enclosing ball: center = midpoint of min/max, radius = max distance to center
	ball<d> calc_ball()
	{
		ball<d> b;

		// compute min/max per coordinate to find center
		point<d> min;
		point<d> max;

		__m256 min_v;
		__m256 max_v;
		__m256 v;
		float max_a[8];
		float min_a[8];

		for (int i=0; i<d; i++){ // For each dimension
			// Load first 8 values
			min_v=_mm256_loadu_ps(&(data[i][0]));
			max_v=_mm256_loadu_ps(&(data[i][0]));

			// Compare rest of values, storing min and max in vectors
			for(int j=8; j<n;j+=8){
				v =_mm256_loadu_ps(&(data[i][j]));
				min_v = _mm256_min_ps(min_v, v);
				max_v = _mm256_max_ps(max_v, v);
			}

			_mm256_store_ps(max_a, max_v);
			_mm256_store_ps(min_a, min_v);

			min.v[i]=min_a[0];
			max.v[i]=max_a[0];
			
			// Final comparison to get overall min and max for this dimension
			for (int j = 1; j < 8; ++j){
				min.v[i] = min_a[j] < min.v[i] ? min_a[j] : min.v[i];
				max.v[i] = max_a[j] > max.v[i] ? max_a[j] : max.v[i];
			}
		}

		// center = midpoint of min/max
		for (int i = 0; i < d; ++i)
			b.center.v[i] = (max.v[i] - min.v[i]) * 0.5f + min.v[i];

		// compute radius = max distance from center
		__m256 *center_v = new __m256[d];
		__m256 diff_v;
		__m256 sqdiff_v;
		__m256 sum_v = _mm256_setzero_ps();
		max_v = _mm256_setzero_ps();
		
		// Load center vectors
		for(int j = 0; j < d; j++){
			center_v[j] = _mm256_set1_ps(b.center.v[j]);
		}
		
		// Process points in chunks of 8
		for(int i = 0; i < n; i += 8){
			sum_v = _mm256_setzero_ps(); // Reset sum vector for each chunk
			
			// Calculate squared distance for each point in the chunk
			for(int j = 0; j < d; j++){
				v = _mm256_loadu_ps(&(data[j][i]));
				diff_v = _mm256_sub_ps(v, center_v[j]);
				sqdiff_v = _mm256_mul_ps(diff_v, diff_v);
				sum_v = _mm256_add_ps(sum_v, sqdiff_v);
			}
			
			// Update max_v with the maximum squared distances
			max_v = _mm256_max_ps(max_v, sum_v);
		}
		
		// Final comparison to get overall max squared distance
		float maxsqdst = 0.0f;
		_mm256_store_ps(max_a, max_v);

		for (int j = 0; j < 8; ++j){
			maxsqdst = max_a[j] > maxsqdst ? max_a[j] : maxsqdst;
		}
		
		// sqrt only once for efficiency
		b.radius = sqrtf(maxsqdst);

		return b; // return enclosing ball
	}

	// return index of the farthest point from given point p
	int farthest(point<d> p)
	{	
		__m256 v;
		__m256 *center_v = new __m256[d];
		__m256 diff_v;
		__m256 sqdiff_v;
		__m256 sum_v = _mm256_setzero_ps();
		__m256 max_v = _mm256_setzero_ps();
		__m256i max_idx_v = _mm256_setzero_si256();
		__m256i idx_v;
		__m256 mask;
		__m256i mask_i;
		
		// Load center vectors
		for(int j = 0; j < d; j++){
			center_v[j] = _mm256_set1_ps(p.v[j]);
		}
		
		// Process points in chunks of 8
		for(int i = 0; i < n; i += 8){
			sum_v = _mm256_setzero_ps(); // Reset sum vector for each chunk
			
			// Calculate squared distance for each point in the chunk
			for(int j = 0; j < d; j++){
				v = _mm256_loadu_ps(&(data[j][i]));
				diff_v = _mm256_sub_ps(v, center_v[j]);
				sqdiff_v = _mm256_mul_ps(diff_v, diff_v);
				sum_v = _mm256_add_ps(sum_v, sqdiff_v);
			}

			idx_v = _mm256_set_epi32(i+7, i+6, i+5, i+4, i+3, i+2, i+1, i+0);

            // Mask: sum_v > max_v
            mask = _mm256_cmp_ps(sum_v, max_v, _CMP_GT_OQ);
            mask_i = _mm256_castps_si256(mask);

            // Update max_v and max_idx_v based on mask
            max_v = _mm256_blendv_ps(max_v, sum_v, mask);
            max_idx_v = _mm256_blendv_epi8(max_idx_v, idx_v, mask_i);
		}

		// Reduce to find overall max and index
        float max_a[8];
        int max_idx_a[8];
        _mm256_storeu_ps(max_a, max_v);
        _mm256_storeu_si256((__m256i*)max_idx_a, max_idx_v);

        float max = max_a[0];
        int max_idx = max_idx_a[0];
        for(int i=1; i<8; ++i){
            if(max_a[i] > max){
                max = max_a[i];
                max_idx = max_idx_a[i];
            }
        }

        return max_idx;
	}

private:
	const int n = 0;		  // number of points
	float **data = nullptr; // matrix storing the coordinates of the points
};