#include "tests.h"

//Risat Pathan et al.  “Scheduling parallel real-time recurrent tasks on multicore platforms”. (IEEE Transactions on Parallel and Distributed Systems 2017)

std::vector<int> computeSxi(const DAGTask& tau_x, const int k ){
    //topological order to priorities nodes
    std::vector<int> topo_ord = tau_x.getTopologicalOrder();
    auto V = tau_x.getVertices();
    int idx = 0;
    bool is_succ = false;

    std::vector<int> S;
    for(int i=0; i<topo_ord.size(); ++i){
        idx = topo_ord[i];
        //if the node is before in topological order
        if(idx == k) break;

        is_succ = false;
        
        //and if it is not one of v_k ancestors
        tau_x.isSuccessor(V[k], V[idx], is_succ);
        if(!is_succ)
            S.push_back(idx);
    }

    return S;
}

float computeLatestReadyTime(const std::vector<SubTask *>& ancst_i){
    float max_R = 0;

    for(int j=0; j< ancst_i.size(); ++j){
        if(ancst_i[j]->r > max_R)
            max_R = ancst_i[j]->r;
    }

    return max_R;
}

float computeWorloadIntra(const DAGTask& tau_x, const int k ){
    std::vector<int> S = computeSxi(tau_x, k);
    auto V = tau_x.getVertices();
    
    float W_intra = 0;
    float R_part = 0;
    int idx = 0;
    for(int i=0; i< S.size(); ++i){
        idx = S[i];

        R_part = std::max( float(0) , V[idx]->r - computeLatestReadyTime(V[k]->pred) );
        W_intra += std::min(V[idx]->c, R_part);
    }

    return W_intra;
}

float computeX(const DAGTask& tau_y, const DAGTask& tau_x, const int k, const float interval, const int m ){
    auto V = tau_x.getVertices();
    float ci_b = computeLatestReadyTime(V[k]->pred) + interval - tau_y.getWCW() / m;
    return std::max( float(0), ci_b );
}

float computeTcin(const DAGTask& tau_y, const DAGTask& tau_x, const int k, const float interval, const int m ){

    float X_y = computeX(tau_y, tau_x, k, interval, m);
    float W_y = tau_x.getWCW();
    float T_y = tau_x.getPeriod();

    float ci = interval - (T_y - tau_y.R) - std::floor ( X_y / T_y ) * T_y - W_y / m;
    return std::max( float(0), ci);

}

float computeCR(const DAGTask& tau_y, const DAGTask& tau_x, const int k, const float interval, const int m ){
    auto V_x = tau_x.getVertices();
    auto V_y = tau_y.getVertices();
    float A = 0;

    for(int i=0; i<V_y.size(); ++i)
        A += std::min(V_y[i]->c, std::max(float(0), V_y[i]->r - computeLatestReadyTime(V_x[k]->pred)));
    
    return std::min (m * computeTcin(tau_y, tau_x, k, interval, m) , A);
}

float computeWorloadInter(const Taskset& taskset, const int x, const int i, const float interval, const int m){

    //for all hp of tau_x
    float W_inter = 0;
    float T_y = 0, W_y = 0, t_cin_y = 0, CR_y = 0, X_y = 0;
    for(int y=0; y < x; ++y){

        T_y = taskset.tasks[y].getPeriod();
        W_y = taskset.tasks[y].getWCW();
        t_cin_y = computeTcin(taskset.tasks[y], taskset.tasks[x], i, interval, m);
        X_y = computeX(taskset.tasks[y], taskset.tasks[x], i, interval, m);
        CR_y = computeCR(taskset.tasks[y], taskset.tasks[x], i, t_cin_y, m);

        W_inter += CR_y + std::floor( X_y / T_y ) * W_y + W_y;

        //NB: doesn't make much sense to account for all the workload of W_y: here's the pessimism
    }

    return W_inter;
}

bool GP_FP_DM_Pathan17_C(Taskset taskset, const int m){

    std::sort(taskset.tasks.begin(), taskset.tasks.end(), deadlineMonotonicSorting);

    for(int x=0; x<taskset.tasks.size(); ++x){

        std::vector<int> topo_ord = taskset.tasks[x].getTopologicalOrder();
        auto V = taskset.tasks[x].getVertices();

        std::vector<float> R_old (V.size(), 0);
        std::vector<float> R (V.size(), 0);


        // compute the response of a subtask in topological order
        for(int idx=0, i; idx<topo_ord.size(); ++idx){
            i = topo_ord[idx];

            R_old[i] = V[i]->c;
            V[i]->r = V[i]->c;
            
            if(R_old[i] > taskset.tasks[x].getDeadline())
                return false;

            bool init = true;
            float W_intra = 0, W_inter = 0;
            while(!areEqual<float>(R[i], R_old[i]) && R[i] <= taskset.tasks[x].getDeadline()){
                if(!init){
                    R_old[i] = R[i];
                    R[i] = 0;
                }

                W_inter = computeWorloadInter(taskset, x, i, R_old[i], m);
                W_intra = computeWorloadIntra(taskset.tasks[x], i);

                R[i] += computeLatestReadyTime(V[i]->pred) + (1. / m) * (W_intra + W_inter) + V[i]->c;

                init = false;
            }

            if(R[i] > taskset.tasks[x].getDeadline())
                return false;
            V[i]->r = R[i];
        }

        // the response time of the task is the max response time of one of its nodes
        taskset.tasks[x].R = 0;
        for(int i=0; i<V.size(); ++i){
            if(V[i]->r > taskset.tasks[x].R)
                taskset.tasks[x].R = V[i]->r;
        }

        if (taskset.tasks[x].R > taskset.tasks[x].getDeadline())
            return false;
        
    }
    return true;
    
}