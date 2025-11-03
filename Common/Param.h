#ifndef _PARAM_H
#define _PARAM_H
#include <cstdint>

#define WEIGHT

enum DetectMode{
    FREQ_VERTEX_HEAVY_EDGE_HEAVY,
    FREQ_VERTEX_HEAVY_EDGE_TOPK,
    FREQ_VERTEX_TOPK_EDGE_TOPK,
    FREQ_VERTEX_TOPK_EDGE_HEAVY,
};

const int COUNT_BITS = 20;
const int FP_BITS = 8;
const int FLAG_BITS = 4;

const static int memory_list[] = {50,100,150,200};
const int mode_flag = FREQ_VERTEX_TOPK_EDGE_HEAVY;

const double vertex_frequency_heavy_thres_ratio = 0.0006;
int vertex_frequency_heavy_threshold = 0;

const double vertex_degree_heavy_ratio = 0.0005;
int vertex_degree_heavy_threshold = 0;

const double edge_frequency_heavy_thres_ratio = 0.0002;
int edge_frequency_heavy_threshold = 0;

int vertex_top_k = 100;
int edge_top_k = 3;

//filter
double filterMemoryRate = 0.3;
double widthheightRate = 0.65;


double freq_filter_ratio = 0.0;
double graph_thres_ratio = 0.0;
int graph_thres = 15;
int freq_filter_thres = 20;

//GSS Param
int widthGss = 500;
int rangeGss = 16;
int p_numGss = 16;
int sizeGss = 2;
int f_numGss = 12;
int bufferMaxGss = 25;

//Heavy Param
int vertex_bucket_size = 5;
int edge_bucket_size = (edge_top_k * 2) * vertex_bucket_size;
int id_bucket_size = edge_bucket_size * 0.1;

const int BUBBLE_ARRAY_SIZE = 2;
const int MAX_KICK_OUT = 1;
#endif