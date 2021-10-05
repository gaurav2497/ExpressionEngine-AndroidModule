#ifndef GRAPH_H
#define GRAPH_H

#include <iostream>
#include <stack>
#include <list>
#include <vector>
#include <string>
#include "errors.h"

using namespace std;

using Cycles = vector<vector<string>>;

void printCycles(Cycles &cycles)
{
    for (int i = 0; i < cycles.size(); i++)
    {
        for (int j = 0; j < cycles[i].size(); j++)
        {
            cout << cycles[i][j] << "->";
        }
        cout << cycles[i][0] << "\n";
    }
}

enum State
{
    NOT_VISITED = 1,
    IN_STACK,
    VISITED
};

class Graph
{
    std::vector<string> vertices;
    int V;
    list<int> *adj; // pointer to the vector consisting the edges.
    void dfs(int v, bool visited[], stack<int> &_stack);
    void processDFStree(State dfs_state[], stack<int> &dfs_stack, Cycles &cycles);
    vector<string> sprintCycle(stack<int> &dfs_stack, int v);

public:
    Graph();
    Graph(vector<string> &vertices);
    void addEdge(int v, int w); // from v to w
    void sort(vector<string> &sorted);
    void findCycles(Cycles &cycles);
    void print();
};

Graph::Graph()
{
    this->vertices = {};
    V = vertices.size();
    adj = new list<int>[vertices.size()]; // declaring the holding array
}

Graph::Graph(vector<string> &vertices)
{
    this->vertices = vertices;
    V = vertices.size();
    adj = new list<int>[vertices.size()]; // declaring the holding array
}

void Graph::addEdge(int v, int w)
{
    adj[v].push_back(w);
}

void Graph::dfs(int v, bool visited[], stack<int> &_stack)
{
    visited[v] = true;
    list<int>::iterator j;

    for (j = adj[v].begin(); j != adj[v].end(); ++j)
    {
        if (!visited[*j])
        {
            dfs(*j, visited, _stack);
        }
    }
    // once all adjacend nodes are visited
    _stack.push(v);
}

void Graph::sort(vector<string> &sorted)
{
    stack<int> _stack;
    bool *visited = new bool[V];
    for (int i = 0; i < V; i++)
        visited[i] = false;

    for (int i = 0; i < V; i++)
    {
        if (visited[i] == false)
        {
            dfs(i, visited, _stack);
        }
    }

    while (!_stack.empty())
    {
        sorted.push_back(vertices[_stack.top()]);
        _stack.pop();
    }
}

vector<string> Graph::sprintCycle(stack<int> &dfs_stack, int v)
{
    stack<int> temp;
    vector<string> cycle;
    temp.push(dfs_stack.top());
    dfs_stack.pop();
    while (temp.top() != v)
    {
        temp.push(dfs_stack.top());
        dfs_stack.pop();
    }
    while (!temp.empty())
    {
        cycle.push_back(vertices[temp.top()]);
        dfs_stack.push(temp.top());
        temp.pop();
    }
    return cycle;
}

void Graph::processDFStree(State dfs_state[], stack<int> &dfs_stack, Cycles &cycles)
{
    int v = dfs_stack.top();
    list<int>::iterator j;
    for (j = adj[v].begin(); j != adj[v].end(); ++j)
    {
        if (dfs_state[*j] == IN_STACK)
        {
            // this is a cycle
            cycles.push_back(sprintCycle(dfs_stack, *j));
        }
        else if (dfs_state[*j] == NOT_VISITED)
        {
            // explore this
            dfs_stack.push(*j);
            dfs_state[*j] = IN_STACK;
            processDFStree(dfs_state, dfs_stack, cycles);
        }
    }
    //we have printed all the cycles
    dfs_state[dfs_stack.top()] = VISITED;
    dfs_stack.pop();
}

void Graph::findCycles(Cycles &cycles)
{
    stack<int> dfs_stack;
    State *dfs_state = new State[V]; // the array to track the state of a vertice in DFS;
    for (int i = 0; i < V; i++)
        dfs_state[i] = NOT_VISITED;

    for (int i = 0; i < V; i++)
    {
        if (dfs_state[i] == NOT_VISITED)
        {
            dfs_stack.empty();
            dfs_stack.push(i);
            dfs_state[i] = IN_STACK;
            processDFStree(dfs_state, dfs_stack, cycles);
        }
    }
}

void Graph::print()
{
    int i = 0, j;
    for (i = 0; i < V; i++)
    {
        j = 0;
        cout << vertices[i] << "->";
        if (adj[i].size() > 0)
        {
            // print the list
            list<int>::iterator j;
            for (j = adj[i].begin(); j != adj[i].end(); ++j)
            {
                cout << vertices[*j] << "->";
            }
        }
        cout << endl;
    }
}

#endif