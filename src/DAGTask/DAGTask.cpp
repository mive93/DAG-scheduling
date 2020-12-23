#include "DAGTask.h"

void DAGTask::cloneVertices(const std::vector<SubTask*>& to_clone_V){
    V.clear();
    for(int i=0; i<to_clone_V.size();++i){
        SubTask * v = new SubTask;
        *v = *to_clone_V[i];
        V.push_back(v);
    }

    for(int i=0; i<to_clone_V.size();++i){
        V[i]->pred.clear();
        V[i]->succ.clear();

        for(int j=0; j<to_clone_V[i]->succ.size();++j)
            V[i]->succ.push_back(V[to_clone_V[i]->succ[j]->id]);

        for(int j=0; j<to_clone_V[i]->pred.size();++j)
            V[i]->pred.push_back(V[to_clone_V[i]->pred[j]->id]);
        
    }
}

void DAGTask::destroyVerices(){
    for(int i=0; i<V.size();++i)
        delete V[i];
}

void DAGTask::isSuccessor(const SubTask* v, const SubTask *w, bool &is_succ) const{
    for(size_t i=0; i<w->succ.size(); ++i){
        
        if(w->succ[i] == v){
            is_succ = true;
            return;
        }
        else{
            
            isSuccessor(v, w->succ[i], is_succ);
        }
    }
    return;
}

std::vector<SubTask*> DAGTask::getSubTaskAncestors(const int i) const{
    
    std::vector<SubTask*> ancst;
    bool is_succ = false;
    for(int j=0; j<V.size(); ++j){
        is_succ = false;
        isSuccessor(V[i], V[j], is_succ);
        if( is_succ )
            ancst.push_back(V[j]);
    }

    return ancst;
}

std::vector<SubTask*> DAGTask::getSubTaskDescendants(const int i) const{
    
    std::vector<SubTask*> desc;
    bool is_succ = false;
    for(int j=0; j<V.size(); ++j){
        is_succ = false;
        isSuccessor(V[j], V[i], is_succ);
        if( is_succ )
            desc.push_back(V[j]);
    }

    return desc;
}

void DAGTask::transitiveReduction(){
    for(int i=0; i<V.size(); ++i){
        std::vector<SubTask*> succ_desc;
        //find all the successors descendants
        for(int j=0; j<V[i]->succ.size();++j){
            std::vector<SubTask*> succ_desc_j = getSubTaskDescendants(V[i]->succ[j]->id);
            succ_desc.insert(succ_desc.end(), succ_desc_j.begin(), succ_desc_j.end());
        }
        
        // is a successor is also a successors descendants, mark to remove
        std::vector<SubTask*> to_remove;
        for(int j=0; j<V[i]->succ.size();++j){
            if ( std::find(succ_desc.begin(), succ_desc.end(), V[i]->succ[j]) != succ_desc.end() ){
                to_remove.push_back(V[i]->succ[j]);
            }
        }

        //remove 
        for(auto& r:to_remove){
            V[i]->succ.erase(std::remove(V[i]->succ.begin(), V[i]->succ.end(), r), V[i]->succ.end());
            r->pred.erase(std::remove(r->pred.begin(), r->pred.end(), V[i]), r->pred.end());
        }
    }
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

void DAGTask::computeWorstCaseWorkload(){
    // Algorithm 1, Melani et al. "Response-Time Analysis of Conditional DAG Tasks in Multiprocessor Systems"
    // computes volume both for DAGs and Conditinal DAGs
    if(!ordIDs.size())
        topologicalSort();

    std::vector<std::set<int>> paths (V.size());
    paths[ordIDs[ordIDs.size()-1]].insert(ordIDs[ordIDs.size()-1]);
    int idx;
    for(int i = ordIDs.size()-2; i >= 0; --i ){
        idx = ordIDs[i];
        paths[idx].insert(idx);

        if(V[idx]->succ.size()){
            if(V[idx]->mode != C_SOURCE_T){
                for(int j=0; j<V[idx]->succ.size(); ++j)
                    paths[idx].insert(paths[V[idx]->succ[j]->id].begin(), paths[V[idx]->succ[j]->id].end());
            }
            else{
                std::vector<int> sum (V[idx]->succ.size(), 0);
                int max_id = 0;
                float max_sum = 0;
                for(int j=0; j<V[idx]->succ.size(); ++j){
                    for(auto k: paths[V[idx]->succ[j]->id])
                        sum[j] += V[k]->c;
                    if(sum[j] > max_sum){
                        max_sum = sum[j];
                        max_id = j;
                    }
                }
                paths[idx].insert(paths[V[idx]->succ[max_id]->id].begin(), paths[V[idx]->succ[max_id]->id].end());
            }
        }
    }

    wcw = 0;
    for(auto i:paths[ordIDs[0]]){
        wcw += V[i]->c;
    }
}

void DAGTask::computeTypedVolume(){
    for(size_t i=0; i<V.size();++i){
        if ( typedVol.find(V[i]->gamma) == typedVol.end() ) 
            typedVol[V[i]->gamma] = V[i]->c;
        else
            typedVol[V[i]->gamma] += V[i]->c;
    }
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
    L = 0;    
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