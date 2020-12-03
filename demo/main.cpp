#include <iostream>
#include "DAGTask.h"
#include "Taskset.h"

#include <ctime>    

int main(int argc, char **argv){

   // srand (time(NULL));
    srand (1);


    std::cout<<"This is a DAG!"<<std::endl;

    // DAGTask t;
    // t.generateTaskMelani();

    // // t.readTaskFromYaml("../demo/taskset.yaml");

    // std::cout<<t<<std::endl;

    Taskset taskset;
    taskset.generate_taskset_Melani(2,2,4);
    for(int i=0; i<taskset.tasks.size();++i)
        taskset.tasks[i].saveAsDot("test"+std::to_string(i)+".dot");

}