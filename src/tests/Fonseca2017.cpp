#include "tests.h"

//José Fonseca et al. “Improved response time analysis of sporadic dag tasks for global fp scheduling”.  (RTNS 2017) 

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

    printPairVector<float, float> (WD, "WD");

    return WD;

}

float computeCarryInUpperBound(const DAGTask& task, const int interval){

    //equation 6 in the paper

    std::vector<std::pair<float, float>> WD_UCI_y = computeWorkloadDistributionCI(task);
    float CI = 0, CI_tmp = 0;

    for(int i=0; i< WD_UCI_y.size();++i){
        CI_tmp = interval - task.getPeriod() + task.R; 

        for(int j=i+1; j< WD_UCI_y.size();++j)
            CI_tmp -= WD_UCI_y[j].first;

        CI += WD_UCI_y[i].second * std::max( float(0), std::min (WD_UCI_y[i].first, CI_tmp));
    }

    return CI;

}

std::vector<std::pair<float, float>> computeWorkloadDistributionCO(const DAGTask& task){
    // algorithm 1
    //TODO
}


float computeImprovedCarryInUpperBound(const DAGTask& task, const int interval, const int m){
    //theorem 2 in the paper

    float CI_up = computeCarryInUpperBound(task, interval);
    return std::min( CI_up, m * std::max( float(0) , interval - task.getPeriod() + task.R ));
}

float computeCarryOutUpperBound(const DAGTask& task, const int interval){

    //equation 9 in the paper

    std::vector<std::pair<float, float>> WD_UCO_y = computeWorkloadDistributionCO(task);
    float CO = 0, CO_tmp = 0;

    for(int i=0; i< WD_UCO_y.size();++i){
        CO_tmp = interval; 

        for(int j=0; j < i;++j)
            CO_tmp -= WD_UCO_y[j].first;

        CO += WD_UCO_y[i].second * std::max( float(0), std::min (WD_UCO_y[i].first, CO_tmp));
    }

    return CO;

}

float computeImprovedCarryOutUpperBound(const DAGTask& task, const float interval, const int m){
    //theorem 4 in the paper

    float CO_up = computeCarryOutUpperBound(task, interval);
    float CO =  std::min( CO_up, interval * m);
    CO =  std::min( CO, task.getVolume() - std::max( float(0) ,task.getLength() - interval ));

    return CO;
}

float computeCarryWorkload(const DAGTask& task, const float interval, const int m){
    // Algorithm 2
    //TODO
}

float computeDeltaC(const DAGTask& task, const float interval){
    // equation 11
    float L = task.getLength();
    float T = task.getPeriod();
    return interval - std::max( float(0), std::floor( (interval - L) / T )) * T;
}

float interTaskWorkload(const DAGTask& task, const float interval, const int m){
    // equation 10

    float carry_workload = computeCarryWorkload(task, interval, m);
    float delta_c = computeDeltaC(task, interval);
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


    taskset.tasks[0].R = 15;
    std::cout<<computeCarryInUpperBound(taskset.tasks[0], 4)<<std::endl;

    return  false;

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
                    R[i] = interTaskWorkload(taskset.tasks[j], R_old[i], m); //TODO: fixme

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
