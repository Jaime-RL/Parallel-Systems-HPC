#include <iostream>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include "gol_seq.hpp"
#include "gol_mult.hpp"
#include "tui.hpp"

double gol_benchmark(GOL_Base& gol, int iters) {
    // Warm up
    for (int i = 0; i < 10; ++i) gol.run_iteration();

    // Real measurement
    double start = omp_get_wtime();
    for (int i = 0; i < iters; ++i) {
        gol.run_iteration();
    }
    double end = omp_get_wtime();
    
    double diff = end - start;
    return diff;
}

void full_benchmark(int width, int height, int iters, int n_threads) {
    std::cout << "=== BENCHMARK: GRID " << width << "x" << height << " (" << iters << " iters) ===\n";

    GOL_Seq  seq_no_pad(width, height, false);
    GOL_Seq  seq_pad(width, height, true);
    GOL_Mult mult_no_pad(width, height, false, n_threads);
    GOL_Mult mult_pad(width, height, true, n_threads);

    double t_seq_no = gol_benchmark(seq_no_pad, iters);
    double t_seq_pad = gol_benchmark(seq_pad, iters);
    double t_mult_no = gol_benchmark(mult_no_pad, iters);
    double t_mult_pad = gol_benchmark(mult_pad, iters);

    std::cout << "Sequential without padding: " << t_seq_no << " seconds\n";
    std::cout << "Sequential with padding:    " << t_seq_pad << " seconds\n";
    std::cout << "Multithreaded without pad:  " << t_mult_no << " seconds\n";
    std::cout << "Multithreaded with padding: " << t_mult_pad << " seconds\n";
}

/*
Accepts: 
--benchmark <width> <height> <n iterations> <n threads> 
or 
--demo <width> <height> as arguments
*/
int main(int argc, char *argv[]) {
    if (argc == 4 && std::string(argv[1]) == "--demo") {
        int width = std::stoi(argv[2]);
        int height = std::stoi(argv[3]);
        
        GOL_Mult gol(width, height, true, 4); // We'll run the demo with 4 threads
        run_demo(gol, width, height);
    }
    else if(argc == 6 && std::string(argv[1]) == "--benchmark"){
        int width = std::stoi(argv[2]);
        int height = std::stoi(argv[3]);
        int iters = std::stoi(argv[4]);
        int n_threads = std::stoi(argv[5]);

        full_benchmark(width, height, iters, n_threads);
    }
    else{
        std::cout << "Usage:\n";
        std::cout << "  " << argv[0] << " --demo <width> <height>\n";
        std::cout << "  " << argv[0] << " --benchmark <width> <height> <iters> <threads>\n";
    }

    return 0;
}