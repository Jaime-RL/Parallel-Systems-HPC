#pragma once // Ensures this header file is only included once per compilation unit

#include <cassert> // For runtime assertions (debug checks)
#include <cstdlib> // For malloc() and free()
#include <cmath>   // For sqrtf()
#include <immintrin.h> // For AVX intrinsics
#include <xmmintrin.h> // For SSE intrinsics
#include <iostream>
#include <cstdio>

// Inline helper function to compute the square of a number
static inline double sq(double x)
{
	return x * x;
}

// A simple dynamically allocated vector of doubles
// Provides basic functionality and a cosine similarity operation
struct vector
{
	// Constructor: allocates memory for n elements
	// Requires n to be positive and a multiple of 8 (likely for alignment/SIMD use)
	vector(const int n) : n(n)
	{
		assert(n > 0);								 // Validate size
		data = (double *)malloc(sizeof(double) * n); // Allocate raw memory
		assert(data);								 // Ensure allocation succeeded
	}

	// Destructor: frees allocated memory
	// Invoked automatically when a vector object goes out of scope or is deleted
	~vector()
	{
		free(data);
	}

	// Sets the value of the i-th element in the vector
	void set(double x, int i)
	{
		assert(i >= 0 && i < n); // Bounds check
		data[i] = x;
	}

	// Computes the cosine similarity between two vectors:
	static double cosine_similarity(const vector &a, const vector &b)
	{
		assert(a.n == b.n); // Vectors must be of the same dimension
		const int n = a.n;
		double ab = 0.0; // Dot product accumulator
		double aa = 0.0; // Magnitude accumulator for vector a
		double bb = 0.0; // Magnitude accumulator for vector b
		int i;

		#if defined(__AVX__)
			__m256d	av;
			__m256d bv;
			__m256d aav;
			__m256d abv;
			__m256d bbv;
			__m256d acc_aav = _mm256_setzero_pd(); // Accumulation of aav
			__m256d acc_bbv = _mm256_setzero_pd(); // Accumulation of bbv
			__m256d acc_abv = _mm256_setzero_pd(); // Accumulation of abv
			
			// We need a loop to load all the data into the registers
			for (i=0; i<4*(n/4); i+=4){
				av = _mm256_loadu_pd(a.data + i);
				bv = _mm256_loadu_pd(b.data + i);
				
				aav = _mm256_mul_pd(av, av);
				abv = _mm256_mul_pd(av, bv);
				bbv = _mm256_mul_pd(bv, bv);

				acc_aav = _mm256_add_pd(acc_aav, aav);
				acc_bbv = _mm256_add_pd(acc_bbv, bbv);
				acc_abv = _mm256_add_pd(acc_abv, abv);
			}
			
			__m256d hadd1 = _mm256_hadd_pd(acc_aav, acc_bbv); // Horizontal add for both aav and bbv
			__m256d hadd2 = _mm256_hadd_pd(acc_abv, acc_abv); // Horizontal add for abv
			
			aa = ((double*)&hadd1)[0] + ((double*)&hadd1)[2];
			bb = ((double*)&hadd1)[1] + ((double*)&hadd1)[3];
			ab = ((double*)&hadd2)[0] + ((double*)&hadd2)[2];
			
			// Handle remainin g elements
			while (i < n) {
				aa += sq(a.data[i]);
				bb += sq(b.data[i]);
				ab += a.data[i] * b.data[i];
				i++;
			}
			return ab / (sqrt(aa) * sqrt(bb));
		#elif defined(__SSE__)
			__m128d	av;
			__m128d bv;
			__m128d aav;
			__m128d abv;
			__m128d bbv;
			__m128d acc_aav = _mm_setzero_pd(); // Accumulation of aav
			__m128d acc_bbv = _mm_setzero_pd(); // Accumulation of bbv
			__m128d acc_abv = _mm_setzero_pd(); // Accumulation of abv
			
			// We need a loop to load all the data into the registers
			for (i=0; i<2*(n/2); i+=2){
				av = _mm_loadu_pd(a.data + i);
				bv = _mm_loadu_pd(b.data + i);
				
				aav = _mm_mul_pd(av, av);
				abv = _mm_mul_pd(av, bv);
				bbv = _mm_mul_pd(bv, bv);

				acc_aav = _mm_add_pd(acc_aav, aav);
				acc_bbv = _mm_add_pd(acc_bbv, bbv);
				acc_abv = _mm_add_pd(acc_abv, abv);
			}
			__m128d hadd1 = _mm_hadd_pd(acc_aav, acc_bbv); // Horizontal add for both aav and bbv
			__m128d hadd2 = _mm_hadd_pd(acc_abv, acc_abv); // Horizontal add for abv

			aa = ((double*)&hadd1)[0];
			bb = ((double*)&hadd1)[1];
			ab = ((double*)&hadd2)[0];

			// Handle remaining elements
			if(n%2 != 0)
			{	
				aa += sq(a.data[i]);
				bb += sq(b.data[i]);
				ab += a.data[i] * b.data[i];
			}

			return ab / (sqrt(aa) * sqrt(bb));;
		#else
			for (int i = 0; i < n; ++i)
			{
				aa += sq(a.data[i]);
				bb += sq(b.data[i]);
				ab += a.data[i] * b.data[i];
			}
			return ab / (sqrtf(aa) * sqrtf(bb));
		#endif
	}

private:
	const int n = 0;		// Dimension (size) of the vector
	double *data = nullptr; // Pointer to dynamically allocated array
};
