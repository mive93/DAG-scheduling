#include "evaluate.h"


void saveProcessorInfo(std::ofstream& out_file, const int n_proc ){
    out_file << "---- PROCESADORES	[Numero de procesadores]\n";
    out_file << n_proc << "\n";
    out_file << "[Indice_Procesador	Indice_Sincronizacion]\n";

    for(int i=0; i<n_proc; ++i)
        out_file <<i<<"\t"<<i<<"\n";

}
void savePartitionsInfo(std::ofstream& out_file, const int n_proc){
    out_file << "---- PARTICIONES	[Numero_Particiones]\n";
    out_file << n_proc << "\n";
    out_file << "--- #Particion   ---- Procesador	----  Available CPU [0-1]   \n";

    for(int i=0; i<n_proc; ++i)
        out_file <<i<<"\t"<<i<<"\t"<<1<<"\n";
}
void saveTransactionsInfo(std::ofstream& out_file, const int n_tasks){
    out_file << "---- TRANSACCIONES	[Numero_Transacciones]\n";
    out_file << n_tasks << "\n";
}

void saveDAG(std::ofstream& out_file, const DAGTask task){
    auto V = task.getVertices();

    out_file << "[Periodo	N_Tareas	]\n";
    out_file << task.getDeadline()<<"\t"<< V.size() << "\n";

    out_file << "[Cij	Cbij   	Offset	Prio		Deadlineij	   	   Bij		Particion   	Sincronizada(0=False)	   M_entrada		M_salida		NTin	TaskIn		NTout	TaskOut]  \n";
    for(int i=0; i<V.size(); ++i){
        out_file    << V[i]->c << "\t"
                    << V[i]->c << "\t"
                    << 0 << "\t"
                    << 1 << "\t"
                    << task.getDeadline() << "\t"
                    << 0 << "\t"
                    << 0 << "\t"
                    << 0 << "\t"
                    << 0 << "\t"
                    << 0 << "\t"
                    << V[i]->pred.size() << "\t";

                
        for(auto p:V[i]->pred)
            out_file<< p->id << " ";

        if(V[i]->pred.size() == 0)
            out_file<< 0;

        out_file << "\t" << V[i]->succ.size() << "\t";

        for(auto s:V[i]->succ)
            out_file<< s->id << " ";

        if(V[i]->succ.size() == 0)
            out_file<< 0;

        out_file<< "\n";
    }
    
}

void saveMessagesInfo(std::ofstream& out_file, const int n_proc){
    out_file << "[N_Mensajes]\n";
    out_file << 0 << "\n";
    out_file << "[ID 	LatMax   	LatMin] \n";

    for(int i=0; i<n_proc; ++i)
        out_file <<i<<"\t"<<0<<"\t"<<0<<"\n";
}

void saveInTxt(const int test_idx, const Taskset& task_set, const int n_proc){

    std::string out_file_path = "test_"+std::to_string(test_idx)+".txt";
    std::ofstream out_file;
    out_file.open (out_file_path);

    saveProcessorInfo(out_file, n_proc);
    savePartitionsInfo(out_file, n_proc);
    saveTransactionsInfo(out_file, task_set.tasks.size());

    for(int x=0; x<task_set.tasks.size();++x)
        saveDAG(out_file, task_set.tasks[x]);

    saveMessagesInfo(out_file, n_proc);

    out_file.close();

}

int main(int argc, char *argv[]) {

    if(REPRODUCIBLE) srand (1);
    else srand (time(NULL));

    std::string genparams_path = "../data/Andoni_test.yml";
    if(argc > 1)
        genparams_path = argv[1];
    bool save_dot_files = false;
    if(argc > 2)
        save_dot_files = std::atoi(argv[2]);


    //parameters of the generator
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


    int test_idx = -1;

    //for loop to generate nTasksets task sets
    for(int i=0; i<gp.nTasksets; ++i){
        if(gp.gType == GenerationType_t::VARYING_U && i % gp.tasksetPerVarFactor == 0)
            U_curr += gp.stepU;
        else if(gp.gType == GenerationType_t::VARYING_N && i % gp.tasksetPerVarFactor == 0)
            n_tasks += gp.stepN;
        else if(gp.gType == GenerationType_t::VARYING_M && i % gp.tasksetPerVarFactor == 0)
            m += gp.stepM;

        test_idx++;
        
        //actual generation of a single taskset
        Taskset task_set;
        task_set.generate_taskset_Melani(n_tasks, U_curr, m, gp);

        if(save_dot_files){
            for(int x=0; x<task_set.tasks.size();++x){
                std::string filename = "test_"+std::to_string(test_idx)+"_"+std::to_string(x);
                task_set.tasks[x].saveAsDot(filename+".dot");
                std::string dot_command = "dot -Tpng "+filename+".dot > "+filename+".png";
                system(dot_command.c_str());
            }
        }

        saveInTxt(test_idx, task_set, m);

        std::cout   <<"taskset: "   <<i
                    <<" U: "        <<U_curr
                    <<" ntasks: "   <<task_set.tasks.size()
                    <<" m:"         <<m
                    <<" test_idx: " <<test_idx
                    <<std::endl;

        //task set destruction
        for(auto &t:task_set.tasks)
            t.destroyVerices();
    }

    

    return 0;
}