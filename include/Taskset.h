#ifndef TASKSET_H
#define TASKSET_H

#include <vector>

#include "DAGTask.h"

class Taskset{


    public:

    std::vector<DAGTask> tasks;

    Taskset(){};
    ~Taskset(){};
    

    float UUniFast_Upart(float& sum_U, const int i, const int n_tasks, const DAGTask& t ){
        float next_sum_U=0, U_part = 0;
        double r;
        if(i < n_tasks-1){
            r = ((double) rand() / (RAND_MAX));
            next_sum_U = sum_U * std::pow(r, 1./(n_tasks-i-1));
            U_part = sum_U - next_sum_U;

            while(t.getLenght() > std::ceil(t.getWCW() / U_part)){
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

        float U = 0, U_part = 0;
        float sum_U = U_tot;

        for(int i=0; i<n_tasks; ++i){
            DAGTask t;
            t.generateTaskMelani();

            U_part = UUniFast_Upart(sum_U, i, n_tasks, t) ;

            std::cout<<"U_part "<<U_part<<std::endl;

            t.assignSchedParametersUUniFast(U_part);
            U += float(t.getWCW()) / float(t.getPeriod());

            std::cout<<"U "<<U<<std::endl;


            int Z = t.computeZk(n_proc);
            std::cout<<"Z "<<Z<<std::endl;

            int mksp = t.computeMakespanUB(n_proc);
            std::cout<<"mskpUB "<<mksp<<std::endl;
            tasks.push_back(t);
        }
    }
};


#endif /* TASKSET_H */