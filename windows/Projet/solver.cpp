#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include"Letter_Finder.h"
#pragma warning (disable : 4996)


struct solution{
    size_t x, y, tox, toy;
};


size_t directions[8][2] = {
    {0, 1},
    {0, -1},
    {1, 0},
    {-1, 0},
    {1, 1},
    {1, -1},
    {-1, 1},
    {-1, -1}
};

struct solution solver_find(struct str*grid,int cols,struct str *word) {
    size_t len = word->size-1;
    size_t rows = grid->size / cols;

    struct solution ret;
    memset(&ret, 0, sizeof(ret));
    for (size_t i = 0; i < rows; i++) {
        for (size_t j = 0; j < cols; j++) {
            if (grid->data[i* cols +j] == word->data[0]){
                for (int d = 0; d < 8; d++) {
                    size_t dx = directions[d][0];
                    size_t dy = directions[d][1];
                    size_t x = i, y = j, k;
                    for (k = 1; k < len; k++) {
                        x += dx;
                        y += dy;
                        if ( x >= rows ||  y >= cols)
                            break;
                        if (grid->data[x * cols + y] != word->data[k])
                            break;
                    }

                    if (k == len) {
                        ret.x = j;
                        ret.y = i;
                        ret.tox = y;
                        ret.toy = x;
                        return ret;
                    }
                }
            }
        }
    }
    return ret;
}

