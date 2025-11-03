#ifndef _CORRECTDETECTOR_H
#define _CORRECTDETECTOR_H

#include "DetectorAbstract.h"
#include "Param.h"
#include <unordered_map>
#include <array>
#include <algorithm>
#include <set>
#include <queue>
template<typename VERTEX_ID_TYPE, typename WEIGHT_TYPE>
class CorrectDetector:DetectorAbstract<VERTEX_ID_TYPE,WEIGHT_TYPE>{
public:
    
    CorrectDetector() {
    }
    void Insert(VERTEX_ID_TYPE _v, VERTEX_ID_TYPE _u, WEIGHT_TYPE _weight){
        if(_vertex.find(_v)==_vertex.end()){
            _vertex[_v] = Vertex<VERTEX_ID_TYPE,WEIGHT_TYPE>(_v);
        }
        _vertex[_v].Insert(_u,_weight);
        auto prev_freq_sum = _freq_sum;
        _freq_sum += _weight;
        _max_v = std::max(_vertex[_v]._sum,_max_v);
        _max_e = std::max(_vertex[_v]._edges[_u],_max_e);
        assert(_freq_sum>prev_freq_sum);
        _max_w = std::max(_max_w,_weight);
    };
    void CountDegree(){
        for(auto elem:_vertex){
            _edge_degree_sum+=elem.second._edges.size();
        }
        _vertex_degree_sum = _vertex.size();
    }
    std::vector<Vertex<VERTEX_ID_TYPE,WEIGHT_TYPE>> Query(){
        std::vector<Vertex<VERTEX_ID_TYPE,WEIGHT_TYPE>> vertex_vec;
        if constexpr (mode_flag == FREQ_VERTEX_TOPK_EDGE_HEAVY || mode_flag == FREQ_VERTEX_TOPK_EDGE_TOPK){
            vertex_vec = QueryTopKNodes(vertex_top_k);
            if constexpr (mode_flag == FREQ_VERTEX_TOPK_EDGE_TOPK){
                for(auto& vertex:vertex_vec){
                    vertex.QueryTopK(edge_top_k);
                }
            }else if constexpr(mode_flag == FREQ_VERTEX_TOPK_EDGE_HEAVY){
                for(auto& vertex:vertex_vec){
                    vertex.QueryHeavy(edge_frequency_heavy_threshold);
                }
            }
        }
        else if constexpr (mode_flag == FREQ_VERTEX_HEAVY_EDGE_HEAVY || mode_flag == FREQ_VERTEX_HEAVY_EDGE_TOPK){
            vertex_vec = QueryHeavyNodes(vertex_frequency_heavy_threshold);
            if constexpr (mode_flag == FREQ_VERTEX_HEAVY_EDGE_TOPK){
                for(auto& vertex:vertex_vec){
                    vertex.QueryTopK(edge_top_k);
                }
            }else if constexpr(mode_flag == FREQ_VERTEX_HEAVY_EDGE_HEAVY){
                for(auto& vertex:vertex_vec){
                    vertex.QueryHeavy(edge_frequency_heavy_threshold);
                }
            }
        }
        return vertex_vec;
    };
    std::vector<Vertex<VERTEX_ID_TYPE,WEIGHT_TYPE>> QueryTopKNodes(int top_k){
        using Pair = Vertex<VERTEX_ID_TYPE, WEIGHT_TYPE>;
        auto cmp = [](const Pair& a, const Pair& b) {
            return a._sum > b._sum;
        };
        std::priority_queue<Pair, std::vector<Pair>, decltype(cmp)> pq(cmp);
        for (const auto& elem : _vertex) {
            if (pq.size() < top_k) {
                pq.push(elem.second);
            } else if (elem.second._sum > pq.top()._sum) {
                pq.pop();
                pq.push(elem.second);
            }
        }
        std::vector<Vertex<VERTEX_ID_TYPE, WEIGHT_TYPE>> res;
        res.reserve(pq.size());
        while (!pq.empty()) {
            res.push_back(pq.top());
            pq.pop();
        }
        return res;
    };
    std::vector<Vertex<VERTEX_ID_TYPE,WEIGHT_TYPE>> QueryHeavyNodes(int threshold){
        std::vector<Vertex<VERTEX_ID_TYPE,WEIGHT_TYPE>>  res;
        for(auto elem:_vertex){
            if(elem.second._sum >= threshold){
                res.emplace_back(elem.second);
            }
        }
        return res;
    };
    void Check(const std::vector<Vertex<VERTEX_ID_TYPE,WEIGHT_TYPE>>& res){
        int error_vertex_num = 0;
        int error_edge_num = 0;
        std::set<VERTEX_ID_TYPE> s;
        for(auto elem:res){
            assert(s.find(elem._v)==s.end());
            s.insert(elem._v);
            if(_vertex.find(elem._v)==_vertex.end()){
                error_vertex_num++;
            }
            else{
                auto vertex = _vertex[elem._v]; 
                for(auto edge:elem._edges){
                    if(vertex._edges.find(edge.first)==vertex._edges.end()){
                        error_edge_num++;
                    }
                }
            }
        }
    }
private:
    std::unordered_map<VERTEX_ID_TYPE,Vertex<VERTEX_ID_TYPE,WEIGHT_TYPE>> _vertex;
public:
    WEIGHT_TYPE _freq_sum = 0;
    WEIGHT_TYPE _max_v = 0;
    WEIGHT_TYPE _max_e = 0;
    WEIGHT_TYPE _edge_degree_sum = 0;
    WEIGHT_TYPE _vertex_degree_sum = 0;
    WEIGHT_TYPE _max_w = 0;
};

#endif