#include "parser.hpp"
#include "tool.hpp"


Parser::Parser () {
	buffer = NULL;
	memory = 0;
	current = 0;
}


Parser::~Parser () {}


void Parser::set_buffer(char * line) {
	buffer = line;
	memory = 0;
	current = 0;
	while (is_blanc(buffer[current])) (current)++;
}


char Parser::get_current () {
	return buffer[current];
}


int Parser::read_number () {
	int value = 0;
	memory = current;
	if (!is_digits(buffer[current])) parse_error((char *)"not a number", false);
	while (is_digits(buffer[current])) {
		value = 10 * value + buffer[current] - '0';
		current++;
	}
	while (is_blanc(buffer[current])) current++;
	return value;
}


void Parser::read_string (char *string) {
	int i = 0;
	memory = current;
	if (!is_letter(buffer[current])) parse_error((char *)"not a letter", false);
	while (is_letter(buffer[current])) {
		string[i] = buffer[current];
		i++;
		current++;
	}
	string[i] = '\0';
	while (is_blanc(buffer[current])) current++;
}


void Parser::read_char (char c) {
	if (buffer[current] != c) parse_error((char *) "not corresponding symbol", false);
	current++;
	while (is_blanc(buffer[current])) current++;
}


bool Parser::is_eof () {
	return buffer[current] == '\0';
}


void Parser::next () {
	do {current++;} while (is_blanc(buffer[current]));
}


void Parser::parse_error (char * message, bool on_memory) {
	int stop = on_memory ? memory : current;

	printf("%s\n", buffer);
	for(int i = 0; i < stop; i++) {
		if (buffer[i] == '\t')
			printf("\t");
		else
			printf(" ");
	}
	printf("^");
	error(message);
}


