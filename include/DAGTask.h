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

class GeneratorParams{

    public:

    // Hyperparameters for the generation of DAG and conditional DAGs proposed by Melani et al. (MelaniGen)
    int maxCondBranches     = 2;    // max conditional branches allowed 
    int maxParBranches      = 6;    // max parallel branches allowed 
    int recDepth            = 2;    // maximum recursion depth for the generation of the task graphs 

    float pCond             = 0;    // probability of generating a conditional branch 
    float pPar              = 0.2;  // probability of generating a parallel branch 
    float pTerm             = 0.8;  // probability of generating a terminal vertex 
    float Cmin              = 1;    // minimum WCET for subtasks 
    float Cmax              = 10;  // maximum WCET for subtasks 
    float addProb           = 0.1;  // probability to add an edge between 2 nodes, if possible 
    float probSCond         = 0.5;  // probability that the source is conditional 

    //distribution to add branches 
    std::discrete_distribution<int> dist;
    std::vector<double> weights;
    std::mt19937 gen;     

    void configureParams();  

};

class DAGTask{

    std::vector<SubTask*> V;

    float t = 0;          // period
    float d = 0;          // deadline
    float L = 0;          // longest chain
    float vol  = 0;       // volume
    float wcw = 0;        // worst case workload (= to volume if no conditional branch exist)
    float delta = 0;      // density
    float u = 0;          // utilization

    std::vector<int> ordIDs;        // ids in topological order
    std::map<int, float> typedVol;  // volume for typed DAG [type of core, volume]

    public:

    float R = 0;          // response time

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
    void isSuccessor(const SubTask* v, const SubTask *w, bool &is_succ) const;
    bool allPrecAdded(std::vector<SubTask*> pred, std::vector<int> ids);
    void topologicalSort();
    bool checkIndexAndIdsAreEqual();
    void computeAccWorkload();
    void computeLength();
    void computeVolume();
    void computeTypedVolume();
    void computeWorstCaseWorkload();
    void computeUtilization();
    void computeDensity();

    // EST, EFT, LST, LFT
    void localDeadline(SubTask *task, const int i);
    void localOffset(SubTask *task, const int i);
    void computeLocalOffsets();
    void computeLocalDeadlines();
    void computeEFTs();
    void computeLSTs();

    std::vector<SubTask*> getSubTaskAncestors(const int i) const;
    std::vector<SubTask*> getSubTaskDescendants(const int i) const;
    void transitiveReduction();

    //getters
    float getLength() const {return L;};
    float getVolume() const {return vol;};
    std::map<int, float> getTypedVolume() const {return typedVol;};
    float getWorstCaseWorkload() const {return wcw;};
    float getWCW() const {return wcw;};
    float getPeriod() const {return t;};
    float getDeadline() const {return d;};
    float getUtilization() const {return u;};
    float getDensity() const {return delta;};
    std::vector<int> getTopologicalOrder() const {return ordIDs;};
    std::vector<SubTask*> getVertices() const {return V;};

    //setters
    void setVertices(std::vector<SubTask*> given_V){ V.clear(); V = given_V; }

    //Melani generation methods
    void assignWCET(const int minC, const int maxC);
    void expandTaskSeriesParallel(SubTask* source,SubTask* sink,const int depth,const int numBranches, const bool ifCond, GeneratorParams& gp);
    void makeItDag(float prob);
    void assignSchedParametersUUniFast(const float U);

};

#endif /* DAGTASK_H */