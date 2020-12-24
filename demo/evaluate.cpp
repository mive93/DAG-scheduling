#include "evaluate.h"

int main(int argc, char *argv[]) {

    if(REPRODUCIBLE) srand (1);
    else srand (time(NULL));

    std::string gp_file = "../data/n_task_DAG_EDF_C_varyingU.yml";
    if(argc > 1)
        gp_file = argv[1]; 

    evaluate(gp_file);
    return 0;
}