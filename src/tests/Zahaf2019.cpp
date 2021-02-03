#include "tests.h"

#ifdef ZAHAF2019

#include "task/task.hpp"
#include "gramm/hdag_driver.hpp"
#include "code_gen/taskset_code.hpp"
#include "platform/platform.hpp"
#include "analysis/hpc_dag/analysis.hpp"

bool P_LP_EDF_Zahaf2019_C(const Taskset& taskset, const int m){

    std::ofstream output_file;
    std::string filename = "cur_zahaf2019.txt";
    output_file.open (filename);

    int prev_v = 0;
  
    for(int x=0; x<taskset.tasks.size(); ++x){    
        std::vector<SubTask*> V = taskset.tasks[x].getVertices();
        for(int i=0;i<V.size();++i)
            output_file<<"Node J"<<(V[i]->id+1)+prev_v<<" (C="<<V[i]->c<<", TAG=CPU);\n";

        output_file<<"Graph tau"<<x+1<<" (D="<<taskset.tasks[x].getDeadline()<<", T="<<taskset.tasks[x].getPeriod()<<");\n";
        output_file<<"tau"<<x+1<<" = {\n";

        for(int i=0;i<V.size();++i)
            for(int j=0; j<V[i]->succ.size(); ++j)
                output_file<<"precond( J"<<(V[i]->id+1)+prev_v<<", J"<<(V[i]->succ[j]->id+1)+prev_v<<");\n";
        output_file<<"};\n";

        prev_v +=  V.size();
    }

    // output_file<<"generate(tau1,\"tau.dot\", BOTH);\n";
    // system("dot -Tpng tau.dot > tau.png");
    output_file<<"Platform((CPU,EDFFP)*"<<m<<");\n";
    output_file<<"build();\n";
    output_file<<"analyse(PARTIAL, FAIR, WF, RANDOM);\n";

    output_file.close();

    hdag_driver *driver = new hdag_driver();
    int res = driver->parse(filename);

    delete driver;

    return res;
}

#endif