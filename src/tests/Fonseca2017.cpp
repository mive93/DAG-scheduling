#include "tests.h"

#include "SP-Tree.h"

//Jstd::couté Fonseca et al. “Improved response time analysis of sporadic dag tasks for global fp scheduling”.  (RTNS 2017) 

std::vector<std::pair<float, float>> computeWorkloadDistributionCI(const DAGTask& task){

    std::vector<std::pair<float, float>> WD;
    auto V = task.getVertices();

    std::vector<float> F;
    std::set<float> F_set;

    F_set.insert(0);

    for(int i=0; i<V.size() ;++i)
        F_set.insert(V[i]->EFT);
    
    for(const auto& f:F_set)
        F.push_back(f);
    std::sort(F.begin(), F.end());

    float w, h;
    for(int t=1; t<F.size(); ++t){
        
        //width
        w = F[t] - F[t-1];

        //height
        h = 0;
        for(int i=0; i<V.size() ;++i)
            if (F[t-1] >= V[i]->localO && F[t-1] < V[i]->EFT)
                h++;

        WD.push_back(std::make_pair(w,h));

    }

    return WD;
}

float computeCarryInUpperBound(const DAGTask& task, const int interval, std::vector<std::pair<float, float>> WD_UCI_y){

    //equation 6 in the paper

    float CI = 0, CI_tmp = 0;

    for(int i=0; i< WD_UCI_y.size();++i){
        CI_tmp = interval - task.getPeriod() + task.R; 

        for(int j=i+1; j< WD_UCI_y.size();++j)
            CI_tmp -= WD_UCI_y[j].first;

        CI += WD_UCI_y[i].second * std::max( float(0), std::min (WD_UCI_y[i].first, CI_tmp));
    }

    return CI;

}


float computeImprovedCarryInUpperBound(const DAGTask& task, const int interval, std::vector<std::pair<float, float>> WD_UCI_y, const int m){
    //theorem 2 in the paper

    float CI_up = computeCarryInUpperBound(task, interval, WD_UCI_y);
    return std::min( CI_up, m * std::max( float(0) , interval - task.getPeriod() + task.R ));
}




void removeConflictingEdge(std::vector<SubTask*> & V, const int i, const int j, const std::vector<int> ordIDs){

    // std::cout<<"removing:"<< V[j]->id <<" to "<< V[i]->id<<std::endl;
    //removing conflicting edge
    V[i]->pred.erase(std::remove(V[i]->pred.begin(), V[i]->pred.end(), V[j]), V[i]->pred.end());
    V[j]->succ.erase(std::remove(V[j]->succ.begin(), V[j]->succ.end(), V[i]), V[j]->succ.end());

    // if V_j has no successor, link it to the end
    if(V[j]->succ.size() == 0 &&  j != ordIDs[ordIDs.size() - 1]){
        V[j]->succ.push_back(V[ordIDs.size() - 1]);
        V[ordIDs.size() - 1]->pred.push_back(V[j]);
    }
}

void convertDAGintoNFJDAG(DAGTask& t1, const int task_idx, bool save=true){

    auto V = t1.getVertices();
    auto ordIDs = t1.getTopologicalOrder();
    bool is_succ = false;
    bool confl_edge = true;
    bool same_fork = false, same_join = false;

    confl_edge = false;

    for(int idx_1=ordIDs.size() -1 , i; idx_1>=0 ; --idx_1 ){
        i = ordIDs[idx_1];
        if(V[i]->pred.size() > 1){

            // std::cout<<"join:"<<V[i]->id<<std::endl;
            // join node Vi
            for(int j = 0; j < V[i]->pred.size(); ++j){
                for(int idx_2=0, k; idx_2 < idx_1; ++idx_2 ){

                    k = ordIDs[idx_2];
                    is_succ = false;
                    t1.isSuccessor(V[i]->pred[j], V[k], is_succ);
                    if(V[k]->succ.size() > 1 && is_succ){
                        // if Vk is a fork node and Vj one of its successors

                        // std::cout<<"fork:"<<V[k]->id<<std::endl;

                        for(int s=0; s<V[i]->pred[j]->succ.size(); ++s){

                            if(V[i]->pred[j]->succ[s] == V[i])
                                continue;

                            same_fork = same_join = false;
                            t1.isSuccessor(V[i]->pred[j]->succ[s], V[k], same_fork);
                            t1.isSuccessor(V[i], V[i]->pred[j]->succ[s], same_join);
                            if(!same_fork || !same_join){
                                confl_edge = true;
                                removeConflictingEdge(V, i, V[i]->pred[j]->id, ordIDs);
                            }
                        }

                        for(int p=0; p<V[i]->pred[j]->pred.size(); ++p){

                            if(V[i]->pred[j]->pred[p] == V[k])
                                continue;

                            same_fork = same_join = false;

                            t1.isSuccessor(V[i]->pred[j]->pred[p], V[k], same_fork);
                            t1.isSuccessor(V[i], V[i]->pred[j]->pred[p], same_join);

                            if(!same_fork || !same_join){
                                confl_edge = true;
                                removeConflictingEdge(V, i, V[i]->pred[j]->id, ordIDs);
                            }
                        }
                    }
                }
            }
        }
    }

    t1.setVertices(V);

    if(save){
        t1.saveAsDot("test_transf"+std::to_string(task_idx)+".dot");
        std::string dot_command = "dot -Tpng test_transf"+std::to_string(task_idx)+".dot > test_transf"+std::to_string(task_idx)+".png";
        system(dot_command.c_str());
    }

}

std::vector<std::pair<float, float>> computeWorkloadDistributionCO(const DAGTask& t, const int task_idx){
    // algorithm 1

    //clone task
    DAGTask t1 = t;
    t1.cloneVertices(t.getVertices());

    //convert DAG into NFJ DAG
    convertDAGintoNFJDAG(t1, task_idx);
    
    // convert NFJ DAG into SP-Tree decomposition
    SPTree tree;
    tree.convertNFJDAGtoSPTree(t1, task_idx);

    //compute WD_UCO
    auto WD_UCO_y = tree.computeWDUCO(t1, task_idx);

    //destroy new task (tree will call deconstructor)
    t1.destroyVerices();

    return WD_UCO_y;
}

float computeCarryOutUpperBound(const DAGTask& task, const int interval, const int task_idx, std::vector<std::pair<float, float>> WD_UCO_y){

    //equation 9 in the paper
    float CO = 0, CO_tmp = 0;

    for(int i=0; i< WD_UCO_y.size();++i){
        CO_tmp = interval; 

        for(int j=0; j < i;++j)
            CO_tmp -= WD_UCO_y[j].first;

        CO += WD_UCO_y[i].second * std::max( float(0), std::min (WD_UCO_y[i].first, CO_tmp));
    }

    return CO;

}

float computeImprovedCarryOutUpperBound(const DAGTask& task, const float interval, std::vector<std::pair<float, float>> WD_UCO_y, const int m, const int task_idx){
    //theorem 4 in the paper

    float CO_up = computeCarryOutUpperBound(task, interval, task_idx, WD_UCO_y);
    float CO =  std::min( CO_up, interval * m);
    CO =  std::min( CO, task.getVolume() - std::max( float(0) ,task.getLength() - interval ));

    return CO;
}

float computeCarryWorkload(const DAGTask& task, const float interval, std::vector<std::pair<float, float>> WD_UCO_y, std::vector<std::pair<float, float>> WD_UCI_y, const int task_id){
    // Algorithm 2

    float WyC = computeCarryOutUpperBound(task, interval, task_id, WD_UCO_y );
    float x1 = task.getPeriod() - task.R, x2;
    float CI = 0, CO = 0;

    for(int i=WD_UCI_y.size() -1; i>=0; --i){
        x1 += WD_UCI_y[i].first;
        x2 = interval - x1;
        CI = computeCarryInUpperBound(task, x1, WD_UCI_y);
        CO = computeCarryOutUpperBound(task, x2, task_id, WD_UCO_y);
        WyC = std::max(WyC, CI + CO);
    }

    CI = computeCarryInUpperBound(task, interval, WD_UCI_y);
    WyC = std::max(WyC, CI);

    x2= 0;

    for(int i=0; i<WD_UCO_y.size(); ++i){
        x2 += WD_UCO_y[i].first;
        x1 = interval - x2;

        CI = computeCarryInUpperBound(task, x1, WD_UCI_y);
        CO = computeCarryOutUpperBound(task, x2, task_id, WD_UCO_y);
        WyC = std::max(WyC, CI + CO);

    }
    return WyC;
}

float computeDeltaC(const DAGTask& task, const float interval){
    // equation 11
    float L = task.getLength();
    float T = task.getPeriod();
    return interval - std::max( float(0), std::floor( (interval - L) / T )) * T;
}

float interTaskWorkload(const DAGTask& task, const float interval, const int task_idx){
    // equation 10

    auto WD_UCO_y = computeWorkloadDistributionCO(task, task_idx);
    auto WD_UCI_y = computeWorkloadDistributionCI(task);

    
    float delta_c = computeDeltaC(task, interval);
    float carry_workload = computeCarryWorkload(task, delta_c, WD_UCO_y, WD_UCI_y, task_idx);
    float T = task.getPeriod();
    float vol = task.getVolume();

    return carry_workload + std::max( float(0), std::floor( (interval - delta_c) /  T )) * vol;
}

bool GP_FP_FTP_Fonseca17_C(Taskset taskset, const int m){
    std::sort(taskset.tasks.begin(), taskset.tasks.end(), deadlineMonotonicSorting);

    std::vector<float> R_old (taskset.tasks.size(), 0);
    std::vector<float> R (taskset.tasks.size(), 0);
    for(int i=0; i<taskset.tasks.size(); ++i){
        R_old[i] = taskset.tasks[i].getLength();
        taskset.tasks[i].R = taskset.tasks[i].getLength();
        taskset.tasks[i].computeEFTs();
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
                    R[i] = interTaskWorkload(taskset.tasks[j], R_old[i], j); 

                R[i] *= (1. / m);
                R[i] += taskset.tasks[i].getLength() + 1. / m * (taskset.tasks[i].getVolume() - taskset.tasks[i].getLength());
            }
            else
                R[i] = R_old[i];

            init = false;
        }

        if( areEqual<float>(R[i], R_old[i]))
            taskset.tasks[i].R = R[i];
        else if (R[i] > taskset.tasks[i].getDeadline())
            return false;
    }

    return true;
}
