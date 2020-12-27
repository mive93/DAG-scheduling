#include "evaluate.h"
int main(int argc, char *argv[]) {

    if(REPRODUCIBLE) srand (1);
    else srand (time(NULL));

    std::string gp_file = "../data/test_micaela.yml";
    if(argc > 1)
        gp_file = argv[1];

    std::string o_file;
    removePathAndExtension(gp_file, o_file);

    evaluate(gp_file, o_file);
    return 0;
}