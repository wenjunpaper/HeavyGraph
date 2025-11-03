#ifndef _GRAPHFILTER_H
#define _GRAPHFILTER_H

#include <vector>
#include <cstring>
#include <cstdint>
#include "Param.h"

template<typename ID_TYPE, typename FP_TYPE, typename WEIGHT_TYPE>
class GraphFilter{
public:
    class GraphEntry{
    public:
        FP_TYPE v_;
        FP_TYPE u_;
        WEIGHT_TYPE weight_;

        explicit GraphEntry():v_(0), u_(0), weight_(0){}

        inline void Clear(){v_ = 0;u_=0;weight_=0;}
        inline bool IsEmpty(){return weight_ == 0;}

    };

public:
    const int width_;
    const int height_;
    std::vector<std::vector<GraphEntry>> matrix_;

public:
    uint32_t Hash(ID_TYPE _id){return (*hfunc[0])((unsigned char*)(&_id), sizeof(_id) );}

public:
    GraphFilter(int _width, int _height):width_(_width), height_(_height){
        matrix_.resize(height_);
        for(auto &it : matrix_){
            it.resize(width_);
        }
    }

    bool Insert(ID_TYPE _v, ID_TYPE _u, WEIGHT_TYPE _weight);
    std::vector<typename GraphFilter<ID_TYPE, FP_TYPE, WEIGHT_TYPE>::GraphEntry*> NodeValQuery(ID_TYPE _v,WEIGHT_TYPE& weight);
    typename GraphFilter<ID_TYPE, FP_TYPE, WEIGHT_TYPE>::GraphEntry* EdgeValQuery(ID_TYPE _v,ID_TYPE _u);
};

template<typename ID_TYPE, typename FP_TYPE, typename WEIGHT_TYPE>
bool GraphFilter<ID_TYPE, FP_TYPE, WEIGHT_TYPE>::Insert(ID_TYPE _v, ID_TYPE _u, WEIGHT_TYPE _weight) {
    uint32_t vHash = Hash(_v);
    uint32_t uHash = Hash(_u);
    uint32_t colIndex = (vHash >> ((sizeof(ID_TYPE) - sizeof(FP_TYPE)) * 8)  ) % height_;
    uint32_t rowIndex = (uHash >> ((sizeof(ID_TYPE) - sizeof(FP_TYPE)) * 8)  ) % width_;
    FP_TYPE colFp = vHash;
    FP_TYPE rowFp = uHash;

    auto& entry = matrix_[colIndex][rowIndex];
    if(entry.IsEmpty()){
        entry.v_ = colFp;
        entry.u_ = rowFp;
        entry.weight_ = _weight;
    }else if(entry.v_ == colFp && entry.u_ == rowFp){
        entry.weight_ += _weight;
    }else{
        if(entry.weight_ <= _weight){
            entry.Clear();
            entry.v_ = colFp;
            entry.u_ = rowFp;
            entry.weight_ = _weight;
            return false;
        }else{
            entry.weight_ -= _weight;
            return false;
        }
    }
    return true;    
}

template<typename ID_TYPE, typename FP_TYPE, typename WEIGHT_TYPE>
std::vector<typename GraphFilter<ID_TYPE, FP_TYPE, WEIGHT_TYPE>::GraphEntry*>  GraphFilter<ID_TYPE, FP_TYPE, WEIGHT_TYPE>::NodeValQuery(ID_TYPE _v,WEIGHT_TYPE& weight) {
    uint32_t vHash = Hash(_v);
    uint32_t colIndex = (vHash >> ((sizeof(ID_TYPE) - sizeof(FP_TYPE)) * 8)  ) % height_;
    FP_TYPE colFp = vHash;
    std::vector<typename GraphFilter<ID_TYPE, FP_TYPE, WEIGHT_TYPE>::GraphEntry*> res;
    for(int rowIndex = 0;rowIndex<width_;rowIndex++){
        auto& entry = matrix_[colIndex][rowIndex];
        if(entry.v_ == colFp){
            res.emplace_back(&entry);
            weight += entry.weight_;
        }
    }
    return res;
}

template<typename ID_TYPE, typename FP_TYPE, typename WEIGHT_TYPE>
typename GraphFilter<ID_TYPE, FP_TYPE, WEIGHT_TYPE>::GraphEntry* GraphFilter<ID_TYPE, FP_TYPE, WEIGHT_TYPE>::EdgeValQuery(ID_TYPE _v,ID_TYPE _u){
    uint32_t vHash = Hash(_v);
    uint32_t uHash = Hash(_u);
    uint32_t colIndex = (vHash >> ((sizeof(ID_TYPE) - sizeof(FP_TYPE)) * 8)  ) % height_;
    uint32_t rowIndex = (uHash >> ((sizeof(ID_TYPE) - sizeof(FP_TYPE)) * 8 ) ) % width_;
    FP_TYPE colFp = vHash;
    FP_TYPE rowFp = uHash;
    auto& entry = matrix_[colIndex][rowIndex];
    if(entry.v_ == colFp && entry.u_ == rowFp){
        return &entry;
    }
    return nullptr;
}
#endif
