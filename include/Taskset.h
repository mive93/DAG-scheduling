#ifndef TASKSET_H
#define TASKSET_H

#include <vector>
#include <yaml-cpp/yaml.h>
#include "DAGTask.h"

class Taskset{

    float U = 0; //utilization

    public:

    std::vector<DAGTask> tasks;

    Taskset(){};
    ~Taskset(){};

    float getUtilization() const {return U;}
    

    void readTasksetFromYaml(const std::string& params_path){
        YAML::Node config   = YAML::LoadFile(params_path);
        YAML::Node tasks_node = config["tasks"];

        for(size_t i=0; i<tasks_node.size(); i++){
            DAGTask t;
            t.readTaskFromYamlNode(tasks_node, i);

            t.computeWorstCaseWorkload();
            t.computeVolume();
            t.computeLength();

            int Z = t.computeZk(4);
            std::cout<<"Z "<<Z<<std::endl;

            int mksp = t.computeMakespanUB(4);
            std::cout<<"mskpUB "<<mksp<<std::endl;

            U += float(t.getWCW()) / float(t.getPeriod());

            tasks.push_back(t);

        }
    }

    float UUniFast_Upart(float& sum_U, const int i, const int n_tasks, const DAGTask& t ){
        float next_sum_U=0, U_part = 0;
        double r;
        if(i < n_tasks-1){
            r = ((double) rand() / (RAND_MAX));
            next_sum_U = sum_U * std::pow(r, 1./(n_tasks-i-1));
            U_part = sum_U - next_sum_U;

            while(t.getLength() > std::ceil(t.getWCW() / U_part)){
                r = ((double) rand() / (RAND_MAX));
                next_sum_U = sum_U * std::pow(r, 1./(n_tasks-i-1));
                U_part = sum_U - next_sum_U;
            }
            sum_U = next_sum_U;
        }
        else
            U_part = sum_U;

        return U_part;
    }

    void generate_taskset_Melani(int n_tasks, const float U_tot, const int n_proc){

        U = 0;
        float U_part = 0;
        float sum_U = U_tot;

        for(int i=0; i<n_tasks; ++i){
            DAGTask t;
            t.configureParams();
            t.expandTaskSeriesParallel(nullptr, nullptr,t.rec_depth,0,false);
            t.assignWCET(t.Cmin, t.Cmax);
            t.makeItDag(t.addProb);

            t.computeWorstCaseWorkload();
            t.computeVolume();
            t.computeLength();

            U_part = UUniFast_Upart(sum_U, i, n_tasks, t) ;

            std::cout<<"U_part "<<U_part<<std::endl;

            t.assignSchedParametersUUniFast(U_part);
            U += float(t.getWCW()) / float(t.getPeriod());

            int Z = t.computeZk(n_proc);
            std::cout<<"Z "<<Z<<std::endl;

            int mksp = t.computeMakespanUB(n_proc);
            std::cout<<"mskpUB "<<mksp<<std::endl;
            tasks.push_back(t);
        }
    }
};


#endif /* TASKSET_H */