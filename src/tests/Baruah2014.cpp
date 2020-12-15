#include "tests.h"

// Improved Multiprocessor Global Schedulability Analysis of Sporadic DAG Task Systems (ECRTS 2014)

float work(const DAGTask& t, const float interval, const float sigma){
    DAGTask t1 = t;
    for(auto& v:t1.getVertices())
        v->c /= sigma;

    t1.computeVolume();
    t1.computeAccWorkload();
    t1.computeLength();
    t1.computeLocalOffsets();

    float n_full = 0, n_part = 0, work_part = 0;

    if( interval >= t.getDeadline()){
        n_full = std::floor((interval - t.getDeadline()) / t.getPeriod()) + 1;
        n_part = std::ceil(interval / t.getPeriod()) - n_full;
    }

    float work_full = n_full * t1.getVolume();

    float curr_d = interval - (n_full - 1) * t.getPeriod();
    float curr_r = interval - (t.getDeadline() + (n_full - 1) * t.getPeriod());
    float vert_r = 0;

    for(int i=0; i<n_part; ++i){
        curr_d -= t.getPeriod();
        curr_r -= t.getDeadline();

        for(auto v1: t1.getVertices()){
            vert_r = curr_r + v1->localO;

            if(vert_r  >= 0)
                work_part += v1->c;
            else if(vert_r + v1->c > 0)
                work_part += vert_r + v1->c;
        }
    }

    std::cout<<work_full<<" "<<work_part<<std::endl;

    return work_part + work_full;
}

bool G_EDF_Baruah2014_C(const Taskset& taskset, const int m){

    float cwork = work(taskset.tasks[0], 23, 0.46);

    std::cout<<cwork<<std::endl;

}
