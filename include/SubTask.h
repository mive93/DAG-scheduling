#ifndef SUBTASK_H
#define SUBTASK_H

#include <vector>

enum subTaskMode {NORMAL_T, C_INTERN_T, C_SOURCE_T, C_SINK_T};

class SubTask{

    public:

    int id      = 0; // sub-task id
    int c       = 0; // WCET
    int width   = 0; // width in the graph
    int depth   = 0; // depth in the graph
    int accWork = 0; // accumulated workload

    int localO = 0; // local offset
    int localD = 0; // local deadline

    subTaskMode mode = NORMAL_T;    // type of node

    std::vector<SubTask*> desc;     // descendants 
    std::vector<SubTask*> ancst;    // ancestors

    std::vector<int> getCondPred(){
        std::vector<int> cond_pred;
        for(const auto& p:ancst){
            if(p->mode == C_SOURCE_T)
                cond_pred.push_back(p->id);
        }
        return cond_pred;
    }

    void localOffset(){
        if(ancst.size() == 0)
            localO = 0;
        else{
            localO = 0;
            int temp_local_o = 0;
            for(int i=0; i<ancst.size();++i){
                temp_local_o = ancst[i]->localO + ancst[i]->c;
                if(temp_local_o > localO) localO = temp_local_o;
            }
        }
    }

    void localDeadline(const int task_deadline){
        if(desc.size() == 0)
            localD = task_deadline;
        else{
            localD = 99999;
            int temp_local_d = 0;
            for(int i=0; i<desc.size();++i){
                temp_local_d = desc[i]->localD - desc[i]->c;
                if(temp_local_d < localD) localD = temp_local_d;
            }
        }
    }
   
};


#endif /* SUBTASK_H */