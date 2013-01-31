/**
 * @file lz77.h
 *
 * This file contains the functions encode() and decode() that are
 * an implementation of encode and decode LZ77 algorithm.
 */

#ifndef _LZ77_H_
#define _LZ77_H_

#include <string.h>
#include <stdint.h>
#include "window.h"


/**
 * LZ77 Encode Algorithm Implementation.
 * The encode() function use a binary tree to "store" the strings contained 
 * in the dictionary. This tree is realized as an array where each entry 
 * is a node, for a detailed explanation see tree.h file.
 *
 * @param opt	is a Options structure, for more informations see file options.h
 *
 * @return	0 if sucefully work and -1 if something goes wrong.
 */
int encode(struct options opt);

/**
 * LZ77 Decode Algorithm Implementation
 *
 * @param opt	is a Options structure, for more informations see file options.h
 *
 * @return	0 if sucefully work and -1 if something goes wrong.
 */
int decode(struct options opt);


#endif

