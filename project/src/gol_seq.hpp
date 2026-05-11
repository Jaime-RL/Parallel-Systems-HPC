#ifndef GOL_SEQ_HPP
#define GOL_SEQ_HPP

#include "gol_base.hpp"

class GOL_Seq : public GOL_Base {
public:
    GOL_Seq(int w, int h, bool padding) : GOL_Base(w, h, padding) {}

    void run_iteration() override;

protected:
    void update_ghost_borders();
};

#endif