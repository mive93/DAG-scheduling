#include "dagSched/utils.h"

int intRandMaxMin(const int v_min, const int v_max){
    return rand() %  std::max(v_max - v_min, 1) + v_min;
}

float floatRandMaxMin(const float v_min, const float v_max){
    return std::fmod (rand(), std::max(v_max - v_min, float(1))) + v_min;
}

void removePathAndExtension(const std::string &full_string, std::string &name){
    name = full_string;
    std::string tmp_str = full_string;
	std::string slash = "/";
    std::string dot = ".";
    std::size_t current, previous = 0;

    //remove path /path/to/
	current = tmp_str.find(slash);
    if (current != std::string::npos) {
        while (current != std::string::npos) {
            name = tmp_str.substr(previous, current - previous);
            previous = current + 1;
            current = tmp_str.find(slash, previous);
        }
        name = tmp_str.substr(previous, current - previous);
    }
    // remove extension
    current = name.find(dot);
    previous = 0;
    if (current != std::string::npos) 
        name = name.substr(previous, current);
    
    // std::cout<<"full string: "<<full_string<<" name: "<<name<<std::endl;
}

std::vector<std::pair<std::string, std::string>> separateOnComma(const std::string& line){
    std::string left = "", rigth = "";
    bool read_equal = false;
    std::vector<std::pair<std::string, std::string>> pairs;
    for(int i=0; i<line.size(); ++i){
        if(line[i] == ','){
            if(!read_equal || left.empty() || rigth.empty())
                FatalError("Weird DOT file!");
            pairs.push_back(std::make_pair(left, rigth));
            left = "";
            rigth = "";
            read_equal = false;
            continue;
        }
        else if (line[i] == '='){
            read_equal = true;
            continue;
        }
        else if (line[i] == '\"' || line[i] == ' ')
            continue;

        if(read_equal)
            rigth += line[i];
        else
            left += line[i];
    }
    if(!read_equal || left.empty() || rigth.empty())
        FatalError("Weird DOT file!");
    pairs.push_back(std::make_pair(left, rigth));

    return pairs;
}

dot_info parseDOTLine(const std::string& line){
    dot_info line_info;

    size_t start_graph = line.find("{");
    size_t end_graph = line.find("}");
    size_t arrow = line.find("->");
    size_t box = line.find("box");
    size_t start_node = line.find("[");
    size_t end_node  = line.find("]");

    if( start_graph != std::string::npos)
        line_info.lineType = DOTLine_t::DOT_BEGIN;
    else if( end_graph != std::string::npos)
        line_info.lineType = DOTLine_t::DOT_END;
    else if( arrow != std::string::npos){
        line_info.lineType = DOTLine_t::DOT_EDGE;

        std::string id_from_str = line.substr(0, arrow);
        line_info.id_from = std::stoi( id_from_str );

        std::string id_to_str = line.substr(arrow + 2, line.find(";") - (arrow + 2));
        line_info.id_to = std::stoi( id_to_str );
    }
    else if( box != std::string::npos){
        line_info.lineType = DOTLine_t::DAG_INFO;
        std::string brackets_content = line.substr(start_node + 1 , end_node - (start_node + 1));
        auto pairs = separateOnComma(brackets_content);
        for(auto p:pairs){
            if(p.first == "D")
                line_info.deadline = std::stof(p.second);
            if(p.first == "T")
                line_info.period = std::stof(p.second);
        }
    }
    else if( start_node != std::string::npos && end_node != std::string::npos){
        line_info.lineType = DOTLine_t::DOT_NODE;

        std::string node_id_str = line.substr(0, start_node);
        line_info.id = std::stoi(node_id_str);

        std::string brackets_content = line.substr(start_node + 1 , end_node - (start_node + 1));
        auto pairs = separateOnComma(brackets_content);
        for(auto p:pairs){
            if(p.first == "label")
                line_info.wcet = std::stof(p.second);
            if(p.first == "p")
                line_info.p = std::stof(p.second);
            if(p.first == "s")
                line_info.s = std::stof(p.second);
        }
    }
    else
        line_info.lineType = DOTLine_t::VOID_LINE;
    
    return line_info;
}