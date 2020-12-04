#include <iostream>
#include "DAGTask.h"
#include "Taskset.h"

#include <ctime>    

int main(int argc, char **argv){

    if(REPRODUCIBLE) srand (1);
    else srand (time(NULL));
    
    bool random_creation = true;

    int n_proc = 4;
    Taskset taskset;
    if(random_creation){
        int n_tasks = 6;
        int U_tot = 4;
        taskset.generate_taskset_Melani(n_tasks, U_tot, n_proc);
    }
    else
        taskset.readTasksetFromYaml("../demo/taskset.yaml");
    
    for(int i=0; i<taskset.tasks.size();++i){
        std::cout<<taskset.tasks[i]<<std::endl;
        taskset.tasks[i].saveAsDot("test"+std::to_string(i)+".dot");
    }
    std::cout<<"Taskset utilization: "<<taskset.getUtilization()<<std::endl;

}