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

    int t = 0;      //period
    int d = 0;      //deadline
    int L = 0;      // longest chain
    int vol  = 0;   //volume
    int wcw = 0;    // worst case workload (= to volume if no conditional branch exist)

    
    //distribution to add branches (MelaniGen)
    std::discrete_distribution<int> dist;
    std::vector<int> ordIDs; // ids in topological order
    std::vector<double> weights;
    std::mt19937 gen;       

    public:

    int maxCondBranches     = 2; //max conditional branches allowed (MelaniGen)
    int maxParBranches      = 6; //max parallel branches allowed (MelaniGen)
    float p_cond            = 0; //probability of generating a conditional branch (MelaniGen)
    float p_par             = 0.2; //probability of generating a parallel branch (MelaniGen)
    float p_term            = 0.8; //probability of generating a terminal vertex (MelaniGen)
    int rec_depth           = 2; // maximum recursion depth for the generation of the task graphs (MelaniGen)
    int Cmin                = 1; // minimum WCET for subtasks (MelaniGen)
    int Cmax                = 100; //maximum WCET for subtasks (MelaniGen)
    float addProb           = 0.1; //probability to add an edge between 2 nodes, if possible (MelaniGen)
    float probSCond         = 0.5; //probability that the source is conditional (MelaniGen)

    DAGTask(){};
    DAGTask(const int T, const int D): t(T), d(D) {};
    ~DAGTask(){};

    //input - output
    void readTaskFromYamlNode(YAML::Node tasks, const int i);
    void saveAsDot(const std::string &filename);
    friend std::ostream& operator<<(std::ostream& os, const DAGTask& t);

    //operation on DAG
    void isSuccessor(SubTask* v, SubTask *w, bool &is_succ);
    bool allPrecAdded(std::vector<SubTask*> prec, std::vector<int> ids);
    void topologicalSort();
    bool checkIndexAndIdsAreEqual();
    void computeAccWorkload();
    void computeLength();
    void computeVolume();
    int getLenght() const {return L;};
    int getVolume() const {return vol;};
    int getWCW() const {return wcw;};
    int getPeriod() const {return t;};
    int getDeadline() const {return d;};
    

    //Melani generation methods
    void configureParams();
    void assignWCET(const int minC, const int maxC);
    void expandTaskSeriesParallel(SubTask* source,SubTask* sink,const int depth,const int numBranches, const bool ifCond);
    void makeItDag(float prob);
    void computeWorstCaseWorkload();
    int computeZk(const int n_proc);
    void maximizeMakespan(const SubTask* v, const std::vector<int>& mksp, const std::vector<std::set<int>>& mksp_set,  const std::vector<std::set<int>>& w_set,  std::set<int>& mkspset_tmp, float& max_mksp, const int n_proc);
    int computeMakespanUB(const int n_proc);
    void assignSchedParametersUUniFast(const float U);

};

#endif /* DAGTASK_H */