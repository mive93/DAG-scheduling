#ifndef DAGTASK_H
#define DAGTASK_H

#include <vector>
#include <map>
#include <cmath>
#include <cstdlib>
#include <random>
#include <iostream>
#include <algorithm>

#include <yaml-cpp/yaml.h>
#include "SubTask.h"
   
enum creationStates {CONDITIONAL_T=0, PARALLEL_T=1, TERMINAL_T=2};

class DAGTask{

    int t = 0;  //period
    int d = 0;  //deadline
    std::vector<SubTask*> V;

    int L = 0; // longest chain
    int vol  = 0; //volume


    public:
    DAGTask(){};
    DAGTask(const int T, const int D): t(T), d(D) {};

    ~DAGTask(){};
    void readTaskFromYaml(const std::string& params_path);


    void saveAsDot(const std::string &filename);

    friend std::ostream& operator<<(std::ostream& os, const DAGTask& t);

    int maxCondBranches = 2;
    int maxParBranches = 6;
    float p_cond = 0; //probability of generating a conditional branch
    float p_par = 0.2; //probability of generating a parallel branch
    float p_term = 0.8; //probability of generating a terminal vertex 
    int rec_depth = 2; // maximum recursion depth for the generation of the task graphs
    int Cmin = 1;
    int Cmax = 100;
    float addProb=0.1;

    std::mt19937 gen;
    std::vector<double> weights;
    std::discrete_distribution<int> dist;

    void configureParams(){
        weights.push_back(p_cond);
        weights.push_back(p_par);
        weights.push_back(p_term);
        dist.param(std::discrete_distribution<int> ::param_type(std::begin(weights), std::end(weights)));

        // gen.seed(time(0));
        gen.seed(1);
    }

    void assignWCET(const int minC, const int maxC){
        for(auto &v: V)
            v->c = rand()%maxC + minC;
    }

    void expandTaskSeriesParallel(SubTask* source,SubTask* sink,const int depth,const int numBranches, const bool ifCond){

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
            if (r < 0.5){ //make it conditional
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

    void isSuccessor(SubTask* v, SubTask *w, bool &is_succ){

        for(size_t i=0; i<w->succ.size(); ++i){
            if(w->succ[i] == v){
                is_succ = true;
                return;
            }
            else
                isSuccessor(v, w->succ[i], is_succ);
        }
        return;
    }

    void makeItDag(float prob){
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

    DAGTask generateTaskMelani(){
        configureParams();
        expandTaskSeriesParallel(nullptr, nullptr,rec_depth,0,false);
        assignWCET(Cmin, Cmax);
        makeItDag(addProb);

        // bool is_succ = false;
        // isSuccessor(V[7], V[1], is_succ);
        // std::cout<<is_succ<<std::endl;;

        return *this;
    }

};

#endif /* DAGTASK_H */