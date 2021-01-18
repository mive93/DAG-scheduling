#include "tests.h"


std::vector<int> computeSelfOfPath(const std::vector<int>& path, std::vector<SubTask*> V){
    //equation 4
    std::vector<int> self;
    std::vector<int> path_cores;

    for(int i=0; i<path.size(); ++i)
        path_cores.push_back(V[path[i]]->core);

    for(int i=0; i<V.size(); ++i){
        if( std::find(path.begin(), path.end(), V[i]->id) == path.end() && //does not belong to the path
            std::find(path_cores.begin(), path_cores.end(), V[i]->core) != path_cores.end() && // if shares the core with at least one node in the path
            std::find(V[path[0]]->pred.begin(), V[path[0]]->pred.end(), V[i]) == V[path[0]]->pred.end() && // is not a predecessor of the start
            std::find(V[path.back()]->succ.begin(), V[path.back()]->succ.end(), V[i]) == V[path.back()]->succ.end() )// is not a successor of the end
            self.push_back(V[i]->id);
    }
    return self;
}

SSTask deriveSSTask(const std::vector<SubTask*>& V, const std::vector<int>& path_ss, const int core_id, const std::vector<std::vector<float>>& RTs){
    //setSuspendingTask procedure
    SSTask task_ss;
    bool flag = false;
    for(int i=0; i<path_ss.size();++i){
        if(V[path_ss[i]]->core == core_id){
            if(flag)
                task_ss.S.push_back(0);
            task_ss.C.push_back(V[path_ss[i]]->c);
            task_ss.CvID.push_back(V[path_ss[i]]->id);
            flag = true;
        }
        else{
            if(flag || task_ss.S.empty())
                task_ss.S.push_back(RTs[path_ss[i]][path_ss[i]]);
            else
                task_ss.S.back() += RTs[path_ss[i]][path_ss[i]];
                
            flag = false;
        }
    }

    task_ss.Sub = 0;
    std::set<int> path_cores;
    for(int i=0; i<path_ss.size(); ++i)
        if(V[path_ss[i]]->core != core_id)
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

        task_ss.Sub += RTs[path_ss[first]][path_ss[last]];
    }

    task_ss.coreId = core_id;
    
    if(METHOD_VERBOSE) task_ss.print();

    
    return task_ss;
}

float computeWCRT(const float base, const std::vector<int>& self_ss, const std::vector<int>& hp_ss, const int core_id, const Taskset& taskset, const int task_idx){
    float R_ss = 0;
    float new_R_ss = base;
    float self_int = 0;
    float high_int = 0;
    
    //equation 7
    std::vector<SubTask*> V = taskset.tasks[task_idx].getVertices();
    while(R_ss != new_R_ss){
        R_ss = new_R_ss;
        
        self_int = 0;
        for(const auto s:self_ss)
            self_int += V[s]->c;

        high_int = 0;
        
        for(const auto hp:hp_ss)
            high_int+= std::ceil( R_ss / taskset.tasks[hp].getPeriod()) * taskset.tasks[hp].getpVolume()[core_id];

        if(METHOD_VERBOSE) std::cout<<"self_int: "<<self_int<<" high int: "<<high_int<<std::endl;
        new_R_ss = base + high_int + self_int;
    }

    return new_R_ss;
}

float computeWCRTss(const SSTask& tau_ss, const std::vector<int>& self_ss, const std::vector<int>& hp_ss, const int core_id, const Taskset& taskset, const int task_idx, const bool joint){

    float WCRT_ss = 0;
    if (joint){
        // equation 8
        WCRT_ss += tau_ss.Sub;
        for(int i=0; i<tau_ss.C.size(); ++i)
            WCRT_ss +=  tau_ss.C[i];

        WCRT_ss = computeWCRT(WCRT_ss, self_ss, hp_ss, core_id, taskset, task_idx);
    }
    else{ //split
        //equation 9 
        WCRT_ss += tau_ss.Sub;
        for(int i=0; i<tau_ss.C.size(); ++i)
            WCRT_ss += computeWCRT(tau_ss.C[i], self_ss, hp_ss, core_id, taskset, task_idx);

    }

    return WCRT_ss;
}

void pathAnalysis(const std::vector<int>& path_ss, const std::vector<int>& self, const Taskset& taskset, const int task_idx, std::vector<std::vector<float>>& RTs, const bool joint ){
    //algorithm 1
    if(METHOD_VERBOSE) std::cout<<"starting path analysis"<<std::endl;
    if(METHOD_VERBOSE) printVector<int>(path_ss, "path ss");

    std::vector<SubTask*> V = taskset.tasks[task_idx].getVertices();
    SubTask* first = V[path_ss[0]];
    SubTask* last = V[path_ss.back()];
            


    std::vector<int> hp_ss;
    std::vector<int> self_ss;

    for(int i=0; i<self.size(); ++i){
        if(V[self[i]]->core == first->core)
            self_ss.push_back(self[i]);
    }

    for(int i=0; i<task_idx; ++i){
        std::vector<SubTask*> V_cur = taskset.tasks[i].getVertices();
        for(int j=0; j<V_cur.size();++j){
            if(V_cur[j]->core == first->core){
                hp_ss.push_back(i);
                break;
            }
        }
    }

    if(METHOD_VERBOSE) printVector<int>(hp_ss, "hp tasks");
    if(METHOD_VERBOSE) printVector<int>(self_ss, "self ss");
    


    if(path_ss.size() == 1){
        float new_R_path_ss =  computeWCRT(first->c, self_ss, hp_ss, first->core, taskset, task_idx);
        RTs[first->id][last->id] = new_R_path_ss;
        if(METHOD_VERBOSE) std::cout<<"RT "<<first->id<<", "<<last->id<<": "<<new_R_path_ss<<std::endl;
    }
    else{
        if(first->core == last->core){
            std::vector<int> path_sub (path_ss.begin()+1, path_ss.end()-1);
            if(path_sub.size() > 0)
                pathAnalysis(path_sub, self, taskset, task_idx, RTs, joint);
            SSTask task_ss = deriveSSTask(V, path_ss, first->core, RTs);
            float R_task_ss = computeWCRTss(task_ss, self_ss, hp_ss, first->core, taskset, task_idx, joint);
            if(R_task_ss > RTs[first->id][last->id])
                RTs[first->id][last->id] = R_task_ss;
            if(METHOD_VERBOSE) std::cout<<"RT "<<first->id<<", "<<last->id<<": "<<R_task_ss<<std::endl;
        }
        else{
            
            int i = 0;
            for(i=1; i<path_ss.size()-1; ++i){
                if(V[path_ss[i]]->core == last->core)
                    break;
            }
            std::vector<int> path_sub (path_ss.begin()+i, path_ss.end());
            pathAnalysis(path_sub, self, taskset, task_idx, RTs, joint);
            std::vector<int> path_sub_2 (path_ss.begin(), path_ss.end()-1);
            pathAnalysis(path_sub_2, self, taskset, task_idx, RTs, joint);
        }
    }

}

float computeWCRTDAG(const Taskset& taskset, const int task_idx, const bool joint){
    //corollary 1
    
    std::vector<SubTask*> V = taskset.tasks[task_idx].getVertices();
    std::vector<std::vector<float>> RTs (V.size(), std::vector<float>(V.size(), 0));

    //analyze each path
    std::vector<std::vector<int>> all_paths= taskset.tasks[task_idx].computeAllPaths();
    for(const auto& p:all_paths){
        auto self =  computeSelfOfPath(p,V);
        pathAnalysis(p, self, taskset, task_idx, RTs, joint);
    }

    // compute max response time of paths
    int R_DAG = 0;
    for(int i=0;i<RTs.size();++i){
        
        for(int j=0;j<RTs[i].size();++j){
            if(METHOD_VERBOSE) std::cout<<"i: "<<i<<" j: "<<j<<" RT:"<< RTs[i][j]<<std::endl;
            if(RTs[i][j] > R_DAG)
                R_DAG = RTs[i][j];
        }
    }

    return R_DAG;
}

bool P_FP_FTP_Fonseca2016_C(Taskset taskset, const int m, const bool joint){
    std::sort(taskset.tasks.begin(), taskset.tasks.end(), deadlineMonotonicSorting);

    std::map<int, float> pVol;
    for(int i=0; i<taskset.tasks.size(); ++i){
        taskset.tasks[i].computepVolume();
        pVol = taskset.tasks[i].getpVolume();

        for(const auto& p:pVol)
            if(p.second > taskset.tasks[i].getDeadline())
                return false;

        float R = computeWCRTDAG(taskset, i, joint);
        if(R > taskset.tasks[i].getDeadline())
            return false;
    }

    return true;
}