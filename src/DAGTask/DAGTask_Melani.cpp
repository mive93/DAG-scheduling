#include "DAGTask.h"

void DAGTask::assignWCET(const int minC, const int maxC){
    for(auto &v: V)
        v->c = intRandMaxMin(minC, maxC);
}

void DAGTask::expandTaskSeriesParallel(SubTask* source,SubTask* sink,const int depth,const int numBranches, const bool ifCond, GeneratorParams& gp){

    int depthFactor = std::max(gp.maxCondBranches, gp.maxParBranches);
    int horSpace = std::pow(depthFactor,depth);

    if(source == nullptr && sink==nullptr){
        SubTask *so = new SubTask; //source
        SubTask *si = new SubTask; //sink
        so->depth = depth;
        si->depth = -depth;
        so->width = 0;
        so->width = 0;
        si->id = 1;

        V.push_back(so);
        V.push_back(si);

        double r = ((double) rand() / (RAND_MAX));
        if (r < gp.probSCond){ //make it conditional
            int cond_branches = intRandMaxMin(2, gp.maxCondBranches);
            expandTaskSeriesParallel(V[0], V[1], depth - 1, cond_branches, true, gp);
        }
        else{
            int par_branches = intRandMaxMin(2, gp.maxParBranches);
            expandTaskSeriesParallel(V[0], V[1], depth - 1, par_branches, false, gp);
        }
    }
    else{
        float step = horSpace / (numBranches - 1);
        float w1 = (source->width - horSpace / 2);
        float w2 = (sink->width - horSpace / 2);

        for(int i=0; i<numBranches; ++i){
            
            // int current = V.size();
            creationStates state = TERMINAL_T;
            if (depth != 0) state = static_cast<creationStates>(gp.dist(gp.gen));

            switch (state){
            case TERMINAL_T:{
                SubTask *v = new SubTask;
                v->id = V.size();
                v->pred.push_back(source);
                v->succ.push_back(sink);
                v->mode = ifCond? C_INTERN_T : NORMAL_T;
                v->depth = depth;
                v->width = w1 + step * (i - 1);

                V.push_back(v);

                source->mode    = ifCond ? C_SOURCE_T : NORMAL_T;
                sink->mode      = ifCond ? C_SINK_T : NORMAL_T;
                source->succ.push_back(V[V.size()-1]);
                
                sink->pred.push_back(V[V.size()-1]);
                break;
            }
            case PARALLEL_T: case CONDITIONAL_T:{
                SubTask *v1 = new SubTask;
                v1->id = V.size();
                v1->pred.push_back(source);
                v1->mode = ifCond? C_INTERN_T : NORMAL_T;
                v1->depth = depth;
                v1->width = w1 + step * (i - 1);
                V.push_back(v1);

                source->succ.push_back(V[V.size()-1]);
                source->mode = ifCond ? C_SOURCE_T : NORMAL_T;

                SubTask *v2 = new SubTask;
                v2->id = V.size();
                v2->succ.push_back(sink);
                v2->mode = ifCond? C_INTERN_T : NORMAL_T;
                v2->depth = -depth;
                v2->width = w2 + step * (i - 1);
                V.push_back(v2);

                sink->pred.push_back(V[V.size()-1]);
                sink->mode = ifCond ? C_SINK_T : NORMAL_T;

                int max_branches = (state == PARALLEL_T )? gp.maxParBranches : gp.maxCondBranches;
                float cond = (state == PARALLEL_T) ? false: true;

                int branches = intRandMaxMin(2, max_branches);
            
                expandTaskSeriesParallel(V[V.size()-2], V[V.size()-1], depth - 1, branches, cond, gp);
            
                break;
            }
            
            default:
                break;
            }
        }
    }
}

void DAGTask::makeItDag(float prob){
    bool is_already_succ= false;
    std::vector<int> v_cond_pred;
    std::vector<int> w_cond_pred;
    for(auto &v:V){
        v_cond_pred  = v->getCondPred();

        for(auto &w:V){
            w_cond_pred = w->getCondPred();
            is_already_succ = false;

            isSuccessor(w, v, is_already_succ);

            if( v->depth > w->depth &&
                v->mode != C_SOURCE_T &&
                !is_already_succ &&
                w_cond_pred.size() == v_cond_pred.size() && 
                std::equal(v_cond_pred.begin(), v_cond_pred.end(), w_cond_pred.begin()) && 
                ((double) rand() / (RAND_MAX)) < prob
            )
            {
                //add an edge v -> w
                v->succ.push_back(w);
                w->pred.push_back(v);
            }
        }
    }
}



void DAGTask::assignSchedParametersUUniFast(const float U){
    t = std::ceil(wcw / U);
    d = t;    
}

void DAGTask::assignSchedParameters(const float beta){
    float Tmin = L;
    float Tmax = wcw / beta;
    t = floatRandMaxMin(Tmin, Tmax);
    d = floatRandMaxMin(Tmin, t);
}

void DAGTask::assignFixedSchedParameters(const float period, const float deadline){
    t = period;
    d = deadline;
}