#include "tests.h"

//José Fonseca et al. “Improved response time analysis of sporadic dag tasks for global fp scheduling”.  (RTNS 2017) 

std::vector<std::pair<float, float>> computeWorkloadDistribution(const DAGTask& task){

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

float computeCarriInUpperBound(const Taskset& taskset, const int y, const int interval){

    //equation 6 in the paper

    std::vector<std::pair<float, float>> WD_y = computeWorkloadDistribution(taskset.tasks[y]);
    float CI = 0, CI_tmp = 0;

    for(int i=0; i< WD_y.size();++i){
        CI_tmp = interval - taskset.tasks[y].getPeriod() + taskset.tasks[y].R; 

        for(int j=i+1; j< WD_y.size();++j)
            CI_tmp -= WD_y[j].first;

        CI += WD_y[i].second * std::max( float(0), std::min (WD_y[i].first, CI_tmp));
    }

    return CI;

}

float interTaskWorkload(){
    
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
    std::cout<<computeCarriInUpperBound(taskset, 0, 4)<<std::endl;

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
                    R[i] = interTaskWorkload();

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
