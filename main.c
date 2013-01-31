/**
 * This software is an implementation of the LZ77 compression algortihm created by Jacob Ziv and Abraham Lempel in 1977.
 *
 * Simple description about encoding
 *
 * LZ77 is a dictionary encoding that attempt to compress series of strings by converting the strings into a dictionary 
 * offset and string length. So if I want to compress the string "abcd" and it's appear in the dictionary at position 1234, 
 * it may be encoded as <offset = 1234, length = 4>.
 * LZ77 dictionary is not an external dictionary that lists all known symbol strings. Instead, the dictionary is a sliding 
 * window containing the last N symbols encoded/decoded. When a character is processed and added to the dictionary, the first
 * character is removed. Additional new characters cause an equal number of the oldest characters to slide out.
 * In the example above, after encoding "abcd" as <offset = 1234, length = 4>, the sliding window would shift over 4 characters
 * removing from the dictionary the first 4. 
 * 
 * We've two cases :
 *
 * 		1 - No match  < L, char >
 *				- char is the character so 8 bit.
 *				- L is equal to 0 and is used to specify no match
 *		2 - Match     < L, P > 
 *				- L length
 *				- P position
 * 
 * The number of bits used for L and P depends on the dictionary and look ahead buffe size. When we've a match we check if is 
 * convenient use the couple <L,P> or is better in terms of bits use <L,char>.
 *
 * Example
 *
 * 	Window length = 4096 bytes => 11 bits
 *	Look ahead length = 16 => 4 bits
 *
 * so the couple for a match is 15 bits instead for no match case we've 12. If the match length is 1 is obviously that is better 
 * use the case of no match.   
 *
 * Decode description
 *
 * In order to have the normal file from a compress one the decoder hasn't much work to do, he must read the couples from compress file and
 * fill/update the window.
 *
 * About this software
 * 
 *	Version	1
 *	Author	Pischedda Alessandro
 *	Features 
 *		- window/look ahead length choosen by the user 
 *		- dictionary pre-loaded
 *		- possibility to choose a file that contain a dictionary
 *		- can use STDIN in compression mode and STDOUT in decompression mode
 *		- check if a no match case is better than a match one
 *
 *
 */


#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include "include/option.h"
#include "include/lz77.h"



int main (int argc, char* argv[])
{
	
	struct options opt;
	int ret=0;

	if(argc == 1){
		printf("Too few arguments, use -h to see how to use this program.\n");
		return -1;
	}

	ret = init_opt(&opt);
	if( ret != -1){
		ret = handle_options(&opt,argc,argv);
		if( ret != -1){
	
			if (opt.mode){
				ret = encode(opt);
			}
			else{
				ret = decode(opt);
			}
		}
	}

	if(ret == -1)
		printf("Some error occours, the software'll be terminated.\n");
	
	return ret;	

}
	
