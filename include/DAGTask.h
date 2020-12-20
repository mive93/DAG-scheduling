#ifndef DAGTASK_H
#define DAGTASK_H

#include <vector>
#include <map>
#include <cmath>
#include <cstdlib>
#include <random>
#include <algorithm>
#include <iostream>
#include <fstream>

#include <yaml-cpp/yaml.h>
#include "SubTask.h"
#include "utils.h"
   
enum creationStates {CONDITIONAL_T=0, PARALLEL_T=1, TERMINAL_T=2};

class DAGTask{

    std::vector<SubTask*> V;

    float t = 0;          // period
    float d = 0;          // deadline
    float L = 0;          // longest chain
    float vol  = 0;       // volume
    float wcw = 0;        // worst case workload (= to volume if no conditional branch exist)
    float delta = 0;      // density
    float u = 0;          // utilization
    

    //distribution to add branches (MelaniGen)
    std::discrete_distribution<int> dist;
    std::vector<int> ordIDs; // ids in topological order
    std::vector<double> weights;
    std::mt19937 gen;       

    public:

    float R = 0;          // response time

    int maxCondBranches     = 2; //max conditional branches allowed (MelaniGen)
    int maxParBranches      = 6; //max parallel branches allowed (MelaniGen)
    float p_cond            = 0; //probability of generating a conditional branch (MelaniGen)
    float p_par             = 0.2; //probability of generating a parallel branch (MelaniGen)
    float p_term            = 0.8; //probability of generating a terminal vertex (MelaniGen)
    int rec_depth           = 2; // maximum recursion depth for the generation of the task graphs (MelaniGen)
    float Cmin                = 1; // minimum WCET for subtasks (MelaniGen)
    float Cmax                = 100; //maximum WCET for subtasks (MelaniGen)
    float addProb           = 0.1; //probability to add an edge between 2 nodes, if possible (MelaniGen)
    float probSCond         = 0.5; //probability that the source is conditional (MelaniGen)

    DAGTask(){};
    DAGTask(const float T, const float D): t(T), d(D) {};
    ~DAGTask(){};

    //input - output
    void readTaskFromYamlNode(YAML::Node tasks, const int i);
    void saveAsDot(const std::string &filename);
    friend std::ostream& operator<<(std::ostream& os, const DAGTask& t);

    //operation on DAG
    void cloneVertices(const std::vector<SubTask*>& to_clone_V);
    void destroyVerices();
    void isSuccessor(SubTask* v, SubTask *w, bool &is_succ) const;
    bool allPrecAdded(std::vector<SubTask*> pred, std::vector<int> ids);
    void topologicalSort();
    bool checkIndexAndIdsAreEqual();
    void computeAccWorkload();
    void computeLength();
    void computeVolume();
    void computeUtilization();
    void computeDensity();

    void localDeadline(SubTask *task, const int i);
    void localOffset(SubTask *task, const int i);
    void computeLocalOffsets();
    void computeLocalDeadlines();
    void computeEFTs();
    void computeLSTs();

    float getLength() const {return L;};
    float getVolume() const {return vol;};
    float getWorstCaseWorkload() const {return wcw;};
    float getWCW() const {return wcw;};
    float getPeriod() const {return t;};
    float getDeadline() const {return d;};
    float getUtilization() const {return u;};
    float getDensity() const {return delta;};
    std::vector<int> getTopologicalOrder() const {return ordIDs;};
    std::vector<SubTask*> getVertices() const {return V;};

    //Melani generation methods
    void configureParams();
    void assignWCET(const int minC, const int maxC);
    void expandTaskSeriesParallel(SubTask* source,SubTask* sink,const int depth,const int numBranches, const bool ifCond);
    void makeItDag(float prob);
    void computeWorstCaseWorkload();
    void assignSchedParametersUUniFast(const float U);

};

#endif /* DAGTASK_H */