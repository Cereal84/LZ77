/**
 * @file window.h
 * 
 * This file contains functions and structures data used in econde() and decode() functions in
 * order to perform the compression/decompression.
 * Structure data :
 *	- window 
 *  	- match  
 *	- header	
 */


#ifndef _window_H_
#define _window_H_

#include "option.h"
#include "bitio.h"

// the following instructions are needed to use macro __BYTE_ORDER__
#if __FreeBSD__
	#include <sys/endian.h>
#endif
#if __LINUX__
	#include <endian.h>
#endif

#define K 2	// window_length * K
#define BIG_EN 1
#define LITTLE_EN 0

/**
 * This structure is the main data structure, it has  all
 * the informations needed to perform the LZ77 algorithm. 
 *  - window_length	: dictionary length
 *  - look_ah_length	: max data we'll try to encode
 *  - data_position	: indicate the position of look ahead buffer in the window 
 *  - dict-position	: indicate the begining of dictionaty in window array
 *  - window		: text array that will contains text data and dictionary data
 */
struct window{
        // dictionary length 
	uint16_t window_length;
	// max data we'll try to encode
	uint8_t look_ah_length;
	// indicate the position of look ahead buffer in the window
	int data_position;
    	// indicate the begining of dictionaty in window array
	int dict_position;
	unsigned char* window;	
};

/** 
 * The function copy_dictionary() copy a dictionary from a file to 
 * window structure.
 *
 * @param w		is the window structure where copy the dictionary
 * @param filename	is the filename of the file that has the dictionary
 *
 * @return 		0 success and -1 if something goes wrong
 */
int load_dictionary(const struct window* w, char* filename);

/**
 * Check if it'll be a wrap-around and in case return the correct position of the
 * character in the window (dictionary space).
 */
int wrap(int pos, int limit, int offset);

/**
 * Return the number of bits necessary to represent the number X.
 * 
 * @param x	number
 *
 * @return 	-1 if we've an error
 *		the number of bits necessary to represent the number X  
 */
int number_of_bits(int x);

/* match PART 
 * The structure data and function find_match() are used only in encode() function (file lz77encode.c)
 */

/**  
 * This structure store the informations about a match.
 * It's used only by encode() function.
 * 
 *  - len 	: is the length of the match
 *  - is 	: the position where start the match
 *  - type 	: 0 if the match is given by normal way or 
 *		  1 if the match look into look_ahead buffer
 */
struct match{
    uint8_t len;
    uint16_t position;
    uint8_t type;
};


/* header FILE PART */

/**
 * Contain the information about the header for the compress file.
 * header is used to contain information in order to have a correct decompressor's 
 * work and a description of the file.
 * Simplest description of the fields :
 *  - magic number  : define the file format
 *  - header_len    : it's the header length
 *  - ver           : version of the software used to do the file
 *  - byte_order    : if the file was writen with a big/little endian processor
 *  - look_ah_len   : look ahead buffer length
 *  - window_len    : sliding window length
 *  - dict          : dictionary preloaded. It can be "it", "en" or others
 *
 *  31             23               15               7              0
 *  ______________ ________________ ________________ ________________
 * |                                                                 |
 * |                      MAGIC    NUMBER                            |
 * |_________________________________________________________________|
 * |              |                |                |                |
 * |  LOOK AH LEN |    ENDIAN      |    VERSION     |  header LEN    |
 * |______________|________________|________________|________________|
 * |                               |                                 |
 * |           DICTIONARY          |            window LEN	     |
 * |_______________________________|_________________________________|
 *
 */
struct header{
	// magic number
	uint8_t magic[4]; 
	// header length
	uint8_t header_len;
	// version of compressor/decompressor
	uint8_t ver; 
	// store the byte order used
	uint8_t byte_order;
	// lookahead length
	uint8_t look_ah_len; 
	// window length
	uint16_t window_len; 
	// dictionary name
	char dict[2];
};

/**
 * This function build and initialize the header structure with the informations
 * contained in the options structure.
 *
 * @param options	structure options (see option.h)
 * @return 		header structure pointer
 *			NULL if there is some error and set errno
 *
 * ERRORS
 *	EINVAL		if some function's arguments isn't correct.
 *	Others		are all the possible error returned by calloc() function.
 *
 */
struct header * build_header(const struct options *opt);


/**
 * This function write the header in a file specified by variable b_file.
 * Because we're using bitio library there isn't an "append mode" so the 
 * bitfile it must be pre-open before using this function.
 * 
 * @param b_file	it's the "file" where we want to store the header
 * @param header	structure which contain the header's data
 *
 * @return		-1 if there is some error, and set errno.
 *			0 otherside.
 *
 * ERRORS
 *	EINVAL		if some function's arguments isn't correct.
 *	Others		are all the possible error returned by write() function.
 *
 */
int write_header(struct bitfile *b_file, const struct header *header);

/**
 * This function read the header informations from a file specified by
 * b_file (bitfile structure).
 * Because we're using bitio library there isn't an "append mode" so the 
 * bitfile it must be pre-open before using this function.
 *
 * @param b_file	 
 * @param header	where I store the informations read from the file
 *
 * @return 		-1 if there is some error, and set errno.
 *			0 otherside.
 * ERRORS
 *	EINVAL		if some function's arguments isn't correct.
 *	Others		are all the possible error returned by read() function.
 */
int read_header(struct bitfile *b_file, struct header *header);


#endif
