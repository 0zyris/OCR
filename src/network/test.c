#include "learn.h"

int main(void){
    srand(0);

    //Dataset
    Matrix *X = create_matrix(4, 2);
    double xs[8] = {0,0,  0,1,  1,0,  1,1};
    for (int i = 0; i < 8; i++){
        X->data[i] = xs[i];
    }

    Matrix *y = create_matrix(4, 1);
    double ys[4] = {1, 0, 0, 1};
    for (int i = 0; i < 4; i++){
        y->data[i] = ys[i];
    }

    //Réseau
    Network *net = network_init(2, 4, 1);
    double lr = 1.0;
    int repeats = 10000;

    //Apprentissage
    for (int repeat = 0; repeat < repeats; repeat++){
        ForwardCache *cache = forward(net, X);

        if (repeat % 1000 == 0){
            printf("repeat %5d   loss = %f\n", repeat, cost(cache->a2, y));
        }

        Gradients *grads = backward(net, cache, X, y);
        update_weights(net, grads, lr);

        free_cache(cache);
        free_gradients(grads);
    }

    //Fin on passe les données
    ForwardCache *final = forward(net, X);
    printf("Résultats :\n");
    for (int i = 0; i < 4; i++){
        printf("(%.0f, %.0f) -> attendu %.0f, obtenu %.3f\n",
               X->data[i*2], X->data[i*2 + 1], y->data[i], final->a2->data[i]);
    }

    free_cache(final);
    free_network(net);
    free_matrix(X);
    free_matrix(y);
    return 0;
}
