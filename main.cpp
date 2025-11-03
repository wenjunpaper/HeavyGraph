#include "Dataset.h"
#include "CorrectDetector.hpp"
#include "GSSDetector.hpp"
#include "HeavyDetector.hpp"
#include <iomanip>
#include <stdio.h>
#include <chrono>
#include "util.h"

std::vector<std::string> csv_file = {
    "freq_vertex_heavy_edge_heavy.csv",
    "freq_vertex_heavy_edge_topk.csv",
    "freq_vertex_topk_edge_topk.csv",
    "freq_vertex_topk_edge_heavy.csv",
};

using VERTEX_ID_TYPE = uint32_t;
using FP_TYPE = uint8_t;
using WEIGHT_TYPE = uint32_t;

#define CAIDA

int main(void){
    std::string datasetPath="./Data/caida.dat";
    for(auto& elem:csv_file){
        #ifdef CAIDA
            #ifdef WEIGHT
                elem = "TrailRes/Caida_Weight/"+elem;
                datasetPath = "./Data/caida_weight.dat";
            #else
                elem = "TrailRes/Caida/"+elem;
            #endif
        #else
            #ifdef WEIGHT
                elem = "TrailRes/Campus_Weight/"+elem; 
                datasetPath = "./Data/campus_weight.dat";
            #else
                elem = "TrailRes/Campus/"+elem;
                datasetPath = "./Data/campus.dat";
            #endif 
        #endif
    }
    std::cout<<"data is "<<datasetPath<<std::endl;
    Dataset dataset(datasetPath,-1);
    CorrectDetector<VERTEX_ID_TYPE,WEIGHT_TYPE> detector;
    for(int i = 0;i<dataset.dataset.size();i++){
        detector.Insert(dataset.dataset[i].u,dataset.dataset[i].v,dataset.dataset[i].weight);
    }
    if constexpr(mode_flag == FREQ_VERTEX_HEAVY_EDGE_HEAVY || mode_flag == FREQ_VERTEX_HEAVY_EDGE_TOPK ){
        vertex_frequency_heavy_threshold = detector._freq_sum * vertex_frequency_heavy_thres_ratio;
    }
    if constexpr(mode_flag == FREQ_VERTEX_HEAVY_EDGE_HEAVY || mode_flag == FREQ_VERTEX_TOPK_EDGE_HEAVY ){
        edge_frequency_heavy_threshold = detector._freq_sum * edge_frequency_heavy_thres_ratio;
    }
    auto output = detector.Query();
    detector.CountDegree();
    if constexpr(mode_flag == FREQ_VERTEX_HEAVY_EDGE_HEAVY || mode_flag == FREQ_VERTEX_HEAVY_EDGE_TOPK){
        vertex_top_k = output.size();
    }
    else{
        vertex_frequency_heavy_threshold = detector._freq_sum * vertex_frequency_heavy_thres_ratio;
    }
    ofstream outFile(csv_file[mode_flag],std::ios::app);
    outFile << "algrithms" <<"," << "Memory(KB)" << "," << "precision"<<"," << "recall"<<"," << "f1" <<","<<"are"<<","<<"aae"<<","<<"throughput"<<"\n";
    ParamInitialize();
    for(auto mem : memory_list){
        auto gss_mem = mem* 1024;
        gss_mem -= ( ( (10*sizeof(WEIGHT_TYPE)+sizeof(VERTEX_ID_TYPE)) * edge_top_k + 4 + 2*sizeof(WEIGHT_TYPE) ) + (10*sizeof(WEIGHT_TYPE)+sizeof(VERTEX_ID_TYPE*)) +sizeof(VERTEX_ID_TYPE))*vertex_top_k+12;
        assert(gss_mem>0);
        outFile<<"Strawman"<<","<<mem<<",";
        GSSDetector<VERTEX_ID_TYPE,WEIGHT_TYPE> gss(gss_mem);
        auto time_start = std::chrono::steady_clock::now();
        for(int i = 0;i<dataset.dataset.size();i++){
            gss.Insert(dataset.dataset[i].u,dataset.dataset[i].v,dataset.dataset[i].weight);
        }
        auto time_end = std::chrono::steady_clock::now();
        std::chrono::duration<double> time_span = std::chrono::duration_cast<std::chrono::duration<double>>(time_end - time_start);
        std::cout << "Throughput(Mips):     " << dataset.dataset.size()/ (1000 * 1000 * time_span.count()) << std::endl<<std::endl;
        auto output1 = gss.Query();
        Compare(output,output1,outFile);
        outFile<<dataset.dataset.size()/ (1000 * 1000 * time_span.count())<<"\n";
        std::cout<<std::endl;
        outFile<<"HeavyGraph"<<","<<mem<<",";
        HeavyDetector<VERTEX_ID_TYPE,WEIGHT_TYPE,FP_TYPE> heavy(mem);
        auto time_start1 = std::chrono::steady_clock::now();
        for(int i = 0;i<dataset.dataset.size();i++){
            heavy.Insert(dataset.dataset[i].u,dataset.dataset[i].v,dataset.dataset[i].weight);
        }
        auto time_end1 = std::chrono::steady_clock::now();
        auto time_span1 = std::chrono::duration_cast<std::chrono::duration<double>>(time_end1 - time_start1);
        std::cout << "Throughput(Mips):     " << dataset.dataset.size()/ (1000 * 1000 * time_span1.count()) << std::endl<<std::endl;
        auto output2 = heavy.Query();
        Compare(output,output2,outFile);
        outFile<<dataset.dataset.size()/ (1000 * 1000 * time_span1.count())<<"\n";
        std::cout<<std::endl;
    }
    outFile.close();
    return 0;
}