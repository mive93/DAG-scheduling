#include "evaluate.h"
int main(int argc, char *argv[]) {

    if(REPRODUCIBLE) srand (1);
    else srand (time(NULL));

    std::string gp_file = "../data/n_task_DAG_EDF_C_varyingU.yml";
    if(argc > 1)
        gp_file = argv[1];
    bool show_plots = true;
    if(argc > 2)
        show_plots = atoi(argv[2]);

    std::string o_file;
    removePathAndExtension(gp_file, o_file);

    system("mkdir -p res");

    evaluate(gp_file, "res/" + o_file, show_plots);
    return 0;
}