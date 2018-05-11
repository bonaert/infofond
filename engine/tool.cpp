#include <string.h>
#include "tool.hpp"


bool is_letter (char c) {
	return (c >= 'a' && c <= 'z') or (c >= 'A' && c <= 'Z');
}


bool is_blanc (char c) {
	switch (c) {
	case '\n': case '\t': case '\v': case '\r': case ' ':
		return true;
	default:
		return false;
	}
}


bool is_digits (char c) {
	return c >= '0' && c <= '9';
}


char *get_line (FILE *stream) {
	static char buffer[BUFFER_SIZE];
	int length;

	if (!fgets(buffer, BUFFER_SIZE, stream)) return NULL;
	length = strlen(buffer);

	if (length == BUFFER_SIZE-1) error("file line too long");
	if (buffer[length-1] == '\n') buffer[length-1] = '\0';

	return buffer;
}


