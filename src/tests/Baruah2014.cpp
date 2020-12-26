#include "tests.h"

// Improved Multiprocessor Global Schedulability Analysis of Sporadic DAG Task Systems (ECRTS 2014)

float work(const DAGTask& t, const float interval, const float sigma){
    DAGTask t1 = t;
    t1.cloneVertices(t.getVertices());

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

    t1.destroyVerices();
    return work_part + work_full;
}

float work(const Taskset& taskset, const float interval, const float sigma ){
    float w = 0;

    for(auto& t:taskset.tasks)
        w += work(t, interval, sigma);

    return w;
}

std::vector<float> getTestingSet(const Taskset& taskset, const float sigma, const float bound ){
    std::set<float> interval_set;

    for(const auto&task: taskset.tasks){
        
        std::vector<float> lambda_set;
        std::vector<float> t_lambda_up;
        std::vector<float> t_lambda_down;
        std::vector<std::pair<float, int>> l_table;
        for(const auto&v: task.getVertices()){
            t_lambda_up.push_back(v->localO);
            t_lambda_down.push_back(v->localO + v->c);
        }

        std::sort(t_lambda_up.begin(), t_lambda_up.end());
        std::sort(t_lambda_down.begin(), t_lambda_down.end());

        bool pres = false;
        int pres_idx = -1;

        l_table.push_back(std::make_pair<float, int>(0.,1));

        for(int i=1; i<t_lambda_up.size();++i){
            pres = false;
            for(int j=0; j< l_table.size(); ++j){
                if(t_lambda_up[i] == l_table[j].first){
                    pres = true;
                    pres_idx = j;
                }
            }

            if(pres)
                l_table[pres_idx].second++;
            else
                l_table.push_back(std::make_pair<float, int>(float(t_lambda_up[i]),1));
        }

        for(int i=1; i<l_table.size();++i){
            l_table[i].second += l_table[i-1].second;
        }
  
        for(int i=0; i<t_lambda_down.size();++i){
            pres = false;
            for(int j=0; j< l_table.size(); ++j){
                if(t_lambda_down[i] == l_table[j].first){
                    pres = true;
                    pres_idx = j;
                }
            }

            if(pres){
                l_table[pres_idx].second--;
                for(int j=pres_idx +1 ; j<l_table.size();++j)
                    l_table[j].second--;
            }
            else
            {
                int lev = 0;
                for(int j=0; j<l_table.size();++j){
                    if(t_lambda_down[i] >= l_table[j].first)
                        lev = j;
                }

                for(int j=lev +1 ; j<l_table.size();++j)
                    l_table[j].second--;

                l_table.push_back(std::make_pair<float, int>(float(t_lambda_down[i]),l_table[lev].second -1));

                std::sort(l_table.begin(), l_table.end());
            }
        }

        lambda_set.push_back(l_table[0].first);
        for(int i=1; i<l_table.size();++i){
            if(l_table[i].second != l_table[i-1].second)
                lambda_set.push_back(l_table[i].first);
        }

        bool exit = false;

        std::vector<float> new_V;

        for(int n=0; true; n++){
            for(int i=0; i<lambda_set.size();++i){            
                new_V.push_back(lambda_set[i]/sigma + n*task.getPeriod());
            }

            for(int i=0; i<new_V.size();++i){
                if( areEqual<float>(new_V[i], bound) ||  new_V[i] < bound)
                    interval_set.insert(new_V[i]);
                else{
                    exit = true;
                    break;
                }
            }

            if(exit) break;
        }
    }

    std::vector<float> intervals;

    for(const auto& i:interval_set)
        intervals.push_back(i);

    std::sort(intervals.begin(), intervals.end());
    return intervals;
}

bool GP_FP_EDF_Baruah2014_C(Taskset taskset, const int m){

    std::sort(taskset.tasks.begin(), taskset.tasks.end(), deadlineMonotonicSorting);

    for(auto& task:taskset.tasks){
        if(!(task.getDeadline() <= task.getPeriod()))
            FatalError("This test requires constrained deadline tasks");

        task.computeLocalOffsets();
    }

    bool sched = true;
    float sigma_tmp = taskset.getMaxDensity();
    float sigma_inc = 0.025;
    float eps = 5e-8;
    float interval_tmp = 0;
    float U = taskset.getUtilization();
    float HP = taskset.getHyperPeriod();

    float b1 = 0, b2 = 0;

    float tot_vol = 0;

    for(auto& t:taskset.tasks)
        tot_vol += t.getVolume();

    while(true){
        if (sigma_tmp > ( m - U - eps ) / ( m - 1 ))
            return false;
            
        b2 = tot_vol / ( m - (m-1) * sigma_tmp - U);

        b1 = std::fmin(HP, b2);


        std::vector<float> ts = getTestingSet(taskset, sigma_tmp, b1);

        for(int i=0; i< ts.size(); ++i){
            if (ts[i] >= interval_tmp){
                float w = work(taskset, ts[i], sigma_tmp);
                if( w > ts[i] * ( m - (m-1) * sigma_tmp) ){
                    interval_tmp = ts[i];
                    sched = false;
                    break;
                }
            }
        }

        if(sched) return true;
        sigma_tmp += sigma_inc;
    }
}
