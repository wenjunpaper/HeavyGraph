#ifndef _HEAVYDETECTOR_H
#define _HEAVYDETECTOR_H

#include "DetectorAbstract.h"
#include "GraphFilter.hpp"
#include "hashTable.h"

#pragma pack(1)
#define OVERFLOW_DETECT
#define OVERFLOW_CORRECT

template<typename VERTEX_ID_TYPE,typename WEIGHT_TYPE,typename FP_TYPE>
class BubbleArrays{
    class EdgeEntry{
        public:
            bool IsEmpty(){return _count == 0;}
            EdgeEntry(){};
            EdgeEntry(VERTEX_ID_TYPE u):_u(u){};
            EdgeEntry(VERTEX_ID_TYPE u,WEIGHT_TYPE count):_u(u),_count(count){};
            bool Equal(VERTEX_ID_TYPE u,WEIGHT_TYPE flag){return u == _u &&  _flag == flag;}
            bool EqualFp(FP_TYPE fp,WEIGHT_TYPE flag){
                static FP_TYPE fp_mask = ((uint64_t)1<<FP_BITS)-1;
                if(_flag != flag){
                    return false;
                }
                fp = fp & fp_mask;
                return _fp == fp;
            }
            bool EqualFlag(WEIGHT_TYPE flag){return flag == _flag;}
            WEIGHT_TYPE GetFlag(){return _flag;}
            void SetFlag(int flag){_flag = flag;}
            FP_TYPE GetFp(){return _fp;} 
            void Insert(WEIGHT_TYPE weight){
                #ifdef OVERFLOW_DETECT
                auto prev_count = _count;
                #endif
                _count+=weight;
                #ifdef OVERFLOW_CORRECT
                if(_count < prev_count){
                    _count=0;
                    _count--;
                }
                #endif
                #ifdef OVERFLOW_DETECT
                assert(_count>=prev_count);
                #endif
            }
            void InsertId(VERTEX_ID_TYPE u,WEIGHT_TYPE weight,WEIGHT_TYPE flag){_u = u;_count = weight;_flag = flag;}
            void InsertFp(FP_TYPE fp,WEIGHT_TYPE weight,WEIGHT_TYPE flag){_fp = fp;_count = weight;_flag = flag;}
            bool operator>(const EdgeEntry &e) { return _count > e._count; }
            void Clear(){_u = 0;_count = 0;_fp = 0;_flag=0;}
            void Loss(WEIGHT_TYPE weight){_count-=weight;}
            VERTEX_ID_TYPE GetId(){return _u;}
            WEIGHT_TYPE GetCount(){return _count;}
            EdgeEntry& operator=(const EdgeEntry& other) {
                if (this == &other) return *this; 
                this->_count = other._count;
                this->_u = other._u;
                this->_flag = other._flag;
                this->_fp = other._fp;
                return *this;
            }
        public:
            VERTEX_ID_TYPE _u = 0;
            WEIGHT_TYPE _fp:FP_BITS = 0;
            WEIGHT_TYPE _count:COUNT_BITS = 0;
            WEIGHT_TYPE _flag:FLAG_BITS = 0;
    };
    class VertexEntry{
        public:
            VertexEntry(){};
            VertexEntry(VERTEX_ID_TYPE v):_v(v){};
            VertexEntry(VERTEX_ID_TYPE v,WEIGHT_TYPE count):_v(v),_count(count){};
            WEIGHT_TYPE GetFlag(){return _flag;}
            void SetFlag(WEIGHT_TYPE flag){_flag = flag;}
            void SetId(VERTEX_ID_TYPE id){_v = id;}
            bool IsEmpty(){return _count == 0;}
            bool Equal(VERTEX_ID_TYPE v){return v == _v;}
            WEIGHT_TYPE GetCount(){return _count;}
            VertexEntry& operator=(const VertexEntry& other) {
                if (this == &other) return *this; 
                this->_count = other._count;
                this->_v = other._v;
                this->_flag = other._flag;
                return *this;
            }
            void Insert(WEIGHT_TYPE weight){
                    #ifdef OVERFLOW_DETECT
                    auto prev_count = _count;
                    #endif
                    _count+=weight;
                    #ifdef OVERFLOW_CORRECT
                    if(_count < prev_count){
                        _count=0;
                        _count--;
                    }
                    #endif
                    #ifdef OVERFLOW_DETECT
                    assert(_count>=prev_count);
                    #endif
                    assert(_count!=0);
            }
            void Loss(WEIGHT_TYPE weight){
                #ifdef OVERFLOW_DETECT
                assert(_count>=weight);
                #endif
                _count-=weight;
            }
            VERTEX_ID_TYPE GetId(){return _v;}
            void Clear(){_count = 0;_v = 0;
            }
        private:
            VERTEX_ID_TYPE _v = 0;
            WEIGHT_TYPE _flag:FLAG_BITS = 0;
            WEIGHT_TYPE _count:COUNT_BITS = 0;
    };
    class VertexBucket{
        public:
            VertexBucket(){
                _entries.resize(vertex_bucket_size);
                assert((1<<FLAG_BITS)-1>=_entries.size());
                for(int i = 0;i<_entries.size();i++){
                    _entries[i].SetFlag(i+1);
                }
                _edge_entries.resize(edge_bucket_size);
            }
            bool Equal(int index, VERTEX_ID_TYPE id) {
                return _entries[index].Equal(id);
            }
            void Insert(int index,const VertexEntry& entry,const std::vector<EdgeEntry>& id_edge_entry,const std::vector<EdgeEntry>& fp_edge_entry){
                auto prev_flag = _entries[index].GetFlag();
                _entries[index] = entry;
                _entries[index].SetFlag(prev_flag);
                for(auto edge:id_edge_entry){
                    edge.SetFlag(prev_flag);
                    EdgeInsertById(edge.GetId(),edge.GetCount(),edge.GetFlag());
                }
                for(auto edge:fp_edge_entry){
                    edge.SetFlag(prev_flag);
                    EdgeInsertByFp(edge.GetFp(),edge.GetCount(),edge.GetFlag());
                }
            }
            std::vector<EdgeEntry> FindEdgeWithId(int index){
                auto flag = GetEntry(index).GetFlag();
                std::vector<EdgeEntry> res;
                for(int i = 0;i<id_bucket_size;i++){
                    if(_edge_entries[i].EqualFlag(flag)){
                        res.emplace_back(_edge_entries[i]);
                    }
                }
                return res;
            }
            std::vector<EdgeEntry> FindEdgeWithFp(int index){
                auto flag = GetEntry(index).GetFlag();
                std::vector<EdgeEntry> res;
                for(int i = id_bucket_size;i<edge_bucket_size;i++){
                    if(_edge_entries[i].EqualFlag(flag)){
                        res.emplace_back(_edge_entries[i]);
                    }
                }
                return res;
            }
            void EdgeInsertByFp(FP_TYPE _fp,WEIGHT_TYPE _weight,WEIGHT_TYPE _flag){
                int index = -1;
                for(int i = id_bucket_size;i<edge_bucket_size;i++){
                    if(_edge_entries[i].Equal(_fp,_flag)){
                        _edge_entries[i].Insert(_weight);
                        index = i;
                        break;
                    }
                    if(_edge_entries[i].IsEmpty()){
                        _edge_entries[i].InsertFp(_fp,_weight,_flag);
                        index = i;
                        break;
                    }
                }
                while(index>id_bucket_size && _edge_entries[index]>_edge_entries[index-1]){
                    swap(_edge_entries[index],_edge_entries[index-1]);
                    index--;
                }
                if(index == -1){
                    if(_edge_entries[edge_bucket_size-1].GetCount()<_weight){
                        _edge_entries[edge_bucket_size-1].InsertFp(_fp,_weight,_flag);
                        index = edge_bucket_size-1;
                        while(index > id_bucket_size && _edge_entries[index]>_edge_entries[index-1]){
                            swap(_edge_entries[index],_edge_entries[index-1]);
                            index--;
                        }
                    }else{
                        _edge_entries[edge_bucket_size-1].Loss(_weight);
                    }
                }
            }
            void EdgeInsertById(VERTEX_ID_TYPE _u,WEIGHT_TYPE _weight,WEIGHT_TYPE _flag){
                int index = -1;
                for(int i = 0;i<id_bucket_size;i++){
                    if(_edge_entries[i].Equal(_u,_flag)){
                        _edge_entries[i].Insert(_weight);
                        index = i;
                        break;
                    }
                    if(_edge_entries[i].IsEmpty()){
                        _edge_entries[i].InsertId(_u,_weight,_flag);
                        index = i;
                        break;
                    }
                }
                if(index >= 0){
                    while(index>0 && _edge_entries[index]>_edge_entries[index-1]){
                        swap(_edge_entries[index],_edge_entries[index-1]);
                        index--;
                    }
                    return;
                }
                if(_weight > _edge_entries[id_bucket_size-1].GetCount()){
                    auto prev_entry = _edge_entries[id_bucket_size-1];
                    _edge_entries[id_bucket_size-1].InsertId(_u,_weight,_flag);
                    index = id_bucket_size-1;
                    while(index>0 && _edge_entries[index]>_edge_entries[index-1]){
                        swap(_edge_entries[index],_edge_entries[index-1]);
                        index--;
                    }
                    EdgeInsertByFp(HashFp(prev_entry.GetId()),prev_entry.GetCount(),prev_entry.GetFlag());
                    return;
                }
                EdgeInsertByFp(HashFp(_u),_weight,_flag);
            }
            int EdgeInsert(VERTEX_ID_TYPE _u,FP_TYPE _fp,WEIGHT_TYPE _weight,WEIGHT_TYPE _flag){
                int index = -1;
                bool is_new_edge = false;

                for(int i = id_bucket_size;i<edge_bucket_size;i++){
                    if(_edge_entries[i].EqualFp(_fp,_flag)){
                        _edge_entries[i].Insert(_weight);
                        index = i;
                        break;
                    }
                }
                if(index >= 0){
                    while(index>id_bucket_size && _edge_entries[index]>_edge_entries[index-1]){
                        swap(_edge_entries[index],_edge_entries[index-1]);
                        index--;
                    }
                    if(_edge_entries[index] > _edge_entries[id_bucket_size-1]){
                        _edge_entries[id_bucket_size-1]._fp = BubbleArrays::HashFp(_edge_entries[id_bucket_size-1]._u);
                        _edge_entries[index]._u = _u;
                        swap(_edge_entries[index],_edge_entries[id_bucket_size-1]);
                        while(index+1<edge_bucket_size && _edge_entries[index+1]>_edge_entries[index]){
                            swap(_edge_entries[index],_edge_entries[index+1]);
                            index++;
                        }
                        index = id_bucket_size-1;
                        while(index>0 && _edge_entries[index]>_edge_entries[index-1]){
                            swap(_edge_entries[index],_edge_entries[index-1]);
                            index--;
                        }
                    }
                    return _weight;
                }

                for(int i = 0;i<id_bucket_size;i++){
                    if(_edge_entries[i].Equal(_u,_flag)){
                        _edge_entries[i].Insert(_weight);
                        index = i;
                        break;
                    }
                    if(_edge_entries[i].IsEmpty()){
                        _edge_entries[i].InsertId(_u,_weight,_flag);
                        index = i;
                        is_new_edge = true;
                        break;
                    }
                }
                if(index >= 0){
                    while(index>0 && _edge_entries[index]>_edge_entries[index-1]){
                        swap(_edge_entries[index],_edge_entries[index-1]);
                        index--;
                    }
                    return _weight;
                }
                
                for(int i = id_bucket_size;i<edge_bucket_size;i++){
                    if(_edge_entries[i].IsEmpty()){
                        _edge_entries[i].InsertFp(_fp,_weight,_flag);
                        index = i;
                        is_new_edge = true;
                        break;
                    }
                }
                if(index >= 0){
                    while(index>id_bucket_size && _edge_entries[index]>_edge_entries[index-1]){
                        swap(_edge_entries[index],_edge_entries[index-1]);
                        index--;
                    }
                    if(_edge_entries[index] > _edge_entries[id_bucket_size-1]){
                        _edge_entries[id_bucket_size-1]._fp = BubbleArrays::HashFp(_edge_entries[id_bucket_size-1]._u);
                        _edge_entries[index]._u = _u;
                        swap(_edge_entries[index],_edge_entries[id_bucket_size-1]);
                        while(index+1<edge_bucket_size && _edge_entries[index+1]>_edge_entries[index]){
                            swap(_edge_entries[index],_edge_entries[index+1]);
                            index++;
                        }
                        index = id_bucket_size-1;
                        while(index>0 && _edge_entries[index]>_edge_entries[index-1]){
                            swap(_edge_entries[index],_edge_entries[index-1]);
                            index--;
                        }
                    }
                    return _weight;
                }
            
                is_new_edge = true;
                if(_edge_entries[id_bucket_size-1].GetCount()<_weight){
                    auto prev_entries = _edge_entries[id_bucket_size-1];
                    _edge_entries[id_bucket_size-1].InsertId(_u,_weight,_flag);
                    index = id_bucket_size-1;
                    while(index > 0 && _edge_entries[index]>_edge_entries[index-1]){
                        swap(_edge_entries[index],_edge_entries[index-1]);
                        index--;
                    }
                    if(prev_entries > _edge_entries[edge_bucket_size-1]){
                        _edge_entries[edge_bucket_size-1].InsertFp(BubbleArrays::HashFp(prev_entries.GetId()),prev_entries.GetCount(),prev_entries.GetFlag());
                        index = edge_bucket_size-1;
                        while(index > id_bucket_size && _edge_entries[index]>_edge_entries[index-1]){
                            swap(_edge_entries[index],_edge_entries[index-1]);
                            index--;
                        }
                    }else{
                        _edge_entries[edge_bucket_size-1].Loss(prev_entries.GetCount());
                    }
                }else if(_edge_entries[edge_bucket_size-1].GetCount()<_weight){
                    _edge_entries[edge_bucket_size-1].InsertFp(_fp,_weight,_flag);
                    index = edge_bucket_size-1;
                    while(index>id_bucket_size && _edge_entries[index]>_edge_entries[index-1]){
                        swap(_edge_entries[index],_edge_entries[index-1]);
                        index--;
                    }
                }else{
                    _edge_entries[edge_bucket_size-1].Loss(_weight);
                    is_new_edge = false;
                }
                return _weight;
            }
            void ClearEdge(int index){
                auto entry = GetEntry(index);
                bool clear_flag = false;
                for(int i = 0;i<id_bucket_size;i++){
                    if(_edge_entries[i].EqualFlag(entry.GetFlag())){
                        _edge_entries[i].Clear();
                        clear_flag = true;
                    }
                }
                if(clear_flag){
                    std::sort(_edge_entries.begin(),_edge_entries.begin()+id_bucket_size,[](auto &a, auto &b) {
                        return a.GetCount() > b.GetCount();
                    });
                }
                clear_flag = false;
                for(int i = id_bucket_size;i<edge_bucket_size;i++){
                    if(_edge_entries[i].EqualFlag(entry.GetFlag())){
                        _edge_entries[i].Clear();
                        clear_flag = true;
                    }
                }
                if(clear_flag){
                    std::sort(_edge_entries.begin()+id_bucket_size,_edge_entries.end(),[](auto &a, auto &b) {
                        return a.GetCount() > b.GetCount();
                    });
                }
            }
            void Insert(int index,VERTEX_ID_TYPE u,WEIGHT_TYPE weight){
                auto fp = HashFp(u); 
                auto vertex_weight = EdgeInsert(u,fp,weight,_entries[index].GetFlag());
                _entries[index].Insert(vertex_weight);
            }
            VertexEntry& GetEntry(int index){
                return _entries[index];
            }
            WEIGHT_TYPE GetEntryCount(int index){
                return _entries[index].GetCount();
            }
            WEIGHT_TYPE GetEntryFlag(int index){return _entries[index].GetFlag();}
            bool IsEmpty(int index){
                return _entries[index].IsEmpty();
            }
            void SetId(int index,VERTEX_ID_TYPE id){
                _entries[index].SetId(id);
            }
            void BubbleSort(int index){
                while(true){
                    if(index <= 0 ){
                        break;
                    }
                    if(_entries[index].GetCount() <= _entries[index-1].GetCount()){
                        break;
                    }
                    std::swap(_entries[index],_entries[index-1]);
                    index--;
                }
            }
            void DownStairs(int index){
                int cur_index = vertex_bucket_size-1;
                ClearEdge(cur_index);
                if(cur_index<=index){
                    return;
                }
                auto prev_flag = _entries[cur_index].GetFlag();
                while(cur_index > index){
                    _entries[cur_index] = _entries[cur_index-1];
                    cur_index--;
                }
                _entries[cur_index].SetFlag(prev_flag);
            }
            void Remove(int index){
                auto prev_flag = _entries[index].GetFlag();
                ClearEdge(index);
                while(index+1<vertex_bucket_size){
                    _entries[index] = _entries[index+1];
                    index++;
                }
                _entries[index].SetFlag(prev_flag);
                _entries[index].Clear();
            }
            
        private:
            std::vector<VertexEntry> _entries;
        public:
            std::vector<EdgeEntry> _edge_entries;
    };
    private:
        std::vector<std::vector<VertexBucket>> _buckets;
        int _topk;
        int _f_max;
        int _thres;
        int _bucket_num;
        std::unordered_map<VERTEX_ID_TYPE,Vertex<VERTEX_ID_TYPE,WEIGHT_TYPE>> _answer;
        static uint32_t Hash(VERTEX_ID_TYPE id){return (*hfunc[1])((unsigned char*)(&id), sizeof(id));}
        static uint32_t Hash(uint8_t fp){return (*hfunc[1])((unsigned char*)(&fp), sizeof(fp));} 
        static uint32_t HashFp(VERTEX_ID_TYPE id){return (*hfunc[0])((unsigned char*)(&id), sizeof(id));}
        template<typename V, typename W, typename F>
        friend class  HeavyDetector;
        const static FP_TYPE fp_mask = ((uint64_t)1<<FP_BITS)-1;
    public:
        BubbleArrays(int bucket_num,int thres,int topk){
            _buckets.resize(BUBBLE_ARRAY_SIZE,std::vector<VertexBucket>(bucket_num));
            _f_max = 0;
            _thres = thres;
            _topk = topk;
            _bucket_num = bucket_num;
        }
        bool Check(VERTEX_ID_TYPE _v, VERTEX_ID_TYPE _u, WEIGHT_TYPE _weight){
            uint32_t hash_key = Hash(_v);
            uint32_t fp = HashFp(_v);
#ifdef TEST1 
            uint32_t hash_value[2] = {hash_key, hash_key + (fp&fp_mask)};
#else
            uint32_t hash_value[2] = {hash_key,
                              hash_key ^ Hash(static_cast<uint8_t>(fp & fp_mask))};
#endif
            uint32_t keys[2] = {hash_value[0]%_bucket_num,hash_value[1]%_bucket_num};
            VertexBucket& bucket0 = _buckets[0][keys[0]];
            VertexBucket& bucket1 = _buckets[1][keys[1]];
            for(int i = 0;i<vertex_bucket_size;i++){
                if(bucket0.Equal(i,_v)){
                    bucket0.Insert(i,_u,_weight);
                    if(i == 0 && bucket0.GetEntryCount(0)>_f_max){
                        _f_max = bucket0.GetEntryCount(0);
                        _thres = std::max(_thres,static_cast<int>(_f_max * 1.5 / _topk));
                        return true;
                    }
                    bucket0.BubbleSort(i);
                    if(bucket0.GetEntryCount(1)>_thres){
                        if(KickOut(MAX_KICK_OUT,Hash(bucket0.GetEntry(1).GetId()),bucket0,1,0)){
                            bucket0.Remove(1);
                        }
                    }
                    return true;
                }
            }
            for(int i = 0;i<vertex_bucket_size;i++){
                if(bucket1.Equal(i,_v)){
                    bucket1.Insert(i,_u,_weight);
                    if(i == 0 && bucket1.GetEntryCount(0)>_f_max){
                        _f_max = bucket1.GetEntryCount(0);
                        _thres = std::max(_thres,static_cast<int>(_f_max * 1.5 / _topk));
                        return true;
                    }
                    bucket1.BubbleSort(i);
                    if(bucket1.GetEntryCount(1)>_thres){
                        auto value = Hash(bucket1.GetEntry(1).GetId());
                        auto fp = HashFp(bucket1.GetEntry(1).GetId());
                        if(KickOut(MAX_KICK_OUT,value^Hash(static_cast<uint8_t>(fp & fp_mask)),bucket1,1,1)){
                            bucket1.Remove(1);
                        }
                    }
                    return true;
                }
            }
            return false;
        }
        bool KickOut(int kick_num,uint32_t hash_value,VertexBucket& cur_bucket,int entry_index,int array_index){
            if(kick_num == 0){
                return false;
            }
            auto v = cur_bucket.GetEntry(entry_index).GetId();
            uint32_t fp = HashFp(v);
#ifdef TEST1
            uint32_t next_hash_value = hash_value;
            if (array_index == 0) {
                next_hash_value += (fp&fp_mask);
            } else {
                next_hash_value -= (fp&fp_mask);
            }
#else
            uint32_t next_hash_value = hash_value ^ Hash(static_cast<uint8_t>(fp & fp_mask));
#endif 
            VertexBucket &next_bucket = _buckets[1 - array_index][next_hash_value % _bucket_num];
            if (cur_bucket.GetEntryCount(entry_index) >
                next_bucket.GetEntryCount(0)) {
                next_bucket.DownStairs(0);
                next_bucket.Insert(0, cur_bucket.GetEntry(entry_index),cur_bucket.FindEdgeWithId(entry_index),cur_bucket.FindEdgeWithFp(entry_index));
                return true;
            }

            if (KickOut(kick_num - 1, next_hash_value, next_bucket, 0,
                        1 - array_index)) {
                next_bucket.DownStairs(0);
                next_bucket.Insert(0, cur_bucket.GetEntry(entry_index),cur_bucket.FindEdgeWithId(entry_index),cur_bucket.FindEdgeWithFp(entry_index));
                return true;
            }
            return false;
        }      
        void Insert(VERTEX_ID_TYPE _v,VERTEX_ID_TYPE _u,WEIGHT_TYPE _weight){
            uint32_t hash_key = Hash(_v);
            uint32_t fp = HashFp(_v);
#ifdef TEST1
            uint32_t hash_value[2] = {hash_key, hash_key + (fp & fp_mask)};
#else
            uint32_t hash_value[2] = {hash_key,
                              hash_key ^ Hash(static_cast<uint8_t>(fp & fp_mask))};
#endif
            uint32_t keys[2] = {hash_value[0]%_bucket_num,hash_value[1]%_bucket_num};
            VertexBucket& bucket0 = _buckets[0][keys[0]];
            VertexBucket& bucket1 = _buckets[1][keys[1]];
            if(bucket0.Equal(0,_v)){
                bucket0.Insert(0,_u,_weight);
                if(bucket0.GetEntryCount(0)>_f_max){
                    _f_max = bucket0.GetEntryCount(0);
                    _thres = std::max(_thres,static_cast<int>(_f_max * 1.5 / _topk));
                }
                return;
            }
            if(bucket0.IsEmpty(0)){
                bucket0.ClearEdge(0);
                bucket0.SetId(0,_v);
                bucket0.Insert(0,_u,_weight);
                return;
            }
            if(bucket1.Equal(0,_v)){
                bucket1.Insert(0,_u,_weight);
                if(bucket1.GetEntryCount(0)>_f_max){
                    _f_max = bucket1.GetEntryCount(0);
                    _thres = std::max(_thres,static_cast<int>(_f_max * 1.5 / _topk));
                }
                return;
            }
            if(bucket1.IsEmpty(0)){
                bucket1.ClearEdge(0);
                bucket1.SetId(0,_v);
                bucket1.Insert(0,_u,_weight);
                return;
            }
            for(int i = 1;i<vertex_bucket_size;i++){
                if(bucket0.Equal(i,_v)){
                    bucket0.Insert(i,_u,_weight);
                    bucket0.BubbleSort(i);
                    if(bucket0.GetEntryCount(1)>_thres){
                        if(KickOut(MAX_KICK_OUT,Hash(bucket0.GetEntry(1).GetId()),bucket0,1,0)){
                            bucket0.Remove(1);
                        }
                    }
                    return;
                }
                if (bucket0.IsEmpty(i)) {
                    bucket0.ClearEdge(i);
                    bucket0.SetId(i,_v);
                    bucket0.Insert(i,_u,_weight);
                    return;
                }
                if(bucket1.Equal(i,_v)){
                    bucket1.Insert(i,_u,_weight);
                    bucket1.BubbleSort(i);
                    if(bucket1.GetEntryCount(1)>_thres){
                        auto value = Hash(bucket1.GetEntry(1).GetId());
                        auto fp = HashFp(bucket1.GetEntry(1).GetId());
                        if(KickOut(MAX_KICK_OUT,value^Hash(static_cast<uint8_t>(fp & fp_mask)),bucket1,1,1)){
                            bucket1.Remove(1);
                        }
                    }
                    return;
                }
                if (bucket1.IsEmpty(i)) {
                    bucket1.ClearEdge(i);
                    bucket1.SetId(i,_v);
                    bucket1.Insert(i,_u,_weight);
                    return;
                }
            }
            if constexpr(mode_flag <= FREQ_VERTEX_TOPK_EDGE_HEAVY){
                if (bucket0.GetEntryCount(vertex_bucket_size - 1) <
                    bucket1.GetEntryCount(vertex_bucket_size - 1)) {
                    if(bucket0.GetEntryCount(vertex_bucket_size-1)<_weight){
                        bucket0.ClearEdge(vertex_bucket_size-1);
                        bucket0.SetId(vertex_bucket_size-1,_v);
                        bucket0.Insert(vertex_bucket_size-1,_u,_weight);
                        bucket0.BubbleSort(vertex_bucket_size-1);
                        if(bucket0.GetEntryCount(1)>_thres){
                            if(KickOut(MAX_KICK_OUT,Hash(bucket0.GetEntry(1).GetId()),bucket0,1,0)){
                                bucket0.Remove(1);
                            }
                        }
                    }else{
                        bucket0.GetEntry(vertex_bucket_size-1).Loss(_weight);
                    }
                } else {
                    if(bucket1.GetEntryCount(vertex_bucket_size-1)<_weight){
                        bucket1.ClearEdge(vertex_bucket_size-1);
                        bucket1.SetId(vertex_bucket_size-1,_v);
                        bucket1.Insert(vertex_bucket_size-1,_u,_weight);
                        bucket1.BubbleSort(vertex_bucket_size-1);
                        if(bucket1.GetEntryCount(1)>_thres){
                            auto value = Hash(bucket1.GetEntry(1).GetId());
                            auto fp = HashFp(bucket1.GetEntry(1).GetId());
                            if(KickOut(MAX_KICK_OUT,value^Hash(static_cast<uint8_t>(fp & fp_mask)),bucket1,1,1)){
                                bucket1.Remove(1);
                            }
                        }
                    }
                    bucket1.GetEntry(vertex_bucket_size-1).Loss(_weight);
                }
            }else{
                if (bucket0.GetEntryCount(vertex_bucket_size - 1) <
                    bucket1.GetEntryCount(vertex_bucket_size - 1)) {
                    bucket0.GetEntry(vertex_bucket_size-1).Loss(1);
                }else{
                    bucket1.GetEntry(vertex_bucket_size-1).Loss(1);
                }
            }
        }
        bool Insert(VERTEX_ID_TYPE _v,const std::vector<typename GraphFilter<VERTEX_ID_TYPE, FP_TYPE, WEIGHT_TYPE>::GraphEntry*>& entries){
            uint32_t hash_key = Hash(_v);
            uint32_t fp = HashFp(_v);
#ifdef TEST1
            uint32_t hash_value[2] = {hash_key, hash_key + (fp & fp_mask)};
#else
            uint32_t hash_value[2] = {hash_key,
                              hash_key ^ Hash(static_cast<uint8_t>(fp & fp_mask))};
#endif
            uint32_t keys[2] = {hash_value[0]%_bucket_num,hash_value[1]%_bucket_num};
            VertexBucket& bucket0 = _buckets[0][keys[0]];
            VertexBucket& bucket1 = _buckets[1][keys[1]];
            if(bucket0.Equal(0,_v)){
                for(auto elem:entries){
                    bucket0.EdgeInsertByFp(elem->u_,elem->weight_,bucket0.GetEntryFlag(0));
                }
                if constexpr(mode_flag <= FREQ_VERTEX_TOPK_EDGE_HEAVY){
                    int sum = 0;
                    for(auto elem:entries){
                        sum+=elem->weight_;
                    }
                    bucket0.GetEntry(0).Insert(sum);
                }
                else{
                    bucket0.GetEntry(0).Insert(entries.size());
                }
                if(bucket0.GetEntryCount(0)>_f_max){
                    _f_max = bucket0.GetEntryCount(0);
                    _thres = std::max(_thres,static_cast<int>(_f_max * 1.5 / _topk));
                }
                return true;
            }
            if(bucket0.IsEmpty(0)){
                bucket0.ClearEdge(0);
                bucket0.SetId(0,_v);
                for(auto elem:entries){
                    bucket0.EdgeInsertByFp(elem->u_,elem->weight_,bucket0.GetEntryFlag(0));
                }
                if constexpr(mode_flag <= FREQ_VERTEX_TOPK_EDGE_HEAVY){
                    int sum = 0;
                    for(auto elem:entries){
                        sum+=elem->weight_;
                    }
                    bucket0.GetEntry(0).Insert(sum);
                }
                else{
                    bucket0.GetEntry(0).Insert(entries.size());
                }
                if(bucket0.GetEntryCount(0)>_f_max){
                    _f_max = bucket0.GetEntryCount(0);
                    _thres = std::max(_thres,static_cast<int>(_f_max * 1.5 / _topk));
                }
                return true;
            }
            if(bucket1.Equal(0,_v)){
                for(auto elem:entries){
                    bucket1.EdgeInsertByFp(elem->u_,elem->weight_,bucket1.GetEntryFlag(0));
                }
                if constexpr(mode_flag <= FREQ_VERTEX_TOPK_EDGE_HEAVY){
                    int sum = 0;
                    for(auto elem:entries){
                        sum+=elem->weight_;
                    }
                    bucket1.GetEntry(0).Insert(sum);
                }
                else{
                    bucket1.GetEntry(0).Insert(entries.size());
                }
                if(bucket1.GetEntryCount(0)>_f_max){
                    _f_max = bucket1.GetEntryCount(0);
                    _thres = std::max(_thres,static_cast<int>(_f_max * 1.5 / _topk));
                }
                return true;
            }
            if(bucket1.IsEmpty(0)){
                bucket1.ClearEdge(0);
                bucket1.SetId(0,_v);
                for(auto elem:entries){
                    bucket1.EdgeInsertByFp(elem->u_,elem->weight_,bucket1.GetEntryFlag(0));
                }
                if constexpr(mode_flag <= FREQ_VERTEX_TOPK_EDGE_HEAVY){
                    int sum = 0;
                    for(auto elem:entries){
                        sum+=elem->weight_;
                    }
                    bucket1.GetEntry(0).Insert(sum);
                }
                else{
                    bucket1.GetEntry(0).Insert(entries.size());
                }
                if(bucket1.GetEntryCount(0)>_f_max){
                    _f_max = bucket1.GetEntryCount(0);
                    _thres = std::max(_thres,static_cast<int>(_f_max * 1.5 / _topk));
                }
                return true;
            }
            for(int i = 1;i<vertex_bucket_size;i++){
                if(bucket0.Equal(i,_v)){
                    for(auto elem:entries){
                        bucket0.EdgeInsertByFp(elem->u_,elem->weight_,bucket0.GetEntryFlag(i));
                    }
                    if constexpr(mode_flag <= FREQ_VERTEX_TOPK_EDGE_HEAVY){
                        int sum = 0;
                        for(auto elem:entries){
                            sum+=elem->weight_;
                        }
                        bucket0.GetEntry(i).Insert(sum);
                    }
                    else{
                        bucket0.GetEntry(i).Insert(entries.size());
                    }
                    bucket0.BubbleSort(i);
                    if(bucket0.GetEntryCount(1)>_thres){
                        if(KickOut(MAX_KICK_OUT,Hash(bucket0.GetEntry(1).GetId()),bucket0,1,0)){
                            bucket0.Remove(1);
                        }
                    }
                    return true;
                }
                if (bucket0.IsEmpty(i)) {
                    bucket0.ClearEdge(i);
                    bucket0.SetId(i,_v);
                    for(auto elem:entries){
                        bucket0.EdgeInsertByFp(elem->u_,elem->weight_,bucket0.GetEntryFlag(i));
                    }
                    if constexpr(mode_flag <= FREQ_VERTEX_TOPK_EDGE_HEAVY){
                        int sum = 0;
                        for(auto elem:entries){
                            sum+=elem->weight_;
                        }
                        bucket0.GetEntry(i).Insert(sum);
                    }
                    else{
                        bucket0.GetEntry(i).Insert(entries.size());
                    }
                    bucket0.BubbleSort(i);
                    if(bucket0.GetEntryCount(1)>_thres){
                        if(KickOut(MAX_KICK_OUT,Hash(bucket0.GetEntry(1).GetId()),bucket0,1,0)){
                            bucket0.Remove(1);
                        }
                    }
                    return true;
                }
                if(bucket1.Equal(i,_v)){
                    for(auto elem:entries){
                        bucket1.EdgeInsertByFp(elem->u_,elem->weight_,bucket1.GetEntryFlag(i));
                    }
                    if constexpr(mode_flag <= FREQ_VERTEX_TOPK_EDGE_HEAVY){
                        int sum = 0;
                        for(auto elem:entries){
                            sum+=elem->weight_;
                        }
                        bucket1.GetEntry(i).Insert(sum);
                    }
                    else{
                        bucket1.GetEntry(i).Insert(entries.size());
                    }
                    bucket1.BubbleSort(i);
                    if(bucket1.GetEntryCount(1)>_thres){
                        auto value = Hash(bucket1.GetEntry(1).GetId());
                        auto fp = HashFp(bucket1.GetEntry(1).GetId());
                        if(KickOut(MAX_KICK_OUT,value^Hash(static_cast<uint8_t>(fp & fp_mask)),bucket1,1,1)){
                            bucket1.Remove(1);
                        }
                    }
                    return true;
                }
                if (bucket1.IsEmpty(i)) {
                    bucket1.ClearEdge(i);
                    bucket1.SetId(i,_v);
                    for(auto elem:entries){
                        bucket1.EdgeInsertByFp(elem->u_,elem->weight_,bucket1.GetEntryFlag(i));
                    }
                    if constexpr(mode_flag <= FREQ_VERTEX_TOPK_EDGE_HEAVY){
                        int sum = 0;
                        for(auto elem:entries){
                            sum+=elem->weight_;
                        }
                        bucket1.GetEntry(i).Insert(sum);
                    }
                    else{
                        bucket1.GetEntry(i).Insert(entries.size());
                    }
                    bucket1.BubbleSort(i);
                    if(bucket1.GetEntryCount(1)>_thres){
                        auto value = Hash(bucket1.GetEntry(1).GetId());
                        auto fp = HashFp(bucket1.GetEntry(1).GetId());
                        if(KickOut(MAX_KICK_OUT,value^Hash(static_cast<uint8_t>(fp & fp_mask)),bucket1,1,1)){
                            bucket1.Remove(1);
                        }
                    }
                    return true;
                }
            }
            if (bucket0.GetEntryCount(vertex_bucket_size - 1) < bucket1.GetEntryCount(vertex_bucket_size - 1)) {
                int sum = 0;
                if constexpr(mode_flag <= FREQ_VERTEX_TOPK_EDGE_HEAVY){
                    for(auto elem:entries){
                        sum+=elem->weight_;
                    }
                }else{
                    sum = entries.size();
                }
                if(bucket0.GetEntryCount(vertex_bucket_size-1)<sum){
                    bucket0.ClearEdge(vertex_bucket_size-1);
                    bucket0.SetId(vertex_bucket_size-1,_v);
                    for(auto elem:entries){
                        bucket0.EdgeInsertByFp(elem->u_,elem->weight_,bucket0.GetEntryFlag(vertex_bucket_size-1));
                    }
                    bucket0.GetEntry(vertex_bucket_size-1).Insert(sum);
                    bucket0.BubbleSort(vertex_bucket_size-1);
                    if(bucket0.GetEntryCount(1)>_thres){
                        if(KickOut(MAX_KICK_OUT,Hash(bucket0.GetEntry(1).GetId()),bucket0,1,0)){
                            bucket0.Remove(1);
                        }
                    }
                    return true;
                }else{
                    return false;
                }
            }else{
                int sum = 0;
                if constexpr(mode_flag <= FREQ_VERTEX_TOPK_EDGE_HEAVY){
                    for(auto elem:entries){
                        sum+=elem->weight_;
                    }
                }else{
                    sum = entries.size();
                }
                if(bucket1.GetEntryCount(vertex_bucket_size-1)<sum){
                    bucket1.ClearEdge(vertex_bucket_size-1);
                    bucket1.SetId(vertex_bucket_size-1,_v);
                    for(auto elem:entries){
                        bucket1.EdgeInsertByFp(elem->u_,elem->weight_,bucket1.GetEntryFlag(vertex_bucket_size-1));
                    }
                    bucket1.GetEntry(vertex_bucket_size-1).Insert(sum);
                    bucket1.BubbleSort(vertex_bucket_size-1);
                    if(bucket1.GetEntryCount(1)>_thres){
                        auto value = Hash(bucket1.GetEntry(1).GetId());
                        auto fp = HashFp(bucket1.GetEntry(1).GetId());
                        if(KickOut(MAX_KICK_OUT,value^Hash(static_cast<uint8_t>(fp & fp_mask)),bucket1,1,1)){
                            bucket1.Remove(1);
                        }
                    }
                    return true;
                }else{
                    return false;
                }
            }
            return false;    
        }
        void SetAnswer(const std::unordered_map<VERTEX_ID_TYPE,Vertex<VERTEX_ID_TYPE,WEIGHT_TYPE>>& answer){_answer = answer;}
};

template<typename VERTEX_ID_TYPE, typename WEIGHT_TYPE, typename FP_TYPE>
class HeavyDetector:public DetectorAbstract<VERTEX_ID_TYPE,WEIGHT_TYPE>{
    private:
        GraphFilter<VERTEX_ID_TYPE,FP_TYPE,WEIGHT_TYPE>* _filter;
        BubbleArrays<VERTEX_ID_TYPE,WEIGHT_TYPE,FP_TYPE>* _arrays;
    public:
        HeavyDetector(int mem){
            auto filter_mem = mem * filterMemoryRate*1024;
            auto array_mem = mem*(1-filterMemoryRate)*1024;
            int entryAmount = filter_mem / sizeof(typename GraphFilter<VERTEX_ID_TYPE,WEIGHT_TYPE,FP_TYPE>::GraphEntry);
            int width = static_cast<int>(sqrt(entryAmount * widthheightRate));
            int height = width / widthheightRate;
            assert(width* height <= entryAmount);
            _filter = new GraphFilter<VERTEX_ID_TYPE,FP_TYPE,WEIGHT_TYPE>(width,height);
            auto bucket_num = array_mem / 2 / (vertex_bucket_size * sizeof(typename BubbleArrays<VERTEX_ID_TYPE,WEIGHT_TYPE,FP_TYPE>::VertexEntry) + id_bucket_size*(sizeof(VERTEX_ID_TYPE)+(COUNT_BITS+FLAG_BITS)/8) + (edge_bucket_size-id_bucket_size)*(COUNT_BITS+FLAG_BITS+FP_BITS)/8);
            _arrays = new BubbleArrays<VERTEX_ID_TYPE,WEIGHT_TYPE,FP_TYPE>(bucket_num,graph_thres,vertex_top_k); 
        }
        ~HeavyDetector(){
            delete _filter;
            delete _arrays;
        }
        void Insert(VERTEX_ID_TYPE _v, VERTEX_ID_TYPE _u, WEIGHT_TYPE _weight){
            if(_arrays->Check(_v,_u,_weight)){
                return;
            }
            if(_filter->Insert(_v,_u,_weight)){
                WEIGHT_TYPE sum = 0;
                auto res = _filter->NodeValQuery(_v,sum);
                if(sum >= freq_filter_thres){
                    if(_arrays->Insert(_v,res)){
                        for(auto elem:res){
                            elem->Clear();
                        }
                        return;
                    }
                }
            }
            return;
        }
        std::vector<Vertex<VERTEX_ID_TYPE,WEIGHT_TYPE>> Query(){
            std::vector<Vertex<VERTEX_ID_TYPE,WEIGHT_TYPE>> res;
            for(int i = 0;i<BUBBLE_ARRAY_SIZE;i++){
                for(int j = 0;j<_arrays->_bucket_num;j++){
                    for(int k = 0;k<vertex_bucket_size;k++){
                        if(!_arrays->_buckets[i][j].GetEntry(k).IsEmpty()){
                            Vertex<VERTEX_ID_TYPE,WEIGHT_TYPE> elem(_arrays->_buckets[i][j].GetEntry(k).GetId(),_arrays->_buckets[i][j].GetEntry(k).GetCount());
                            int edge_weight_sum = 0;
                            for(auto edge:_arrays->_buckets[i][j].FindEdgeWithId(k)){
                                if(!edge.IsEmpty()){
                                    elem.Insert(edge.GetId(),edge.GetCount());
                                    edge_weight_sum += edge.GetCount();
                                }
                            }
                            for(auto edge:_arrays->_buckets[i][j].FindEdgeWithFp(k)){
                                if(!edge.IsEmpty()){
                                    edge_weight_sum += edge.GetCount();
                                }
                            }
                            elem._sum = _arrays->_buckets[i][j].GetEntry(k).GetCount();
                            if constexpr(mode_flag == FREQ_VERTEX_HEAVY_EDGE_HEAVY || mode_flag ==  FREQ_VERTEX_TOPK_EDGE_HEAVY){
                                elem.QueryHeavy(edge_frequency_heavy_threshold);
                            }else{
                                elem.QueryTopK(edge_top_k);
                            }
                            res.emplace_back(elem);
                        }else{
                            break;
                        }
                    }
                }
            }
            std::sort(res.begin(),res.end(),[](auto const &a, auto const &b) {
                return a._sum > b._sum;
            });
            int max_index = 0;
            if (mode_flag == FREQ_VERTEX_HEAVY_EDGE_HEAVY || mode_flag == FREQ_VERTEX_HEAVY_EDGE_TOPK){
                for(auto elem:res){
                    auto thres = (mode_flag == FREQ_VERTEX_HEAVY_EDGE_HEAVY || mode_flag == FREQ_VERTEX_HEAVY_EDGE_TOPK)?vertex_frequency_heavy_threshold:vertex_degree_heavy_threshold;
                    if(elem._sum >= thres){
                        max_index++;
                    }else{
                        break;
                    }
                }
            }else{
                max_index = std::min(vertex_top_k,_arrays->_bucket_num*2*vertex_bucket_size);
            }
            return std::vector<Vertex<VERTEX_ID_TYPE,WEIGHT_TYPE>>(res.begin(),res.begin()+max_index);
        };
};

#endif