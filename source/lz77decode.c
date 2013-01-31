#include "../include/lz77.h"
#include <arpa/inet.h>

int convert_data(int data, uint8_t current_order, uint8_t file_order){
	
	if( current_order != file_order ){
		if(current_order) // from little to big
			return htonl(data);
		else		  // from big to little
			return ntohl(data);
	}
	return data;
	

}


int decode(struct options opt)
{
    // Variables
    struct window win; 
    FILE *file_output = NULL;
    struct bitfile *b_file = NULL;
    struct header header;
    int i,ret;
    int bits_length;
    int bits_position;
    unsigned char letter;
    int length;
    int position;
    int forward_code;
    int eof_code; 
    int current_order; 
    int pippo = 1;
	

    /* Test for a little-endian machine */
    #if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
	current_order = LITTLE_EN;
    #else 
	current_order = BIG_EN;
    #endif


    if((pippo >> 1) == 0)
	printf("LITTLE\n"); 
    else 
	printf("BIG\n");

    // read the header
    b_file = bit_open(opt.file_in,BIT_RD,128);
    ret = read_header(b_file,&header);
    if(ret == -1)
	return -1;
    
    // Initialize window structure
    bzero(&win, sizeof(struct window));
    win.look_ah_length = header.look_ah_len;
    win.window_length = header.window_len;
    win.dict_position = 0;
    win.data_position = win.window_length;
    win.window = calloc( win.window_length*K, sizeof(char) );
    
    // special code
    eof_code = win.look_ah_length + 1;
    forward_code = win.look_ah_length + 2;

    // plus 2 for eof_code and forward_code
    bits_length = number_of_bits(win.look_ah_length + 2);
    bits_position = number_of_bits(win.window_length);

    strncpy(opt.dict, header.dict,2);
    opt.look_ahead_len = header.look_ah_len;
    opt.window_len = header.window_len;

    print_options(opt);

    if(current_order)
	    printf("current version : BIG ENDIAN \n");
    else
	    printf("current version : LITTLE ENDIAN \n");

    load_dictionary(&win, opt.dict);

    if( opt.file_out == NULL){
	file_output = stdout;
    }else{
	file_output = fopen(opt.file_out,"w");

	if (file_output == NULL) {
		return -1;
	}
    }	

    // decode cycle
    for(;;){
	length = 0;     

        ret = bit_read(b_file, (char*)(&length), bits_length , 0);

	if(ret == -1){
		free(win.window);	
		break;
	}

	if( length == eof_code ) // end file
		break;
	
        if( (length > 0) && (length != forward_code) ){ // wrap
		
	        position = 0;
		ret = bit_read(b_file, (char*)(&position), bits_position , 0);
		if(ret == -1)
			break;
		
		position = convert_data(position, current_order, header.byte_order);

		// must add the offset in the position
		position += win.dict_position;	

		// write and update
		for(i=0;i<length; i++){
			ret = fwrite( &win.window[wrap(position+i,win.dict_position+win.window_length,win.dict_position)], sizeof(char), 1, file_output );
			if(ret == -1)
				break;
			win.window[win.data_position + i] = win.window[wrap(position+i, win.dict_position+win.window_length, win.dict_position) ];
		}

	} else if (length == forward_code ){ // forward
		// read the length
		length = 0;
        	ret = bit_read(b_file, (char*)(&length), bits_length , 0);
		if(ret == -1)
			break;

	        position = 0;
		ret = bit_read(b_file, (char*)(&position), bits_position , 0);
		if(ret == -1)
			break;

		position = convert_data(position, current_order, header.byte_order);

		// must add the offset in the position
		position += win.dict_position;	

		// write and update the dictionary
		for(i=0; i<length; i++){
			ret = fwrite( &win.window[ position + i ], sizeof(char), 1, file_output );
			if(ret == -1)
				break;
			win.window[win.data_position + i] = win.window[ position+i ];
		}
		
        } else	if( length == 0 ){ //no match

		ret = bit_read(b_file, (char*)&letter, 8 , 0);
		if(ret == -1)
			break;

		win.window[win.data_position] = letter;	
		ret = fwrite(&letter, sizeof(char), 1, file_output );
		if(ret == -1)
			break;

		length = 1;
        }	

	win.data_position += length;
	win.dict_position += length;	    	


        if( win.data_position > win.window_length*K - win.look_ah_length ){
	
            // I must reload the window so I've to copy the dictionary at the beggining
	    memmove(win.window, win.window + win.dict_position , win.window_length);

            // update the following data
            win.dict_position = 0;
            win.data_position = win.window_length*K - win.window_length;

        }

    }// end for(;;)

    // free memory
    free(win.window);

    // close both file
    if( opt.file_out != NULL){
	    fclose(file_output);
	    // if there is some error remove the output file
	    if(ret == -1)
		remove(opt.file_out);
    }

    bit_close(b_file);

    if(ret == -1)
	return ret;	
	
	
    return 0;
}

