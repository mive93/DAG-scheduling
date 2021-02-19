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
#include "GeneratorParams.h"
   
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

    std::vector<int> ordIDs;        // ids in topological order
    std::map<int, float> typedVol;  // volume for typed DAG [type of core, volume]
    std::map<int, float> pVol;      // volume for partitioned DAG [core id, volume]

    public:

    float R = 0;          // response time

    DAGTask(){};
    DAGTask(const float T, const float D): t(T), d(D) {};
    ~DAGTask(){};

    //input - output
    void readTaskFromYamlNode(YAML::Node tasks, const int i);
    void readTaskFromDOT(const std::string &filename);
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
    void computepVolume();
    void computeWorstCaseWorkload();
    void computeUtilization();
    void computeDensity();
    std::vector<std::vector<int>> computeAllPathsSingleSource(std::vector<int>& path, std::vector<std::vector<int>>& all_paths) const;
    std::vector<std::vector<int>> computeAllPaths() const;

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
    std::map<int, float> getpVolume() const {return pVol;};
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
    void setDeadline(const float deadline) { d = deadline; }
    void setPeriod(const float period) { t = period; }

    //Melani generation methods
    void assignWCET(const int minC, const int maxC);
    void expandTaskSeriesParallel(SubTask* source,SubTask* sink,const int depth,const int numBranches, const bool ifCond, GeneratorParams& gp);
    void makeItDag(float prob);
    void assignSchedParametersUUniFast(const float U);
    void assignSchedParameters(const float beta);
    void assignFixedSchedParameters(const float period, const float deadline);

};


bool compareDAGsDeadlineInc(const DAGTask& a, const DAGTask& b);
bool compareDAGsDeadlineDec(const DAGTask& a, const DAGTask& b);
bool compareDAGsPeriodInc(const DAGTask& a, const DAGTask& b);
bool compareDAGsPeriodDec(const DAGTask& a, const DAGTask& b);
bool compareDAGsUtilInc(const DAGTask& a, const DAGTask& b);
bool compareDAGsUtilDec(const DAGTask& a, const DAGTask& b);

#endif /* DAGTASK_H */