#include "Taskset.h"


void Taskset::computeUtilization(){
    for(const auto& task: tasks){
        U += task.getUtilization();
    }
}

void Taskset::computeHyperPeriod(){
    for(const auto& task: tasks)
        HP =  std::lcm((int) task.getPeriod(), HP);
}

void Taskset::computeMaxDensity(){
    for(const auto& task: tasks)
        if(task.getDensity() > maxDelta)
            maxDelta = task.getDensity();
}

void Taskset::readTasksetFromYaml(const std::string& params_path){
    YAML::Node config   = YAML::LoadFile(params_path);
    YAML::Node tasks_node = config["tasks"];

    for(size_t i=0; i<tasks_node.size(); i++){
        DAGTask t;
        t.readTaskFromYamlNode(tasks_node, i);

        t.transitiveReduction();

        t.computeWorstCaseWorkload();
        t.computeVolume();
        t.computeLength();
        t.computeUtilization();
        t.computeDensity();

        tasks.push_back(t);
    }

    computeUtilization();
    computeHyperPeriod();
    computeMaxDensity();
}

float Taskset::UUniFast_Upart(float& sum_U, const int i, const int n_tasks, const DAGTask& t ){
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

void Taskset::generate_taskset_Melani(int n_tasks, const float U_tot, const int n_proc, GeneratorParams& gp){

    float U_part = 0;
    float sum_U = U_tot;

    for(int i=0; i<n_tasks; ++i){
        DAGTask t;
        t.expandTaskSeriesParallel(nullptr, nullptr,gp.recDepth,0,false,gp);
        t.assignWCET(gp.Cmin, gp.Cmax);
        t.makeItDag(gp.addProb);

        t.transitiveReduction();

        t.computeWorstCaseWorkload();
        t.computeVolume();
        t.computeLength();
        t.computeUtilization();
        t.computeDensity();

        U_part = UUniFast_Upart(sum_U, i, n_tasks, t) ;

        std::cout<<"U_part "<<U_part<<std::endl;

        t.assignSchedParametersUUniFast(U_part);

        tasks.push_back(t);
    }

    computeUtilization();
    computeHyperPeriod();
    computeMaxDensity();
}