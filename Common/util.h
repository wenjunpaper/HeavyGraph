#ifndef _UTIL_H
#define _UTIL_H

#include "Param.h"
#include <assert.h>
#include <cmath>

void ParamInitialize() {
    if constexpr(mode_flag == FREQ_VERTEX_HEAVY_EDGE_HEAVY){
        filterMemoryRate = 0.1;
        widthheightRate = 0.45;
        freq_filter_ratio = 0.0010;
        graph_thres_ratio = 2;
		vertex_bucket_size = 2;
        edge_bucket_size = (edge_top_k * 6) * vertex_bucket_size ;
        id_bucket_size = edge_bucket_size * 0.6;
        assert(id_bucket_size>=1);
	}
    else if constexpr(mode_flag == FREQ_VERTEX_HEAVY_EDGE_TOPK){
        filterMemoryRate = 0.3;
        widthheightRate = 0.65;
        freq_filter_ratio = 0.0020;
        graph_thres_ratio = 2;
        vertex_bucket_size = 8;
        edge_bucket_size = (edge_top_k * 3) * vertex_bucket_size;
        id_bucket_size = edge_bucket_size * 0.8;
        assert(id_bucket_size>=1);
    }
    else if constexpr(mode_flag == FREQ_VERTEX_TOPK_EDGE_TOPK){
        filterMemoryRate = 0.1;
        widthheightRate = 0.65;
        freq_filter_ratio = 0.0010;
        graph_thres_ratio = 2;

        vertex_bucket_size = 5;
        edge_bucket_size = (edge_top_k * 5) * vertex_bucket_size;
        id_bucket_size = edge_bucket_size * 1;
        assert(id_bucket_size>=1);
    }
    else if constexpr(mode_flag == FREQ_VERTEX_TOPK_EDGE_HEAVY){
        filterMemoryRate = 0.1;
        widthheightRate = 0.45;
        freq_filter_ratio = 0.0010;
        graph_thres_ratio = 2;

        vertex_bucket_size = 5;
        edge_bucket_size = (edge_top_k * 5) * vertex_bucket_size;
        id_bucket_size = edge_bucket_size * 0.6;
        assert(id_bucket_size>=1);
    }
    freq_filter_thres = std::max(int(vertex_frequency_heavy_threshold * freq_filter_ratio),4);
    graph_thres = (freq_filter_thres)*(double(1)+graph_thres_ratio);
}

template<typename WEIGHT_TYPE>
inline WEIGHT_TYPE abs(WEIGHT_TYPE a,WEIGHT_TYPE b){
    return (a > b) ? (a - b) : (b - a);
}
template<typename VERTEX_ID_TYPE,typename WEIGHT_TYPE>
void Compare(std::vector<Vertex<VERTEX_ID_TYPE,WEIGHT_TYPE>>& truth,std::vector<Vertex<VERTEX_ID_TYPE,WEIGHT_TYPE>>& detect,ofstream& outFile){
    std::unordered_map<VERTEX_ID_TYPE,Vertex<VERTEX_ID_TYPE,WEIGHT_TYPE>> truth_map;
    int edge_truth =  0;
    for(auto elem:truth){
        truth_map[elem._v] = elem;
        edge_truth+=elem._edges.size();
    }
    uint32_t correct = 0;
    uint32_t edge_correct = 0;
    uint32_t edge_find = 0;
    int AAE = 0;
    double ARE = 0.0;
    int E_AAE = 0;
    double E_ARE = 0.0;
    for(auto elem:detect){
        edge_find += elem._edges.size();
        if(truth_map.find(elem._v)!=truth_map.end()){
            correct++;
            auto ver = truth_map[elem._v];
            AAE += abs<WEIGHT_TYPE>(truth_map[elem._v]._sum,elem._sum);
            ARE += abs<WEIGHT_TYPE>(truth_map[elem._v]._sum,elem._sum)/(truth_map[elem._v]._sum+0.0);
            for(auto edge:elem._edges){
                if(truth_map[elem._v]._edges.find(edge.first)!=truth_map[elem._v]._edges.end()){
                    edge_correct++;
                    E_AAE += abs(truth_map[elem._v]._edges[edge.first],edge.second);
                    E_ARE += abs(truth_map[elem._v]._edges[edge.first],edge.second)/(truth_map[elem._v]._edges[edge.first]+0.0);
                }
            }
        }
    }
    
    std::cout<<"Vertex:"<<std::endl;
    std::cout << "correct: " << correct << " find: " << detect.size() << " truth: " << truth.size() << std::endl;
    std::cout << "Precision:            " << std::fixed << std::setprecision(4) << 100.0 * (double)correct / detect.size() << std::endl;
    std::cout << "Recall:               " << std::fixed << std::setprecision(4) << 100.0 * (double)correct / truth.size() << std::endl;
    std::cout << "F1:                   " << std::fixed << std::setprecision(4) << 2 * (double)correct / detect.size() * (double)correct / truth.size() / ((double)correct / detect.size() + (double)correct / truth.size()) << std::endl;
    printf("\tARE: %.10f\n", ARE / correct);
    printf("\tAAE: %.10f\n", AAE / (correct + 0.0));

    std::cout<<"Edge:"<<std::endl;
    std::cout << "correct: " << edge_correct << " find: " << edge_find << " truth: " << edge_truth << std::endl;
    std::cout << "Precision:            " << std::fixed << std::setprecision(4) << 100.0 * (double)edge_correct / edge_find << std::endl;
    std::cout << "Recall:               " << std::fixed << std::setprecision(4) << 100.0 * (double)edge_correct / edge_truth << std::endl;
    std::cout << "F1:                   " << std::fixed << std::setprecision(4) << 2 * (double)edge_correct / edge_find * (double)edge_correct / edge_truth / ((double)edge_correct / edge_find + (double)edge_correct / edge_truth) << std::endl;
    printf("\tARE: %.10f\n", E_ARE / edge_correct);
    printf("\tAAE: %.10f\n", E_AAE / (edge_correct + 0.0));

    outFile<<(double)(edge_correct+correct) / (edge_find+detect.size())<<","<<(double)(edge_correct+correct) / (edge_truth+truth.size())<<","<<
    2 * (double)(edge_correct+correct) / (edge_find+detect.size()) * (double)(edge_correct+correct) / (edge_truth+truth.size()) / ((double)(edge_correct+correct) / (edge_find+detect.size()) + (double)(edge_correct+correct) / (edge_truth+truth.size()))
    <<","<<(E_ARE + ARE) / (edge_correct+correct)<<","<<(E_AAE + AAE)/(edge_correct + correct + 0.0)<<",";
    return;
}

#endif