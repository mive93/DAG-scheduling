#include "DAGTask.h"

std::ostream& operator<<(std::ostream& os, const DAGTask& t)
{

    os<<"----------------------------------------------------\n";
    os<< "deadline :" << t.d <<std::endl;
    os<< "period :" << t.t <<std::endl;
    os<< "length :" << t.L <<std::endl;
    os<< "volume :" << t.vol <<std::endl;
    os<< "wcw :" << t.wcw <<std::endl;
    os<< "utilization :" << t.u <<std::endl;
    os<< "density :" << t.delta <<std::endl;
    os<< "vertices :"<<std::endl;
    for(auto v: t.V){
        os<< "\t v_" << v->id << " - c: "<<v->c <<" \tsucc: ";
        for(auto s:v->succ)
            os<< s->id << " ";
        os<<" \tprec: ";
        for(auto p:v->pred)
            os<< p->id << " ";

        os<<" \tlocal O: " << v->localO;
        os<<" \tlocal D: " << v->localD;
        os<<" \n";
    }
    if(t.ordIDs.size()){
        std::cout<<"Topological order: ";
        for(auto id:t.ordIDs)
            std::cout<<id<< " ";
        std::cout<<std::endl;
    }

    os<<"----------------------------------------------------\n";
    return os;

}

void DAGTask::readTaskFromYamlNode(YAML::Node tasks, const int i){
    t = tasks[i]["t"].as<int>();
    d = tasks[i]["d"].as<int>();

    YAML::Node vert = tasks[i]["vertices"];

    std::map<int, int> id_pos;

    for(int j=0; j<vert.size(); j++){
        SubTask *v = new SubTask;
        v->id = j;
        v->c = vert[j]["c"].as<int>();

        id_pos[vert[j]["id"].as<int>()] = j;

        if(vert[j]["s"])
            v->gamma = vert[j]["s"].as<int>();

        if(vert[j]["p"])
            v->core = vert[j]["p"].as<int>();

        V.push_back(v);
    }

    YAML::Node edges = tasks[i]["edges"];
    int form_id, to_id;
    for(int j=0; j<edges.size(); j++){

        //add check
        form_id = id_pos[edges[j]["from"].as<int>()];
        to_id = id_pos[edges[j]["to"].as<int>()];

        V[form_id]->succ.push_back(V[to_id]);
        V[to_id]->pred.push_back(V[form_id]);
    }
    
}

void DAGTask::saveAsDot(const std::string &filename){
    std::ofstream of(filename);

    of<<"digraph Task {\n";

    of<<"i [shape=box, label=\"D="<<d<<" T="<<t<<"\"]; \n";
    for (const auto &v: V){
        of<<v->id<<" [label=\""<<v->c<<"("<<v->id<<", p:"<<v->core<<")"<<"\"";
        if(v->mode == C_SOURCE_T) of<<",shape=diamond";
        else if(v->mode == C_SINK_T) of<<",shape=box";
        of<<"];\n";
    }

    for (const auto &v: V){
        for(auto s: v->succ)
            of<<v->id<<" -> "<<s->id<<";\n";
    }
    of<<"}";

    of.close();
}

