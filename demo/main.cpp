#include <iostream>
#include "task.h"

int main(int argc, char **argv){

    std::cout<<"This is a DAG!"<<std::endl;

    DAGTask t;
    t.readTaskFromYaml("../demo/taskset.yaml");

    std::cout<<t<<std::endl;

    t.saveAsDot("test.dot");

}