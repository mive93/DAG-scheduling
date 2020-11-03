#include <iostream>
#include "DAGTask.h"

#include <ctime>    

int main(int argc, char **argv){

    // srand (time(NULL));
    srand (1);


    std::cout<<"This is a DAG!"<<std::endl;

    DAGTask t;
    t.generateTaskMelani();

    // t.readTaskFromYaml("../demo/taskset.yaml");

    std::cout<<t<<std::endl;

    t.saveAsDot("test.dot");

}