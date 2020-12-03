#include <iostream>
#include "DAGTask.h"
#include "Taskset.h"

#include <ctime>    

int main(int argc, char **argv){

   // srand (time(NULL));
    srand (1);


    // std::cout<<"This is a DAG!"<<std::endl;

    // DAGTask t;
    // t.generateTaskMelani();

    // // t.readTaskFromYaml("../demo/taskset.yaml");

    // std::cout<<t<<std::endl;

    int n_tasks = 6;
    int U_tot = 4;
    int n_proc = 4;

    Taskset taskset;
    taskset.generate_taskset_Melani(n_tasks, U_tot, n_proc);
    for(int i=0; i<taskset.tasks.size();++i){
        std::cout<<taskset.tasks[i]<<std::endl;
        taskset.tasks[i].saveAsDot("test"+std::to_string(i)+".dot");
    }

}