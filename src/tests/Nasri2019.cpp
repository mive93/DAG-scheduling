#include "tests.h"

#include"problem.hpp"
#include "io.hpp"
#include "global/space.hpp"

#include "tbb/task_scheduler_init.h"

bool test(std::istream &in,
	std::istream &dag_in,
	std::istream &aborts_in, const int m){

    tbb::task_scheduler_init init(8);

    NP::Scheduling_problem<dtime_t> problem{
		NP::parse_file<dtime_t>(in),
		NP::parse_dag_file(dag_in),
		NP::parse_abort_file<dtime_t>(aborts_in),
		m};

    // Set common analysis options
	NP::Analysis_options opts;
	opts.timeout = 0;
	opts.max_depth = 0;
	opts.early_exit = true;
	opts.num_buckets = problem.jobs.size();
	opts.be_naive = 0;

	// Actually call the analysis engine
	auto space = NP::Global::State_space<dtime_t>::explore(problem, opts);

	// Extract the analysis results
	// auto graph = std::ostringstream();
	// auto rta = std::ostringstream();


	return space.is_schedulable(); 
}

std::string convertTasksetToCsv(Taskset& taskset){

    std::string task_set = "   Task ID,     Job ID,          Arrival min,          Arrival max,             Cost min,             Cost max,             Deadline,             Priority\n";
    for(int x=0; x<taskset.tasks.size();++x){
        std::vector<SubTask*> V = taskset.tasks[x].getVertices();
        // taskset.tasks[x].computeEFTs();
        // taskset.tasks[x].computeLSTs();

        for(int i=0; i<V.size(); ++i){

            
            task_set += std::to_string(x) +  //Task ID
                        ", " + std::to_string(i) +  //Job ID
                        ", 0 " + //Release min
                        ", 0 " + //Release max
                        ", " + std::to_string((int)V[i]->c) + //Cost min
                        ", " + std::to_string((int)V[i]->c) + //Cost max
                        ", " + std::to_string((int)taskset.tasks[x].getDeadline()) + //Deadline
                        ", " + std::to_string((int)taskset.tasks[x].getDeadline()) + "\n";  //Priority
            
        }
    }

    return task_set;

}

std::string convertDAGsToCsv(const Taskset& taskset){
    std::string prec = "Predecessor TID,	Predecessor JID,	Successor TID, Successor JID\n";
    for(int x=0; x<taskset.tasks.size();++x){
        std::vector<SubTask*> V = taskset.tasks[x].getVertices();
        for(int i=0; i<V.size(); ++i){
            for(int j=0; j<V[i]->succ.size(); ++j){
                prec = prec + std::to_string(x) + ", " + std::to_string(i) + ", " + std::to_string(x) + ", " + std::to_string(V[i]->succ[j]->id) + "\n";
            }
        }
    }

    return prec;
}

bool G_LP_FTP_Nasri2019_C(Taskset taskset, const int m){

    
    std::string task_set = convertTasksetToCsv(taskset);
    std::string prec = convertDAGsToCsv(taskset);

    // std::cout<<task_set<<std::endl;
    // std::cout<<prec<<std::endl;

    // std::string fig1a = "   Task ID,     Job ID,          Arrival min,          Arrival max,             Cost min,             Cost max,             Deadline,             Priority\n"
    //                     "1, 1,  0,  0, 1,  2, 10, 10\n"
    //                     "1, 2, 10, 10, 1,  2, 20, 20\n"
    //                     "1, 3, 20, 20, 1,  2, 30, 30\n"
    //                     "1, 4, 30, 30, 1,  2, 40, 40\n"
    //                     "1, 5, 40, 40, 1,  2, 50, 50\n"
    //                     "1, 6, 50, 50, 1,  2, 60, 60\n"
    //                     "2, 7,  0,  0, 7,  8, 30, 30\n"
    //                     "2, 8, 30, 30, 7,  7, 60, 60\n"
    //                     "3, 9,  0,  0, 3, 13, 60, 60\n";

    // std::string fig1a_prec = "Predecessor TID,	Predecessor JID,	Successor TID, Successor JID\n"
    //                             "1, 1,    1, 2\n"
    //                             "1, 2,    1, 3\n"
    //                             "1, 3,    1, 4\n"
    //                             "1, 4,    1, 5\n"
    //                             "1, 5,    1, 6\n"
    //                             "2, 7,    2, 8\n"
    //                             "1, 2,    3, 9\n";
    std::stringstream in;
    in << task_set;

    std::stringstream dag_in;
    dag_in << prec;

    std::stringstream aborts_in;

    return test(in, dag_in, aborts_in, m);
}

