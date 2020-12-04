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
    ordIDs.clear();

    for(const auto &v: V)
        V_copy.push_back(*v);

    bool nodes_to_add = true;
    while(nodes_to_add){
        nodes_to_add = false;
        for(auto &v: V_copy){
            if((v.prec.empty() || allPrecAdded(v.prec, ordIDs))){
                if(v.id != -1 ){
                    ordIDs.push_back(v.id);
                    v.id = -1;
                }                    
            }
            else
                nodes_to_add = true;
        }
    }

    if(!checkIndexAndIdsAreEqual())
        FatalError("Ids and Indexes do not correspond, can't use computed topological order!");
    if(ordIDs.size() != V.size())
        FatalError("Ids and V sizes differ!");
}

bool DAGTask::checkIndexAndIdsAreEqual(){
    for(size_t i=0; i<V.size();++i)
        if(V[i]->id != i)
            return false;
    return true;
}

void DAGTask::computeVolume(){
    vol = 0;
    for(size_t i=0; i<V.size();++i)
        vol += V[i]->c;
}

void DAGTask::computeAccWorkload(){
    int max_acc_prec;
    for(size_t i=0; i<ordIDs.size();++i){
        max_acc_prec = 0;
        for(size_t j=0; j<V[ordIDs[i]]->prec.size();++j){
            if(V[ordIDs[i]]->prec[j]->accWork > max_acc_prec)
                max_acc_prec = V[ordIDs[i]]->prec[j]->accWork;
        }

        V[ordIDs[i]]->accWork = V[ordIDs[i]]->c + max_acc_prec;
    }
}

void DAGTask::computeLength(){
    if(!ordIDs.size())
        topologicalSort();
        
    computeAccWorkload();
    for(const auto&v :V)
        if(v->accWork > L)
            L = v->accWork;
}