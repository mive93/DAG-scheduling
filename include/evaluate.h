#ifndef EVALUATE_H
#define EVALUATE_H

#include "utils.h"
#include "GeneratorParams.h"
#include "Taskset.h"
#include "tests.h"
#include "plot_utils.h"

void evaluate(const std::string& genparams_path, const std::string& output_fig_path, const bool show_plots){
    GeneratorParams gp;
    gp.readFromYaml(genparams_path);
    gp.configureParams(gp.gType);

    float U_curr = gp.Utot;
    if(gp.gType == GenerationType_t::VARYING_U)
        U_curr = gp.Umin;

    int m = gp.m;
    if(gp.gType == GenerationType_t::VARYING_M)
        m = gp.mMin - 1;

    int n_tasks = gp.nTasks;
    if(gp.gType == GenerationType_t::VARYING_N)
        n_tasks = gp.nMin - 1;

    std::map<std::string,std::vector<float>> sched_res;
    std::map<std::string,std::vector<double>> time_res;
    std::vector<float> x;

    int test_idx = -1;
    SimpleTimer timer;

    int min_V_all  = 100;
    int max_V_all  = 0;

    for(int i=0; i<gp.nTasksets; ++i){
        if(gp.gType == GenerationType_t::VARYING_U && i % gp.tasksetPerVarFactor == 0){
            U_curr += gp.stepU;
            x.push_back(U_curr);
            test_idx++;
        }
        else if(gp.gType == GenerationType_t::VARYING_N && i % gp.tasksetPerVarFactor == 0){
            n_tasks += gp.stepN;
            x.push_back(n_tasks);
            test_idx++;
        }
        else if(gp.gType == GenerationType_t::VARYING_M && i % gp.tasksetPerVarFactor == 0){
            m += gp.stepM;
            x.push_back(m);
            test_idx++;
        }
        
        Taskset task_set;
        task_set.generate_taskset_Melani(n_tasks, U_curr, m, gp);

        int max_v_size = 0;
        for(int ii=0; ii<task_set.tasks.size(); ++ii)
            if(task_set.tasks[ii].getVertices().size() > max_v_size)
                max_v_size = task_set.tasks[ii].getVertices().size();

        if (max_v_size > max_V_all) max_V_all = max_v_size;
        if (max_v_size < min_V_all) min_V_all = max_v_size;

        std::cout<<"taskset: "<<i<<" U: "<<U_curr<<" ntasks: "<<task_set.tasks.size()<<" max|V|: "<<max_V_all<<" min|V|: "<<min_V_all<<" m:"<<m<< " test_idx: "<<test_idx<<std::endl;

        // for(int x=0; x<task_set.tasks.size();++x){
        //     task_set.tasks[x].saveAsDot("test"+std::to_string(x)+".dot");
        //     std::string dot_command = "dot -Tpng test"+std::to_string(x)+".dot > test"+std::to_string(i)+".png";
        //     system(dot_command.c_str());
        // }

        if(gp.sType == SchedulingType_t::GLOBAL){
            switch (gp.dtype){
            case DeadlinesType_t::CONSTRAINED: case DeadlinesType_t::IMPLICIT:
                if(gp.aType == AlgorithmType_t::EDF && gp.DAGType == DAGType_t::DAG ){
                    if(i % gp.tasksetPerVarFactor == 0){
                        sched_res["Qamhieh2013"].push_back(0);
                        // sched_res["Baruah2014"].push_back(0);
                        sched_res["Melani2015"].push_back(0);
                    }

                    timer.tic();
                    sched_res["Qamhieh2013"][test_idx] += GP_FP_EDF_Qamhieh2013_C(task_set, m);
                    time_res["Qamhieh2013"].push_back(timer.toc());

                    // timer.tic();
                    // sched_res["Baruah2014"][test_idx] += GP_FP_EDF_Baruah2014_C(task_set, m);
                    // time_res["Baruah2014"].push_back(timer.toc());

                    timer.tic();
                    sched_res["Melani2015"][test_idx] += GP_FP_EDF_Melani2015_C(task_set, m);
                    time_res["Melani2015"].push_back(timer.toc());
                    
                    if(gp.wType == workloadType_t::SINGLE_DAG){
                        if(i % gp.tasksetPerVarFactor == 0)
                            sched_res["Baruah2012"].push_back(0);
                        
                        timer.tic();
                        sched_res["Baruah2012"][test_idx] += GP_FP_EDF_Baruah2012_C(task_set.tasks[0], m);
                        time_res["Baruah2012"].push_back(timer.toc());
                    }

                    if(gp.dtype != DeadlinesType_t::CONSTRAINED){
                        if(i % gp.tasksetPerVarFactor == 0)
                            sched_res["Li2013"].push_back(0);
                        
                        timer.tic();
                        sched_res["Li2013"][test_idx] += GP_FP_EDF_Li2013_I(task_set, m);
                        time_res["Li2013"].push_back(timer.toc());
                    }
                }
                else if(gp.aType == AlgorithmType_t::FTP && gp.DAGType ==DAGType_t::DAG){
                    if(i % gp.tasksetPerVarFactor == 0){
                        sched_res["Bonifaci2013"].push_back(0);
                        sched_res["Melani2015"].push_back(0);
                        sched_res["Serrano2016"].push_back(0);
                        sched_res["Pathan2017"].push_back(0);
                        sched_res["Fonseca2017"].push_back(0);
                        sched_res["Fonseca2019"].push_back(0);
                        sched_res["Nasri2019"].push_back(0);
                        sched_res["He2019"].push_back(0);
                    }

                    timer.tic();
                    sched_res["Bonifaci2013"][test_idx] +=  GP_FP_DM_Bonifaci2013_C(task_set, m);
                    time_res["Bon2013"].push_back(timer.toc());

                    timer.tic();
                    sched_res["Melani2015"][test_idx] +=  GP_FP_FTP_Melani2015_C(task_set, m);
                    time_res["Mel2015"].push_back(timer.toc());

                    timer.tic();
                    sched_res["Serrano2016"][test_idx] += GP_LP_FTP_Serrano16_C(task_set, m);
                    time_res["Ser2016"].push_back(timer.toc());

                    timer.tic();
                    sched_res["Pathan2017"][test_idx] +=  GP_FP_DM_Pathan2017_C(task_set, m);
                    time_res["Pat2017"].push_back(timer.toc());

                    timer.tic();
                    sched_res["Fonseca2017"][test_idx] +=  GP_FP_FTP_Fonseca2017_C(task_set, m);
                    time_res["Fon2017"].push_back(timer.toc());

                    timer.tic();
                    sched_res["Fonseca2019"][test_idx] +=  GP_FP_FTP_Fonseca2019(task_set, m);
                    time_res["Fon2019"].push_back(timer.toc());

                    timer.tic();
                    sched_res["Nasri2019"][test_idx] +=  G_LP_FTP_Nasri2019_C(task_set, m);
                    time_res["Nas2019"].push_back(timer.toc());

                    timer.tic();
                    sched_res["He2019"][test_idx] +=  GP_FP_FTP_He2019_C(task_set, m);
                    time_res["He2019"].push_back(timer.toc());
                }

                else if(gp.aType == AlgorithmType_t::EDF && gp.DAGType ==DAGType_t::CDAG ){
                    if(i % gp.tasksetPerVarFactor == 0)
                        sched_res["Melani2015"].push_back(0);

                    timer.tic();
                    sched_res["Melani2015"][test_idx] +=  GP_FP_EDF_Melani2015_C(task_set, m);
                    time_res["Melani2015"].push_back(timer.toc());
                }
                else if(gp.aType == AlgorithmType_t::FTP && gp.DAGType ==DAGType_t::CDAG){
                    if(i % gp.tasksetPerVarFactor == 0){
                        sched_res["Melani2015"].push_back(0);
                        sched_res["Pathan2017"].push_back(0);
                    }

                    timer.tic();
                    sched_res["Melani2015"][test_idx] +=  GP_FP_FTP_Melani2015_C(task_set, m);
                    time_res["Melani2015"].push_back(timer.toc());

                    timer.tic();
                    sched_res["Pathan2017"][test_idx] +=  GP_FP_DM_Pathan2017_C(task_set, m);
                    time_res["Pathan2017"].push_back(timer.toc());
                }

                else if(gp.aType == AlgorithmType_t::FTP && gp.DAGType ==DAGType_t::TDAG && gp.wType == workloadType_t::SINGLE_DAG){
                    if(i % gp.tasksetPerVarFactor == 0)
                        sched_res["Han2019"].push_back(0);

                    timer.tic();
                    sched_res["Han2019"][test_idx] +=  GP_FP_Han2019_C_1(task_set.tasks[0], gp.typedProc);
                    time_res["Han2019"].push_back(timer.toc());
                }

                
                break;
            case DeadlinesType_t::ARBITRARY:
                if(gp.aType == AlgorithmType_t::EDF && gp.DAGType ==DAGType_t::DAG ){
                    if(i % gp.tasksetPerVarFactor == 0)
                        sched_res["Bonifaci2013"].push_back(0);
                    
                    timer.tic();
                    sched_res["Bonifaci2013"][test_idx] += GP_FP_EDF_Bonifaci2013_A(task_set, m);
                    time_res["Bonifaci2013"].push_back(timer.toc());

                    if(gp.wType == workloadType_t::SINGLE_DAG){
                        if(i % gp.tasksetPerVarFactor == 0)
                            sched_res["Baruah2012"].push_back(0);
                        
                        timer.tic();
                        sched_res["Baruah2012"][test_idx] += GP_FP_EDF_Baruah2012_A(task_set.tasks[0], m);
                        time_res["Baruah2012"].push_back(timer.toc());
                    }
                }
                else if(gp.aType == AlgorithmType_t::FTP && gp.DAGType ==DAGType_t::DAG ){
                    if(i % gp.tasksetPerVarFactor == 0){
                        sched_res["Bonifaci2013"].push_back(0);
                        sched_res["Fonseca2019"].push_back(0);
                    }

                    timer.tic();
                    sched_res["Bonifaci2013"][test_idx] += GP_FP_DM_Bonifaci2013_A(task_set, m);
                    time_res["Bonifaci2013"].push_back(timer.toc());

                    timer.tic();
                    sched_res["Fonseca2019"][test_idx] += GP_FP_FTP_Fonseca2019(task_set, m, false);
                    time_res["Fonseca2019"].push_back(timer.toc());

                    if(gp.wType == workloadType_t::SINGLE_DAG){
                        if(i % gp.tasksetPerVarFactor == 0)
                            sched_res["Graham1969"].push_back(0);

                        timer.tic();
                        sched_res["Graham1969"][test_idx] += Graham1969(task_set.tasks[0], m);
                        time_res["Graham1969"].push_back(timer.toc());
                    }
                }
                break;
            }
        }
        else if(gp.sType == SchedulingType_t::PARTITIONED){
            
            if(gp.aType == AlgorithmType_t::FTP && gp.DAGType == DAGType_t::DAG && gp.dtype != DeadlinesType_t::ARBITRARY){
                 if(i % gp.tasksetPerVarFactor == 0){
                    sched_res["Fonseca2016"].push_back(0);
                    sched_res["Casini2018"].push_back(0);
                    // sched_res["Casini2018_W"].push_back(0);
                    // sched_res["Casini2018_B"].push_back(0);
                    // sched_res["Casini2018_F"].push_back(0);
                    // sched_res["Casini2018_P"].push_back(0);
                    #ifdef ZAHAF2019
                    sched_res["Zahaf2019"].push_back(0);
                    #endif
                 }
                
                WorstFitProcessorsAssignment(task_set, m);

                timer.tic();
                sched_res["Fonseca2016"][test_idx] += P_FP_FTP_Fonseca2016_C(task_set, m);
                time_res["Fonseca2016"].push_back(timer.toc());

                timer.tic();
                sched_res["Casini2018"][test_idx] += P_LP_FTP_Casini2018_C(task_set, m);
                time_res["Casini2018"].push_back(timer.toc());
                
                // timer.tic();
                // int worst = P_LP_FTP_Casini2018_C_withAssignment(task_set, m, PartitioningCoresOrder_t::WORST_FIT);
                // sched_res["Casini2018_W"][test_idx] += worst;
                // time_res["Casini2018_W"].push_back(timer.toc());

                // timer.tic();
                // int best = P_LP_FTP_Casini2018_C_withAssignment(task_set, m, PartitioningCoresOrder_t::BEST_FIT);
                // sched_res["Casini2018_B"][test_idx] += best;
                // time_res["Casini2018_B"].push_back(timer.toc());

                // timer.tic();
                // int first = P_LP_FTP_Casini2018_C_withAssignment(task_set, m, PartitioningCoresOrder_t::FIRST_FIT);
                // sched_res["Casini2018_F"][test_idx] += first;
                // time_res["Casini2018_F"].push_back(timer.toc());

                // std::cout<<"PARTITIONED: "<<(first || best || worst)<<std::endl;
                // sched_res["Casini2018_P"][test_idx] += (int) (first || best || worst);
                // time_res["Casini2018_P"].push_back(time_res["Casini2018_F"].back() + time_res["Casini2018_B"].back() + time_res["Casini2018_W"].back());

                #ifdef ZAHAF2019
                timer.tic();
                sched_res["Zahaf2019"][test_idx] += P_LP_EDF_Zahaf2019_C(task_set, m);
                time_res["Zahaf2019"].push_back(timer.toc());
                #endif
            }
        }

        else if(gp.sType == SchedulingType_t::SOTA){
            if(gp.aType == AlgorithmType_t::FTP && gp.DAGType == DAGType_t::DAG && gp.dtype != DeadlinesType_t::ARBITRARY){
                if(i % gp.tasksetPerVarFactor == 0){
                    sched_res["Melani2015"].push_back(0);
                    sched_res["Fonseca2016"].push_back(0);
                    // sched_res["Serrano2016"].push_back(0);
                    sched_res["Casini2018"].push_back(0);
                    sched_res["Fonseca2019"].push_back(0);
                    sched_res["Nasri2019"].push_back(0);
                    
                }

                WorstFitProcessorsAssignment(task_set, m);

                timer.tic();
                sched_res["Melani2015"][test_idx] +=  GP_FP_FTP_Melani2015_C(task_set, m);
                time_res["Mel2015"].push_back(timer.toc());

                // timer.tic();
                // sched_res["Serrano2016"][test_idx] += GP_LP_FTP_Serrano16_C(task_set, m);
                // time_res["Serrano2016"].push_back(timer.toc());

                timer.tic();
                sched_res["Fonseca2016"][test_idx] += P_FP_FTP_Fonseca2016_C(task_set, m);
                time_res["Fonseca2016"].push_back(timer.toc());

                timer.tic();
                sched_res["Casini2018"][test_idx] += P_LP_FTP_Casini2018_C(task_set, m);
                time_res["Casini2018"].push_back(timer.toc());

                timer.tic();
                sched_res["Fonseca2019"][test_idx] +=  GP_FP_FTP_Fonseca2019(task_set, m);
                time_res["Fon2019"].push_back(timer.toc());

                timer.tic();
                sched_res["Nasri2019"][test_idx] +=  G_LP_FTP_Nasri2019_C(task_set, m);
                time_res["Nasri2019"].push_back(timer.toc());

            }
        }

        for(auto &t:task_set.tasks)
            t.destroyVerices();
    }


    std::string x_axis_label;
    if(gp.gType == GenerationType_t::VARYING_M)
        x_axis_label = "Number of cores";
    else if(gp.gType == GenerationType_t::VARYING_M)
        x_axis_label = "Number of tasks";
    else if(gp.gType == GenerationType_t::VARYING_U)
        x_axis_label = "Taskset utilization";

    plotResults(sched_res, x, x_axis_label, "Taskset scheduled", output_fig_path, show_plots);
    plotTimes(time_res, output_fig_path, show_plots);

}




#endif /* EVALUATE_H */