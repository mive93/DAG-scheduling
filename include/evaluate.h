#ifndef EVALUATE_H
#define EVALUATE_H

#include "utils.h"
#include "GeneratorParams.h"
#include "Taskset.h"
#include "tests.h"
#include "plot_utils.h"

std::string getSuffixCoreOrder(PartitioningCoresOrder_t core_order){
    switch (core_order){
    case PartitioningCoresOrder_t::BEST_FIT:
        return "BF";
    case PartitioningCoresOrder_t::WORST_FIT:
        return "WF";
    case PartitioningCoresOrder_t::FIRST_FIT:
        return "FF";
    }
    return "";
}

std::string getSuffixNodeOrder(PartitioningNodeOrder_t node_order){
   switch (node_order){
    case PartitioningNodeOrder_t::NONE:
        return "NONE";
    case PartitioningNodeOrder_t::DEC_DENS:
        return "DD";
    case PartitioningNodeOrder_t::DEC_UTIL:
        return "DU";
    case PartitioningNodeOrder_t::DEC_DENS_DEC_UTIL:
        return "DD_DU";
    case PartitioningNodeOrder_t::DEC_UTIL_DEC_DENS:
        return "DU_DD";
    case PartitioningNodeOrder_t::REM_UTIL:
        return "DRU";
    }  
    return "";
}

bool isPartitioned(const std::string& name){
    if(name == "Fonseca2016" || name == "Casini2018" || name == "Zahaf2019" || name == "Baruah2020" )
        return true;
    return false;
}

bool callMethod(const std::string& name, const GeneratorParams& gp,  Taskset& task_set, const int m, std::vector<int>& typedProc){
    
    if(name == "Melani2015"){
        if(gp.aType == AlgorithmType_t::EDF)
            return GP_FP_EDF_Melani2015_C(task_set, m);
        if(gp.aType == AlgorithmType_t::FTP)
            return GP_FP_FTP_Melani2015_C(task_set, m);
    }
    if(name == "Bonifaci2013"){
        if(gp.aType == AlgorithmType_t::EDF && gp.dtype ==  DeadlinesType_t::ARBITRARY)
            return GP_FP_EDF_Bonifaci2013_A(task_set, m);
        if (gp.aType == AlgorithmType_t::FTP ){
            if(gp.dtype ==  DeadlinesType_t::ARBITRARY)
                return GP_FP_DM_Bonifaci2013_A(task_set, m);
            if(gp.dtype ==  DeadlinesType_t::CONSTRAINED)
                return GP_FP_DM_Bonifaci2013_C(task_set, m);
        }
    }
    if(name == "Baruah2012"){
        if(gp.dtype ==  DeadlinesType_t::ARBITRARY)
            return GP_FP_EDF_Baruah2012_A(task_set.tasks[0], m);
        else
            return GP_FP_EDF_Baruah2012_C(task_set.tasks[0], m);
    }
    if(name == "Fonseca2019"){
        if ( gp.dtype ==  DeadlinesType_t::ARBITRARY)
            return GP_FP_FTP_Fonseca2019(task_set, m, false);
        else
            return GP_FP_FTP_Fonseca2019(task_set, m);
    }

    if(name == "Qamhieh2013")
        return GP_FP_EDF_Qamhieh2013_C(task_set, m);
    if(name == "Li2013")
        return GP_FP_EDF_Li2013_I(task_set, m);
    if(name == "Serrano2016")
        return GP_LP_FTP_Serrano16_C(task_set, m);
    if(name == "Pathan2017")
        return  GP_FP_DM_Pathan2017_C(task_set, m);
    if(name == "Fonseca2017")
        return GP_FP_FTP_Fonseca2017_C(task_set, m);
    if(name == "Nasri2019")
        return G_LP_FTP_Nasri2019_C(task_set, m);
    if(name == "He2019")
        return GP_FP_FTP_He2019_C(task_set, m);
    if(name == "Han2019" && !typedProc.empty())
        return GP_FP_Han2019_C_1(task_set.tasks[0], typedProc);
    if(name == "Graham1969")
        return Graham1969(task_set.tasks[0], m);
    if(name == "Fonseca2016")
        return P_FP_FTP_Fonseca2016_C(task_set, m);
    if(name == "Casini2018")
        return P_LP_FTP_Casini2018_C(task_set, m);

    #ifdef BARUAH2020
    if(name == "Baruah2020")
        return G_LP_FTP_Baruah2020_C_exact(task_set.tasks[0], m);
    #endif
    #ifdef ZAHAF2019
    if(name == "Zahaf2019")
        return P_LP_EDF_Zahaf2019_C(task_set, m);
    #endif

    FatalError(name + " is not supported!");
}

std::vector<std::string> getMethods(SchedulingType_t sched_type, DeadlinesType_t d_type, workloadType_t w_type, AlgorithmType_t a_type, DAGType_t dag_type){

    std::vector<std::string> methods;

    if(sched_type == SchedulingType_t::GLOBAL){
        switch (d_type){
        case DeadlinesType_t::CONSTRAINED: case DeadlinesType_t::IMPLICIT:
            if(a_type == AlgorithmType_t::EDF && dag_type == DAGType_t::DAG ){
                methods.push_back("Qamhieh2013");
                methods.push_back("Melani2015");
                if(w_type == workloadType_t::SINGLE_DAG) methods.push_back("Baruah2012");
                if(d_type != DeadlinesType_t::CONSTRAINED) methods.push_back("Li2013");
            }
            else if(a_type == AlgorithmType_t::FTP && dag_type ==DAGType_t::DAG){
                methods.push_back("Bonifaci2013");
                methods.push_back("Bonifaci2013");
                methods.push_back("Melani2015");
                methods.push_back("Serrano2016");
                methods.push_back("Pathan2017");
                methods.push_back("Fonseca2017");
                methods.push_back("Fonseca2019");
                methods.push_back("Nasri2019");
                methods.push_back("He2019");
            }

            else if(a_type == AlgorithmType_t::EDF && dag_type ==DAGType_t::CDAG ){
                methods.push_back("Melani2015");
            }
            else if(a_type == AlgorithmType_t::FTP && dag_type ==DAGType_t::CDAG){
                methods.push_back("Melani2015");
                methods.push_back("Pathan2017");
            }

            else if(a_type == AlgorithmType_t::FTP && dag_type ==DAGType_t::TDAG && w_type == workloadType_t::SINGLE_DAG){
                methods.push_back("Han2019");
            }

            break;
        case DeadlinesType_t::ARBITRARY:
            if(a_type == AlgorithmType_t::EDF && dag_type ==DAGType_t::DAG ){
                methods.push_back("Bonifaci2013");
                if(w_type == workloadType_t::SINGLE_DAG) methods.push_back("Baruah2012");
            }
            else if(a_type == AlgorithmType_t::FTP && dag_type ==DAGType_t::DAG ){
                methods.push_back("Bonifaci2013");
                methods.push_back("Fonseca2019");

                if(w_type == workloadType_t::SINGLE_DAG) methods.push_back("Graham1969");
            }
            break;
        }
    }
    else if(sched_type == SchedulingType_t::PARTITIONED){
        
        if(a_type == AlgorithmType_t::FTP && dag_type == DAGType_t::DAG && d_type != DeadlinesType_t::ARBITRARY){
            methods.push_back("Fonseca2016");
            methods.push_back("Casini2018");

            #ifdef ZAHAF2019
            methods.push_back("Zahaf2019");
            #endif

            #ifdef BARUAH2020
            methods.push_back("Baruah2020");
            #endif

        }
    }

    return methods;
}

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

        if(gp.methods.empty()){
            gp.methods = getMethods(gp.sType, gp.dtype, gp.wType, gp.aType, gp.DAGType);
            gp.names = gp.methods;
        }

        if(gp.coresOrders.empty()) gp.coresOrders.push_back(2); // by default worst fit
        if(gp.nodesOrders.empty()) gp.nodesOrders.push_back(5); // by default remaining utilization

        // partitioned methods
        for(int co=0; co<gp.coresOrders.size(); ++co){
            for(int no=0; no<gp.nodesOrders.size(); ++no){

                // allocate nodes to cores
                bool allocated = processorsAssignment(task_set, m, (PartitioningCoresOrder_t) gp.coresOrders[co], (PartitioningNodeOrder_t) gp.nodesOrders[no]);
                if (!allocated) continue;

                for(int j=0; j<gp.methods.size(); ++j){
                    if(isPartitioned(gp.methods[j])){
                        std::string complete_name = gp.names[j] + "_" + getSuffixCoreOrder((PartitioningCoresOrder_t) gp.coresOrders[co]) + "_" + getSuffixNodeOrder((PartitioningNodeOrder_t) gp.nodesOrders[no]);
                        if(i % gp.tasksetPerVarFactor == 0)
                            sched_res[complete_name].push_back(0);

                        timer.tic();
                        sched_res[complete_name][test_idx] += callMethod(gp.names[j], gp,  task_set, m, gp.typedProc);
                        time_res[complete_name].push_back(timer.toc());
                    }
                }
            }
        }

        // global methods, not affected by allocation
        for(int j=0; j<gp.methods.size(); ++j){
            if(!isPartitioned(gp.methods[j])){
                if(i % gp.tasksetPerVarFactor == 0)
                    sched_res[gp.names[j]].push_back(0);

                timer.tic();
                sched_res[gp.names[j]][test_idx] += callMethod(gp.names[j], gp,  task_set, m, gp.typedProc);
                time_res[gp.names[j]].push_back(timer.toc());
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