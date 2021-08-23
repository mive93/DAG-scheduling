#include "dagSched/tests.h"

#ifdef BARUAH2020

namespace dagSched{
//  Baruah, Sanjoy. "Scheduling DAGs When Processor Assignments Are Specified." RTNS 2020.

bool G_LP_FTP_Baruah2020_C_exact(const DAGTask& task, const int m){

    std::ofstream DAG_file;
    DAG_file.open ("curDAG.txt");

    std::vector<int> core_count(m, 0);
    
    std::vector<SubTask*> V = task.getVertices();

    // write WCETS
    for(int i=0; i<V.size(); ++i){
        DAG_file << V[i]->c << ",";
        core_count[V[i]->core]++;
    }

    DAG_file << "\n";

    // // write node count per processor
    // for(int i=0; i<core_count.size(); ++i)
    //     DAG_file << core_count[i] << ",";
    
    // write processor assignment
    for(int i=0; i<V.size(); ++i)
        DAG_file << V[i]->core << ",";
    DAG_file << "\n";

    // write edges
    for(int i=0; i<V.size(); ++i)
        for(int j=0; j<V[i]->succ.size(); ++j)
            DAG_file << "("<< i << "," << V[i]->succ[j]->id << ")" << ",";
    DAG_file << "\n";

    // write deadline
    DAG_file << task.getDeadline() << "\n";

    DAG_file.close();

    int status = system("python ../ExactSchedDAG/exactDAG_partitioned_general.py curDAG.txt");
    if (status < 0 || (WEXITSTATUS(status) != 1 && WEXITSTATUS(status) != 2 && WEXITSTATUS(status) != 42))
        FatalError("Problem with Python. Maybe you need to activate the correct environment (conda activate exactDAG)");

    if(WEXITSTATUS(status) == 42)
        return true;
    return false;
}

}
#endif