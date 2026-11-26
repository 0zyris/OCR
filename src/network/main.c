#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

static uint32_t be32(FILE *f) {
    unsigned char b[4];
    fread(b, 1, 4, f);
    return (b[0] << 24) | (b[1] << 16) | (b[2] << 8) | b[3];
}

int main(void) {
    FILE *fi = fopen("emnist-letters-train-images-idx3-ubyte", "rb");
    FILE *fl = fopen("emnist-letters-train-labels-idx1-ubyte", "rb");
    if (!fi || !fl) return 1;

    be32(fi);
    uint32_t n = be32(fi);
    uint32_t rows = be32(fi), cols = be32(fi);
    be32(fl); be32(fl);

    unsigned char *raw = malloc(rows * cols);
    for (uint32_t i = 0; i < 3 && i < n; i++) {
        fread(raw, 1, rows * cols, fi);
        int label = fgetc(fl);
        printf("Image %u : lettre %c\n", i, 'A' + label - 1);
        for (uint32_t r = 0; r < rows; r++) {
            for (uint32_t c = 0; c < cols; c++)
                putchar(raw[c * rows + r] > 128 ? '#' : '.');
            putchar('\n');
        }
    }
    free(raw);
    fclose(fi); fclose(fl);
    return 0;
}
