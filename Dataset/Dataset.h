#ifndef _CAIDADATASET_H
#define _CAIDADATASET_H

//#define SRC_VERTEX

#include <cstdint>
#include <string>
#include <vector>
#include <fstream>
#include <assert.h>
#include <iostream>
#include "Param.h"

struct __attribute__((packed)) IpSet {
    uint32_t src_ip;
    uint16_t src_port;
    uint32_t dst_ip;
    uint16_t dst_port;
    uint8_t protocol;
    #ifdef WEIGHT
    uint32_t weight;
    #endif
};

struct __attribute__((packed)) Tuple {
    uint32_t u;
    uint32_t v;
    uint32_t weight = 1;

};

class Dataset {
    public:
    
        Dataset(const std::string& PATH, uint32_t _len) {
            std::ifstream ofs;
            ofs.open(PATH, std::ios::binary);
            for(int i = 0; _len == -1 || i < _len ; i++){
                Tuple tuple;
                IpSet ipset;
                ofs.read((char *) &ipset, sizeof(IpSet));
                std::streamsize bytes_read = ofs.gcount();
                assert(bytes_read==sizeof(IpSet) || bytes_read == 0);
                if(bytes_read==0){
                    break;
                }
                #ifdef SRC_VERTEX
                tuple.u = ipset.src_ip;
                tuple.v = ipset.dst_ip;
                #else
                tuple.v = ipset.src_ip;
                tuple.u = ipset.dst_ip;
                #endif
                #ifdef WEIGHT
                tuple.weight = ipset.weight;
                #endif
                dataset.push_back(tuple);
            }
            ofs.close();
        }
        ~Dataset() {}

    public:
    std::vector<Tuple> dataset;
};

#endif