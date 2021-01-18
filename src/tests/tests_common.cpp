#include "tests.h"

bool deadlineMonotonicSorting (const DAGTask& tau_x, const DAGTask& tau_y) { 
    return (tau_x.getDeadline() < tau_y.getDeadline()); 
}

bool sortPairProcInc(const std::pair<int, float>& a, const std::pair<int, float>& b){
    return a.second < b.second; 
}

bool sortPairProcDec(const std::pair<int, float>& a, const std::pair<int, float>& b){
    return a.second > b.second; 
}

std::vector<int> getCandidatesProcInOrder(const std::vector<float>& proc_util, const float cur_util, const PartitioningCoresOrder_t& c_order){

    std::vector<int> candidates_cores;

    std::vector<std::pair<int, float>> proc_val;

    int min_proc_util_diff = 1;
    int min_proc_util = 1;
    int min_idx = -1;

    switch (c_order){
    case PartitioningCoresOrder_t::FIRST_FIT:
        //first processor that can accomodate the current util
        for(int p=0; p<proc_util.size(); ++p)
            if( 1 - proc_util[p] >= cur_util ) // can accomodate 
                candidates_cores.push_back(p);
        // no sorting, first fit
        break;
    case PartitioningCoresOrder_t::BEST_FIT:
        //processor that has the free slot most similar to the current util
        for(int p=0; p<proc_util.size(); ++p)
            if( 1 - proc_util[p] >= cur_util                            // can accomodate             
                && 1 - proc_util[p] - cur_util < min_proc_util_diff )       // the difference is smaller than in other processor (best fit)
                proc_val.push_back(std::make_pair(p, proc_util[p] - cur_util));
        std::sort(proc_val.begin(), proc_val.end(), sortPairProcInc);   // sorting for increasing difference (best fit)
        for(const auto& pv:proc_val)
            candidates_cores.push_back(pv.first);
        break;

    case PartitioningCoresOrder_t::WORST_FIT:
        //processor that has the biggest free slot
        for(int p=0; p<proc_util.size(); ++p)
            if( 1 - proc_util[p] >= cur_util                            // can accomodate
                && proc_util[p] < min_proc_util )                       // is the one with least used utilization (worst-fit)
                proc_val.push_back(std::make_pair(p, 1 - proc_util[p]));
        std::sort(proc_val.begin(), proc_val.end(), sortPairProcDec);   // sorting for decreasing left capacity (worst fit)
        for(const auto& pv:proc_val)
            candidates_cores.push_back(pv.first);
        break;
    
    }
    return candidates_cores;
}

class node_infos{
    public:
    int task_id = 0;
    int v_id = 0;
    float utilization = 0;
    float density = 0;

    void print(){
        std::cout<<"x: "<<task_id<<" i: "<<v_id<<" U: "<<utilization<<" dens: "<<density<<std::endl;
    }
};

bool sortUtilDec(const node_infos& a, const node_infos& b){
    return a.utilization > b.utilization; 
}

bool sortDensDec(const node_infos& a, const node_infos& b){
    return a.density > b.density; 
}

bool sortUtilDecDensDec(const node_infos& a, const node_infos& b){
    if(areEqual<float>(a.utilization,  b.utilization))
        return a.density > b.density; 
    return a.utilization > b.utilization;
}

bool sortDensDecUtilDec(const node_infos& a, const node_infos& b){
    if(areEqual<float>(a.density,  b.density))
        return a.utilization > b.utilization; 
    return a.density > b.density ;
}

bool WorstFitProcessorsAssignment(Taskset& taskset, const int m){

    std::vector<float> proc_util (m,0);
    float min_proc_util = 1;
    int min_idx = -1;

    std::vector<node_infos> taskset_nodes;

    for(int x=0; x<taskset.tasks.size();++x){
        std::vector<SubTask*> V = taskset.tasks[x].getVertices();
        for(int i=0; i<V.size(); ++i){
            node_infos n;
            n.task_id = x;
            n.v_id = i;
            n.density = taskset.tasks[x].getLength() / taskset.tasks[x].getDeadline();
            n.utilization = V[i]->c / taskset.tasks[x].getPeriod();

            taskset_nodes.push_back(n);
        }
    }

    std::sort(taskset_nodes.begin(), taskset_nodes.end(), sortUtilDecDensDec);

    for(int i=0; i<taskset_nodes.size(); ++i){

        min_proc_util = 1;
        for(int p=0; p<proc_util.size(); ++p){
            if( 1 - proc_util[p] >= taskset_nodes[i].utilization                // can accomodate
                && proc_util[p] < min_proc_util){           // is the one with least used utilization (worst-fit)
                min_proc_util = proc_util[p];
                min_idx = p;
            }
        }

        if(min_idx == -1)
            return false;

        SubTask *v = taskset.tasks[taskset_nodes[i].task_id].getVertices()[taskset_nodes[i].v_id];
        v->core = min_idx;
        proc_util[min_idx] += taskset_nodes[i].utilization;    

    }

    return true;
}

bool BestFitProcessorsAssignment(Taskset& taskset, const int m){

    std::vector<float> proc_util (m,0);
    float min_proc_util = 1;
    int min_idx = -1;

    float cur_util;
    for(int x=0; x<taskset.tasks.size();++x){
        std::vector<SubTask*> V = taskset.tasks[x].getVertices();
        for(int i=0; i<V.size(); ++i){
            cur_util = V[i]->c / taskset.tasks[x].getPeriod();

            min_proc_util = 1;
            min_idx = -1;
            for(int p=0; p<proc_util.size(); ++p){
                // std::cout<<"proc: "<<p<<" u: "<<proc_util[p]<<" cur_util: "<<cur_util<<std::endl;
                if( 1 - proc_util[p] >= cur_util                // can accomodate
                    && 1 - proc_util[p] - cur_util < min_proc_util){           // is the one with least used utilization (worst-fit)
                    min_proc_util = 1 - proc_util[p] - cur_util;
                    min_idx = p;
                }
            }

            if(min_idx == -1)
                return false;

            V[i]->core = min_idx;
            proc_util[min_idx] += cur_util;            
        }

        // std::cout<<taskset.tasks[x];
    }



    return true;
}