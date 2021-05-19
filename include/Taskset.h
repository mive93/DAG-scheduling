#ifndef TASKSET_H
#define TASKSET_H

#include <vector>
#include <yaml-cpp/yaml.h>
#include <numeric>
#include "DAGTask.h"

class Taskset{

    float U         = 0;    // utilization
    float maxDelta  = 0;    // maximum density
    long int HP          = 1;    // hyper-period

    public:

    std::vector<DAGTask> tasks;

    Taskset(){};
    ~Taskset(){};

    //getters
    float getUtilization() const {return U;}
    int getHyperPeriod() const {return HP;}
    float getMaxDensity() const {return maxDelta;}

    //compute on taskset
    void computeUtilization();
    void computeHyperPeriod();
    void computeMaxDensity();

    //IO
    void readTasksetFromYaml(const std::string& params_path);
    void readTasksetFromDOT(const std::string& dot_file_path);
    void print() const;

    //generate taskset 
    float UUniFast_Upart(float& sum_U, const int i, const int n_tasks, const DAGTask& t );
    void generate_taskset_Melani(int n_tasks, const float U_tot, const int n_proc, GeneratorParams& gp);
};


#endif /* TASKSET_H */