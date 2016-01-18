#ifndef GRAPH_H_KJDKJRCO
#define GRAPH_H_KJDKJRCO

#include <stdio.h>

#include "util/bool.h"

typedef struct G_graph_ * G_graph;
typedef struct G_node_ * G_node;

typedef struct G_nodeList_ * G_nodeList;
struct G_nodeList_
{
    G_node head;
    G_nodeList tail;
};

G_graph G_Graph (void);
G_node G_Node (G_graph g, void * info);

G_nodeList G_NodeList (G_node head, G_nodeList tail);

/**
 * Get the list of nodes belonging to "g"
 */
G_nodeList G_Nodes (G_graph g);

/**
 * Tell if "a" is in the list "l"
 */
bool G_InNodeList (G_node a, G_nodeList l);

/**
 * Make a new edge joining nodes "from" and "to", which must belong to the same graph
 */
void G_AddEdge (G_node from, G_node to);

/**
 * Delete the edge joining "from" and "to"
 */
void G_RemoveEdge (G_node from, G_node to);

/**
 * Show all the nodes and edges in the graph, using the function "showInfo" to print
 * the name of each node
 */
void G_Show (FILE * out, G_nodeList p, void showInfo (void *));
char * G_ToString (G_nodeList p);

/**
 * Get all the successors of node "n"
 */
G_nodeList G_Succ (G_node n);

/**
 * Get all the predecessors of node "n"
 */
G_nodeList G_Pred (G_node n);

/**
 * Tell if there is an edge from "from" to "to"
 */
bool G_GoesTo (G_node from, G_node n);

bool G_Conn (G_node a, G_node b);

/**
 * Tell how many edges lead to or from "n"
 */
int G_Degree (G_node n);

/**
 * Get all the successors and predecessors of "n"
 */
G_nodeList G_Adj (G_node n);

/**
 * Get the "info" associated with node "n"
 */
void * G_NodeInfo (G_node n);

/**
 * The type of "tables" mapping graph-nodes to information
 */
typedef struct TAB_table_t * G_table;

/**
 * Make a new table
 */
G_table G_Empty (void);

/**
 * Enter the mapping "node"->"value" to the table "t"
 */
void G_Enter (G_table t, G_node node, void * value);

/**
 * Tell what "node" maps to in table "t"
 */
const void * G_Look (G_table t, G_node node);

#endif /* end of include guard: GRAPH_H_KJDKJRCO */
