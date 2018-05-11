#ifndef GRAPH_HPP_
#define GRAPH_HPP_


#include "parser.hpp"
typedef enum {Big, Small} Type;


class Graph {
private: //------------------------------------------------------------
	class Node {
	public:
		Node ();
		~Node ();
		int id, capacity;
		char name[BUFFER_SIZE];
		Type type;
		static void parse (int node, int nb_nodes, Graph *graph, Parser *parser);
		void print();
	};

	int size;
	int names_size;
	Node * nodes;
	int ** durations;
	void add (int from, int to, int duration);
public:  //------------------------------------------------------------
	Graph (int size);
	~Graph();
	int get_duration (int from, int to);
	int get_capacity (int node);
	Type get_type (int node);
	char *get_name (int node);
	int get_names_size ();
	int get_size ();
	void print ();
	static Graph* parse(char *filename);
};


#endif


