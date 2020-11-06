#include "DAGTask.h"

void DAGTask::configureParams(){
    maxCondBranches = 2;
    maxParBranches  = 6;
    p_cond          = 0; 
    p_par           = 0.2;
    p_term          = 0.8;
    rec_depth       = 2;
    Cmin            = 1;
    Cmax            = 10;
    addProb         = 0.1;
    probSCond       = 0.5;

    weights.push_back(p_cond);
    weights.push_back(p_par);
    weights.push_back(p_term);
    dist.param(std::discrete_distribution<int> ::param_type(std::begin(weights), std::end(weights)));

    // gen.seed(time(0));
    gen.seed(1);
}

void DAGTask::assignWCET(const int minC, const int maxC){
    for(auto &v: V)
        v->c = rand()%maxC + minC;
}

void DAGTask::expandTaskSeriesParallel(SubTask* source,SubTask* sink,const int depth,const int numBranches, const bool ifCond){

    int depthFactor = std::max(maxCondBranches, maxParBranches);
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
        if (r < probSCond){ //make it conditional
            int cond_branches = rand() % maxCondBranches + 2;
            expandTaskSeriesParallel(V[0], V[1], depth - 1, cond_branches, true);
        }
        else{
            int par_branches = rand() % maxParBranches + 2;
            expandTaskSeriesParallel(V[0], V[1], depth - 1, par_branches, false);
        }
    }
    else{
        float step = horSpace / (numBranches - 1);
        float w1 = (source->width - horSpace / 2);
        float w2 = (sink->width - horSpace / 2);

        for(int i=0; i<numBranches; ++i){
            
            // int current = V.size();
            creationStates state = TERMINAL_T;
            if (depth != 0) state = static_cast<creationStates>(dist(gen));

            switch (state){
            case TERMINAL_T:{
                SubTask *v = new SubTask;
                v->id = V.size();
                v->prec.push_back(source);
                v->succ.push_back(sink);
                v->mode = ifCond? C_INTERN_T : NORMAL_T;
                v->depth = depth;
                v->width = w1 + step * (i - 1);

                V.push_back(v);

                source->mode    = ifCond ? C_SOURCE_T : NORMAL_T;
                sink->mode      = ifCond ? C_SINK_T : NORMAL_T;
                source->succ.push_back(V[V.size()-1]);
                
                sink->prec.push_back(V[V.size()-1]);
                break;
            }
            case PARALLEL_T: case CONDITIONAL_T:{
                SubTask *v1 = new SubTask;
                v1->id = V.size();
                v1->prec.push_back(source);
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

                sink->prec.push_back(V[V.size()-1]);
                sink->mode = ifCond ? C_SINK_T : NORMAL_T;

                int max_branches = (state == PARALLEL_T )? maxParBranches : maxCondBranches;
                float cond = (state == PARALLEL_T) ? false: true;

                int branches = rand() % max_branches + 2;               
            
                expandTaskSeriesParallel(V[V.size()-2], V[V.size()-1], depth - 1, branches, cond);
            
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
                w->prec.push_back(v);
            }
        }
    }
}

void DAGTask::computeAccWorkload(){
    auto ordIDs = topologicalSort();
    if(!checkIndexAndIdsAreEqual())
        FatalError("Ids and Indexes do not correspond, can't use computed topological order!");
    if(ordIDs.size() != V.size())
        FatalError("Ids and V sizes differ!");

    int max_acc_prec;
    for(size_t i=0; i<ordIDs.size();++i){
        max_acc_prec = 0;
        for(size_t j=0; j<V[ordIDs[i]]->prec.size();++j){
            if(V[ordIDs[i]]->prec[j]->accWork > max_acc_prec)
                max_acc_prec = V[ordIDs[i]]->prec[j]->accWork;
        }

        V[ordIDs[i]]->accWork = V[ordIDs[i]]->c + max_acc_prec;
    }
}

DAGTask DAGTask::generateTaskMelani(){
    configureParams();
    expandTaskSeriesParallel(nullptr, nullptr,rec_depth,0,false);
    assignWCET(Cmin, Cmax);
    makeItDag(addProb);
    computeAccWorkload();

    for(const auto&v :V)
        if(v->accWork > L)
            L = v->accWork;

    std::cout<<"length = "<<L<<std::endl;
    
    return *this;
}
