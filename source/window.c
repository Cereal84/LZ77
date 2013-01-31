#include <stdint.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <arpa/inet.h>
#include "../include/window.h"
     


int wrap(int pos, int limit, int offset)
{
	 if ( pos < limit )
		 return pos;
	 return ( pos % limit ) + offset;
}

int number_of_bits(int x){

	if(x<0){
		errno = EINVAL;
		return -1;
	}
		
	// log(8)/ log(2) = 3 but with 3 bit I haven't the number 8
	// so I must add 1 bit 	
	return (int)(log(x)/log(2) +1);
}

int load_dictionary(const struct window* w, char* filename){

    FILE *dict = NULL;
    int quanti,resto;
    int ret,i;

    // open the dictionary file
    dict = fopen(filename,"r");

    if (dict == NULL) {
		errno = EINVAL;
		return -1;
    }

    quanti = w->window_length;
 
    i = 0;
    // preload dictionary in window
    ret = fread(w->window, 1, w->window_length, dict);
    if(ferror(dict)){
        printf("Some error occured with dictionary\n");
	fclose(dict);
	free(w->window);
        return -1;
    } 
    fclose(dict);

    // (window_length - ret)/ret = window/ret -1
    quanti = (w->window_length)/ret -1;
    resto =  (w->window_length - ret)%ret;   
   
    for(i=1; i<=quanti; i++)
	strncpy((char*)(w->window + (ret*i)),(char*)(w->window),ret);	
     
    if(resto)
	strncpy((char*)(w->window + (ret*quanti)),(char*)(w->window),resto);	

	
    return 0;
}

struct header * build_header(const struct options *opt){

	struct header *header = NULL;

	if(opt == NULL ){
		errno = EINVAL;
		return NULL;
	}
		
	
	// first of all allocate memory for header
	header = calloc(1,sizeof(struct header));
	if(header == NULL)
		return NULL;

	// init
	header->magic[0] = 1;
	header->magic[1] = 9;
	header->magic[2] = 8;
	header->magic[3] = 4;
	header->header_len = 12;
	header->ver = 1;
	/* Test for a little-endian machine */
	#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
		header->byte_order = LITTLE_EN;
	#else 
		header->byte_order = BIG_EN;
	#endif
	
	header->look_ah_len = opt->look_ahead_len;
	header->window_len = htons(opt->window_len);
	strncpy(header->dict, opt->dict,2);

	return header;
}


int write_header(struct bitfile *b_file, const struct header *h){
	

	int ret;

	if( b_file==NULL || h == NULL){
		errno = EINVAL;
		return -1;
	}

	// write magic number [32 bits]
	ret = bit_write(b_file,(char*)(h->magic),32,0);
	if(ret == -1)
		return -1;

	// header len [8 bits]
	ret = bit_write(b_file,(char*)(&h->header_len),8,0);
	if(ret == -1)
		return -1;

	// write version [8 bits]	
	ret = bit_write(b_file,(char*)(&h->ver),8,0);
	if(ret == -1)
		return -1;

	// write byte order (BIG/LITTLE ENDIAN) [8 bits]	
	ret = bit_write(b_file,(char*)(&h->byte_order),8,0);
	if(ret == -1)
		return -1;
	
	// write look ahead len [8 bits]	
	ret = bit_write(b_file,(char*)(&h->look_ah_len),8,0);
	if(ret == -1)
		return -1;

	// write window len [16 bits]	
	ret = bit_write(b_file,(char*)(&h->window_len),16,0);
	if(ret == -1)
		return -1;
	
	// write dictionary name [16 bits]	
	ret = bit_write(b_file,h->dict,16,0);
	if(ret == -1)
		return -1;

	return 0;

}

int read_header(struct bitfile *b_file, struct header *h){

	int ret;

	if( b_file == NULL || h == NULL ){
		errno = EINVAL;
		return -1;
	}	

	// magic number [32 bit]
	ret = bit_read(b_file,(char*)(h->magic),32,0);
	if (ret == -1)
		return -1;


	if( (h->magic[0] != 1) && (h->magic[1] != 9) && (h->magic[2] != 8) && (h->magic[3] != 4) ){
		printf("This file isn't compatible with this software\n");
		return -1;
	}

	// in case of error the errno is set 

	// header length field
	ret = bit_read(b_file,(char*)(&h->header_len),8,0);
	if (ret == -1)
		return -1;

	// version number
	ret = bit_read(b_file, (char*)(&h->ver), 8, 0);
	if (ret == -1)
		return -1;
	
	ret = bit_read(b_file, (char*)(&h->byte_order),8 , 0);
	if (ret == -1)
		return -1;


	ret = bit_read(b_file, (char*)(&h->look_ah_len), 8, 0);
	if (ret == -1)
		return -1;

	ret = bit_read(b_file, (char*)(&h->window_len), 16, 0);
	if (ret == -1)
		return -1;

	ret = bit_read(b_file, h->dict, 16, 0);
	if (ret == -1)
		return -1;

	h->window_len = ntohs(h->window_len);

	return 0;
}

