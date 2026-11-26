#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include "learn.h"

#define TRAIN_IMAGES "emnist-letters-train-images-idx3-ubyte"
#define TRAIN_LABELS "emnist-letters-train-labels-idx1-ubyte"
#define TEST_IMAGES  "emnist-letters-test-images-idx3-ubyte"
#define TEST_LABELS  "emnist-letters-test-labels-idx1-ubyte"
#define WEIGHTS_FILE "weights.bin"

#define IMG_SIZE   784
#define NB_CLASSES 26
#define HIDDEN     256
#define BATCH      64
#define EPOCHS     20
#define LR         0.5

//Dataset loading

static uint32_t be32(FILE *f){
    unsigned char b[4];
    if (fread(b, 1, 4, f) != 4)
        return 0;
    return ((uint32_t)b[0] << 24) | ((uint32_t)b[1] << 16)
         | ((uint32_t)b[2] << 8)  |  (uint32_t)b[3];
}

//Returns n images of 784 bytes each, already transposed (upright)
static uint8_t *load_images(const char *path, uint32_t *n){
    FILE *f = fopen(path, "rb");
    if (!f){ perror(path); return NULL; }

    if (be32(f) != 0x803){
        fprintf(stderr, "%s: bad magic number\n", path);
        fclose(f);
        return NULL;
    }
    *n = be32(f);
    uint32_t rows = be32(f), cols = be32(f);
    if (rows * cols != IMG_SIZE){
        fprintf(stderr, "%s: unexpected image size\n", path);
        fclose(f);
        return NULL;
    }

    size_t total = (size_t)(*n) * IMG_SIZE;
    uint8_t *data = malloc(total);
    if (!data || fread(data, 1, total, f) != total){
        fprintf(stderr, "%s: read error\n", path);
        free(data);
        fclose(f);
        return NULL;
    }
    fclose(f);

    //EMNIST is stored transposed: swap across the diagonal
    for (uint32_t i = 0; i < *n; i++){
        uint8_t *img = data + (size_t)i * IMG_SIZE;
        for (int r = 0; r < 28; r++){
            for (int c = r + 1; c < 28; c++){
                uint8_t tmp = img[r * 28 + c];
                img[r * 28 + c] = img[c * 28 + r];
                img[c * 28 + r] = tmp;
            }
        }
    }
    return data;
}

//Returns n labels in 0..25 (the file stores 1..26)
static uint8_t *load_labels(const char *path, uint32_t *n){
    FILE *f = fopen(path, "rb");
    if (!f){ perror(path); return NULL; }

    if (be32(f) != 0x801){
        fprintf(stderr, "%s: bad magic number\n", path);
        fclose(f);
        return NULL;
    }
    *n = be32(f);

    uint8_t *labels = malloc(*n);
    if (!labels || fread(labels, 1, *n, f) != *n){
        fprintf(stderr, "%s: read error\n", path);
        free(labels);
        fclose(f);
        return NULL;
    }
    fclose(f);

    for (uint32_t i = 0; i < *n; i++){
        if (labels[i] < 1 || labels[i] > NB_CLASSES){
            fprintf(stderr, "%s: invalid label\n", path);
            free(labels);
            return NULL;
        }
        labels[i]--;
    }
    return labels;
}

//Batches

//Fills X (bs x 784) in [0,1] and Y (bs x 26) one-hot from sample indices idx
static void fill_batch(Matrix *X, Matrix *Y, const uint8_t *imgs,
    const uint8_t *labels, const uint32_t *idx, int bs){
    memset(Y->data, 0, (size_t)bs * NB_CLASSES * sizeof(double));
    for (int b = 0; b < bs; b++){
        const uint8_t *img = imgs + (size_t)idx[b] * IMG_SIZE;
        for (int p = 0; p < IMG_SIZE; p++)
            X->data[b * IMG_SIZE + p] = img[p] / 255.0;
        Y->data[b * NB_CLASSES + labels[idx[b]]] = 1.0;
    }
}

static void shuffle(uint32_t *a, uint32_t n){
    for (uint32_t i = n - 1; i > 0; i--){
        uint32_t j = (uint32_t)rand() % (i + 1);
        uint32_t tmp = a[i];
        a[i] = a[j];
        a[j] = tmp;
    }
}

//Accuracy (0..1) of the network on a dataset
static double evaluate(Network *net, const uint8_t *imgs,
    const uint8_t *labels, uint32_t n){
    uint32_t correct = 0;
    uint32_t idx[BATCH];

    for (uint32_t start = 0; start < n; start += BATCH){
        int bs = (n - start < BATCH) ? (int)(n - start) : BATCH;
        for (int b = 0; b < bs; b++)
            idx[b] = start + b;

        Matrix *X = create_matrix(bs, IMG_SIZE);
        Matrix *Y = create_matrix(bs, NB_CLASSES);
        fill_batch(X, Y, imgs, labels, idx, bs);

        ForwardCache *cache = forward_softmax(net, X);
        for (int b = 0; b < bs; b++){
            int best = 0;
            for (int k = 1; k < NB_CLASSES; k++){
                if (cache->a2->data[b * NB_CLASSES + k]
                    > cache->a2->data[b * NB_CLASSES + best])
                    best = k;
            }
            if (best == labels[idx[b]])
                correct++;
        }
        free_cache(cache);
        free_matrix(X);
        free_matrix(Y);
    }
    return (double)correct / n;
}

//Saving

static int write_matrix(FILE *f, const Matrix *m){
    size_t count = (size_t)m->rows * m->cols;
    return fwrite(m->data, sizeof(double), count, f) == count ? 0 : -1;
}

//File layout: 3 x uint32 (input, hidden, output), then the doubles
//of W1, b1, W2, b2 in this order (native endianness)
static int save_network(const Network *net, const char *path){
    FILE *f = fopen(path, "wb");
    if (!f){ perror(path); return -1; }

    uint32_t header[3] = {
        (uint32_t)net->W1->rows,
        (uint32_t)net->W1->cols,
        (uint32_t)net->W2->cols
    };
    int err = fwrite(header, sizeof(uint32_t), 3, f) != 3;
    err |= write_matrix(f, net->W1);
    err |= write_matrix(f, net->b1);
    err |= write_matrix(f, net->W2);
    err |= write_matrix(f, net->b2);

    if (fclose(f) != 0 || err){
        fprintf(stderr, "%s: write error\n", path);
        return -1;
    }
    return 0;
}

//Main

int main(void){
    srand(42);

    uint32_t n_train, n_train_lab, n_test, n_test_lab;
    uint8_t *train_img = load_images(TRAIN_IMAGES, &n_train);
    uint8_t *train_lab = load_labels(TRAIN_LABELS, &n_train_lab);
    uint8_t *test_img  = load_images(TEST_IMAGES, &n_test);
    uint8_t *test_lab  = load_labels(TEST_LABELS, &n_test_lab);

    if (!train_img || !train_lab || !test_img || !test_lab
        || n_train != n_train_lab || n_test != n_test_lab)
        return 1;

    printf("Train: %u images, Test: %u images\n", n_train, n_test);

    Network *net = network_init(IMG_SIZE, HIDDEN, NB_CLASSES);

    uint32_t *order = malloc(n_train * sizeof(uint32_t));
    for (uint32_t i = 0; i < n_train; i++)
        order[i] = i;

    Matrix *X = create_matrix(BATCH, IMG_SIZE);
    Matrix *Y = create_matrix(BATCH, NB_CLASSES);
    uint32_t nb_batches = n_train / BATCH;

    for (int epoch = 1; epoch <= EPOCHS; epoch++){
        clock_t start = clock();
        shuffle(order, n_train);

        double loss_sum = 0;
        uint32_t loss_count = 0;

        for (uint32_t b = 0; b < nb_batches; b++){
            fill_batch(X, Y, train_img, train_lab, order + b * BATCH, BATCH);

            ForwardCache *cache = forward_softmax(net, X);

            double loss = cost_softmax(cache->a2, Y);
            if (isfinite(loss)){
                loss_sum += loss;
                loss_count++;
            }

            Gradients *grads = backward(net, cache, X, Y);
            double lr = LR * pow(0.85, epoch - 1);
            update_weights(net, grads, lr);

            free_cache(cache);
            free_gradients(grads);

            if (b % 200 == 0){
                printf("\r  epoch %d: batch %u/%u", epoch, b, nb_batches);
                fflush(stdout);
            }
        }

        double acc = evaluate(net, test_img, test_lab, n_test);
        double secs = (double)(clock() - start) / CLOCKS_PER_SEC;
        printf("\repoch %2d/%d  loss = %.4f  test accuracy = %.2f%%  (%.0fs)\n",
               epoch, EPOCHS, loss_count ? loss_sum / loss_count : 0.0,
               acc * 100.0, secs);

        if (save_network(net, WEIGHTS_FILE) == 0)
            printf("  weights saved in %s\n", WEIGHTS_FILE);
    }

    free_matrix(X);
    free_matrix(Y);
    free(order);
    free_network(net);
    free(train_img);
    free(train_lab);
    free(test_img);
    free(test_lab);
    return 0;
}
