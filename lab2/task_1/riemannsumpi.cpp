#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <omp.h>
#include <immintrin.h>

constexpr double correctpi = 3.14159265358979323846264338327950288419716939937510;

inline double f(const double x)
{
	return sqrt(1-x*x);
}

double riemann_seq(const unsigned long n)
{
	const double Dx = 1.0/n;
	double sum = 0.0;
	for(unsigned long i=0; i<n; ++i)
		sum += Dx*f(i*Dx);
	return sum;
}

double riemann_omp(const unsigned long n){
	const double Dx = 1.0/n;
	double sum = 0.0;

	#pragma omp parallel
	{
		double partial_sum=0.0;
		#pragma omp for
		for(unsigned long i=0; i<n;i++){
			partial_sum+= Dx*f(i*Dx);
		}

		#pragma omp critical
		{
			sum+=partial_sum;
		}
	}
	return sum;
}

double riemann_vectorized_omp(const unsigned long n)
{
	const double Dx = 1.0/n;
	const unsigned long vec_end = (n/4)*4;
	double sum = 0.0;
	__m256d sum_v = _mm256_setzero_pd(); // fits 4 doubles
	__m256d ones = _mm256_set1_pd(1.0);
	__m256d dx_v = _mm256_set1_pd(Dx);
	
	#pragma omp parallel
	{
		__m256d partial_sum_v = _mm256_setzero_pd(); // fits 4 doubles

		#pragma omp for
		for(unsigned long i = 0; i < vec_end; i += 4)
		{
			__m256d x = _mm256_set_pd((i+3)*Dx, (i+2)*Dx, (i+1)*Dx, (i+0)*Dx);
			__m256d fx = _mm256_sqrt_pd(_mm256_sub_pd(ones, _mm256_mul_pd(x, x)));
			__m256d dfx = _mm256_mul_pd(dx_v, fx);
			partial_sum_v = _mm256_add_pd(partial_sum_v, dfx);
		}

		#pragma omp critical
		{
			sum_v = _mm256_add_pd(sum_v, partial_sum_v);
		}
	}
	
	double buffer[4];
	_mm256_storeu_pd(buffer, sum_v);
	sum = buffer[0] + buffer[1] + buffer[2] + buffer[3];

	for (unsigned long i = vec_end; i < n; ++i)
	{
		sum += Dx * f(i * Dx);
	}

	return sum;
}

int main(int argc, char* argv[]) {

	const unsigned long n = strtoul(argv[1], nullptr, 0);
	const int nproc = omp_get_max_threads();
	assert(n>0 && n%2==0);

	printf("n = %lu\n", n);
	printf("nproc = %d\n", nproc);
	
	double pi;

	double ts = omp_get_wtime();
	pi = 4.0*riemann_seq(n);
	ts = omp_get_wtime()-ts;
	printf("seq, elapsed time = %.3f seconds, err = %.15f\n", ts, abs(correctpi-pi));

	ts = omp_get_wtime();
	pi = 4.0*riemann_omp(n);
	ts = omp_get_wtime()-ts;
	printf("omp, elapsed time = %.3f seconds, err = %.15f\n", ts, abs(correctpi-pi));

	ts = omp_get_wtime();
	pi = 4.0*riemann_vectorized_omp(n);
	ts = omp_get_wtime()-ts;
	printf("vec, elapsed time = %.3f seconds, err = %.15f\n", ts, abs(correctpi-pi));

	return EXIT_SUCCESS;
}
