#ifndef EXPRESSIONS_H
#define EXPRESSIONS_H

#include <map>
#include <vector>
#include <iostream>
#include <string>
#include "parser.h"
#include "graph.h"
#include "errors.h"

template <class T>
void printMap(std::map<std::string, T> &Map)
{
    typename std::map<std::string, T>::iterator itr;
    for (itr = Map.begin(); itr != Map.end(); ++itr)
    {
        cout << '\t' << itr->first
             << '\t' << itr->second << '\n';
    }
}

class ExpEvaluator
{
    void initErrors(std::map<std::string, std::string> &errors, std::map<std::string, std::string> &expressions);
    void testExpressionIds(std::vector<std::string> ids, std::map<std::string, std::string> &expressions, std::map<std::string, double> &symbol_table);
    std::map<std::string, std::string> expressions;
    std::map<std::string, double> symbol_table;
    std::map<std::string, double> result;

    Graph graph;
    void populateIdGraph();
    void validatedIdGraph();

public:
    ExpEvaluator();
    std::map<std::string, double> evaluate();
    void insert_expression(std::string id, std::string expression);
    void insert_value(std::string id, double value);
    void print_expressions();
    std::map<std::string, int> getIdCount();
    std::vector<std::string> getUniqueIds();
    double get_result(std::string id);
};

ExpEvaluator::ExpEvaluator()
{
}

double ExpEvaluator::get_result(std::string id)
{
    return result[id];
}

void ExpEvaluator::testExpressionIds(std::vector<std::string> ids, std::map<std::string, std::string> &expressions, std::map<std::string, double> &symbol_table)
{
    std::string not_found_ids = "";
    bool hasError = false;
    for (int i = 0; i < ids.size(); i++)
    {
        if (expressions.find(ids[i]) == expressions.end() && symbol_table.find(ids[i]) == symbol_table.end())
        {
            // this id is not present in any
            hasError = true;
            not_found_ids = ids[i] + "," + not_found_ids;
        }
    }
    if (hasError)
    {
        throw Syntax_error{not_found_ids + " not found"};
    }
}

void ExpEvaluator::initErrors(std::map<std::string, std::string> &errors, std::map<std::string, std::string> &expressions)
{
    std::map<std::string, std::string>::iterator itr;
    for (itr = expressions.begin(); itr != expressions.end(); ++itr)
    {
        // do nothing.
    }
}

void ExpEvaluator::populateIdGraph()
{
    std::ostringstream errorStream;
    Parser parser;
    std::vector<std::string> id_list;
    std::map<std::string, std::string>::iterator itr;
    std::map<std::string, int> position_map;
    std::vector<std::string> vertices;
    int pos = 0;
    // initalize all data structures
    for (itr = expressions.begin(); itr != expressions.end(); ++itr)
    {
        vertices.push_back(itr->first);
        position_map.insert(pair<std::string, int>(itr->first, pos++));
    }
    graph = Graph(vertices);
    // looping through expressions
    for (itr = expressions.begin(); itr != expressions.end(); ++itr)
    {
        // for each expression , get ids from that std::string;
        id_list.clear();
        try
        {
            parser.get_ids(itr->second, id_list);
        } // then for each id test if id has a either a value given in the symbol table std::map or in the expresions std::map
        catch (const Lexical_error &e)
        {
            errorStream << "Lexical error: Not a valid token '" << e.get_message() << "'"
                        << ":expression"
                        << "[" << position_map[itr->first] << "]:" << itr->second << "\n";
        }
        for (int i = 0; i < id_list.size(); i++)
        {
            if (expressions.find(id_list[i]) == expressions.end() && symbol_table.find(id_list[i]) == symbol_table.end())
            {
                // this id is not present in any
                size_t found = itr->second.find(id_list[i]);
                if (found == std::string::npos)
                    found = 0;
                errorStream << "SyntaxError: "
                            << "'" + id_list[i] + "'"
                            << " Not found"
                            << ":expression"
                            << "[" << position_map[itr->first] << "]:" << itr->second << ":" << found << "\n";
            }
            else if (expressions.find(id_list[i]) != expressions.end())
            {
                // the id is an expression add it to the graph with its position in the vertices used to create the graph
                graph.addEdge(position_map[itr->first], position_map[id_list[i]]);
            }
        }
        //if no error till now then all expressions have valid id tokens ;
        // add ids and their dependent ids into a graph
    }
    std::string errors = errorStream.str();
    if (errors.size() > 0)
    {
        throw ParsingError{errors};
    }
}

void ExpEvaluator::validatedIdGraph()
{
    // check if the id Graph has any cycles
    Cycles cycles;
    graph.findCycles(cycles);
    std::ostringstream errorStream;
    if (cycles.size() > 0)
    {
        for (int i = 0; i < cycles.size(); i++)
        {
            for (int j = 0; j < cycles[i].size(); j++)
            {
                errorStream << cycles[i][j] << "={" + expressions[cycles[i][j]] + "}"
                            << "->";
            }
            errorStream << cycles[i][0] << "\n";
        }
        throw CyclicDependencyError{errorStream.str()};
    }
}

std::map<std::string, double> ExpEvaluator::evaluate()
{
    populateIdGraph();
    // graph.print(); // for debug purposes
    validatedIdGraph();
    // sort the graph according to topological order
    Parser parser;
    std::ostringstream errorStream;

    std::vector<std::string> sorted;
    graph.sort(sorted);
    // evaluate each expression
    // add all symbols
    std::map<std::string, double>::iterator symbol_table_itr;
    for (symbol_table_itr = symbol_table.begin(); symbol_table_itr != symbol_table.end(); ++symbol_table_itr)
    {
        parser.add_symbol(symbol_table_itr->first, symbol_table_itr->second);
    }
    double v;
    // topologically sorted functions are sorted in descending order
    for (int i = sorted.size() - 1; i >= 0; i--)
    {
        // evaluate the expression
        try
        {
            v = parser(expressions[sorted[i]]);
            parser.add_symbol(sorted[i], v);
            result.insert(pair<std::string, double>(sorted[i], v));
        }
        catch (const Lexical_error &e)
        {
            errorStream << "Lexical error: " << e.get_message()
                        << ":expression"
                        << "[" << (sorted.size() - 1 - i) << "]:" << expressions[sorted[i]] << "\n";
        }
        catch (const Syntax_error &e)
        {
            errorStream << "Syntax error: " << e.get_message() << ":expression"
                        << "[" << (sorted.size() - 1 - i) << "]:" << expressions[sorted[i]] << "\n";
            ;
        }
        catch (const Runtime_error &e)
        {
            errorStream << "Runtime error: " << e.get_message() << ":expression"
                        << "[" << (sorted.size() - 1 - i) << "]:" << expressions[sorted[i]] << "\n";
            ;
        }
    }
    std::string errors = errorStream.str();
    if (errors.size() > 0)
    {
        throw ParsingError{errors};
    }

    return result;

    // then add the id -> evaluate(expression) to symbol std::map
}

void ExpEvaluator::insert_expression(std::string id, std::string expression)
{
    expressions.insert(pair<std::string, std::string>(id, expression));
}
void ExpEvaluator::insert_value(std::string id, double value)
{
    symbol_table.insert(pair<std::string, double>(id, value));
}

std::map<std::string, int> ExpEvaluator::getIdCount()
{
    // iterate through each expression , getIds()
    // for each id add to map [id] count ++;
    // map.keys.ids
    std::map<std::string, int> id_count_map;
    std::map<std::string, std::string>::iterator itr;
    std::map<std::string, double>::iterator symbol_itr;

    std::vector<std::string> id_list;

    for (symbol_itr = symbol_table.begin(); symbol_itr != symbol_table.end(); ++symbol_itr)
    {
        id_count_map[symbol_itr->first] = 0;
    }

    for (itr = expressions.begin(); itr != expressions.end(); ++itr)
    {
        Parser parser;
        id_list.clear();
        try
        {
            parser.get_ids(itr->second, id_list);
            for (int i = 0; i < id_list.size(); i++)
            {
                if (expressions.find(id_list[i]) == expressions.end())
                    id_count_map[id_list[i]]++;
            }
        } // then for each id test if id has a either a value given in the symbol table std::map or in the expresions std::map
        catch (const Lexical_error &e)
        {
            std::cerr << "Lexical error: Not a valid token '" << e.get_message();
        }
    }
    return id_count_map;
}

std::vector<std::string> ExpEvaluator::getUniqueIds()
{
    std::vector<std::string> unique_ids;
    std::map<std::string, int> id_count_map;
    std::map<std::string, int>::iterator itr;
    id_count_map = getIdCount();
    for (itr = id_count_map.begin(); itr != id_count_map.end(); ++itr)
    {
        if (itr->second > 0)
            unique_ids.push_back(itr->first);
    }
    return unique_ids;
}

void ExpEvaluator::print_expressions()
{
    printMap(expressions);
}

#endif