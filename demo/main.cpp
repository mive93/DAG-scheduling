#include <iostream>
#include "DAGTask.h"
#include "Taskset.h"
#include "tests.h"

#include <ctime>    

int main(int argc, char **argv){

    if(REPRODUCIBLE) srand (1);
    else srand (time(NULL));
    
    bool random_creation = false;

    int n_proc = 4;
    Taskset taskset;
    if(random_creation){
        int n_tasks = 6;
        int U_tot = 4;
        taskset.generate_taskset_Melani(n_tasks, U_tot, n_proc);
    }
    else
        taskset.readTasksetFromYaml("../demo/taskset.yaml");
    
    std::string dot_command = "";
    for(int i=0; i<taskset.tasks.size();++i){
        std::cout<<taskset.tasks[i]<<std::endl;
        taskset.tasks[i].saveAsDot("test"+std::to_string(i)+".dot");

        //convert dot to png to visualize it
        dot_command = "dot -Tpng test"+std::to_string(i)+".dot > test"+std::to_string(i)+".png";
        system(dot_command.c_str());
    }
    std::cout<<"Taskset utilization: "<<taskset.getUtilization()<<std::endl;


    //single DAG tests
    std::cout<<"Single task tests: \n";
    bool constrained_taskset = true;
    for(int i=0; i<taskset.tasks.size();++i){
        std::cout<<"\tTask "<<i<<std::endl;
        //constrained
        if(taskset.tasks[i].getDeadline() < taskset.tasks[i].getPeriod())
            std::cout<< "\t\tBaruah 2012 constrained (G-EDF): " <<G_EDF_Baruah2012_C(taskset.tasks[i], n_proc)<<std::endl;
        std::cout<< "\t\tBaruah 2012 arbitrary (G-EDF): "   <<G_EDF_Baruah2012_A(taskset.tasks[i], n_proc)<<std::endl;

        if(taskset.tasks[i].getDeadline() > taskset.tasks[i].getPeriod())
            constrained_taskset = false;
    }

    std::cout<<"Taskset tests: \n";

    //arbitrary
    std::cout<< "\tBonifaci 2013 arbitrary (G-EDF): "   <<G_EDF_Bonifaci2013_A(taskset, n_proc)<<std::endl;
    std::cout<< "\tBonifaci 2013 arbitrary (G-DM): "   <<G_DM_Bonifaci2013_A(taskset, n_proc)<<std::endl;

    //constrained
    if(constrained_taskset){
        std::cout<< "\tBonifaci 2013 constrained (G-DM): "   <<G_DM_Bonifaci2013_C(taskset, n_proc)<<std::endl;
    }

    //taskset tests

}