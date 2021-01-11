#include "tests.h"


float computeSI(const std::vector<int>& path, const DAGTask& task){
    // lemma 8
    std::vector<int> self;
    std::vector<int> path_cores;

    std::vector<SubTask*> V = task.getVertices();

    auto desc = task.getSubTaskDescendants(path.back());
    auto ancst = task.getSubTaskAncestors(path[0]);

    int core_id = V[path[0]]->core;

    if(METHOD_VERBOSE){
        printVector<int>(path, "path computeSI");
        std::cout<<"Ancst:";
        for(auto a:ancst)
            std::cout<<a->id<<" ";
        std::cout<<std::endl;

        std::cout<<"Desc:";
        for(auto d:desc)
            std::cout<<d->id<<" ";
        std::cout<<std::endl;
    }

    for(int i=0; i<V.size(); ++i){
        
        if( std::find(path.begin(), path.end(), V[i]->id) == path.end() && //does not belong to the path
            V[i]->core == core_id && // if it's on the same core as start and end
            std::find(ancst.begin(), ancst.end(), V[i]) == ancst.end() && // is not an ancestor of the start
            std::find(desc.begin(), desc.end(), V[i]) == desc.end() )// is not a descendant of the end
            self.push_back(V[i]->id);
    }

    
    if(METHOD_VERBOSE) printVector<int>(self, "self computeSI");

    float SI = 0;
    for(const auto& s: self)
        SI += V[s]->c;
    return SI;
}

std::vector<float> computeMultisetB(const int k, const float interval, const std::vector<std::vector<float>>& R_nodes, const Taskset& taskset, const int task_idx, int core_id ){
    float eta;
    std::vector<float> multiset_C;
    for(int y=task_idx+1; y<taskset.tasks.size(); ++y){
        std::vector<SubTask*> V = taskset.tasks[y].getVertices();
        for(int j=0;j<V.size(); ++j)
            if(V[j]->core == core_id)
            float eta = 1 + std::floor( ( interval + R_nodes[y][j] - V[j]->c) / taskset.tasks[y].getPeriod() );

        for(int j=0;j<V.size(); ++j)
            if(V[j]->core == core_id)
                for(int e=0;e<eta; ++e)
                    multiset_C.push_back(V[j]->c);
    }


    std::sort(multiset_C.begin(), multiset_C.end(), std::greater<>());
    std::vector<float> multiset_B(k, 0);

    for(int i=0; i<k && i<multiset_C.size();++i)
        multiset_B[i] = multiset_C[i];

    return multiset_B;
}

float computeMaximumInterference(const float interval, const std::vector<std::vector<float>>& R_nodes, const Taskset& taskset, const int task_idx, const int core_id){

    float first_term = 0, second_term = 0, tot_R;  
    for(int y=0; y<task_idx; ++y){
        std::vector<SubTask*> V = taskset.tasks[y].getVertices();

        tot_R = 0;
        for(int j=0; j<V.size(); ++j){
            if(V[j]->core == core_id){
                first_term += V[j]->c * ( 1 + std::floor( (interval + R_nodes[y][j] - V[j]->c) / taskset.tasks[y].getPeriod() ));
            }
            tot_R += R_nodes[y][j];
        }



        second_term += (1 + std::floor( (interval + tot_R - taskset.tasks[y].getpVolume()[core_id] ) / taskset.tasks[y].getPeriod())) * taskset.tasks[y].getpVolume()[core_id];
    }

    if(METHOD_VERBOSE) std::cout<<"interval:"<<interval<<std::endl;
    if(METHOD_VERBOSE) std::cout<<"first term: "<<first_term<<std::endl;
    if(METHOD_VERBOSE) std::cout<<"second term: "<<second_term<<std::endl;

    return std::min(first_term, second_term);
}

float Theorem1Casini2018(const SSTask& tau_ss, const std::vector<std::vector<float>>& R_nodes, const Taskset& taskset, const int task_idx, const float SI){

    float base = 0;

    for(int i=0; i<tau_ss.C.size() -1; ++i)
        base += tau_ss.C[i];
    
    float S_part = 0;
    for(int i=0; i<tau_ss.S.size() ; ++i)
        S_part += tau_ss.S[i];

    base += std::min(S_part, tau_ss.Sub);

    float R_prime_old = -1;
    float R_prime = base;
    float b, I;

    std::vector<float> multiset_B;

    if(METHOD_VERBOSE) std::cout<<"base: "<<base<<std::endl;
    while(R_prime_old != R_prime){
        R_prime_old = R_prime;

        //computing blocking from lp
        multiset_B = computeMultisetB(tau_ss.C.size(), R_prime_old, R_nodes, taskset, task_idx, tau_ss.coreId);
        if(METHOD_VERBOSE) printVector<float>(multiset_B, "multiset_B");
        b = 0;
        for(const auto B:multiset_B)
            b += B;

        if(METHOD_VERBOSE) std::cout<<"b: "<<b<<std::endl;

        //computing interference from hp
        I = computeMaximumInterference(R_prime_old, R_nodes, taskset, task_idx, tau_ss.coreId);

        if(METHOD_VERBOSE) std::cout<<"I: "<<I<<std::endl;

        //equation 4
        R_prime = base + b + I + SI;
    }

    
    //equation 3
    float R = R_prime + tau_ss.C.back();
    return R;

}

float Theorem2Casini2018(const int k, const SSTask& tau_ss, const std::vector<float>& R_ss, const std::vector<std::vector<float>>& R_nodes, const Taskset& taskset, const int task_idx, const float SI){
    float R = 0;

    for(int i=0; i<= k; ++i)
        R += tau_ss.C[i];
    
    float S_part = 0;
    for(int i=0; i< k; ++i)
        S_part += tau_ss.S[i];
    R += std::min(S_part, tau_ss.Sub);

    float r_k = (k == 0)? 0 : R_ss[k-1] + tau_ss.S[k-1];
    std::vector<float> multiset_B = computeMultisetB(k, r_k, R_nodes, taskset, task_idx, tau_ss.coreId);

    float bI = 0;
    float delta = 0;
    float new_delta = 0;

    // This does not convince me at all, but it is implemented as it is in the paper. 
    // We account for high interference only when:
    //  (i) there is more than 1 C block in tau_ss 
    //  (ii) there is at least one lower priority job
    // that does not work imho
    for(const auto B:multiset_B){

        delta = 0;
        new_delta = B;

        //equation 6
        while(new_delta != delta){
            delta = new_delta;
            new_delta = B + computeMaximumInterference(delta, R_nodes, taskset, task_idx, tau_ss.coreId);
        }

        bI += new_delta;

    }
    
    if(METHOD_VERBOSE) std::cout<<"R: "<<R<<" bI: "<<bI<<" SI: "<<SI<<std::endl;
    //equation 7
    return R + bI + SI;
}


float computeWCRTssCasini(const SSTask& tau_ss, const std::vector<int>& path_ss, const Taskset& taskset, const int task_idx, const std::vector<std::vector<float>>& R_nodes){

    float SI = computeSI(path_ss, taskset.tasks[task_idx]);

    if(METHOD_VERBOSE) std::cout<<"SI:"<<SI<<std::endl;
    std::vector<float> R_ss(tau_ss.C.size(), 0);
    
    float R1 = Theorem1Casini2018(tau_ss, R_nodes, taskset, task_idx, SI);
    if(METHOD_VERBOSE) std::cout<<"R1: "<<R1<<std::endl;
    float R2, R1_j;
    float final_R = 0;
    for(int j = 0; j< tau_ss.C.size(); ++j){
        R2 = Theorem2Casini2018(j, tau_ss, R_ss, R_nodes, taskset, task_idx, SI);

        R1_j = R1;
        for(int k=j+1; k<tau_ss.C.size(); ++k)
            R1_j -= tau_ss.C[k];

        for(int k=j+1; k<tau_ss.S.size(); ++k)
            R1_j -= tau_ss.S[k];

        if(METHOD_VERBOSE) std::cout<<"R2: "<<R2<<std::endl;
        if(METHOD_VERBOSE) std::cout<<"R1_j: "<<R1_j<<std::endl;
        R_ss[j] = std::min(R2, R1_j);
        if(R_ss[j] > final_R)
            final_R = R_ss[j];
    }

    final_R = R1;
    if(METHOD_VERBOSE) std::cout<<"final R: "<<final_R<<std::endl;

    return final_R;
}


float pathAnalysisImproved(const std::vector<int>& path_ss, const Taskset& taskset, const int task_idx, std::vector<std::vector<float>>& RTs, const bool is_root, const std::vector<std::vector<float>>& R_nodes ){
    //algorithm 5
    if(METHOD_VERBOSE) std::cout<<"starting path analysis improved"<<std::endl;
    if(METHOD_VERBOSE) printVector<int>(path_ss, "path ss");

    std::vector<SubTask*> V = taskset.tasks[task_idx].getVertices();
    SubTask* first = V[path_ss[0]];
    SubTask* last = V[path_ss.back()];
    int core_ss = first->core;

    if(path_ss.size() == 1){
        SSTask task_ss = deriveSSTask(V, path_ss, first->core, RTs);
        float R_task_ss = computeWCRTssCasini(task_ss, path_ss, taskset, task_idx, R_nodes);
        RTs[first->id][last->id] = R_task_ss;
        if(METHOD_VERBOSE) std::cout<<"RT "<<first->id<<", "<<last->id<<": "<<R_task_ss<<std::endl;
    }
    else{
        if(first->core == last->core){
            std::vector<int> path_sub (path_ss.begin()+1, path_ss.end()-1);
            if(path_sub.size() > 0)
                pathAnalysisImproved(path_sub, taskset, task_idx, RTs, false, R_nodes);
            SSTask task_ss = deriveSSTask(V, path_ss, first->core, RTs);
            float R_task_ss = computeWCRTssCasini(task_ss, path_ss, taskset, task_idx, R_nodes);
            RTs[first->id][last->id] = R_task_ss - task_ss.Sub;
            if(METHOD_VERBOSE) std::cout<<"RT "<<first->id<<", "<<last->id<<": "<<RTs[first->id][last->id]<<std::endl;
        }
        else{
            
            int i = 0;
            for(i=1; i<path_ss.size()-1; ++i){
                if(V[path_ss[i]]->core == last->core)
                    break;
            }
            std::vector<int> path_sub (path_ss.begin()+i, path_ss.end());
            pathAnalysisImproved(path_sub, taskset, task_idx, RTs, false, R_nodes);
            std::vector<int> path_sub_2 (path_ss.begin(), path_ss.end()-1);
            pathAnalysisImproved(path_sub_2, taskset, task_idx, RTs, false, R_nodes);
        }
    }

    if(is_root){
        float RT = 0;
        std::set<int> path_cores;
        for(int i=0; i<path_ss.size(); ++i)
            path_cores.insert(V[path_ss[i]]->core);
        
        if(METHOD_VERBOSE) printSet<int>(path_cores, "path_cores");

        int first = 0, last = 0;
        for(const auto& core:path_cores){
            //first node allocate to core i
            for(first=0; first<path_ss.size();++first)
                if(V[path_ss[first]]->core == core)
                    break;
            //first node allocate to core j
            for(last=path_ss.size()-1; last>=0;--last)
                if(V[path_ss[last]]->core == core)
                    break;

            RT += RTs[path_ss[first]][path_ss[last]];
        }
        
        if(METHOD_VERBOSE) std::cout<<"RT: "<<RT<<std::endl;
        return RT;
    }
    return 0;
}

bool P_LP_FTP_Casini2018_C(Taskset taskset, const int m){
    //to give prios
    std::sort(taskset.tasks.begin(), taskset.tasks.end(), deadlineMonotonicSorting);
    //algorithm 3

    std::vector<std::vector<float>> new_R_nodes(taskset.tasks.size());
    for(int x=0; x<taskset.tasks.size(); ++x){    
        taskset.tasks[x].R = 0;
        taskset.tasks[x].computepVolume();
        
        std::vector<SubTask*> V = taskset.tasks[x].getVertices();
        new_R_nodes[x].resize(V.size());

        for(int i=0;i<V.size();++i){
            auto desc = taskset.tasks[x].getSubTaskDescendants(i);
            float sum_desc_same_core = 0;
            for(int j=0; j<desc.size();++j){
                if(desc[j]->core == V[i]->core)
                    sum_desc_same_core += desc[j]->c;
            }
            V[i]->r = taskset.tasks[x].getDeadline() - sum_desc_same_core;
        }
    }
        
    bool at_least_one_update = true;
    
    while(at_least_one_update){

        if(METHOD_VERBOSE) std::cout<<"NEW ITERATION-----------------------------------"<<std::endl;
        at_least_one_update = false;

        for(int x=0; x<new_R_nodes.size(); ++x)
            for(int i=0; i<new_R_nodes[x].size(); ++i)
                new_R_nodes[x][i] = 0;

        for(int x=0; x<taskset.tasks.size(); ++x){    
            std::vector<std::vector<int>> all_paths= taskset.tasks[x].computeAllPaths();
            std::vector<SubTask*> V = taskset.tasks[x].getVertices();
            std::vector<std::vector<float>> RTs (V.size(), std::vector<float>(V.size(), 0));
            for(const auto& p:all_paths){
                float RT = pathAnalysisImproved(p,taskset, x, RTs, true, new_R_nodes);
                taskset.tasks[x].R = std::max(taskset.tasks[x].R, RT);
                
                if(METHOD_VERBOSE) std::cout<<"taskset.tasks["<<x<<"].R "<<taskset.tasks[x].R<<std::endl;
                if(taskset.tasks[x].R > taskset.tasks[x].getDeadline())
                    return false;

                for(const auto& v:p)
                    new_R_nodes[x][v] = std::max(new_R_nodes[x][v], RTs[v][v]);

            }

        }

        for(int x=0; x<new_R_nodes.size(); ++x){
            std::vector<SubTask*> V = taskset.tasks[x].getVertices();
            for(int i=0; i<new_R_nodes[x].size(); ++i){
                if(new_R_nodes[x][i] < V[i]->r)
                    at_least_one_update = true;
                
                V[i]->r = std::min(new_R_nodes[x][i], V[i]->r);
            }
        }
    }
    
    return true;
}
