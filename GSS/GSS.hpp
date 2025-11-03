#ifndef _GSS_H_
#define _GSS_H_

#include<iostream>
#include<string>
#include<vector>
#include<queue>
#include<set>
#include<map>
#include<cmath>
#include<stdlib.h>
#include<bitset>
#include<memory.h>
#ifndef HASHTABLE_H
#define HASHTABLE_H
#include "GraphUndirected.h"
#endif
using namespace std;
#define prime 739
#define bigger_p 1048576
#define timer 5


#define Roomnum 2 // This is the parameter to controll the maximum number of rooms in a bucket. 

template <typename WEIGHT_TYPE>
struct basket
{
	unsigned short src[Roomnum];
	unsigned short dst[Roomnum];
	WEIGHT_TYPE  weight[Roomnum];
	unsigned int idx; // need to change to unsigned long long if Roomnum is larger than 4.
};
struct mapnode
{
	unsigned int h;
	unsigned short g;
};
template <typename WEIGHT_TYPE>
struct linknode
{
	unsigned int key;
    WEIGHT_TYPE weight;
	linknode<WEIGHT_TYPE>* next;
};

template<typename VERTEX_ID_TYPE,typename WEIGHT_TYPE>
class GSS
{
private:
	int w;
	int r;
	int p;
	int s;
	int f;
    int bufferMax;
	
	basket<WEIGHT_TYPE>* value;

	public:
		vector<linknode<WEIGHT_TYPE>*> buffer;
		map<unsigned int, int> index;
		int n;
		int edge_num; // count the number of edges in the buffer to assist buffer size analysis. Self loop edge is not included as it does not use additional memory.
		GSS(int width, int range, int p_num, int size,int f_num,int _bufferMax); // table size is set approximately the the number of nodes in the graph.
		~GSS()
		{
			delete[] value;
			CleanupBuffer();

		 }
        void Insert(VERTEX_ID_TYPE s1, VERTEX_ID_TYPE s2,WEIGHT_TYPE weight);
		 void CleanupBuffer();
		 WEIGHT_TYPE EdgeQuery(VERTEX_ID_TYPE s1, VERTEX_ID_TYPE s2);
		 WEIGHT_TYPE NodeValueQuery(VERTEX_ID_TYPE s1, int type);//src_type = 0 dst_type = 1

};


template<typename VERTEX_ID_TYPE,typename WEIGHT_TYPE>
GSS<VERTEX_ID_TYPE,WEIGHT_TYPE>::GSS(int width, int range, int p_num, int size,int f_num,int _bufferMax)//the side length of matrix, the length of hash addtress list, the number of candidate bucekt
// the number of rooms, whether to use hash table, and the size of the table.
// Hash table which stores the original nodes can be omitted if not needed. For nodequery, 
//  reachability, edgequery not needed. But needed for triangel counting, degree query, and successor / precursor queries.
{
	w = width;
	r = range; /* r x r mapped baskets */
	p = p_num; /*candidate buckets*/
	s = size; /*multiple rooms*/
	f = f_num; /*finger print lenth*/
	n = 0;
	edge_num = 0;
	value = new basket<WEIGHT_TYPE>[w*w];
    bufferMax = _bufferMax;
	memset(value, 0, sizeof(basket<WEIGHT_TYPE>)*w*w);
	
}

template<typename VERTEX_ID_TYPE,typename WEIGHT_TYPE>
void GSS<VERTEX_ID_TYPE,WEIGHT_TYPE>::CleanupBuffer()
{
	typename vector<linknode<WEIGHT_TYPE>*>::iterator IT = buffer.begin();
	linknode<WEIGHT_TYPE>* e, *tmp;
	for (; IT != buffer.end(); ++IT)
	{
		e = *IT;
		while (e != NULL)
		{
			tmp = e->next;
			delete e;
			e = tmp;
		}
	}
}

template<typename VERTEX_ID_TYPE,typename WEIGHT_TYPE>
void GSS<VERTEX_ID_TYPE,WEIGHT_TYPE>::Insert(VERTEX_ID_TYPE s1, VERTEX_ID_TYPE s2,WEIGHT_TYPE weight)// s1 is the ID of the source node, s2 is the ID of the destination node, weight is the edge weight.
{
    //todo
    unsigned int hash1 = (*hfunc[0])((unsigned char*)(&s1), sizeof(s1) );
    unsigned int hash2 = (*hfunc[0])((unsigned char*)(&s2), sizeof(s2) );
	unsigned int tmp = pow(2,f)-1;
	unsigned short g1 = hash1 & tmp;
	if(g1==0) g1+=1;
	unsigned int h1 = (hash1>>f)%w;
	unsigned short g2 = hash2 & tmp;
	if(g2==0) g2+=1;
	unsigned int h2 = (hash2>>f)%w;
	
	unsigned int k1 = (h1<<f)+g1;
	unsigned int k2 = (h2<<f)+g2;
	
	int* tmp1 = new int[r];
	int* tmp2 = new int[r];
	tmp1[0] = g1;
	tmp2[0] = g2;
	for(int i=1;i<r;i++)
	{
		tmp1[i]=(tmp1[i-1]*timer+prime)%bigger_p;
		tmp2[i]=(tmp2[i-1]*timer+prime)%bigger_p;
	}
	bool inserted=false;
	long key = g1+g2; 
	for(int i=0;i<p;i++)
	{
		key = (key*timer+prime)%bigger_p;
		int index = key%(r*r);
		int index1 = index/r;
		int index2 = index%r; 
		int p1 = (h1+tmp1[index1])%w;
		int p2 = (h2+tmp2[index2])%w;

		int pos = p1*w + p2;
		for (int j = 0; j < s; j++)
		{
			if ( ( ((value[pos].idx>>(j<<3))&((1<<8)-1)) == (index1|(index2<<4)) ) && (value[pos].src[j]== g1) && (value[pos].dst[j] == g2) )
			{
				value[pos].weight[j] += weight;
				inserted = true;
				break;
			}
			if (value[pos].src[j] == 0)
			{
				value[pos].idx |= ((index1 | (index2 << 4)) << (j<<3));
				value[pos].src[j] = g1;
				value[pos].dst[j] = g2;
				value[pos].weight[j] = weight;
				inserted = true;
				break;
			}
		}
		if(inserted)
			break;
	}
	if(!inserted)
	{
		map<unsigned int, int>::iterator it = index.find(k1);
		if(it!=index.end())
		{
			int tag = it->second;
			linknode<WEIGHT_TYPE>* node = buffer[tag];
			while(true)
			{
				if (node->key == k2)
				{   
					node->weight += weight;
					break;
				}
				if(node->next==NULL)
				{
					linknode<WEIGHT_TYPE>* ins = new linknode<WEIGHT_TYPE>;
					ins->key = k2;
					ins->weight = weight;
					ins->next = NULL;
					node->next = ins;
					edge_num++;
					break;
				}
				node = node->next;
			}
		}
		else if(buffer.size() < bufferMax || bufferMax == -1)
		{
			index[k1] = n;
			n++;
			linknode<WEIGHT_TYPE>* node = new linknode<WEIGHT_TYPE>;
			node->key = k1;
			node->weight = 0;
			if (k1 != k2)//k1==k2 means loop
			{
				linknode<WEIGHT_TYPE>* ins = new linknode<WEIGHT_TYPE>;
				ins->key = k2;
				ins->weight = weight;
				ins->next = NULL;
				node->next = ins;
				edge_num++;
			}
			else
			{ 
				node->weight += weight;
				node->next = NULL;
			}
			buffer.push_back(node); 
		}	
	}
	delete [] tmp1;
	delete [] tmp2;
	return;
}

template<typename VERTEX_ID_TYPE,typename WEIGHT_TYPE>
WEIGHT_TYPE GSS<VERTEX_ID_TYPE,WEIGHT_TYPE>::EdgeQuery(VERTEX_ID_TYPE s1, VERTEX_ID_TYPE s2)// s1 is the ID of the source node, s2 is the ID of the destination node, return the weight of the edge
{
    unsigned int hash1 = (*hfunc[0])((unsigned char*)(&s1), sizeof(s1) );
	unsigned int hash2 = (*hfunc[0])((unsigned char*)(&s2), sizeof(s2));
	int tmp = pow(2, f) - 1;
	unsigned short g1 = hash1 & tmp;
	if (g1 == 0) g1 += 1;
	unsigned int h1 = (hash1 >> f) % w;
	unsigned short g2 = hash2 & tmp;
	if (g2 == 0) g2 += 1;
	unsigned int h2 = (hash2 >> f) % w;
	int* tmp1 = new int[r];
	int* tmp2 = new int[r];
	tmp1[0] = g1;
	tmp2[0] = g2;
	for (int i = 1; i<r; i++)
	{
		tmp1[i] = (tmp1[i - 1] * timer + prime) % bigger_p;
		tmp2[i] = (tmp2[i - 1] * timer + prime) % bigger_p;
	}
	long key = g1 + g2;

	for (int i = 0; i<p; i++)
	{
		key = (key * timer + prime) % bigger_p;
		int index = key % (r*r);
		int index1 = index / r;
		int index2 = index%r;
		int p1 = (h1 + tmp1[index1]) % w;
		int p2 = (h2 + tmp2[index2]) % w;
		int pos = p1*w + p2;
		for (int j = 0; j<s; j++)
		{
		
			if ((((value[pos].idx >> (j << 3))&((1 << 8) - 1)) == (index1 | (index2 << 4))) && (value[pos].src[j] == g1) && (value[pos].dst[j] == g2))
			{
				delete []tmp1;
				delete []tmp2;
				return value[pos].weight[j];
			}
		}
	}
	unsigned int k1 = (h1 << f) + g1;
	unsigned int k2 = (h2 << f) + g2;
	map<unsigned int, int>::iterator it = index.find(k1);
	if (it != index.end())
	{
		int tag = it->second;
		linknode<WEIGHT_TYPE>* node = buffer[tag];
		while (node!=NULL)
		{
			if (node->key == k2)
			{
				delete []tmp1;
				delete []tmp2;
				return node->weight;
			}
			node = node->next;
		}
	}
	delete []tmp1;
	delete []tmp2;
	return 0;
}

/*type 0 is for successor query, type 1 is for precusor query*/

template<typename VERTEX_ID_TYPE,typename WEIGHT_TYPE>
WEIGHT_TYPE GSS<VERTEX_ID_TYPE,WEIGHT_TYPE>::NodeValueQuery(VERTEX_ID_TYPE s1, int type) // s1 is the ID of the queried node, function for node query.
{
	
	WEIGHT_TYPE weight = 0;
    unsigned int hash1 = (*hfunc[0])((unsigned char*)(&s1), sizeof(s1) );
	int tmp = pow(2, f) - 1;
	unsigned short g1 = hash1 & tmp;
	if (g1 == 0) g1 += 1;
	unsigned int h1 = (hash1 >> f) % w;
	int* tmp1 = new int[r];
	tmp1[0] = g1;
	for (int i = 1; i < r; i++)
	{
		tmp1[i] = (tmp1[i - 1] * timer + prime) % bigger_p;
	}
	for (int i = 0; i < r; i++)
	{
		int p1 = (h1 + tmp1[i]) % w;
		for (int k = 0; k < w; k++)
		{
			if (type == 0)/*successor query*/
			{
				int pos = p1*w + k;
				for (int j = 0; j < s; ++j)
				{
					if (type == 0 && (((value[pos].idx >> ((j << 3)))&((1 << 4) - 1)) == i) && (value[pos].src[j] == g1))
					{
                       
						weight += value[pos].weight[j];
					}
				}
			}
			else if (type == 1)/*precursor query*/
			{
				int pos = p1 + k*w;
				for (int j = 0; j < s; ++j)
				{
					if (type == 1 && (((value[pos].idx >> ((j << 3) + 4))&((1 << 4) - 1)) == i) && (value[pos].dst[j] == g1))
					{
						
						weight += value[pos].weight[j];
					}
				}
			}
		}
	}
	if (type == 0)
	{
		unsigned int k1 = (h1 << f) + g1;
		map<unsigned int, int>::iterator it = index.find(k1);
		if (it != index.end())
		{
			int tag = it->second;
			linknode<WEIGHT_TYPE>* node = buffer[tag];
			weight += node->weight;
			node = node->next;
			while (node != NULL)
			{	
				weight += node->weight;
				node = node->next;
			}
		}
	}
	else if (type==1)
	{
		unsigned int k1 = (h1 << f) + g1;
		map<unsigned int, int>::iterator it = index.find(k1);
		if(it!=index.end())
			weight += buffer[it->second]->weight;
		for (map<unsigned int, int>::iterator it = index.begin(); it != index.end(); ++it)
		{
			int tag = it->second;
			linknode<WEIGHT_TYPE>* node = buffer[tag];			
			node = node->next;
			while (node != NULL)
			{
				if(node->key == k1){
					weight += node->weight;
				}
				node = node->next;
			}
		}
	}
	delete []tmp1;
	return weight;
}

#endif