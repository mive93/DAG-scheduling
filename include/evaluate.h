#ifndef EVALUATE_H
#define EVALUATE_H

#include "utils.h"
#include "GeneratorParams.h"
#include "Taskset.h"
#include "tests.h"
#include "plot_utils.h"

void evaluate(const std::string& genparams_path){
    GeneratorParams gp;
    gp.readFromYaml(genparams_path);
    gp.configureParams(gp.gType);

    float U_curr = gp.Umin;
    int m = gp.mMin - 1;
    int n_tasks = gp.nMin - 1;

    std::map<std::string,std::vector<float>> sched_res;
    std::vector<float> x;

    int test_idx = -1;

    for(int i=0; i<gp.nTasksets; ++i){
        if(gp.gType == GenerationType_t::VARYING_U && i % gp.tasksetPerVarFactor == 0){
            U_curr += gp.stepU;
            x.push_back(U_curr);
            test_idx++;
        }
        
        Taskset task_set;
        task_set.generate_taskset_Melani(gp.nTasks, U_curr, gp.m, gp);

        std::cout<<"taskset: "<<i<<" U: "<<U_curr<< " test_idx: "<<test_idx<<" ntasks: "<<task_set.tasks.size()<<std::endl;

        // for(int x=0; x<task_set.tasks.size();++x){
        //     task_set.tasks[x].saveAsDot("test"+std::to_string(x)+".dot");
        //     std::string dot_command = "dot -Tpng test"+std::to_string(x)+".dot > test"+std::to_string(i)+".png";
        //     system(dot_command.c_str());
        // }

        switch (gp.dtype){
        case DeadlinesType_t::CONSTRAINED: case DeadlinesType_t::IMPLICIT:
            if(gp.sType == SchedulingType_t::EDF && gp.DAGType == DAGType_t::DAG ){
                if(i % gp.tasksetPerVarFactor == 0){
                    sched_res["Qamhieh2013"].push_back(0);
                    // sched_res["Baruah2014"].push_back(0);
                    sched_res["Melani2015"].push_back(0);
                }

                sched_res["Qamhieh2013"][test_idx] += GP_FP_EDF_Qamhieh2013_C(task_set, gp.m);
                // sched_res["Baruah2014"][test_idx] += GP_FP_EDF_Baruah2014_C(task_set, gp.m);
                sched_res["Melani2015"][test_idx] += GP_FP_EDF_Melani2015_C(task_set, gp.m);
                
                if(gp.wType == workloadType_t::SINGLE_DAG){
                    if(i % gp.tasksetPerVarFactor == 0)
                        sched_res["Baruah2012"].push_back(0);
                    sched_res["Baruah2012"][test_idx] += GP_FP_EDF_Baruah2012_C(task_set.tasks[0], gp.m);
                }

                if(gp.dtype != DeadlinesType_t::CONSTRAINED){
                    if(i % gp.tasksetPerVarFactor == 0)
                        sched_res["Li2013"].push_back(0);
                    sched_res["Li2013"][test_idx] += GP_FP_EDF_Li2013_I(task_set, gp.m);
                }
            }
            else if(gp.sType == SchedulingType_t::FTP && gp.DAGType ==DAGType_t::DAG){
                if(i % gp.tasksetPerVarFactor == 0){
                    sched_res["Bonifaci2013"].push_back(0);
                    sched_res["Melani2015"].push_back(0);
                    sched_res["Pathan2017"].push_back(0);
                    sched_res["Fonseca2017"].push_back(0);
                    sched_res["Fonseca2019"].push_back(0);
                    sched_res["He2019"].push_back(0);
                }
                
                sched_res["Bonifaci2013"][test_idx] +=  GP_FP_DM_Bonifaci2013_C(task_set, gp.m);
                sched_res["Melani2015"][test_idx] +=  GP_FP_FTP_Melani2015_C(task_set, gp.m);
                sched_res["Pathan2017"][test_idx] +=  GP_FP_DM_Pathan2017_C(task_set, gp.m);
                sched_res["Fonseca2017"][test_idx] +=  GP_FP_FTP_Fonseca2017_C(task_set, gp.m);
                sched_res["Fonseca2019"][test_idx] +=  GP_FP_FTP_Fonseca2019(task_set, gp.m);
                sched_res["He2019"][test_idx] +=  GP_FP_FTP_He2019_C(task_set, gp.m);
            }

            else if(gp.sType == SchedulingType_t::EDF && gp.DAGType ==DAGType_t::CDAG ){
                if(i % gp.tasksetPerVarFactor == 0)
                    sched_res["Melani2015"].push_back(0);
                sched_res["Melani2015"][test_idx] +=  GP_FP_EDF_Melani2015_C(task_set, gp.m);
            }
            else if(gp.sType == SchedulingType_t::FTP && gp.DAGType ==DAGType_t::CDAG){
                if(i % gp.tasksetPerVarFactor == 0){
                    sched_res["Melani2015"].push_back(0);
                    sched_res["Pathan2017"].push_back(0);
                }
                sched_res["Melani2015"][test_idx] +=  GP_FP_FTP_Melani2015_C(task_set, gp.m);
                sched_res["Pathan2017"][test_idx] +=  GP_FP_DM_Pathan2017_C(task_set, gp.m);
            }

            else if(gp.sType == SchedulingType_t::FTP && gp.DAGType ==DAGType_t::TDAG && gp.wType == workloadType_t::SINGLE_DAG){
                if(i % gp.tasksetPerVarFactor == 0)
                    sched_res["Han2019"].push_back(0);
                sched_res["Han2019"][test_idx] +=  GP_FP_Han2019_C_1(task_set.tasks[0], gp.typedProc);
            }

            
            break;
        case DeadlinesType_t::ARBITRARY:
            if(gp.sType == SchedulingType_t::EDF && gp.DAGType ==DAGType_t::DAG ){
                if(i % gp.tasksetPerVarFactor == 0)
                    sched_res["Bonifaci2013"].push_back(0);
                
                sched_res["Bonifaci2013"][test_idx] += GP_FP_EDF_Bonifaci2013_A(task_set, gp.m);
                if(gp.wType == workloadType_t::SINGLE_DAG){
                    if(i % gp.tasksetPerVarFactor == 0)
                        sched_res["Baruah2012"].push_back(0);
                    sched_res["Baruah2012"][test_idx] += GP_FP_EDF_Baruah2012_A(task_set.tasks[0], gp.m);
                }
            }
            else if(gp.sType == SchedulingType_t::FTP && gp.DAGType ==DAGType_t::DAG ){
                if(i % gp.tasksetPerVarFactor == 0){
                    sched_res["Bonifaci2013"].push_back(0);
                    sched_res["Fonseca2019"].push_back(0);
                }

                sched_res["Bonifaci2013"][test_idx] += GP_FP_DM_Bonifaci2013_A(task_set, gp.m);
                // sched_res["Fonseca2019"][test_idx] += GP_FP_FTP_Fonseca2019(task_set, m, false);
                if(gp.wType == workloadType_t::SINGLE_DAG){
                    if(i % gp.tasksetPerVarFactor == 0)
                        sched_res["Graham1969"].push_back(0);

                    sched_res["Graham1969"][test_idx] += Graham1969(task_set.tasks[0], gp.m);
                }
            }
            break;
        }

        for(auto &t:task_set.tasks)
            t.destroyVerices();

            

    }

    plotResults(sched_res, x, "Utilization", "Schedulability");

}




#endif /* EVALUATE_H */