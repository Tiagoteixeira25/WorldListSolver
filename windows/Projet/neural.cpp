#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include "neural.h"
static double rand_weight()
{
    return ((double)rand() / RAND_MAX) * 0.2 - 0.1;
}

static double sigmoid(double x)
{
    return 1.0 / (1.0 + exp(-x));
}

static double sigmoid_derivative(double y)
{
    return y * (1.0 - y);
}


Network create_network(int num_inputs, int num_hidden, int num_outputs, double lr)
{
    Network net;
    net.learning_rate = lr;
    net.num_layers = 3;
    net.layers = (Layer*)malloc(sizeof(Layer) * net.num_layers);
    net.layers[0].num_neurons = num_inputs;
    net.layers[0].neurons = (Neuron*)calloc(num_inputs, sizeof(Neuron));
    net.layers[1].num_neurons = num_hidden;
    net.layers[1].neurons = (Neuron*)malloc(sizeof(Neuron) * num_hidden);
    for (int i = 0; i < num_hidden; i++)
    {
        net.layers[1].neurons[i].num_inputs = num_inputs;
        net.layers[1].neurons[i].weights =(double*) malloc(sizeof(double) * num_inputs);
        for (int j = 0; j < num_inputs; j++)
        {
            net.layers[1].neurons[i].weights[j] = rand_weight();
        }
        net.layers[1].neurons[i].bias = rand_weight();
    }
    net.layers[2].num_neurons = num_outputs;
    net.layers[2].neurons =(Neuron*) malloc(sizeof(Neuron) * num_outputs);
    for (int i = 0; i < num_outputs; i++)
    {
        net.layers[2].neurons[i].num_inputs = num_hidden;
        net.layers[2].neurons[i].weights = (double*)malloc(sizeof(double) * num_hidden);
        for (int j = 0; j < num_hidden; j++)
        {
            net.layers[2].neurons[i].weights[j] = rand_weight();
        }
        net.layers[2].neurons[i].bias = rand_weight();
    }

    return net;
}





void free_network(Network* net)
{
    for (int l = 0; l < net->num_layers; l++)
    {
        Layer* layer = &net->layers[l];
        for (int n = 0; n < layer->num_neurons; n++)
        {
            free(layer->neurons[n].weights);
        }
        free(layer->neurons);
    }
    free(net->layers);
}

void forward(Network* net, double* inputs) {
    for (int i = 0; i < net->layers[0].num_neurons; i++)
    {
        net->layers[0].neurons[i].output = inputs[i];
    }

    for (int l = 1; l < net->num_layers; l++)
    {
        Layer* prev = &net->layers[l - 1];
        Layer* curr = &net->layers[l];
        for (int i = 0; i < curr->num_neurons; i++)
        {
            Neuron* n = &curr->neurons[i];
            double sum = n->bias;
            for (int j = 0; j < n->num_inputs; j++)
            {
                sum += n->weights[j] * prev->neurons[j].output;
            }
            n->output = sigmoid(sum);
        }
    }
}

void backprop_alpha(Network* net, double* inputs, double* targets, double alpha_weight) {
    forward(net, inputs);
    Layer* out = &net->layers[2];
    for (int i = 0; i < out->num_neurons; i++)
    {
        double o = out->neurons[i].output;
        double error = (targets[i] - o) * alpha_weight;
        out->neurons[i].delta = error * sigmoid_derivative(o);
    }
    Layer* hidden = &net->layers[1];
    for (int i = 0; i < hidden->num_neurons; i++)
    {
        double error = 0.0;
        for (int j = 0; j < out->num_neurons; j++)
        {
            error += out->neurons[j].delta * out->neurons[j].weights[i];
        }
        hidden->neurons[i].delta = error * sigmoid_derivative(hidden->neurons[i].output);
    }
    for (int l = 1; l < net->num_layers; l++)
    {
        Layer* curr = &net->layers[l];
        Layer* prev = &net->layers[l - 1];
        for (int i = 0; i < curr->num_neurons; i++)
        {
            for (int j = 0; j < curr->neurons[i].num_inputs; j++)
            {
                double delta = net->learning_rate * curr->neurons[i].delta *
                    prev->neurons[j].output;
                curr->neurons[i].weights[j] += delta;
            }
            curr->neurons[i].bias += net->learning_rate * curr->neurons[i].delta;
        }
    }
}

/*
typedef struct {
    int num_inputs;
    double* weights;
    double bias;
    double output;
    double delta;
} Neuron;

typedef struct {
    int num_neurons;
    Neuron* neurons;
} Layer;

typedef struct {
    int num_layers;
    Layer* layers;
    double learning_rate;
} Network;
*/
#pragma warning(disable : 4996)
#include<string.h>
void clone_network(Network* src, Network* dst) {
    dst->num_layers = src->num_layers;

    dst->learning_rate = src->learning_rate;
    Layer* sl,*dl;
    Neuron* sn, * dn;
    //printf("LAYERS:%i\n", dst->num_layers);
    dst->layers = (Layer*)malloc(sizeof(Layer)* src->num_layers);
    if (!dst->layers)        exit(-1);
    for (int i = 0; i != src->num_layers; i++) {
        sl = src->layers + i;
        dl = dst->layers + i;
        dl->num_neurons = sl->num_neurons;
        //printf("LAYER:%i  NEURONS:%i\n", i, dl->num_neurons);
        dl->neurons = (Neuron*)malloc(sizeof(Neuron)* dl->num_neurons);
        if (!dl->neurons)    exit(-1);
        for (int j = 0; j != sl->num_neurons; j++) {
            sn = sl->neurons+j;
            dn = dl->neurons+j;
            dn->num_inputs = sn->num_inputs;
            //printf("LAYER:%i  NEURON:%i  INPUTS:%i\n", i, j, dn->num_inputs);
            memcpy(&dn->bias, &sn->bias, 24);
            dn->weights = (double*)malloc(8 * dn->num_inputs);
            if (!dn->weights)    exit(-1);
            memcpy(dn->weights, sn->weights, 8 * dn->num_inputs);
        }
    }
}

void network_save(Network* net, const char* filename) {
    FILE* foutput = fopen(filename, "wb");
    if (!foutput) { exit(-128); }
    Layer* l;
    Neuron* n;
    fwrite(&net->learning_rate, 8, 1, foutput);
    fwrite(&net->num_layers, 4, 1, foutput);
    for (int i = 0; i < net->num_layers; i++) {
        l = net->layers+i;
        fwrite(&l->num_neurons, 4, 1, foutput);
        for (int j = 0; j < l->num_neurons; j++) {
            n = l->neurons + j;
            fwrite(&n->num_inputs, 4, 1, foutput);
            fwrite(n->weights, 8* n->num_inputs, 1, foutput);
            fwrite(&n->bias, 24, 1, foutput);
        }
    }
    fclose(foutput);
}
void network_read(Network* net, const char* filename) {
    FILE* foutput = fopen(filename, "rb");
    if (!foutput) { exit(-128); }
    Layer* l;
    Neuron* n;


    fread(&net->learning_rate, 8, 1, foutput);
    fread(&net->num_layers, 4, 1, foutput);
    net->layers = (Layer*)malloc(sizeof(Layer) * net->num_layers);
    if (!net->layers)    exit(-1);
    for (int i = 0; i < net->num_layers; i++) {
        l = net->layers + i;
        fread(&l->num_neurons, 4, 1, foutput);
        l->neurons = (Neuron*)malloc(sizeof(Neuron)* l->num_neurons);
        if (!l->neurons)     exit(-1);
        for (int j = 0; j < l->num_neurons; j++) {
            n = l->neurons + j;
            fread(&n->num_inputs, 4, 1, foutput);
            n->weights = (double*)malloc(sizeof(double) * n->num_inputs);
            if (!n->weights)     exit(-1);
            fread(n->weights, 8 * n->num_inputs, 1, foutput);
            fread(&n->bias, 24, 1, foutput);
        }
    }
    fclose(foutput);

}