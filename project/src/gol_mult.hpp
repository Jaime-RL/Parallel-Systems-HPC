#ifndef GOL_MULT_HPP
#define GOL_MULT_HPP

#include "gol_base.hpp"
#include <omp.h>

class GOL_Mult : public GOL_Base {
private:
    int n_threads;

public:
    // Actualizamos el constructor para recibir el parámetro
    GOL_Mult(int w, int h, bool padding, int threads) 
        : GOL_Base(w, h, padding), n_threads(threads) {}

    void run_iteration() override;

protected:
    void update_ghost_borders();
};

#endif