## HeavyGraph
### Introduction
The main contributions of HeavyGraph are threefold:
(1) we use a two-stage framework that integrates a general
filter with a specific heavy detector for identifying various heavy
patterns, such as heavy hitters and Top-k patterns for nodes
and edges; (2) the general filter employs an asymmetric two-
dimensional compact matrix with fingerprints, achieving high
memory utilization; (3) the specific heavy detector applies a local
in-bucket descending update that incrementally maintains the
frequent nodes and their key edges, improving both accuracy
and throughput. Extensive experiments across a series of datasets
demonstrate that HeavyGraph significantly outperforms the
strawman solution in both error and throughput performance,
achieving improvements of up to two orders of magnitude.

### Datasets
Our code runs on the following dataset:

-[Caida](./Data/caida.dat)

-[Caida_Weight](./Data/caida_weight.dat)

-[Campus](./Data/campus.dat)

-[Campus_Weight](./Data/campus_weight.dat)

### How to Run
Suppose you have already navigated into the project directory.
```sh
cmake ./
make
./main
```