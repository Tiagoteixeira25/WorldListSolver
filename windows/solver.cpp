#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#pragma warning (disable : 4996)
#define MAX 512

int directions[8][2] = {
    {0, 1},
    {0, -1},
    {1, 0},
    {-1, 0},
    {1, 1},
    {1, -1},
    {-1, 1},
    {-1, -1}
};

int solver_find(const char* filename,const char *word) {
    int len = strlen(word);
    FILE *f = fopen(filename, "r");
    if (f==NULL) {
        perror("Can not open file");
        return 1;
    }

    char grid[MAX][MAX];
    int rows = 0;
    int cols = 0;
    char line[512];

    while (fgets(line, sizeof(line), f)) {
        int col = 0;
        char *token = strtok(line, " \n");
        int rest = 512;
        while (*token&&rest--) 
        {
            grid[rows][col++] = toupper(token[0]);
            token++;
        }
        if (cols == 0) cols = col;
        rows++;
    }
    fclose(f);

    
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            if (grid[i][j] == word[0]) 
            {
                for (int d = 0; d < 8; d++) {
                    int dx = directions[d][0];
                    int dy = directions[d][1];
                    int x = i, y = j, k;
                    for (k = 1; k < len; k++) {
                        x += dx;
                        y += dy;

                        if (x < 0 || x >= rows || y < 0 || y >= cols)
                            break;
                        if (grid[x][y] != word[k])
                            break;
                    }

                    if (k == len) {
                        
                        printf("(%d,%d)(%d,%d)\n", i, j, x, y);
                        return 0;
                    }
                }
            }
        }
    }

    printf("Word not found\n");
    return 0;
}

