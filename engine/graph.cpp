#include <string.h>
#include "graph.hpp"
#include "tool.hpp"


Graph::Node::Node () {;
	id = 0;
	capacity = 1;
	type = Big;
	name[0] = '?';
	name[1] = '\0';
}
Graph::Node::~Node () {}


void Graph::Node::print () {
	char big[BUFFER_SIZE] = "big";
	char small[BUFFER_SIZE] = "small";
	char *type = (this->type == Big) ? big : small;
	printf("%s %s station with %d platform.s",name, type, capacity);
}


void Graph::Node::parse (int node, int nb_nodes, Graph *graph, Parser *reader) {
	int to, lenght;
	while (!reader->is_eof()) {
		if (is_digits(reader->get_current())) {
			to = reader->read_number();
			if (to <= 0, to > nb_nodes) reader->parse_error((char *)"id too high or null", true);
			reader->read_char(':');
			lenght = reader->read_number();
			graph->add(node, to-1, lenght);
		}
		else if (reader->get_current()=='B') {
			graph->nodes[node].type=Big;
			reader->next();
		}
		else if (reader->get_current()=='S') {
			graph->nodes[node].type=Small;
			reader->next();
		}
		else if (reader->get_current()=='#') {
			reader->next();
			graph->nodes[node].capacity = reader->read_number();
		}
		else if (reader->get_current()=='@') {
			reader->next();
			reader->read_string(graph->nodes[node].name);
			int l = strlen(graph->nodes[node].name);
			graph->names_size = (graph->names_size > l) ? graph->names_size : l;
		}
		else {
			reader->parse_error((char *)"syntax error", false);
		}
	}
	graph->add(node, node, 0);
}



Graph::Graph (int size) {
	this->size = size;
	this->names_size = 0;
	this->nodes= new Graph::Node[size];
	this->durations = new int*[size];

	for (int i = 0; i < size; i++) {
		this->nodes[i].id = i;
		this->durations[i] = new int[size];
		for (int j = 0; j < size; j++)
			this->durations[i][j] = -1;
	}
}


Graph::~Graph () {
	for (int i = 0; i < size; i++) {
		delete[] durations[i];
	}
	delete[] nodes;
	delete[] durations;
}


void Graph::add (int from, int to, int duration) {
	assert(duration >= 0);
	durations[from][to] = duration;
}


int Graph::get_size () {
	return size;
}

int Graph::get_names_size () {
	return names_size;
}


char *Graph::get_name (int node) {
	return nodes[node].name;
}


int Graph::get_duration (int from, int to) {
	return durations[from][to];
}


Type Graph::get_type(int node) {
	return nodes[node].type;
}


int Graph::get_capacity (int node) {
	return nodes[node].capacity;
}


void Graph::print () {
	printf("Map size : %d\n", size);
	for (int i = 0; i < size; i++) {
		printf("+ ");
		nodes[i].print();
		printf("\n|    ");
		for (int j = 0; j < size; j++) {
			if (durations[i][j] > 0) {
				printf("%s(%d) ", nodes[j].name, durations[i][j]);
			}
		}
		printf("\n");
	}
}


Graph* Graph::parse(char *filename) {
	FILE *file;

	file = fopen(filename, "r");
	if(file == NULL) error("unable to read map");

	int size = 0;
	char *buffer = get_line(file);
	while (buffer != NULL) {
		size++;
		buffer = get_line(file);
	}
	rewind(file);

	Parser reader;
	Graph *graph = new Graph(size);

	for (int i = 0; i < size; i++) {
		buffer = get_line(file);
		assert(buffer != NULL);
		reader.set_buffer(buffer);
		Node::parse(i, size, graph, &reader);
	}

	fclose(file);
	return graph;
}


