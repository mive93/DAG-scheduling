#include "SubTask.h"

std::vector<int> SubTask::getCondPred(){
    std::vector<int> cond_pred;
    for(const auto& p:pred){
        if(p->mode == C_SOURCE_T)
            cond_pred.push_back(p->id);
    }
    return cond_pred;
}

void SubTask::localOffset(){
    if(pred.size() == 0)
        localO = 0;
    else{
        localO = 0;
        float temp_local_o = 0;
        for(int i=0; i<pred.size();++i){
            temp_local_o = pred[i]->localO + pred[i]->c;
            if(temp_local_o > localO) localO = temp_local_o;
        }
    }
}

void SubTask::EasliestFinishingTime(){
    if(localO == -1)
        FatalError("Requires local offsets to be computed");
    EFT = localO + c;
}

void SubTask::localDeadline(const float task_deadline){
    if(succ.size() == 0)
        localD = task_deadline;
    else{
        localD = 99999;
        float temp_local_d = 0;
        for(int i=0; i<succ.size();++i){
            temp_local_d = succ[i]->localD - succ[i]->c;
            if(temp_local_d < localD) localD = temp_local_d;
        }
    }
}

void SubTask::LatestStartingTime(){
    if(localD == -1)
        FatalError("Requires local deadlines to be computed");
    LST = localD - c;
}
   