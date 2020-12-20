#include "DAGTask.h"

void DAGTask::cloneVertices(const std::vector<SubTask*>& to_clone_V){
    V.clear();
    std::vector<SubTask*> cloned_V;
    for(int i=0; i<to_clone_V.size();++i){
        SubTask * v = new SubTask;
        *v = *to_clone_V[i];
        cloned_V.push_back(v);
    }

    for(int i=0; i<to_clone_V.size();++i){
        cloned_V[i]->pred.clear();
        cloned_V[i]->succ.clear();

        for(int j=0; j<to_clone_V[i]->succ.size();++j)
            cloned_V[i]->succ.push_back(cloned_V[to_clone_V[i]->succ[j]->id]);

        for(int j=0; j<to_clone_V[i]->pred.size();++j)
            cloned_V[i]->pred.push_back(cloned_V[to_clone_V[i]->pred[j]->id]);
    }

    V = cloned_V;
}

void DAGTask::destroyVerices(){
    for(int i=0; i<V.size();++i)
        delete V[i];
}

void DAGTask::isSuccessor(SubTask* v, SubTask *w, bool &is_succ) const{

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

bool DAGTask::allPrecAdded(std::vector<SubTask*> pred, std::vector<int> ids){
    bool prec_present;
    for(int i=0; i<pred.size();++i){
        prec_present = false;
        for(int j=0; j<ids.size();++j){
            if(pred[i]->id == ids[j]){
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
            if((v.pred.empty() || allPrecAdded(v.pred, ordIDs))){
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
        for(size_t j=0; j<V[ordIDs[i]]->pred.size();++j){
            if(V[ordIDs[i]]->pred[j]->accWork > max_acc_prec)
                max_acc_prec = V[ordIDs[i]]->pred[j]->accWork;
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

void DAGTask::computeUtilization(){
    if(vol == 0)
        computeWorstCaseWorkload();
    u =  wcw /  t; 
}

void DAGTask::computeDensity(){
    if(L == 0)
        computeLength();
    delta =  L /  d; 
}

void DAGTask::computeLocalOffsets(){
    if(!ordIDs.size())
        topologicalSort();

    for(int i=0; i<ordIDs.size();++i){
        V[ordIDs[i]]->localOffset();
    }
}

void DAGTask::computeLocalDeadlines(){
    if(!ordIDs.size())
        topologicalSort();

    for(int i=ordIDs.size()-1; i>=0;--i){
        V[ordIDs[i]]->localDeadline(d);
    }
}

void DAGTask::computeEFTs(){
    DAGTask::computeLocalOffsets();

    for(auto&v:V)
        v->EasliestFinishingTime();
}
void DAGTask::computeLSTs(){
    DAGTask::computeLocalDeadlines();

    for(auto&v:V)
        v->LatestStartingTime();
}