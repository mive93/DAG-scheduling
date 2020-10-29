#include "task.h"

#include <iostream>
#include <fstream>

std::ostream& operator<<(std::ostream& os, const DAGTask& t)
{

    os<<"----------------------------------------------------\n";
    os<< "deadline :" << t.d <<std::endl;
    os<< "period :" << t.t <<std::endl;
    os<< "vertices :"<<std::endl;
    for(auto v: t.V){
        os<< "\t v_" << v.id << " - c: "<<v.c <<" \tsucc: ";
        for(auto s:v.succ)
            os<< s->id << " ";
        os<<" \tprec: ";
        for(auto p:v.prec)
            os<< p->id << " ";
        os<<" \n";
    }
    os<<"----------------------------------------------------\n";
    return os;

}

void DAGTask::readTaskFromYaml(const std::string& params_path){
    YAML::Node config   = YAML::LoadFile(params_path);
    YAML::Node tasks = config["tasks"];

    for(int i=0; i<tasks.size(); i++){
        t = tasks[i]["t"].as<int>();
        d = tasks[i]["d"].as<int>();

        YAML::Node vert = tasks[i]["vetices"];

        std::map<int, int> id_pos;

        for(int j=0; j<vert.size(); j++){
            SubTask v;
            v.id = vert[j]["id"].as<int>();
            v.c = vert[j]["c"].as<int>();

            id_pos[v.id] = j;

            V.push_back(v);
        }

        YAML::Node edges = tasks[i]["edges"];
        int form_id, to_id;
        for(int j=0; j<edges.size(); j++){

            //add check
            form_id = id_pos[edges[j]["from"].as<int>()];
            to_id = id_pos[edges[j]["to"].as<int>()];

            V[form_id].succ.push_back(&V[to_id]);
            V[to_id].prec.push_back(&V[form_id]);
        }
    }
}

void DAGTask::saveAsDot(const std::string &filename){
    std::ofstream of(filename);

    of<<"digraph Task {\n";

    of<<"i [shape=box, label=\"D="<<d<<" T="<<t<<"\"]; \n";
    for (const auto &v: V)
        of<<v.id<<" [label=\""<<v.c<<"\"];\n";

    for (const auto &v: V){
        for(auto s: v.succ)
            of<<v.id<<" -> "<<s->id<<";\n";
    }
    of<<"}";

    of.close();
}