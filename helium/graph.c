#include <stdio.h>
#include <assert.h>

#include "ext/table.h"
#include "ext/mem.h"

#include "symbol.h"
#include "temp.h"
#include "tree.h"
#include "ast.h"
#include "asm.h"
#include "frame.h"
#include "graph.h"

struct G_graph_
{
    int nodecount;
    G_nodeList nodes, last;
};

struct G_node_
{
    G_graph graph;
    int key;
    G_nodeList succs;
    G_nodeList preds;
    void * info;
};

G_graph G_Graph (void)
{
    G_graph g = checked_malloc (sizeof (* g));
    g->nodecount = 0;
    g->nodes = NULL;
    g->last = NULL;
    return g;
}

G_nodeList G_NodeList (G_node head, G_nodeList tail)
{
    G_nodeList n = checked_malloc (sizeof (* n));
    n->head = head;
    n->tail = tail;
    return n;
}

G_node G_Node (G_graph g, void * info)
{
    assert (g);

    G_node n = checked_malloc (sizeof (* n));
    n->graph = g;
    n->key = g->nodecount++;

    G_nodeList p = G_NodeList (n, NULL);

    if (g->last == NULL)
    {
        g->nodes = g->last = p;
    }
    else
    {
        g->last = g->last->tail = p;
    }

    n->succs = NULL;
    n->preds = NULL;
    n->info = info;
    return n;
}

G_nodeList G_Nodes (G_graph g)
{
    assert (g);
    return g->nodes;
}

bool G_InNodeList (G_node a, G_nodeList l)
{
    G_nodeList p;
    for (p = l; p != NULL; p = p->tail)
    {
        if (p->head == a)
        {
            return TRUE;
        }
    }
    return FALSE;
}

void G_AddEdge (G_node from, G_node to)
{
    assert (from);
    assert (to);
    assert (from->graph == to->graph);
    if (G_GoesTo (from, to))
    {
        return;
    }
    to->preds = G_NodeList (from, to->preds);
    from->succs = G_NodeList (to, from->succs);
}

static G_nodeList RemoveFromList (G_node a, G_nodeList l)
{
    if (!l) return l;
    assert (a && l);
    if (a == l->head)
    {
        return l->tail;
    }
    else
    {
        return G_NodeList (l->head, RemoveFromList (a, l->tail));
    }
}

void G_RemoveEdge (G_node from, G_node to)
{
    assert (from && to);
    to->preds = RemoveFromList (from, to->preds);
    from->succs = RemoveFromList (to, from->succs);
}

void G_Show (FILE * out, G_nodeList p, void showInfo (void *))
{
    for (; p != NULL; p = p->tail)
    {
        G_node n = p->head;
        G_nodeList q;
        assert (n);
        if (showInfo)
        {
            showInfo (n->info);
        }
        fprintf (out, " (%d): ", n->key);
        for (q = G_Succ (n); q != NULL; q = q->tail)
        {
            fprintf (out, "%d ", q->head->key);
        }
        fprintf (out, "\n");
    }
}

char * G_ToString (G_nodeList p)
{
    // TODO: Artyom Goncharov this number is totally arbitrary, needs to be revised
    char * r = checked_malloc (1024);
    int length = 0;

    for (; p != NULL; p = p->tail)
    {
        G_node n = p->head;
        G_nodeList q;
        assert (n);
        length += sprintf (&r[length], " (%d): ", n->key);
        for (q = G_Succ (n); q != NULL; q = q->tail)
        {
            length += sprintf (&r[length], "%d ", q->head->key);
        }
        length += sprintf (&r[length], "\n");
    }
    r[length] = '\0';
    return r;
}

G_nodeList G_Succ (G_node n)
{
    assert (n);
    return n->succs;
}

G_nodeList G_Pred (G_node n)
{
    assert (n);
    return n->preds;
}

bool G_GoesTo (G_node from, G_node n)
{
    return G_InNodeList (n, G_Succ (from));
}

bool G_Conn (G_node a, G_node b)
{
    return G_InNodeList (a, G_Succ (b)) || G_InNodeList (b, G_Succ (a));
}

/* return length of predecessor list for node n */
static int inDegree (G_node n)
{
    int deg = 0;
    G_nodeList p;
    for (p = G_Pred (n); p != NULL; p = p->tail)
    {
        deg++;
    }
    return deg;
}

/* return length of successor list for node n */
static int outDegree (G_node n)
{
    int deg = 0;
    G_nodeList p;
    for (p = G_Succ (n); p != NULL; p = p->tail)
    {
        deg++;
    }
    return deg;
}

int G_Degree (G_node n)
{
    return inDegree (n) + outDegree (n);
}

/* put list b at the back of list a and return the concatenated list */
static G_nodeList JoinLists (G_nodeList a, G_nodeList b)
{
    if (a == NULL)
    {
        return b;
    }
    else
    {
        return G_NodeList (a->head, JoinLists (a->tail, b));
    }
}

G_nodeList G_Adj (G_node n)
{
    return JoinLists (G_Succ (n), G_Pred (n));
}

void * G_NodeInfo (G_node n)
{
    return n->info;
}


G_table G_Empty (void)
{
    return TAB_Empty();
}

void G_Enter (G_table t, G_node node, void * value)
{
    TAB_Enter (t, node, value);
}

const void * G_Look (G_table t, G_node node)
{
    return TAB_Look (t, node);
}


