#ifndef TASK_H
#define TASK_H

#include <vector>
#include <map>

#include <yaml-cpp/yaml.h>

class SubTask{

    public:

    int id = 0;
    int c = 0; // WCET
    bool isCond = false; 
    std::vector<SubTask*> succ;
    std::vector<SubTask*> prec;

    int accWork = 0; // accumulated workload

    
};

class DAGTask{

    int t = 0;  //period
    int d = 0;  //deadline
    std::vector<SubTask> V;

    int L = 0; // longest chain
    int vol  = 0; //volume


    public:
    DAGTask(){};
    DAGTask(const int T, const int D): t(T), d(D) {};

    ~DAGTask(){};
    void readTaskFromYaml(const std::string& params_path);


    void saveAsDot(const std::string &filename);

    friend std::ostream& operator<<(std::ostream& os, const DAGTask& t);

};

#endif /* TASK_h */