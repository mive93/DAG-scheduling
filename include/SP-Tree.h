#ifndef SPTREE_H
#define SPTREE_H

#include <vector>
#include "utils.h"
#include "DAGTask.h"

enum NodeType_t{ S, P, L}; 

struct SPNode{
    NodeType_t type;
    int id          = -1;       // id in the tree
    int V_id        = -1;       // corresponding id in the DAG
    SPNode *left    = nullptr;  // left son
    SPNode *right   = nullptr;  // right son
};

struct STempNode{
    int left            = -1;
    int right           = -1;
    int right_ordids    = -1;  // topological order in the DAG of right
    int left_ordids     = -1;   // topological order in the DAG of left
    bool used           = false;
    bool visited        = false;
};

bool compareSTempNode(const STempNode& a, const STempNode& b);

class SPTree{

    SPNode *root = nullptr;
    int index = 0;

    // output -----------------------------------------------------------------
    void printDotTree(SPNode *node, std::ostream& os, bool print_node =true, bool print_edges=false);
    void saveAsDot(SPNode *node,const std::string filename);

    // node creation and deletion ---------------------------------------------
    SPNode* createNode(const NodeType_t type, const int V_id=-1);
    SPNode* createP(SPNode* node, const int id_right, const int id_left);
    SPNode * freeNode(SPNode *&node);

    //searches in the tree ----------------------------------------------------

    /* Find the node corresponding to VId in a tree */
    void findNode(SPNode* start_node, const int VId, SPNode *& found_node);
    /* Find the dad of a node */
    void findDad(SPNode* start_node, const SPNode* son, SPNode *& dad) const;
    /* Check if a node is son of another*/
    void isSon(const SPNode* dad, const SPNode* son, bool& is_son);
    /* Get all VIds belonging to a tree */
    void getTreeVIDs(SPNode * node, std::vector<std::pair<int, SPNode*>>& V_ids);

    //SP decomposition tree conversion ----------------------------------------

    /* Computes join nodes for each vertex.
        @param dag: dag G (V, E)to work on
        @return a vector with length = |V|, and for each vertex a list of corresponding join vertices
    */
    std::vector<std::vector<int>> computeJoinsForVertices(const DAGTask& dag);

    /* Create initial sequential couple, for each vertex in the dag, 
     * following topological order
     * */
    std::vector<STempNode> computeInitialS(const DAGTask& dag);
    
    /* This function consider all the S couple. If it finds more that one 
     * having the same left VId, then it merges them in a parallel construct.
     * However, if there are many, they are associated wrt their join nodes.
     * */
    std::vector<SPNode*> computePSubtrees(const std::vector<std::vector<int>>& J_nodes, std::vector<STempNode>& S);

    std::vector<SPNode*> mergeSubtrees(std::vector<SPNode*> subtrees);

    /* Finds the common root, among the subtrees, of all the S with same right */
    SPNode * computeCommonRoot(std::vector<SPNode*> subtrees, std::vector<STempNode> S, std::vector<int> S_idx, int& subtree_idx);

    /* Insert all the remaining S (once the Ps are already computed and merged)*/
    void insertS(std::vector<SPNode*>&subtrees, std::vector<STempNode>& initial_S);

    public:

    SPTree(){}
    ~SPTree(){
        // freeNode(root); //FIXME
    }

    /* Convert a NFJ DAG into a decomposition Sequential-Parallel binary tree*/
    void convertNFJDAGtoSPTree(const DAGTask& dag, const int dag_id);

    //Fonseca method (2017 & 2019) --------------------------------------------
    std::vector<std::pair<float, float>> computeWDUCO(const DAGTask& dag, const int dag_id);
    /* Equation 8 Fonseca 2017*/
    std::vector<int> computeP(  SPNode * node, const std::vector<SubTask*>& V);
};


#endif /* SPTREE_H */