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

void DAGTask::computeWorstCaseWorkload(){
    // Algorithm 1, Melani et al. "Response-Time Analysis of Conditional DAG Tasks in Multiprocessor Systems"
    std::vector<std::set<int>> paths (V.size());
    paths[ordIDs[ordIDs.size()-1]].insert(ordIDs[ordIDs.size()-1]);
    int idx;
    for(int i = ordIDs.size()-2; i >= 0; --i ){
        idx = ordIDs[i];
        paths[idx].insert(idx);

        if(V[idx]->succ.size()){
            if(V[idx]->mode != C_SOURCE_T){
                for(int j=0; j<V[idx]->succ.size(); ++j)
                    paths[idx].insert(paths[V[idx]->succ[j]->id].begin(), paths[V[idx]->succ[j]->id].end());
            }
            else{
                std::vector<int> sum (V[idx]->succ.size(), 0);
                int max_id = 0;
                int max_sum = 0;
                for(int j=0; j<V[idx]->succ.size(); ++j){
                    for(auto k: paths[V[idx]->succ[j]->id])
                        sum[j] += V[k]->c;
                    if(sum[j] > max_sum){
                        max_sum = sum[j];
                        max_id = j;
                    }
                }
                paths[idx].insert(paths[V[idx]->succ[max_id]->id].begin(), paths[V[idx]->succ[max_id]->id].end());
            }
        }
    }

    wcw = 0;
    for(auto i:paths[ordIDs[0]]){
        wcw += V[i]->c;
    }
}

int DAGTask::computeZk(const int n_proc){
    // Algorithm 2, Melani et al. "Response-Time Analysis of Conditional DAG Tasks in Multiprocessor Systems"
    std::vector<std::set<int>> S (V.size());
    std::vector<std::set<int>> T (V.size());
    std::vector<int> f (V.size());
    S[ordIDs[ordIDs.size()-1]].insert(ordIDs[ordIDs.size()-1]);
    T[ordIDs[ordIDs.size()-1]].insert(ordIDs[ordIDs.size()-1]);
    f[ordIDs[ordIDs.size()-1]] = V[ordIDs[ordIDs.size()-1]]->c;
    
    int idx;
    int C = 0;
    std::set<int> D;
    for(int i = ordIDs.size()-2; i >= 0; --i ){
        idx = ordIDs[i];

        if(V[idx]->succ.size()){
            S[idx].insert(idx);
            if(V[idx]->mode != C_SOURCE_T){ //if not conditional source
                std::vector<int> U (V[idx]->succ.size(),0);
                for(int j=0; j<V[idx]->succ.size(); ++j){
                    S[idx].insert(S[V[idx]->succ[j]->id].begin(), S[V[idx]->succ[j]->id].end());
                    U[j] = f[V[idx]->succ[j]->id];
                    for(int k=0; k<V[idx]->succ.size(); ++k){
                        if(k!=j){
                            C = 0;
                            D.clear();
                            std::set_difference(S[V[idx]->succ[k]->id].begin(), 
                                                S[V[idx]->succ[k]->id].end(),
                                                S[V[idx]->succ[j]->id].begin(), 
                                                S[V[idx]->succ[j]->id].end(), 
                                                std::inserter(D, D.begin()) );
                        }
                        for (const auto& d:D){
                            C+= V[d]->c / n_proc;
                        }

                        U[j]+=C;
                    }
                }
                int max_U_idx = std::max_element(U.begin(),U.end()) - U.begin();
                T[idx].insert(idx);
                T[idx].insert(T[V[idx]->succ[max_U_idx]->id].begin(), T[V[idx]->succ[max_U_idx]->id].end());
                f[idx] = V[idx]->c + U[max_U_idx];
            }
            else{
                std::vector<int> C (V[idx]->succ.size(),0);
                std::vector<int> ff (V[idx]->succ.size(),0);

                for(int j=0; j<V[idx]->succ.size(); ++j){
                    for(auto k: S[V[idx]->succ[j]->id]){
                        C[j] += V[k]->c;
                    }

                    ff[j] = f[V[idx]->succ[j]->id];
                }

                int max_C_idx = std::max_element(C.begin(),C.end()) - C.begin();
                S[idx].insert(S[V[idx]->succ[max_C_idx]->id].begin(), S[V[idx]->succ[max_C_idx]->id].end());

                int max_ff_idx = std::max_element(ff.begin(),ff.end()) - ff.begin();
                T[idx].insert(idx);
                T[idx].insert(T[V[idx]->succ[max_ff_idx]->id].begin(), T[V[idx]->succ[max_ff_idx]->id].end());

                f[idx] = V[idx]->c + f[V[idx]->succ[max_ff_idx]->id];

            }
        }
    }
    return f[ordIDs[0]];
}

void DAGTask::maximazieMakespan(const SubTask* v, const std::vector<int>& mksp, const std::vector<std::set<int>>& mksp_set,  const std::vector<std::set<int>>& w_set,  std::set<int>& mkspset_tmp, float& max_mksp, const int n_proc){
    max_mksp = 0;
    for(int j=0; j<v->succ.size(); ++j){
        int sum_w = 0;
        std::set<int> wset_tmp;

        for(int k=0; k<v->succ.size(); ++k){
            if(k != j){
                std::set<int> new_vert;
                std::set_difference(w_set[v->succ[k]->id].begin(), 
                                    w_set[v->succ[k]->id].end(),
                                    wset_tmp.begin(), 
                                    wset_tmp.end(), 
                                    std::inserter(new_vert, new_vert.begin()) );
                set_difference_inplace<int>(new_vert, mksp_set[v->succ[j]->id]);
                wset_tmp.insert(new_vert.begin(), new_vert.end());

                for(const auto& nv:new_vert)
                    sum_w+= V[nv]->c;

            }
        }

        if(mksp[v->succ[j]->id] + float(sum_w) / n_proc > max_mksp){
            max_mksp = mksp[v->succ[j]->id] + float(sum_w) / n_proc;
            mkspset_tmp.insert(mksp_set[v->succ[j]->id].begin(), mksp_set[v->succ[j]->id].end());
            mkspset_tmp.insert(wset_tmp.begin(), wset_tmp.end());
        }


    }
}

int DAGTask::computeMakespanUB(const int n_proc){
    std::vector<std::set<int>> mksp_set (V.size());
    std::vector<std::set<int>> w_set (V.size());
    std::vector<int> w (V.size());
    std::vector<int> mksp (V.size());
    
    mksp_set[ordIDs[ordIDs.size()-1]].insert(ordIDs[ordIDs.size()-1]);
    w_set[ordIDs[ordIDs.size()-1]].insert(ordIDs[ordIDs.size()-1]);
    mksp[ordIDs[ordIDs.size()-1]] = V[ordIDs[ordIDs.size()-1]]->c;
    w[ordIDs[ordIDs.size()-1]] = V[ordIDs[ordIDs.size()-1]]->c;
    
    int idx;
    int C = 0;
    std::set<int> D;
    for(int i = ordIDs.size()-2; i >= 0; --i ){
        idx = ordIDs[i];
        mksp_set[idx].insert(idx);
        w_set[idx].insert(idx);
        mksp[idx] = V[idx]->c;
        w[idx] = V[idx]->c;

        if(V[idx]->succ.size()){
             
            if(V[idx]->mode != C_SOURCE_T){ //if not conditional source

            float max_mksp = 0;
            int w_tmp = 0;
            std::set<int> wset_tmp, mkspset_tmp;
            maximazieMakespan(V[idx], mksp, mksp_set, w_set, mkspset_tmp, max_mksp, n_proc);

            mksp[idx] = max_mksp + V[idx]->c;
            mksp_set[idx].insert(idx);
            mksp_set[idx].insert(mkspset_tmp.begin(), mkspset_tmp.end());

            for(int j=0; j<V[idx]->succ.size(); ++j){
                std::set<int> new_vert;
                std::set_difference(w_set[V[idx]->succ[j]->id].begin(), 
                                    w_set[V[idx]->succ[j]->id].end(),
                                    wset_tmp.begin(), 
                                    wset_tmp.end(), 
                                    std::inserter(new_vert, new_vert.begin()) );
                wset_tmp.insert(new_vert.begin(), new_vert.end());

                for(auto nv:new_vert)
                    w_tmp += V[nv]->c;
            }

            w[idx] += w_tmp;
            w_set[idx].insert(wset_tmp.begin(), wset_tmp.end());

            }
            else{
                int max_mksp = 0, max_w = 0, max_idx = 0;
                for(int j=0; j<V[idx]->succ.size(); ++j){
                    if(mksp[V[idx]->succ[j]->id] > max_mksp){
                        max_mksp = mksp[V[idx]->succ[j]->id];
                        max_idx = j;
                    }
                }
                mksp[idx] += max_mksp;
                mksp_set[idx].insert(mksp_set[V[idx]->succ[max_idx]->id].begin(), mksp_set[V[idx]->succ[max_idx]->id].end());

                for(int j=0; j<V[idx]->succ.size(); ++j){
                    if(w[V[idx]->succ[j]->id] > max_w){
                        max_w = w[V[idx]->succ[j]->id];
                        max_idx = j;
                    }
                }

                w[idx] += max_w;
                w_set[idx].insert(w_set[V[idx]->succ[max_idx]->id].begin(),w_set[V[idx]->succ[max_idx]->id].end());
            }
        }
    }

    return mksp[ordIDs[0]];
}

void DAGTask::computeTopologicalOrder(){
    if (!ordIDs.size()){
        ordIDs = topologicalSort();
        if(!checkIndexAndIdsAreEqual())
            FatalError("Ids and Indexes do not correspond, can't use computed topological order!");
        if(ordIDs.size() != V.size())
            FatalError("Ids and V sizes differ!");
    }
}

DAGTask DAGTask::generateTaskMelani(){
    configureParams();
    expandTaskSeriesParallel(nullptr, nullptr,rec_depth,0,false);
    assignWCET(Cmin, Cmax);
    makeItDag(addProb);

    computeTopologicalOrder(); 

    computeAccWorkload();
    computeWorstCaseWorkload();
    computeVolume();

    for(const auto&v :V)
        if(v->accWork > L)
            L = v->accWork;

    std::cout<<"length = "<<L<<std::endl;
    std::cout<<"wcw = "<<wcw<<std::endl;
    std::cout<<"volume = "<<vol<<std::endl;
    
    return *this;
}

void DAGTask::assignSchedParametersUUniFast(const float U){
    t = std::ceil(float(wcw) / U);
    d = t;    
}
