#include "tests.h"

#ifdef ZAHAF2019

#include "task/task.hpp"
#include "gramm/hdag_driver.hpp"
#include "code_gen/taskset_code.hpp"
#include "platform/platform.hpp"
#include "analysis/hpc_dag/analysis.hpp"

bool P_LP_EDF_Zahaf2019_C(const Taskset& taskset, const int m){

    task::Taskset ts;

    for(int x=0; x<taskset.tasks.size(); ++x){    
        std::vector<SubTask*> V = taskset.tasks[x].getVertices();
        task::Task *tau = new task::Task(x);
        tau->_D(taskset.tasks[x].getDeadline());
        tau->_T(taskset.tasks[x].getPeriod());
        for(int i=0;i<V.size();++i)
            task::Subtask *v1 = new task::Subtask(i, V[i]->c, COMPUTE, CPU, tau);

        auto *subtask = tau->_subtasks();

        for(int i=0;i<V.size();++i)
            for(int j=0; j<V[i]->succ.size(); ++j)
                tau->link_two_subtasks((*subtask)[i],(*subtask)[V[i]->succ[j]->id]);

        tau->to_dot("zahaf_"+std::to_string(x)+".dot");
        std::string dot_command = "dot -Tpng zahaf_"+std::to_string(x)+".dot > zahaf_"+std::to_string(x)+".png";
        system(dot_command.c_str());

        ts.add(tau);
    }

    
    platform::Processor p(0, CPU, EDFFP);
    platform::Platform pl(m * p);

    // std::cout<<"Analysis Zahaf started"<<std::endl;
    bool res = analysis::analyse(&pl, &ts, PARTIAL, FAIR, WF,  RANDOM);
    // std::cout<<"Analysis Zahaf concluded"<<std::endl;

    //delete everything

    auto tasks = *ts._list();
    for(int x=0; x<tasks.size(); ++x){
        auto subtask = *tasks[x]->_subtasks();
        for(int i=0; i<subtask.size(); ++i)
            delete subtask[i];

        delete tasks[x];
    }

    return res;
}

#endif