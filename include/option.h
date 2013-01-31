/**
 * @file option.h
 *
 * This file contain the functions to handle options for LZ77
 * [options]
 * -c			-> compression
 * -d			-> decompression
 * -v			-> verbose
 * -w number		-> window dimension in bytes
 * -l number		-> lookahead dimension in bytes
 * -i file_in		-> input file , if c mode is the original file , compress file otherwise.
 * -o file_out		-> output file,  if c mode is the compress file , original file otherwise.
 *
 * @author Pischedda Alessandro
 */

#ifndef _OPTION_H_
#define _OPTION_H_

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>

#define COMPRESSION 1
#define DECOMPRESSION 0
#define NONE 2


/**
 *
 */
struct options{
	char *file_in;
	char *file_out;
	int mode;	// must be COMPRESSION (1) or DECOMPRESSION (0)
	int verbose;
	int window_len;
	int look_ahead_len;
	char *dict;
};


/**
 * Handle the option passed through command line
 *
 * @param opt	structure options
 * @param argc	number of arguments passed in command line
 * @param argv	array with arguments
 * @return    	0 if OK
 *		-1 if something goes wrong
 */
int handle_options(struct options *opt, int argc, char* argv[]);

/**
 * Initialize the struct options :
 *
 *	- file_in	NULL
 *	- file_out	NULL
 *	- mode		NONE
 *	- verbose	OFF
 *	- window_len	1024
 *	- look len	64
 *	- dict		it
 *
 * @param opt : is a pointer to option structure
 *
 * @retun 	0 in case of success
 *		-1 in case of error, in that case errno is setted.
 *
 * ERRORS
 *	EINVAL		if some function's arguments isn't correct.
 *	Others		are all the possible error returned by calloc() function.
 */
int init_opt(struct options *opt);


/**
 * Print the fields contained in the option structure
 * 
 * @param opt : option structure that contain the informations to be show
 */
void print_options(struct options opt);

/**
 * Print the help
 */
void usage();

/**
 * Check if a certain file ( or directory ) has a certain features (existence, read/write permission)
 *
 * @param filename	name of the file
 * @param check_type    F_OK  is used to check his existence
 *			R_OK and W_OK are used to check read/write permission
 *
 * @return 		1 if the file has the features researched -1 otherwise
 */
int file_check(char* filename, int check_type);

#endif
