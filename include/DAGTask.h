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

    //distribution to add branches (MelaniGen)
    std::discrete_distribution<int> dist;
    std::vector<double> weights;
    std::mt19937 gen;       

    public:
    DAGTask(){};
    DAGTask(const int T, const int D): t(T), d(D) {};
    ~DAGTask(){};

    //input - output
    void readTaskFromYaml(const std::string& params_path); //TODO fixme!! id and index should always match
    void saveAsDot(const std::string &filename);
    friend std::ostream& operator<<(std::ostream& os, const DAGTask& t);

    //operation on DAG
    void isSuccessor(SubTask* v, SubTask *w, bool &is_succ);
    bool allPrecAdded(std::vector<SubTask*> prec, std::vector<int> ids);
    std::vector<int> topologicalSort();
    bool checkIndexAndIdsAreEqual();
    

    //Melani generation methods
    void configureParams();
    void assignWCET(const int minC, const int maxC);
    void expandTaskSeriesParallel(SubTask* source,SubTask* sink,const int depth,const int numBranches, const bool ifCond);
    void makeItDag(float prob);
    void computeAccWorkload();
    DAGTask generateTaskMelani();

};

#endif /* DAGTASK_H */