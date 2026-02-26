#pragma once
#ifndef NEURAL_H
#define NEURAL_H


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
Network create_network(int num_inputs, int num_hidden, int num_outputs, double lr);
void forward(Network* net, double* inputs);
void backprop_alpha(Network* net, double* inputs, double* targets, double alpha_weight);
void free_network(Network* net);
void network_save(Network* net, const char* filename);
void network_read(Network* net, const char* filename);
void clone_network(Network* src, Network* dst);
#endif
