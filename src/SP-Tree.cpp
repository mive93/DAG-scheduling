#include "SP-Tree.h"

bool compareSTempNode(const STempNode& a, const STempNode& b){
    return a.right_ordids < b.right_ordids;
}

void SPTree::printDotTree(SPNode *node, std::ostream& os, bool print_node, bool print_edges){

    if(print_node){
        switch (node->type){
        case NodeType_t::S:
            os<<node->id<<" [label=\"S";
            break;
        case NodeType_t::P:
            os<<node->id<<" [label=\"P";
            break;
        case NodeType_t::L:
            os<<node->id<<" [label=\""<<node->V_id;
            break;
        }

        os<<"("<<node->id<<")\"];\n";
    }
    else if(print_edges){
        if(node->left != nullptr)
            os<<node->id<<" -> "<<node->left->id<<";\n";
        if(node->right != nullptr)
            os<<node->id<<" -> "<<node->right->id<<";\n";
    }

    if(node->left == nullptr && node->right == nullptr)
        return;
    
    if(node->left != nullptr)
        printDotTree(node->left, os, print_node, print_edges);
    if(node->right != nullptr)
        printDotTree(node->right, os, print_node, print_edges);
}

void SPTree::saveAsDot(SPNode *node,const std::string filename){
    std::ofstream of(filename + ".dot");
    of<<"digraph Task {\n";

    printDotTree(node, of, true, false);
    printDotTree(node, of, false, true);

    of<<"}";
    of.close();

    std::string dot_command = "dot -Tpng "+filename+".dot > "+filename+".png";
    system(dot_command.c_str());
}

std::vector<std::vector<int>> SPTree::computeJoinsForVertices(const DAGTask& dag){
    auto V = dag.getVertices();
    auto ordIDs = dag.getTopologicalOrder();
    std::vector<std::vector<int>> J_nodes(V.size());
    bool ok_join = false;

    for(int idx_1 = 0, i; idx_1 < ordIDs.size(); ++idx_1 ){
        i = ordIDs[idx_1];
        if (V[i]->pred.size() > 1){ // join
            for(int k=0; k< V.size(); ++k){
                if (k != i){
                    ok_join = false;
                    dag.isSuccessor(V[i], V[k], ok_join);
                    if(ok_join)
                        J_nodes[k].push_back(i);
                }
            }
            J_nodes[i].push_back(i);
        }
    }
    return J_nodes;
}

std::vector<STempNode> SPTree::computeInitialS(const DAGTask& dag){
    auto V = dag.getVertices();
    auto ordIDs = dag.getTopologicalOrder();
    std::vector<struct STempNode> S;

    for(int idx_1 = 0, i; idx_1 < ordIDs.size(); ++idx_1 ){
        i = ordIDs[idx_1];

        for(int j=0; j<V[i]->succ.size(); ++j){
            struct STempNode n;
            n.left = i;
            n.right = V[i]->succ[j]->id;
            n.left_ordids = idx_1;
            for(int idx_2 = 0; idx_2 < ordIDs.size(); ++idx_2 ){
                if(V[ordIDs[idx_2]]->id == V[i]->succ[j]->id){
                    n.right_ordids = idx_2;
                    break;
                }
            }
            S.push_back(n);
        }
    }
    return S;
}

SPNode* SPTree::createNode(const NodeType_t type, const int V_id){
    SPNode * node = new SPNode;
    node->V_id = V_id;
    node->id = index++;
    node->type = type;
    return node;
}

SPNode* SPTree::createP(SPNode* node, const int id_right, const int id_left){
    // P node
    node = createNode(NodeType_t::P);
    // left
    node->left = createNode(NodeType_t::L, id_left);
    //right
    node->right = createNode(NodeType_t::L, id_right);
    return node;
}

std::vector<SPNode*> SPTree::computePSubtrees(const std::vector<std::vector<int>>& J_nodes, std::vector<STempNode>& S){
    std::vector<SPNode*> Ps;

    for(int i=0; i<S.size(); ++i){
        if(!S[i].visited){

            std::vector<int> S_idx;
            S_idx.push_back(i);
            S[i].visited = true;

            // compute all S that have the same left VId
            for(int j=i+1; j<S.size(); ++j){
                if(S[i].left == S[j].left){
                    S_idx.push_back(j);
                    S[j].visited = true;
                }
            }

            if(S_idx.size() > 1){ //make parallel 

                
                SPNode * s_node = createNode(NodeType_t::S) ; //S
                s_node->left = createNode(NodeType_t::L, S[S_idx[0]].left); //S-> left

                if(S_idx.size() == 2){ //easy case
                    s_node->right = createP(s_node->right, S[S_idx[0]].right, S[S_idx[1]].right);
                    Ps.push_back(s_node);

                    S[S_idx[0]].used = true;
                    S[S_idx[1]].used = true;
                }
                else{
                    std::vector<SPNode*> same_left;
                    
                    for(int j=0; j<S_idx.size(); ++j){
                        //associate first the ones with same joins
                        for(int k=0; k<S_idx.size(); ++k){
                            if( j != k && 
                                !S[S_idx[k]].used && !S[S_idx[j]].used && 
                                J_nodes[S[S_idx[k]].right] == J_nodes[S[S_idx[j]].right]){
                                
                                SPNode *node = createP(s_node->right, S[S_idx[j]].right, S[S_idx[k]].right);
                                S[S_idx[j]].used = true;
                                S[S_idx[k]].used = true;
                                same_left.push_back(node);
                            }
                        }

                        //associate additional with same joins to subtrees
                        for(int k=0; k<same_left.size(); ++k){
                            if( !S[S_idx[j]].used && 
                                J_nodes[S[S_idx[j]].right] == J_nodes[same_left[k]->right->V_id]){

                                SPNode *new_p = createNode(NodeType_t::P);
                                new_p->left = same_left[k];
                                new_p->right = createNode(NodeType_t::L, S[S_idx[j]].right);
                                S[S_idx[j]].used = true;
                                same_left[k] = new_p;
                            
                            }
                        }
                    }

                    //merge Ps
                    SPNode * root_p = nullptr;
                    if(same_left.size() == 1)
                        root_p = same_left[0];
                    else 
                        root_p = createNode(NodeType_t::P);
                    for(int k=0; k<same_left.size(); ++k){
                        if(root_p->left == nullptr)
                            root_p->left = same_left[k];
                        else if(root_p->right == nullptr)
                            root_p->right = same_left[k];
                        else{
                            SPNode* new_root_p = createNode(NodeType_t::P);
                            new_root_p->left = root_p;
                            new_root_p->right = same_left[k];
                            root_p = new_root_p;
                        }
                    }

                    // merge lonely nodes with completely different joins
                    for(int j=0; j<S_idx.size(); ++j){
                        if(!S[S_idx[j]].used){
                            SPNode* new_root_p = createNode(NodeType_t::P);
                            new_root_p->left = root_p;
                            new_root_p->right = createNode(NodeType_t::L, S[S_idx[j]].right);
                            root_p = new_root_p;
                        }
                    }

                    s_node->right = root_p;
                    Ps.push_back(s_node);
                }
            }
        }
    }

    return Ps;
}

void SPTree::getTreeVIDs(SPNode * node, std::vector<std::pair<int, SPNode*>>& V_ids){
    if(node->left!= nullptr && node->left->type == NodeType_t::L)
        V_ids.push_back(std::make_pair(node->left->V_id, node));

    if(node->right!= nullptr && node->right->type == NodeType_t::L)
        V_ids.push_back(std::make_pair(node->right->V_id, node));

    //recursion
    if(node->left == nullptr && node->right== nullptr)
        return;

    getTreeVIDs(node->left, V_ids);
    getTreeVIDs(node->right, V_ids);
}

std::vector<SPNode*> SPTree::mergeSubtrees(std::vector<SPNode*> subtrees){

    std::vector<SPNode*> new_subtrees;
    bool init = true;
    while(new_subtrees.size() != subtrees.size()){
        if(!init){
            subtrees = new_subtrees;
            new_subtrees.clear();
        }
        init = false;
        if(subtrees.size() == 1)
            break;

        std::vector<bool> used (subtrees.size(), false);

        for(int i=0; i<subtrees.size();++i){
            // retrieve all the ids of subtree i
            std::vector<std::pair<int, SPNode*>> Vids_i;
            getTreeVIDs(subtrees[i], Vids_i);

            for(int j=0; j<subtrees.size();++j){
                if(i != j && !used[i] && !used[j]){
                    // retrieve all the ids of subtree j
                    std::vector<std::pair<int, SPNode*>> Vids_j;
                    getTreeVIDs(subtrees[j], Vids_j);

                    // if there is a common id -> merge
                    for(int ii=0; ii< Vids_i.size(); ++ii){
                        for(int jj=0; jj< Vids_j.size(); ++jj){
                            if(Vids_i[ii].first == Vids_j[jj].first){ // common id
                                //merge
                                if(Vids_i[ii].second->type == NodeType_t::S && Vids_i[ii].second->left->V_id == Vids_i[ii].first){
                                    if(Vids_j[jj].second->left->V_id == Vids_j[jj].first) // common id is on the left of subtree j
                                        Vids_j[jj].second->left = Vids_i[ii].second;
                                    else // common id is on the right of subtree j
                                        Vids_j[jj].second->right = Vids_i[ii].second;

                                    new_subtrees.push_back(subtrees[j]);
                                }
                                else if(Vids_j[jj].second->type == NodeType_t::S && Vids_j[jj].second->left->V_id == Vids_j[jj].first){
                                    if(Vids_i[ii].second->left->V_id == Vids_i[ii].first) // common id is on the left of subtree i
                                        Vids_i[ii].second->left = Vids_j[jj].second;
                                    else // common id is on the right of subtree i
                                        Vids_i[ii].second->right = Vids_j[jj].second;

                                    new_subtrees.push_back(subtrees[i]);

                                }
                                else
                                    FatalError("The vid is never son of S");

                                used[i] = true;
                                used[j] = true;
                                break;
                            }
                            if(used[j]) break;
                        }
                        if(used[i]) break;
                    }
                }
            }
        }
        for(int i=0; i<subtrees.size();++i){
            if(!used[i])
                new_subtrees.push_back(subtrees[i]);
        }
    }

    return subtrees;

}

void SPTree::findNode(SPNode* start_node, const int VId, SPNode *& found_node){
    if(start_node->type == NodeType_t::L && start_node->V_id == VId){
        found_node = start_node;
        return;
    }

    //recursion        
    if(start_node->left == nullptr && start_node->right== nullptr)
        return;

    findNode(start_node->left, VId, found_node);
    findNode(start_node->right, VId, found_node);
}

void SPTree::findDad(SPNode* start_node, const SPNode* son, SPNode *& dad) const{
    if(start_node->type == NodeType_t::L)
        return;

    if( start_node->left!= nullptr && start_node->left == son){
        dad = start_node;
        return;
    }

    if( start_node->right!= nullptr && start_node->right == son){
        dad = start_node;
        return;
    }

    if(start_node->left == nullptr && start_node->right== nullptr)
        return;

    findDad(start_node->left, son, dad);
    findDad(start_node->right, son, dad);
}

void SPTree::isSon(const SPNode* dad, const SPNode* son, bool& is_son){
    if(dad->type == NodeType_t::L)
        return;

    if(dad->left == son || dad->right == son)
        is_son = true;
    
    if(dad->left != nullptr)
        isSon(dad->left, son, is_son);
    if(dad->right != nullptr)
        isSon(dad->right, son, is_son);

}

SPNode * SPTree::computeCommonRoot(std::vector<SPNode*> subtrees, std::vector<STempNode> S, std::vector<int> S_idx, int& subtree_idx){

    // pick subtree
    subtree_idx = 0;
    if(subtrees.size() > 1){
        subtree_idx = -1;
        for(int i=0; i<subtrees.size(); ++i){
            std::vector<std::pair<int, SPNode*>> Vids_i;
            getTreeVIDs(subtrees[i], Vids_i);

            bool all_s_found = true;
            bool s_found = false;
            for(int s=0; s<S_idx.size(); ++s){
                s_found = false;
                for(int ii=0; ii< Vids_i.size();++ii){
                    if(S[S_idx[s]].left == Vids_i[ii].first){
                        s_found = true;
                        break;
                    }
                    if(!s_found){
                        all_s_found = false;
                        break;
                    }
                }
            }

            if(all_s_found){
                subtree_idx = i;
                break;
            }
        }
    }

    if(subtree_idx == -1)
        FatalError("subtree will all ids not found");


    SPNode *cur_node = nullptr;
    SPNode *dad = nullptr;
    SPNode *tmp_dad = nullptr;
    bool is_son = false;
    for(int s=0; s<S_idx.size(); ++s){
        cur_node = nullptr;
        findNode(subtrees[subtree_idx], S[S_idx[s]].left, cur_node);
        if(cur_node == nullptr){
            std::cout<<S[S_idx[s]].left<<std::endl;
            saveAsDot(subtrees[subtree_idx], "error");
            FatalError("Node not found in the subtree");
        }

        if(dad == nullptr)
            findDad(subtrees[subtree_idx], cur_node, dad);
        else{
            isSon(dad, cur_node, is_son);
            while(!is_son){
                findDad(subtrees[subtree_idx], dad, tmp_dad);
                dad = tmp_dad;
                isSon(dad, cur_node, is_son);
            }
        }
    }

    return dad;
}

void SPTree::insertS(std::vector<SPNode*>&subtrees, std::vector<STempNode>& initial_S){
    for(auto&s:initial_S)
        s.visited = false;

    if(subtrees.size() > 1)
        subtrees = mergeSubtrees(subtrees);

    std::sort(initial_S.begin(), initial_S.end(), compareSTempNode);

    for(int i=0; i<initial_S.size(); ++i){
        if(!initial_S[i].used && !initial_S[i].visited){
            std::vector<int> S_idx;
            S_idx.push_back(i);
            initial_S[i].visited = true;
            initial_S[i].used = true;
            for(int j=i+1; j<initial_S.size();++j){
                if(!initial_S[j].used && !initial_S[j].visited && initial_S[i].right == initial_S[j].right){
                    S_idx.push_back(j);
                    initial_S[j].visited = true;
                    initial_S[j].used = true;
                }
            }

            if(subtrees.size() == 0){
                // there are no P construct in the DAG
                if(S_idx.size() > 1 )
                    FatalError("Problem, the first S can't be already a join");

                SPNode * S_node = createNode(NodeType_t::S);
                S_node->left = createNode(NodeType_t::L, initial_S[i].left);
                S_node->right = createNode(NodeType_t::L, initial_S[i].right);
                subtrees.push_back(S_node);

            }
            else{

                int subtree_idx = 0;
                SPNode * common_root = computeCommonRoot(subtrees, initial_S, S_idx, subtree_idx);
                SPNode * S_node = createNode(NodeType_t::S);
                S_node->left = common_root;
                S_node->right = createNode(NodeType_t::L, initial_S[i].right);
                if(common_root == subtrees[subtree_idx]){
                    subtrees[subtree_idx] = S_node;
                }
                else{

                    if( S_idx.size() == 1 &&
                        (common_root->left->V_id == initial_S[i].left ||
                        common_root->right->V_id == initial_S[i].left)){

                        S_node->left = createNode(NodeType_t::L, initial_S[i].left);

                        if(common_root->left->V_id == initial_S[i].left)
                            common_root->left = S_node;
                        else if(common_root->right->V_id == initial_S[i].left)
                            common_root->right = S_node;

                        else
                            FatalError("Problem in the tree construction");
                    }

                    else{

                        SPNode * common_root_dad = nullptr;
                        findDad(subtrees[subtree_idx], common_root, common_root_dad);

                        if(common_root_dad->left == common_root)
                            common_root_dad->left = S_node;
                        else if(common_root_dad->right == common_root)
                            common_root_dad->right = S_node;
                    }
                }
            }
            if(subtrees.size() > 1)
                subtrees = mergeSubtrees(subtrees);
        }
    }
}

void SPTree::convertNFJDAGtoSPTree(const DAGTask& dag, const int dag_id){

    std::vector<std::vector<int>> J_nodes = computeJoinsForVertices(dag);
    std::vector<STempNode> initial_S = computeInitialS(dag);
    std::vector<SPNode*> subtrees = computePSubtrees(J_nodes, initial_S);
    insertS(subtrees, initial_S);

    if(subtrees.size() > 1)
        FatalError("Was not able to merge all subtrees");

    // saveAsDot(subtrees[0], "final_tree" + std::to_string(dag_id));
    root = subtrees[0];
}

std::vector<int> SPTree::computeP(  SPNode * node, const std::vector<SubTask*>& V){

    std::vector<int> par_ids_l, par_ids_r;
    if(node->left != nullptr)
        par_ids_l = computeP(node->left, V);
    if(node->right != nullptr)
        par_ids_r = computeP(node->right, V);

    if(node->type == NodeType_t::P){
        std::vector<int> par_ids;
        par_ids.insert(par_ids.end(), par_ids_l.begin(), par_ids_l.end());
        par_ids.insert(par_ids.end(), par_ids_r.begin(), par_ids_r.end());
        return par_ids;
    }
    else if(node->type == NodeType_t::S){
        if(par_ids_r.size() > par_ids_l.size())
            return par_ids_r;
        else
            return par_ids_l;
    }
    else{       
        std::vector<int> par_ids;
        if(V[node->V_id]->c > 0)
            par_ids.push_back(node->V_id);
        return par_ids;
    }
}

std::vector<std::pair<float, float>> SPTree::computeWDUCO(const DAGTask& dag, const int dag_id){

    std::vector<std::pair<float, float>> WD_UCO_y;

    auto V = dag.getVertices();
    float width;

    while(true){
        auto Ps = computeP(root, V);
        if(Ps.size() == 0)
            break;
        
        // printVector<int>(Ps);
        
        width = V[Ps[0]]->c;
        for(auto &p:Ps){
            if(V[p]->c < width)
                width = V[p]->c;
        }

        // std::cout<<width << " "<< Ps.size()<<std::endl;

        WD_UCO_y.push_back(std::make_pair(width, Ps.size()));

        for(auto &p:Ps)
            V[p]->c -= width;
        
    }

    return WD_UCO_y;
}

SPNode * SPTree::freeNode(SPNode *&node){
    if(node->left!= nullptr)
        node->left = freeNode(node->left);
    if(node->right != nullptr)
        node->right = freeNode(node->right);

    if(node != nullptr && node->left == nullptr && node->right == nullptr){
        delete node;
        node = nullptr;
        return nullptr;
    }
}
