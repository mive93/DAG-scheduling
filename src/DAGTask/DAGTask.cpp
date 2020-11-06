#include "DAGTask.h"

void DAGTask::isSuccessor(SubTask* v, SubTask *w, bool &is_succ){

    for(size_t i=0; i<w->succ.size(); ++i){
        if(w->succ[i] == v){
            is_succ = true;
            return;
        }
        else
            isSuccessor(v, w->succ[i], is_succ);
    }
    return;
}

bool DAGTask::allPrecAdded(std::vector<SubTask*> prec, std::vector<int> ids){
    bool prec_present;
    for(int i=0; i<prec.size();++i){
        prec_present = false;
        for(int j=0; j<ids.size();++j){
            if(prec[i]->id == ids[j]){
                prec_present = true;
                break;
            }
        }
        if(!prec_present)
            return false;
    }
    return true;
}

void DAGTask::topologicalSort (){
    std::vector<SubTask> V_copy;
    std::vector<int> ordered_ids;

    for(const auto &v: V)
        V_copy.push_back(*v);

    bool nodes_to_add = true;
    while(nodes_to_add){
        nodes_to_add = false;
        for(auto &v: V_copy){
            if((v.prec.empty() || allPrecAdded(v.prec, ordered_ids))){
                if(v.id != -1 ){
                    ordered_ids.push_back(v.id);
                    v.id = -1;
                }                    
            }
            else
                nodes_to_add = true;
        }
    }

    for(auto id:ordered_ids)
        std::cout<<id<< " ";
    std::cout<<std::endl;

}

bool DAGTask::checkIndexAndIdsAreEqual(){
    for(size_t i=0; i<V.size();++i)
        if(V[i]->id != i)
            return false;
    return true;
}