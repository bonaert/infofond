#ifndef PARSER_HPP_
#define PARSER_HPP_


#include "tool.hpp"

class Parser {
private: //------------------------------------------------------------
	char * buffer;
	int current, memory;

public:  //------------------------------------------------------------
	Parser ();
	~Parser();
	void next ();
	char get_current ();
	bool is_eof ();
	void parse_error (char *message, bool on_memory);
	void set_buffer (char *buffer);
	int read_number ();
	void read_string (char *string);
	void read_char (char c);
};


#endif


