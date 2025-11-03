#ifndef _GSSDETECTOR_H
#define _GSSDETECTOR_H

#include <map>
#include "DetectorAbstract.h"
#include "hashTable.h"
#include "Param.h"
#include "GSS.hpp"
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <algorithm>
#include <string>
#include <cstring>

#define MAX_INSERT_PACKAGE 1000000 //zipf
#define N MAX_INSERT_PACKAGE  // maximum flow
#define M MAX_INSERT_PACKAGE  // maximum size of stream-summary
#define MAX_MEM MAX_INSERT_PACKAGE // maximum memory size
#define len2 9973
#define rep(i,a,n) for(int i=a;i<=n;i++)
using namespace std;

#define MAX_INSERT_EDGE 600000 //zipf
#define N_E MAX_INSERT_EDGE  // maximum flow
#define M_EE MAX_INSERT_EDGE  // maximum size of stream-summary
#define MAX_EDGE_MEM MAX_INSERT_EDGE // maximum memory size
#define len_e 9973

template<typename VERTEX_ID_TYPE,typename WEIGHT_TYPE>
class Esummary
{
    public:
        int tot;
        WEIGHT_TYPE sum[M_EE+10],K,last[M_EE+10],Next[M_EE+10],ID[M_EE+10],tmp[M_EE+10];
        WEIGHT_TYPE head[N_E+10],Left[N_E+10],Right[N_E+10],num;
        VERTEX_ID_TYPE vertex[M_EE+10];
        WEIGHT_TYPE head2[len_e+10],Next2[M_EE+10];
        Esummary():K(edge_top_k){Clear();}
        Esummary(int K):K(K) {Clear();}
        void Clear()
        {
            memset(sum,0,sizeof(sum));
            memset(last,0,sizeof(Next));
            memset(Next2,0,sizeof(Next2));
	        memset(tmp,0,sizeof(tmp));
            rep(i,0,N_E)head[i]=Left[i]=Right[i]=0;
            rep(i,0,len_e+9)head2[i]=0;
            tot=0;
            rep(i,1,M_EE+2)ID[i]=i;
            num=M_EE+2;
            Right[0]=N_E;
            Left[N_E]=0;
        }
        int Getid()
        {
            int i=ID[num--];
            last[i]=Next[i]=sum[i]=Next2[i]=0;
            return i;
        }
        int Location(VERTEX_ID_TYPE item)
        {
            return (hfunc[0]((unsigned char*)&item,sizeof(item)))%len_e;
        }
        void Add2(int x,int y)
        {
            Next2[y]=head2[x];
            head2[x]=y;
        }
        int Find(VERTEX_ID_TYPE s)
        {
            for(int i=head2[Location(s)];i;i=Next2[i])
                if(vertex[i]==s){
                    return i;
                }
            return 0;
        }
        void Linkhead(int i,int j)
        {
            Left[i]=j;
            Right[i]=Right[j];
            Right[j]=i;
            Left[Right[i]]=i;
        }
        void Cuthead(int i)
        {
            int t1=Left[i],t2=Right[i];
            Right[t1]=t2;
            Left[t2]=t1;
        }
        int Getmin()
        {
            if (tot<K) return 0;
            if(Right[0]==N_E)return 1;
            return Right[0];
        }
        void Link(int i,int ww)
        {
            ++tot;
            bool flag=(head[sum[i]]==0);
            Next[i]=head[sum[i]];
            if(Next[i])last[Next[i]]=i;
            last[i]=0;
            head[sum[i]]=i;
            if(flag)
            {
                for(int j=sum[i]-1;j>0 && j>sum[i]-10;j--)
                if(head[j]){Linkhead(sum[i],j);return;}
                Linkhead(sum[i],ww);
            }
        }
        void Cut(int i)
        {
            --tot;
            if(head[sum[i]]==i)head[sum[i]]=Next[i];
            if(head[sum[i]]==0)Cuthead(sum[i]);
            int t1=last[i],t2=Next[i];
            if(t1)Next[t1]=t2;
            if(t2)last[t2]=t1;
        }
        void Recycling(int i)
        {
            int w=Location(vertex[i]);
            if (head2[w]==i)
                head2[w]=Next2[i];
            else
            {
                for(int j=head2[w];j;j=Next2[j])
                if(Next2[j]==i)
                {
                    Next2[j]=Next2[i];
                    break;
                }
            }
            ID[++num]=i;
        }
};

template <typename VERTEX_ID_TYPE,typename WEIGHT_TYPE>
class Ssummary
{
    public:
        int tot;
        WEIGHT_TYPE sum[M+10],K,last[M+10],Next[M+10],ID[M+10],tmp[M+10];
        Esummary<VERTEX_ID_TYPE,WEIGHT_TYPE>* e[M+10];
        WEIGHT_TYPE head[N+10],Left[N+10],Right[N+10],num;
        VERTEX_ID_TYPE vertex[M+10];
        WEIGHT_TYPE head2[len2+10],Next2[M+10];
        Ssummary(int K):K(K) {}
        ~Ssummary(){
            for(int i=0;i<M+10;i++){
                if(e[i]!=nullptr){
                    delete e[i];
                }
            }
        }
        void Clear()
        {
            memset(sum,0,sizeof(sum));
            memset(last,0,sizeof(Next));
            memset(Next2,0,sizeof(Next2));
	        memset(tmp,0,sizeof(tmp));
            rep(i,0,N)head[i]=Left[i]=Right[i]=0;
            rep(i,0,len2-1)head2[i]=0;
            tot=0;
            rep(i,1,M+2)ID[i]=i;
            num=M+2;
            Right[0]=N;
            Left[N]=0;
            for(int i=0;i<M+10;i++){
                e[i] = nullptr;
            }
        }
        int Getid()
        {
            int i=ID[num--];
            last[i]=Next[i]=sum[i]=Next2[i]=0;
            return i;
        }
        int Location(VERTEX_ID_TYPE item)
        {
            return (hfunc[0]((unsigned char*)&item,sizeof(item)))%len2;
        }
        void Add2(int x,int y)
        {
            Next2[y]=head2[x];
            head2[x]=y;
        }
        int Find(VERTEX_ID_TYPE s)
        {
            for(int i=head2[Location(s)];i;i=Next2[i])
              if(vertex[i]==s)return i;
            return 0;
        }
        void Linkhead(int i,int j)
        {
            Left[i]=j;
            Right[i]=Right[j];
            Right[j]=i;
            Left[Right[i]]=i;
        }
        void Cuthead(int i)
        {
            int t1=Left[i],t2=Right[i];
            Right[t1]=t2;
            Left[t2]=t1;
        }
        int Getmin()
        {
            if (tot<K) return 0;
            if(Right[0]==N)return 1;
            return Right[0];
        }
        void Link(int i,int ww)
        {
            ++tot;
            bool flag=(head[sum[i]]==0);
            Next[i]=head[sum[i]];
            if(Next[i])last[Next[i]]=i;
            last[i]=0;
            head[sum[i]]=i;
            if(flag)
            {
                for(int j=sum[i]-1;j>0 && j>sum[i]-10;j--)
                if(head[j]){Linkhead(sum[i],j);return;}
                Linkhead(sum[i],ww);
            }
        }
        void Cut(int i)
        {
            --tot;
            if(head[sum[i]]==i)head[sum[i]]=Next[i];
            if(head[sum[i]]==0)Cuthead(sum[i]);
            int t1=last[i],t2=Next[i];
            if(t1)Next[t1]=t2;
            if(t2)last[t2]=t1;
        }
        void Recycling(int i)
        {
            int w=Location(vertex[i]);
            if (head2[w]==i)
                head2[w]=Next2[i];
            else
            {
                for(int j=head2[w];j;j=Next2[j])
                if(Next2[j]==i)
                {
                    Next2[j]=Next2[i];
                    break;
                }
            }
            ID[++num]=i;
            if(e[i]!=nullptr){
                e[i]->Clear();
            }
        }
};


template<typename VERTEX_ID_TYPE, typename WEIGHT_TYPE >
class GSSDetector:public DetectorAbstract<VERTEX_ID_TYPE,WEIGHT_TYPE>{
    
    private:
        GSS<VERTEX_ID_TYPE,WEIGHT_TYPE>* gss;
        Ssummary<VERTEX_ID_TYPE,WEIGHT_TYPE>* ss;
        int thresh = 3;
    public:
        GSSDetector(int memory){
            int mem = memory ;
            int size = 0;
            if(bufferMaxGss >= 0){
                size = (mem - bufferMaxGss * sizeof(linknode<WEIGHT_TYPE>))  / sizeof(basket<WEIGHT_TYPE>);
            }else{
                size = mem / sizeof(basket<WEIGHT_TYPE>);
            }
            assert(size > 0);
            size = sqrt(size);
            gss = new GSS<VERTEX_ID_TYPE,WEIGHT_TYPE>(size, rangeGss, p_numGss, sizeGss, f_numGss, bufferMaxGss);
            ss = new Ssummary<VERTEX_ID_TYPE,WEIGHT_TYPE>(vertex_top_k);
            ss->Clear();
        }
        ~GSSDetector(){
            delete gss;
            delete ss;
        }
        void Insert(VERTEX_ID_TYPE _v, VERTEX_ID_TYPE _u, WEIGHT_TYPE _weight){
            gss->Insert(_v,_u,_weight);
            int maxv = gss->NodeValueQuery(_v,0);
            int edge_maxv = gss->EdgeQuery(_v,_u);
            bool mon = false;
            int p = ss->Find(_v);
            if (p) mon = true;
            if (!mon)
            {
                #ifdef WEIGHT
                if (maxv > ss->Getmin() || ss->tot < vertex_top_k)
                #else
                if (maxv - (ss->Getmin()) == 1 || ss->tot < vertex_top_k)
                #endif
                {
                    int i = ss->Getid();
                    ss->Add2(ss->Location(_v), i);
                    ss->vertex[i] = _v;
                    ss->sum[i] = maxv;
                    ss->tmp[i] = maxv;
                    ss->Link(i, 0);
                    while (ss->tot > vertex_top_k)
                    {
                        int t = ss->Right[0];
                        int tmp = ss->head[t];
                        ss->Cut(ss->head[t]);
                        ss->Recycling(tmp);
                    }

                    if(ss->e[i]==nullptr){
                        ss->e[i] = new Esummary<VERTEX_ID_TYPE,WEIGHT_TYPE>();
                    }
                    auto es = (ss->e[i]);
                    bool emon = false;
                    int ep = es->Find(_u);
                    if (ep) emon = true;

                    if(!emon){
                        #ifdef WEIGHT
                        if (edge_maxv > es->Getmin() || es->tot < edge_top_k)
                        #else
                        if (edge_maxv - (es->Getmin()) == 1 || es->tot < edge_top_k)
                        #endif
                        {
                            int i = es->Getid();
                            es->Add2(es->Location(_u), i);
                            es->vertex[i] = _u;
                            es->sum[i] = edge_maxv;
                            es->tmp[i] = edge_maxv;
                            es->Link(i, 0);
                            while (es->tot > edge_top_k)
                            {
                                int t = es->Right[0];
                                int tmp = es->head[t];
                                es->Cut(es->head[t]);
                                es->Recycling(tmp);
                            }
                        }
                    }
                    else if (edge_maxv > es->sum[ep])
                    {
                        int tmp = es->Left[es->sum[ep]];
                        es->Cut(ep);
                        if (es->head[es->sum[ep]]) tmp = es->sum[ep];
                        es->sum[ep] = edge_maxv;
                        es->Link(ep, tmp);
                    }
                }
            }
            else if (maxv > ss->sum[p])
            {
                int tmp = ss->Left[ss->sum[p]];
                ss->Cut(p);
                if (ss->head[ss->sum[p]]) tmp = ss->sum[p];
                ss->sum[p] = maxv;
                ss->Link(p, tmp);
                assert(p == ss->Find(_v));
                if(ss->e[p]==nullptr){
                    ss->e[p] = new Esummary<VERTEX_ID_TYPE,WEIGHT_TYPE>();   
                }
                auto es = ss->e[p];
                bool emon = false;
                int ep = es->Find(_u);
                if (ep) emon = true;

                if(!emon){
                    #ifdef WEIGHT
                    if (edge_maxv > es->Getmin() || es->tot < edge_top_k)
                    #else
                    if (edge_maxv - (es->Getmin()) == 1 || es->tot < edge_top_k)
                    #endif
                    {
                        int i = es->Getid();
                        es->Add2(es->Location(_u), i);
                        es->vertex[i] = _u;
                        es->sum[i] = edge_maxv;
                        es->tmp[i] = edge_maxv;
                        es->Link(i, 0);
                        while (es->tot > edge_top_k)
                        {
                            int t = es->Right[0];
                            int tmp = es->head[t];
                            es->Cut(es->head[t]);
                            es->Recycling(tmp);
                        }
                    }
                }
                else if (edge_maxv > es->sum[ep])
                {
                    int tmp = es->Left[es->sum[ep]];
                    es->Cut(ep);
                    if (es->head[es->sum[ep]]) tmp = es->sum[ep];
                    es->sum[ep] = edge_maxv;
                    es->Link(ep, tmp);
                }
            }
        }

        struct Node { VERTEX_ID_TYPE x; int y; int thre;Esummary<VERTEX_ID_TYPE,WEIGHT_TYPE>* e=nullptr;}; 
        static int Cmp(Node i, Node j) { return i.y > j.y; }
        void QueryEdge(Vertex<VERTEX_ID_TYPE,WEIGHT_TYPE>& v,Esummary<VERTEX_ID_TYPE,WEIGHT_TYPE>* e,int thres = 0){
            if (e==nullptr){
                return;
            }
            unordered_map<int, int> mp;
            int jump=0, x_num=0;
            std::vector<Node> q;
            q.resize(MAX_EDGE_MEM+10);
            int CNT = 0;
            for (int i = N_E; i && i!=e->Left[i]; i = e->Left[i]){
                for (int j = e->head[i]; j && j!=e->Next[j]; j = e->Next[j])
                {
                    q[CNT].x = e->vertex[j]; q[CNT].y = e->sum[j]; 
                    q[CNT].thre=e->sum[j]-e->tmp[j];
                    CNT++;
                    mp[(*hfunc[0])((unsigned char*)(&(e->vertex[j])),sizeof(e->vertex[j])) >> 24]++;
                }
            }
            sort(q.begin(), q.begin() + CNT, Cmp);
            if constexpr (mode_flag == FREQ_VERTEX_TOPK_EDGE_TOPK || mode_flag == FREQ_VERTEX_HEAVY_EDGE_TOPK){
                auto max_index = std::min(edge_top_k,CNT);
                for(int k = 0;k+jump<max_index;k++){
                    while(mp[(*hfunc[0])((unsigned char*)(&(q[k+jump].x)),sizeof(q[k+jump].x))]>1 && q[k+jump].thre<thresh && x_num<20){
                        x_num++;
                        jump++;
                    }
                    v.Insert(q[k+jump].x,q[k+jump].y);
                }
            }else{
                for(int k = 0;k+jump<CNT  ;k++){
                    while(mp[(*hfunc[0])((unsigned char*)(&(q[k+jump].x)),sizeof(q[k+jump].x))]>1 && q[k+jump].thre<thresh && x_num<20){
                        x_num++;
                        jump++;
                    }
                    if(q[k+jump].y>=thres){
                        v.Insert(q[k+jump].x,q[k+jump].y);
                    }
                }
            }
        }
        std::vector<Vertex<VERTEX_ID_TYPE,WEIGHT_TYPE>> Query()
        {
            unordered_map<int, int> mp;
            int jump=0, x_num=0;
            std::vector<Node> q;
            q.resize(MAX_MEM+10);
            std::vector<Vertex<VERTEX_ID_TYPE,WEIGHT_TYPE>> res;
            int CNT = 0;
            for (int i = N; i; i = ss->Left[i]){
                for (int j = ss->head[i]; j; j = ss->Next[j])
                {
                    q[CNT].x = ss->vertex[j]; q[CNT].y = ss->sum[j]; 
                    q[CNT].thre=ss->sum[j]-ss->tmp[j];
                    q[CNT].e = ss->e[j];
                    CNT++;
                    mp[(*hfunc[0])((unsigned char*)(&(ss->vertex[j])),sizeof(ss->vertex[j])) >> 24]++;
                }
            }
            sort(q.begin(), q.begin() + CNT, Cmp);
            auto max_index = std::min(vertex_top_k,CNT);
            for(int k = 0;k+jump<max_index;k++){
                while(mp[(*hfunc[0])((unsigned char*)(&(q[k+jump].x)),sizeof(q[k+jump].x))]>1 && q[k+jump].thre<thresh && x_num<20){
                    x_num++;
                    jump++;
                }
                Vertex<VERTEX_ID_TYPE,WEIGHT_TYPE> elem(q[k+jump].x,0);
                QueryEdge(elem,q[k+jump].e,edge_frequency_heavy_threshold);
                elem._sum = q[k+jump].y;
                if constexpr (mode_flag == FREQ_VERTEX_HEAVY_EDGE_HEAVY || mode_flag == FREQ_VERTEX_HEAVY_EDGE_TOPK ){
                    if(elem._sum >= vertex_frequency_heavy_threshold){
                        res.emplace_back(elem);
                    }
                }else{
                    res.emplace_back(elem);
                }
            }
            return res;
        }
};

#endif