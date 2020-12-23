#include "tests.h"

//  Meiling Han et al. “Response time bounds for typed dag parallel tasks on heterogeneous multi-cores”. (IEEE Transactions on Parallel and DistributedSystems 2019)

bool GP_FP_Han2019_C_1(DAGTask task, const std::vector<int> m){
    //theorem 3.1

    task.computeTypedVolume();
    auto typed_vol = task.getTypedVolume();
    auto V = task.getVertices();

    // std::cout<<"L: "<<task.getLength()<<std::endl;

    for(int i=0; i<V.size();++i){
        if(V[i]->gamma > m.size())
            FatalError("Problem with types of core");
        V[i]->c *= (1. - 1. / m[V[i]->gamma]);
    }

    task.computeLength();

    float L_scaled = task.getLength();
    
    // std::cout<<"L_scaled: "<<L_scaled<<std::endl;

    float self_int = 0;
    for(int s=0; s<m.size(); ++s){
        if(m[s] != 0 && typed_vol.find(s) != typed_vol.end()){
            self_int += typed_vol[s] / m[s];
        }
    }

    // std::cout<<"R: "<<L_scaled + self_int<<std::endl;

    if( L_scaled + self_int < task.getDeadline())
        return true;

    return false;
}

// bool GP_FP_Han2019_C_2(DAGTask task, const std::vector<int> m){ } //TODO