#include <iostream>
#include "dagSched/DAGTask.h"
#include "dagSched/Taskset.h"
#include "dagSched/tests.h"
#include "dagSched/plot_utils.h"

#include <ctime>    

int main(int argc, char **argv){

    if(REPRODUCIBLE) srand (1);
    else srand (time(NULL));

    bool random_creation = true;
    if(argc > 1)
        random_creation = atoi(argv[1]);
        
    std::string taskset_filename = "../demo/taskset.yaml";
    if(argc > 2)
        taskset_filename = argv[2];
        

    int n_proc = 4;
    std::vector<int> typed_proc = {4,4};
    dagSched::Taskset taskset;
    if(random_creation){
        int n_tasks = 4;
        float U_tot = 1;
        dagSched::GeneratorParams gp;
        gp.configureParams(dagSched::GenerationType_t::VARYING_N);

        taskset.generate_taskset_Melani(n_tasks, U_tot, n_proc, gp);
    }
    else{
        size_t ext  = taskset_filename.find("yaml");
        if( ext != std::string::npos)
            taskset.readTasksetFromYaml(taskset_filename);
        else
            taskset.readTasksetFromDOT(taskset_filename);
    }

    std::cout<<"Assignment: "<< dagSched::processorsAssignment(taskset, n_proc, dagSched::PartitioningCoresOrder_t::WORST_FIT, dagSched::PartitioningNodeOrder_t::REM_UTIL);
    
    std::string dot_command = "";
    for(int i=0; i<taskset.tasks.size();++i){
        std::cout<<taskset.tasks[i]<<std::endl;
        taskset.tasks[i].saveAsDot("test"+std::to_string(i)+".dot");

        //convert dot to png to visualize it
        dot_command = "dot -Tpng test"+std::to_string(i)+".dot > test"+std::to_string(i)+".png";
        system(dot_command.c_str());
    }
    std::cout<<"Taskset utilization: "<<taskset.getUtilization()<<std::endl;
    std::cout<<"Taskset Hyper-Period: "<<taskset.getHyperPeriod()<<std::endl;
    std::cout<<"Taskset max Density: "<<taskset.getMaxDensity()<<std::endl<<std::endl;

    //single DAG tests
    std::cout<<"Single task tests: \n";
    bool constrained_taskset = true;
    bool implicit_taskset = true;
    for(int i=0; i<taskset.tasks.size();++i){
        std::cout<<"\tTask "<<i<<std::endl;
        //constrained
        if(taskset.tasks[i].getDeadline() < taskset.tasks[i].getPeriod()){
            std::cout<< "\t\tBaruah 2012 constrained (GP-FP-EDF): " <<dagSched::GP_FP_EDF_Baruah2012_C(taskset.tasks[i], n_proc)<<std::endl;
            implicit_taskset = false;
        }

        if(taskset.tasks[i].getDeadline() <= taskset.tasks[i].getPeriod()){
            std::cout<< "\t\tHan 2019 constrained typed(GP-FP): " <<dagSched::GP_FP_Han2019_C_1(taskset.tasks[i], typed_proc)<<std::endl;
            std::cout<< "\t\tHe 2019 constrained typed(GP-FP): " <<dagSched::GP_FP_He2019_C(taskset.tasks[i], n_proc)<<std::endl;
            #ifdef BARUAH2020
            std::cout<< "\t\tBaruah 2020 constrained (P-LP-EDF): " <<dagSched::G_LP_FTP_Baruah2020_C_exact(taskset.tasks[i], n_proc)<<std::endl;
            #endif
        }
        std::cout<< "\t\tBaruah 2012 arbitrary (GP-FP-EDF): "   <<dagSched::GP_FP_EDF_Baruah2012_A(taskset.tasks[i], n_proc)<<std::endl;
        std::cout<< "\t\tGraham 1969 : "   <<dagSched::Graham1969(taskset.tasks[i], n_proc)<<std::endl;

        if(taskset.tasks[i].getDeadline() > taskset.tasks[i].getPeriod()){
            constrained_taskset = false;
            implicit_taskset = false;
        }
        
    }

    //taskset tests
    std::cout<<"Taskset tests: \n";


    //arbitrary
    std::cout<< "\tBonifaci 2013 arbitrary (GP-FP-EDF): "   <<dagSched::GP_FP_EDF_Bonifaci2013_A(taskset, n_proc)<<std::endl;
    std::cout<< "\tBonifaci 2013 arbitrary (GP-FP-DM): "   <<dagSched::GP_FP_DM_Bonifaci2013_A(taskset, n_proc)<<std::endl;

    //implicit
    if(implicit_taskset){
        std::cout<< "\tLi 2013 implicit (GP-FP-EDF): "   <<dagSched::GP_FP_EDF_Li2013_I(taskset, n_proc)<<std::endl;
    }

    //constrained
    if(constrained_taskset){
        //global
        std::cout<< "\tBonifaci 2013 constrained (GP-FP-DM): "   <<dagSched::GP_FP_DM_Bonifaci2013_C(taskset, n_proc)<<std::endl;
        std::cout<< "\tQamhieh 2013 constrained (GP-FP-EDF): "   <<dagSched::GP_FP_EDF_Qamhieh2013_C(taskset, n_proc)<<std::endl;
        std::cout<< "\tBaruah 2014 constrained (GP-FP-EDF): "   <<dagSched::GP_FP_EDF_Baruah2014_C(taskset, n_proc)<<std::endl;
        std::cout<< "\tMelani 2015 constrained (GP-FP-FTP): "   <<dagSched::GP_FP_FTP_Melani2015_C(taskset, n_proc)<<std::endl;
        std::cout<< "\tMelani 2015 constrained (GP-FP-EDF): "   <<dagSched::GP_FP_EDF_Melani2015_C(taskset, n_proc)<<std::endl;
        std::cout<< "\tPathan 2017 constrained (GP-FP-DM): "   <<dagSched::GP_FP_DM_Pathan2017_C(taskset, n_proc)<<std::endl;
        std::cout<< "\tFonseca 2017 constrained (GP-FP-FTP): "<<dagSched::GP_FP_FTP_Fonseca2017_C(taskset, n_proc)<<std::endl;
        std::cout<< "\tFonseca 2019 constrained (GP-FP-FTP): "<<dagSched::GP_FP_FTP_Fonseca2019(taskset, n_proc)<<std::endl;
        std::cout<< "\tHe 2019 constrained (GP-FP-FTP): "<<dagSched::GP_FP_FTP_He2019_C(taskset, n_proc)<<std::endl;

        // limited preemption        
        std::cout<< "\tSerrano 2016 constrained (GP-LP-FTP): "   <<dagSched::GP_LP_FTP_Serrano16_C(taskset, n_proc)<<std::endl;
        std::cout<< "\tNasri 2019 constrained (G-LP-FTP): "<<dagSched::G_LP_FTP_Nasri2019_C(taskset, n_proc)<<std::endl;

        //partitioned
        std::cout<< "\tFonseca 2016 constrained (P-FP-FTP): "<<dagSched::P_FP_FTP_Fonseca2016_C(taskset, n_proc)<<std::endl;
        std::cout<< "\tCasini 2018 constrained (P-LP-FTP): "<<dagSched::P_LP_FTP_Casini2018_C(taskset, n_proc)<<std::endl;
        #ifdef ZAHAF2019
        std::cout<< "\tZahaf 2020 constrained (P-LP-EDF): "<<dagSched::P_LP_EDF_Zahaf2019_C(taskset, n_proc)<<std::endl;
        #endif
    }

    std::cout<< "\tFonseca 2019 arbitrary (GP-FP-FTP): "<<dagSched::GP_FP_FTP_Fonseca2019(taskset, n_proc, false)<<std::endl;

}