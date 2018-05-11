#ifndef TOOL_HPP_
#define TOOL_HPP_

#include <stdio.h>
#include <stdlib.h>


#define BUFFER_SIZE (128)


#define error(text)								\
	while(true) {								\
		printf("Error : %s\n", text);			\
		printf("File : %s\n", __FILE__);		\
		printf("Line : %d\n", __LINE__);		\
		printf("Function : %s\n", __func__ );	\
		exit(EXIT_FAILURE);						\
	}

#define flag(text)								\
	do {										\
		printf("Error : %s\n", text);			\
		printf("File : %s\n", __FILE__);		\
		printf("Line : %d\n", __LINE__);		\
		printf("Function : %s\n", __func__ );	\
		fflush(stdout);							\
	} while(0)


#define assert(test)							\
	while (!(test)) {							\
		error("assertion failure");				\
	}


#define malloc_error(p)							\
	while (!(p)) {								\
		error("memory failure")					\
	}


bool is_letter (char c);
bool is_blanc (char c);
bool is_digits (char c);
char *get_line (FILE *stream);


#endif


