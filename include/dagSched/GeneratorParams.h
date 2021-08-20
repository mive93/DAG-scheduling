
#ifndef GENERATORPARAMS_H
#define GENERATORPARAMS_H

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <random>
#include <yaml-cpp/yaml.h>

#include "dagSched/utils.h"

namespace dagSched{

enum GenerationType_t {VARYING_N, VARYING_U, VARYING_M};
enum DeadlinesType_t {CONSTRAINED, IMPLICIT, ARBITRARY};
enum SchedulingType_t {GLOBAL, PARTITIONED, FEDERATED, SOTA};
enum AlgorithmType_t {FTP, EDF};
enum workloadType_t {SINGLE_DAG, TASKSET};
enum DAGType_t {DAG, CDAG, TDAG};

class GeneratorParams{

    public:

    // Hyperparameters for the generation of DAG and conditional DAGs proposed by Melani et al. (MelaniGen)
    int maxCondBranches     = 2;    // max conditional branches allowed 
    int maxParBranches      = 6;    // max parallel branches allowed 
    int recDepth            = 2;    // maximum recursion depth for the generation of the task graphs 

    float pCond             = 0;    // probability of generating a conditional branch 
    float pPar              = 0.2;  // probability of generating a parallel branch 
    float pTerm             = 0.8;  // probability of generating a terminal vertex 
    float Cmin              = 1;    // minimum WCET for subtasks 
    float Cmax              = 100;  // maximum WCET for subtasks 
    float addProb           = 0.1;  // probability to add an edge between 2 nodes, if possible 
    float probSCond         = 0.5;  // probability that the source is conditional 
    float Utot              = 1;
    float beta              = 0.1;

    int saveRate            = 25;

    float Umin              = 0;
    float Umax              = 8;
    float stepU             = 0.25;
    int m                   = 8;
    
    int nMin                = 11;
    int nMax                = 20;
    int nTasks              = 0;
    int stepN               = 1;

    int mMin                = 2;
    int mMax                = 30;
    int stepM               = 1;

    int tasksetPerVarFactor = 1;
    int nTasksets           = 1;

    int diffProcTypes       = 1;
    int minProcPerType      = 1;
    int maxProcPerType      = 1;

    std::vector<int> typedProc;

    std::vector<std::string> methods;
    std::vector<std::string> names;
    std::vector<int> coresOrders;
    std::vector<int> nodesOrders;
    std::vector<int> iterative;

    GenerationType_t gType  = GenerationType_t::VARYING_N;
    DeadlinesType_t dtype   = DeadlinesType_t::CONSTRAINED;
    SchedulingType_t sType  = SchedulingType_t::GLOBAL;
    AlgorithmType_t aType   = AlgorithmType_t::FTP;
    workloadType_t wType    = workloadType_t::TASKSET;
    DAGType_t DAGType       = DAGType_t::DAG;  

    //distribution to add branches 
    std::discrete_distribution<int> dist;
    std::vector<double> weights;
    std::mt19937 gen;     

    void configureParams(GenerationType_t gt){
        gType = gt;

        weights.push_back(pCond);
        weights.push_back(pPar);
        weights.push_back(pTerm);
        dist.param(std::discrete_distribution<int> ::param_type(std::begin(weights), std::end(weights)));

        switch (gt){
        case VARYING_N:
            nTasksets = tasksetPerVarFactor * (nMax - nMin + 1);
            break;
        case VARYING_U:
            nTasksets = tasksetPerVarFactor * ((Umax - Umin) / stepU);
            break;
        case VARYING_M:
            mMin = std::floor(Utot);
            nTasksets = tasksetPerVarFactor * (mMax - mMin + 1);
            break;
        }

        if(DAGType != DAGType_t::CDAG){
            pCond = 0;
            probSCond = 0;
        }

        if(DAGType != DAGType_t::TDAG){
            typedProc.resize(diffProcTypes);
            for(int p=0; p<typedProc.size(); ++p)
                typedProc[p] = rand () % maxProcPerType + minProcPerType;
        }

        if(wType == workloadType_t::SINGLE_DAG)
            nTasks = 1;

        if(names.size() != methods.size())
            FatalError("The number of names should be the same of number of methods");

        if(!iterative.empty() && iterative.size() != methods.size())
            FatalError("If iterative is not empty, it should have the same size as methods");
        
        if(REPRODUCIBLE) gen.seed(1);
        else gen.seed(time(0));
    }

    void readFromYaml(const std::string& params_path){
        YAML::Node config   = YAML::LoadFile(params_path);
        
        if(config["maxCondBranches"]) maxCondBranches = config["maxCondBranches"].as<int>();
        if(config["maxParBranches"]) maxParBranches = config["maxParBranches"].as<int>();
        if(config["recDepth"]) recDepth = config["recDepth"].as<int>();
        if(config["pCond"]) pCond = config["pCond"].as<float>();
        if(config["pPar"]) pPar = config["pPar"].as<float>();
        if(config["pTerm"]) pTerm = config["pTerm"].as<float>();
        if(config["Cmin"]) Cmin = config["Cmin"].as<float>();
        if(config["Cmax"]) Cmax = config["Cmax"].as<float>();
        if(config["addProb"]) addProb = config["addProb"].as<float>();
        if(config["probSCond"]) probSCond = config["probSCond"].as<float>();
        if(config["Utot"]) Utot = config["Utot"].as<float>();
        if(config["beta"]) beta = config["beta"].as<float>();
        if(config["saveRate"]) saveRate = config["saveRate"].as<int>();
        if(config["Umin"]) Umin = config["Umin"].as<float>();
        if(config["Umax"]) Umax = config["Umax"].as<float>();
        if(config["stepU"]) stepU = config["stepU"].as<float>();
        if(config["m"]) m = config["m"].as<int>();
        if(config["nMin"]) nMin = config["nMin"].as<int>();
        if(config["nMax"]) nMax = config["nMax"].as<int>();
        if(config["nTasks"]) nTasks = config["nTasks"].as<int>();
        if(config["stepN"]) stepN = config["stepN"].as<int>();
        if(config["mMin"]) mMin = config["mMin"].as<int>();
        if(config["mMax"]) mMax = config["mMax"].as<int>();
        if(config["stepM"]) stepM = config["stepM"].as<int>();
        if(config["tasksetPerVarFactor"]) tasksetPerVarFactor = config["tasksetPerVarFactor"].as<int>();
        if(config["nTasksets"]) nTasksets = config["nTasksets"].as<int>();
        if(config["diffProcTypes"]) diffProcTypes = config["diffProcTypes"].as<int>();
        if(config["minProcPerType"]) minProcPerType = config["minProcPerType"].as<int>();
        if(config["maxProcPerType"]) maxProcPerType = config["maxProcPerType"].as<int>();
        if(config["gType"]) gType = (GenerationType_t)  config["gType"].as<int>(); 
        if(config["dtype"]) dtype = (DeadlinesType_t) config["dtype"].as<int>();
        if(config["sType"]) sType = (SchedulingType_t) config["sType"].as<int>();
        if(config["aType"]) aType = (AlgorithmType_t) config["aType"].as<int>();
        if(config["wType"]) wType = (workloadType_t) config["wType"].as<int>();
        if(config["DAGType"]) DAGType = (DAGType_t) config["DAGType"].as<int>();
        if(config["methods"]) methods = config["methods"].as<std::vector<std::string>>();
        if(config["names"]) names = config["names"].as<std::vector<std::string>>();
        if(config["core_orders"]) coresOrders = config["core_orders"].as<std::vector<int>>();
        if(config["node_orders"]) nodesOrders = config["node_orders"].as<std::vector<int>>();
        if(config["iterative"]) iterative = config["iterative"].as<std::vector<int>>();

    }

    void print(){
        std::cout<<"maxCondBranches: "<<maxCondBranches<<std::endl;
        std::cout<<"maxParBranches: "<<maxParBranches<<std::endl;
        std::cout<<"recDepth: "<<recDepth<<std::endl;
        std::cout<<"pCond: "<<pCond<<std::endl;
        std::cout<<"pPar: "<<pPar<<std::endl;
        std::cout<<"pTerm: "<<pTerm<<std::endl;
        std::cout<<"Cmin: "<<Cmin<<std::endl;
        std::cout<<"Cmax: "<<Cmax<<std::endl;
        std::cout<<"addProb: "<<addProb<<std::endl;
        std::cout<<"probSCond: "<<probSCond<<std::endl;
        std::cout<<"Utot: "<<Utot<<std::endl;
        std::cout<<"beta: "<<beta<<std::endl;
        std::cout<<"saveRate: "<<saveRate<<std::endl;
        std::cout<<"Umin: "<<Umin<<std::endl;
        std::cout<<"Umax: "<<Umax<<std::endl;
        std::cout<<"stepU: "<<stepU<<std::endl;
        std::cout<<"m: "<<m<<std::endl;
        std::cout<<"nMin: "<<nMin<<std::endl;
        std::cout<<"nMax: "<<nMax<<std::endl;
        std::cout<<"nTasks: "<<nTasks<<std::endl;
        std::cout<<"stepN: "<<stepN<<std::endl;
        std::cout<<"mMin: "<<mMin<<std::endl;
        std::cout<<"mMax: "<<mMax<<std::endl;
        std::cout<<"stepM: "<<stepM<<std::endl;
        std::cout<<"tasksetPerVarFactor: "<<tasksetPerVarFactor<<std::endl;
        std::cout<<"nTasksets: "<<nTasksets<<std::endl;
        std::cout<<"diffProcTypes: "<<diffProcTypes<<std::endl;
        std::cout<<"minProcPerType: "<<minProcPerType<<std::endl;
        std::cout<<"maxProcPerType: "<<maxProcPerType<<std::endl;
        std::cout<<"gType: "<<gType<<std::endl;
        std::cout<<"dtype: "<<dtype<<std::endl;
        std::cout<<"sType: "<<sType<<std::endl;
        std::cout<<"aType: "<<aType<<std::endl;
        std::cout<<"wType: "<<wType<<std::endl;
        std::cout<<"DAGType: "<<DAGType<<std::endl;
        std::cout<<"methods: "; for(const auto& m:methods) std::cout<<m<<", "; std::cout<<std::endl;
        std::cout<<"names: "; for(const auto& n:names) std::cout<<n<<", "; std::cout<<std::endl;
        std::cout<<"coreOrder: "; for(const auto& c:coresOrders) std::cout<<c<<", "; std::cout<<std::endl;
        std::cout<<"nodeOrder: "; for(const auto& n:nodesOrders) std::cout<<n<<", "; std::cout<<std::endl;
        std::cout<<"iterative: "; for(const auto& i:iterative) std::cout<<i<<", "; std::cout<<std::endl;
    }

};

}

#endif /* GENERATORPARAMS_H */