#pragma once


#ifndef _H_SOLVER_
#define _H_SOLVER_
#include"Letter_Finder.h"
struct solution {
    size_t x, y, tox, toy;
};
struct solution solver_find(struct str* grid, int cols, struct str* word);
#endif