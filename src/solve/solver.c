#include "../../include/solve/solver.h"

void free_grid(Grid *grid){
//destroys the grid 
    free(grid->cells);
    free(grid);
    grid = NULL;
}

Grid *init_grid(size_t rows, size_t cols){
// Init a grid of x rows and x cols
    Grid *grid = malloc(sizeof(Grid));
    if (grid == NULL){
        return NULL;
    }
    grid->cells = calloc(rows*cols, sizeof(char));
    if (grid->cells == NULL){
        free(grid);
        return NULL;
    }
    grid->rows = rows;
    grid->cols = cols;
    return grid;
}

void print_grid(Grid *G){
//Displays grid G

    int max_cols = G->cols;

    for (int i = 0; i< G->rows;i++){
        for (int j=0; j< G->cols;j++){
            printf("%c ",G->cells[i*max_cols + j]);
        }
        printf("\n");
    }
}

Grid *get_grid(char* filename){
//Loads a grid of letters from a file
    FILE *file = fopen(filename,"r");

    char* line = NULL;
    size_t len = 0;
    ssize_t read = 0;
    int max_cols = 0;
    int max_rows = 0;

    while ((read = getline(&line,&len,file)) > 0)
    {
        max_rows++;
        max_cols = strlen(line)-1;
    }

    Grid *grid = init_grid(max_rows,max_cols);
    if (grid == NULL){
        free(line);
        fclose(file);
        return NULL;
    }
    rewind(file);
    int rows = 0;

    while ((read = getline(&line,&len,file)) > 0)
    {
    int cols = 0;
        for (size_t i = 0; i < strlen(line)-1;i++){
            grid->cells[cols + rows*max_cols] = line[i];

            cols++;
        }
        rows++;
    }

    free(line);
    return grid;
}
int find_word(const Grid *g, const char *word,
              int *x1, int *y1, int *x2, int *y2){
    //Cherche un mot word dans la grille g
    int len = strlen(word);
    if (len == 0) return 0;
    int rows = g->rows;
    int cols = g->cols;

    for (int r = 0; r<rows; r++){
        for (int c = 0; c < cols; c++){
            for (int dr = -1; dr < 2; dr++){
                for (int dc = -1; dc < 2; dc++){
                    if (dr == 0 && dc == 0) continue;
                    
                    int end_r = r + dr * (len-1);
                    int end_c = c + dc * (len-1);
                    if (end_r < 0 || end_r >= rows){
                        continue;
                    }
                    if (end_c < 0 || end_c >= cols){
                        continue;
                    }

                    int k = 0;
                    char current = g->cells[(r + dr*k) * cols + (c + dc*k)];
                    while (k < len && word[k] - current == 32){
                        k++;
                        current = g->cells[(r + dr*k) * cols + (c + dc*k)];
                    }

                    if (k == len){
                        *x1 = c;
                        *y1 = r;
                        *x2 = end_c;
                        *y2 = end_r;
                        return 1;
                    }
                }
            }
        }
    }
    return 0;
}



int main(int argc, char *argv[]){
    if (argc != 3){
        fprintf(stderr, "Usage: %s <file> <word>\n", argv[0]);
        return 1;
    }

    Grid *g = get_grid(argv[1]);
    if (g == NULL){
        fprintf(stderr,"Error : imposible to read the grid '%s'\n",argv[1]);
        return 1;
    }


    int x1;
    int x2;
    int y1;
    int y2;
    if (find_word(g,argv[2],&x1,&y1,&x2,&y2)){
        printf("(%d,%d)(%d,%d)\n",x1,y1,x2,y2);
    }
    else {
        printf("Not found\n");
    }

    free_grid(g);
    return 0;
}
