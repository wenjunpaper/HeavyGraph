#ifndef _DETECTORABSTRACT_H
#define _DETECTORABSTRACT_H

#include <vector>
#include <unordered_map>
#include "Common/Param.h"
#include <algorithm>
template<typename VERTEX_ID_TYPE,typename WEIGHT_TYPE>
struct Vertex{
    VERTEX_ID_TYPE _v=0;
    WEIGHT_TYPE _sum=0;
    std::unordered_map<VERTEX_ID_TYPE,WEIGHT_TYPE> _edges;
    Vertex(){
        _edges.clear();
    }
    Vertex(VERTEX_ID_TYPE v,WEIGHT_TYPE sum = 0){
        _v = v;
        _sum = sum;
        _edges.clear();
    }
    void Insert(VERTEX_ID_TYPE u,WEIGHT_TYPE weight){
        if(_edges.find(u)==_edges.end()){
            _edges[u] = 0;
        }
        _edges[u] += weight;
        if constexpr (mode_flag <= FREQ_VERTEX_TOPK_EDGE_HEAVY)
            _sum += weight;
    }
    
    void QueryTopK(int top_k){
        if (top_k >= _edges.size()) return;

        std::vector<std::pair<VERTEX_ID_TYPE, WEIGHT_TYPE>> items;
        items.reserve(_edges.size());
        for (const auto &kv : _edges) items.emplace_back(kv.first, kv.second);
    
        std::sort(items.begin(), items.end(),
                  [](auto const &a, auto const &b) {
                      if (a.second != b.second) return a.second > b.second;
                      return a.first < b.first;
                  });
    
        size_t length = std::min((size_t)top_k, items.size());
        for (size_t i = length; i < items.size(); ++i) _edges.erase(items[i].first);
        assert(_edges.size()<=top_k);
    }
    void QueryHeavy(int threshold){
        for(auto it = _edges.begin(); it != _edges.end(); ){
            if(it->second < threshold){
                it = _edges.erase(it);
            }else{
                ++it;
            }
        }
        return;
    }
};

template<typename VERTEX_ID_TYPE, typename WEIGHT_TYPE >
class DetectorAbstract{
public:
    virtual void Insert(VERTEX_ID_TYPE _v, VERTEX_ID_TYPE _u, WEIGHT_TYPE _weight) = 0;
    virtual std::vector<Vertex<VERTEX_ID_TYPE,WEIGHT_TYPE>> Query() = 0;
};

#endif