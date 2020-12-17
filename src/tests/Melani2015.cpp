#include "tests.h"
// Alessandra Melani et al. “Response-time analysis of conditional dag tasks in multiprocessor systems”. (ECRTS 2015)

float computeZk(DAGTask task, const int n_proc){
    // Algorithm 2

    auto V = task.getVertices();
    auto ordIDs = task.getTopologicalOrder();

    if(!ordIDs.size()){
        task.topologicalSort();
        ordIDs = task.getTopologicalOrder();
    }
    std::vector<std::set<int>> S (V.size());
    std::vector<std::set<int>> T (V.size());
    std::vector<float> f (V.size());
    S[ordIDs[ordIDs.size()-1]].insert(ordIDs[ordIDs.size()-1]);
    T[ordIDs[ordIDs.size()-1]].insert(ordIDs[ordIDs.size()-1]);
    f[ordIDs[ordIDs.size()-1]] = V[ordIDs[ordIDs.size()-1]]->c;
    
    int idx;
    float C = 0;
    std::set<int> D;
    for(int i = ordIDs.size()-2; i >= 0; --i ){
        idx = ordIDs[i];

        if(V[idx]->desc.size()){
            S[idx].insert(idx);
            if(V[idx]->mode != C_SOURCE_T){ //if not conditional source
                std::vector<float> U (V[idx]->desc.size(),0);
                for(int j=0; j<V[idx]->desc.size(); ++j){
                    S[idx].insert(S[V[idx]->desc[j]->id].begin(), S[V[idx]->desc[j]->id].end());
                    U[j] = f[V[idx]->desc[j]->id];
                    for(int k=0; k<V[idx]->desc.size(); ++k){
                        if(k!=j){
                            C = 0;
                            D.clear();
                            std::set_difference(S[V[idx]->desc[k]->id].begin(), 
                                                S[V[idx]->desc[k]->id].end(),
                                                T[V[idx]->desc[j]->id].begin(), 
                                                T[V[idx]->desc[j]->id].end(), 
                                                std::inserter(D, D.begin()) );
                        
                            for (const auto& d:D)
                                C+= V[d]->c / n_proc;

                            U[j]+=C;
                        }
                    }
                }
                int max_U_idx = std::max_element(U.begin(),U.end()) - U.begin();
                T[idx].clear();
                T[idx].insert(idx);
                T[idx].insert(T[V[idx]->desc[max_U_idx]->id].begin(), T[V[idx]->desc[max_U_idx]->id].end());
                f[idx] = V[idx]->c + U[max_U_idx];
            }
            else{
                std::vector<float> C (V[idx]->desc.size(),0);
                std::vector<float> ff (V[idx]->desc.size(),0);

                for(int j=0; j<V[idx]->desc.size(); ++j){
                    for(auto k: S[V[idx]->desc[j]->id]){
                        C[j] += V[k]->c;
                    }

                    ff[j] = f[V[idx]->desc[j]->id];
                }

                int max_C_idx = std::max_element(C.begin(),C.end()) - C.begin();
                S[idx].insert(S[V[idx]->desc[max_C_idx]->id].begin(), S[V[idx]->desc[max_C_idx]->id].end());

                int max_ff_idx = std::max_element(ff.begin(),ff.end()) - ff.begin();
                T[idx].clear();
                T[idx].insert(idx);
                T[idx].insert(T[V[idx]->desc[max_ff_idx]->id].begin(), T[V[idx]->desc[max_ff_idx]->id].end());

                f[idx] = V[idx]->c + f[V[idx]->desc[max_ff_idx]->id];

            }
        }
    }
    return f[ordIDs[0]];
}

void maximizeMakespan(const std::vector<SubTask *>& V, const SubTask* v, const std::vector<float>& mksp, const std::vector<std::set<int>>& mksp_set,  const std::vector<std::set<int>>& w_set,  std::set<int>& mkspset_tmp, float& max_mksp, const int n_proc){
    max_mksp = 0;
    for(int j=0; j<v->desc.size(); ++j){
        float sum_w = 0;
        std::set<int> wset_tmp;

        for(int k=0; k<v->desc.size(); ++k){
            if(k != j){
                std::set<int> new_vert;
                std::set_difference(w_set[v->desc[k]->id].begin(), 
                                    w_set[v->desc[k]->id].end(),
                                    wset_tmp.begin(), 
                                    wset_tmp.end(), 
                                    std::inserter(new_vert, new_vert.begin()) );
                set_difference_inplace<int>(new_vert, mksp_set[v->desc[j]->id]);
                wset_tmp.insert(new_vert.begin(), new_vert.end());

                for(const auto& nv:new_vert)
                    sum_w+= V[nv]->c;

            }
        }

        if(mksp[v->desc[j]->id] + sum_w / n_proc > max_mksp){
            max_mksp = mksp[v->desc[j]->id] + sum_w / n_proc;
            mkspset_tmp.insert(mksp_set[v->desc[j]->id].begin(), mksp_set[v->desc[j]->id].end());
            mkspset_tmp.insert(wset_tmp.begin(), wset_tmp.end());
        }
    }
}

float computeMakespanUB(DAGTask task, const int n_proc){
    std::vector<SubTask *> V = task.getVertices();
    std::vector<int> ordIDs = task.getTopologicalOrder();

    if(!ordIDs.size()){
        task.topologicalSort();
        ordIDs = task.getTopologicalOrder();
    }

    std::vector<std::set<int>> mksp_set (V.size());
    std::vector<std::set<int>> w_set (V.size());
    std::vector<float> w (V.size());
    std::vector<float> mksp (V.size());
    
    mksp_set[ordIDs[ordIDs.size()-1]].insert(ordIDs[ordIDs.size()-1]);
    w_set[ordIDs[ordIDs.size()-1]].insert(ordIDs[ordIDs.size()-1]);
    mksp[ordIDs[ordIDs.size()-1]] = V[ordIDs[ordIDs.size()-1]]->c;
    w[ordIDs[ordIDs.size()-1]] = V[ordIDs[ordIDs.size()-1]]->c;
    
    int idx;
    float C = 0;
    std::set<int> D;
    for(int i = ordIDs.size()-2; i >= 0; --i ){
        idx = ordIDs[i];
        mksp_set[idx].insert(idx);
        w_set[idx].insert(idx);
        mksp[idx] = V[idx]->c;
        w[idx] = V[idx]->c;

        if(V[idx]->desc.size()){
             
            if(V[idx]->mode != C_SOURCE_T){ //if not conditional source

            float max_mksp = 0;
            float w_tmp = 0;
            std::set<int> wset_tmp, mkspset_tmp;
            maximizeMakespan(V, V[idx], mksp, mksp_set, w_set, mkspset_tmp, max_mksp, n_proc);

            mksp[idx] = max_mksp + V[idx]->c;
            mksp_set[idx].insert(idx);
            mksp_set[idx].insert(mkspset_tmp.begin(), mkspset_tmp.end());

            for(int j=0; j<V[idx]->desc.size(); ++j){
                std::set<int> new_vert;
                std::set_difference(w_set[V[idx]->desc[j]->id].begin(), 
                                    w_set[V[idx]->desc[j]->id].end(),
                                    wset_tmp.begin(), 
                                    wset_tmp.end(), 
                                    std::inserter(new_vert, new_vert.begin()) );
                wset_tmp.insert(new_vert.begin(), new_vert.end());

                for(auto nv:new_vert)
                    w_tmp += V[nv]->c;
            }

            w[idx] += w_tmp;
            w_set[idx].insert(wset_tmp.begin(), wset_tmp.end());

            }
            else{
                float max_mksp = 0, max_w = 0;
                int max_idx = 0;
                for(int j=0; j<V[idx]->desc.size(); ++j){
                    if(mksp[V[idx]->desc[j]->id] > max_mksp){
                        max_mksp = mksp[V[idx]->desc[j]->id];
                        max_idx = j;
                    }
                }
                mksp[idx] += max_mksp;
                mksp_set[idx].insert(mksp_set[V[idx]->desc[max_idx]->id].begin(), mksp_set[V[idx]->desc[max_idx]->id].end());

                for(int j=0; j<V[idx]->desc.size(); ++j){
                    if(w[V[idx]->desc[j]->id] > max_w){
                        max_w = w[V[idx]->desc[j]->id];
                        max_idx = j;
                    }
                }

                w[idx] += max_w;
                w_set[idx].insert(w_set[V[idx]->desc[max_idx]->id].begin(),w_set[V[idx]->desc[max_idx]->id].end());
            }
        }
    }

    return mksp[ordIDs[0]];
}

float workloadUpperBound(const DAGTask& task, const float t, const int m){

    float wcw = task.getWorstCaseWorkload();
    float T = task.getPeriod();

    float ci_b = std::floor((t + task.R - wcw / m ) / T ) * wcw;
    float co = std::min(wcw, m * std::fmod(t + task.R - wcw / m ,T) );

    return ci_b + co;
}

float interferringWorkload(const DAGTask& task_x, const DAGTask& task_y , const float t, const int m){
    float wcw_y = task_y.getWorstCaseWorkload();
    float D_x = task_x.getDeadline();
    float D_y = task_y.getDeadline();
    float T_y = task_y.getPeriod();

    float ci_b = ( std::floor((D_x - D_y) / T_y) + 1 ) * wcw_y;
    float co = std::min( wcw_y, m * float( std::max(float(0), std::fmod(D_x,T_y) - (D_y - task_y.R))));

    return ci_b + co;
}

void test_Melani(Taskset taskset, const int m){
    std::sort(taskset.tasks.begin(), taskset.tasks.end(), deadlineMonotonicSorting);

    for(int i=0; i<taskset.tasks.size(); ++i){
        std::cout<<"mksp: "<<computeMakespanUB(taskset.tasks[i],m)<<std::endl;
        std::cout<<"Z: "<<computeZk(taskset.tasks[i],m)<<std::endl;
    }
}

bool GP_FP_EDF_Melani2015_C(Taskset taskset, const int m){
    std::vector<float> R_old (taskset.tasks.size(), 0);
    std::vector<float> R (taskset.tasks.size(), 0);
    std::vector<float> mksp (taskset.tasks.size(), 0);
    for(int i=0; i<taskset.tasks.size(); ++i){
        mksp[i] = computeMakespanUB(taskset.tasks[i], m);
        R_old[i] = mksp[i];
        taskset.tasks[i].R = mksp[i];
    }

    bool init = true, changed = true;

    while(changed){
        changed = false;
        for(int i=0; i<taskset.tasks.size(); ++i){
            if(R_old[i] > taskset.tasks[i].getDeadline())
                return false;

            if(!init){
                R_old[i] = R[i];
                R[i] = 0;
            }

            if(taskset.tasks.size() > 1){
                for(int j=0; j<taskset.tasks.size(); ++j){
                    if(j != i){
                        R[i] += (1. / m) * std::min(workloadUpperBound(taskset.tasks[j], R_old[i], m), 
                                                    interferringWorkload(taskset.tasks[i], taskset.tasks[j], R_old[i], m));
                    }
                }
                R[i] += mksp[i];
                R[i] = std::floor(R[i]);
            }
            else
                R[i] = R_old[i];

            taskset.tasks[i].R = R[i];

            if( !areEqual<float>(R[i], R_old[i]) ) 
                changed = true;

            if(R[i] > taskset.tasks[i].getDeadline())
                return false;

        }
        init = false;
    }

    for(int i=0; i<taskset.tasks.size(); ++i)
        taskset.tasks[i].R = R[i];

    return true;

}

bool GP_FP_FTP_Melani2015_C(Taskset taskset, const int m){

    std::sort(taskset.tasks.begin(), taskset.tasks.end(), deadlineMonotonicSorting);

    std::vector<float> R_old (taskset.tasks.size(), 0);
    std::vector<float> R (taskset.tasks.size(), 0);
    std::vector<float> mksp (taskset.tasks.size(), 0);
    for(int i=0; i<taskset.tasks.size(); ++i){
        mksp[i] = computeMakespanUB(taskset.tasks[i],m);
        R_old[i] = mksp[i];
        taskset.tasks[i].R = mksp[i];
    }

    for(int i=0; i<taskset.tasks.size(); ++i){

        if(R_old[i] > taskset.tasks[i].getDeadline())
            return false;

        bool init = true;
        while(!areEqual<float>(R[i], R_old[i]) && R[i] <= taskset.tasks[i].getDeadline()){
            if(!init){
                R_old[i] = R[i];
                R[i] = 0;
            }

            if(i > 0){
                for(int j=0; j<i; ++j)
                    R[i] = R[i] + 1 / m * workloadUpperBound(taskset.tasks[i], R_old[i], m);

                R[i] = std::floor(R[i]);
                R[i] += mksp[i];
            }
            else
                R[i] = R_old[i];

            init = false;
        }

        if( areEqual<float>(R[i], R_old[i]))
            return true;
        else if (R[i] > taskset.tasks[i].getDeadline())
            return false;
    }

    return false;
}