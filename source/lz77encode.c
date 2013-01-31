#include "../include/lz77.h"
#include "../include/tree.h"
#include <arpa/inet.h>

/**
 * Search the number of character in common between s1 and s2
 *
 * @param tree		: tree structure
 * @param w  		: window structure
 *
 * @return		: the length of the match
 */
struct match find_match(struct Node *tree, struct window w);


int encode(struct options opt)
{

    struct window win;
    struct Node *tree = NULL;
    struct match match;
    FILE *file_input = NULL;
    struct bitfile *file_out = NULL;
    struct header *header = NULL;
    int bytes_2_encode = 0; // how many bytes I've to encode
    int i; 
    int ret = 0;
    int flag_EOF = 0; // 0 not EOF 1 I've met the EOF
    int forward_code; 
    int quanti; 
    int bits_length;
    int bits_position;
    int break_event; // used to choose which code is convenient
    int eof_code;
    int from_where;

    // Initialize part of the window structure
    bzero(&win, sizeof(struct window));
    win.look_ah_length = opt.look_ahead_len;
    win.window_length = opt.window_len;
    win.dict_position = 0;
    win.data_position = win.window_length;
    win.window = calloc(win.window_length*2, sizeof(char) );
    if(win.window == NULL)
	return -1;

    /* eof code = look_ahead_len + 1
       forward_code = look_ahead_len +2	
     */
    forward_code = win.look_ah_length +2;
    eof_code = win.look_ah_length + 1;

  
    bits_length = number_of_bits(win.look_ah_length + 2);
    bits_position = number_of_bits(win.window_length);	

    // is used to know if it's conveniente use no match case instead of a match
    break_event = (bits_length+bits_position)/(bits_length+8);

    print_options(opt);

    ret = load_dictionary(&win,opt.dict);
    if(ret == -1){
	free(win.window);
	return -1;
    }    

    tree = build_tree( win.window_length );
    if( tree == NULL){
	free(win.window);
	return -1;
    }

    if(opt.file_in == NULL){
	file_input = stdin;
    }else{
    	 file_input = fopen(opt.file_in,"r");

   	 if (file_input == NULL) {
		free(tree);
		free(win.window);
		return -1;
    	 }
    }

    file_out = bit_open(opt.file_out,BIT_WR,128);
    if(file_out == NULL){
	if(file_input != stdin )
        	fclose(file_input);
       	free(win.window);
        free(tree);
        return -1;
    }

    // write the header
    header = build_header(&opt);

    ret = write_header(file_out, header);
    free(header);

    // insert the dictionary in the tree
    for(i = 0; i < win.window_length ; i++)
	add_node(tree,win.dict_position+i,&win); 	

    // I've just window_length dictionary data, fill the others with data that will be encoded
    quanti = win.window_length*2 - win.window_length;	
    from_where = win.data_position;
    bytes_2_encode = 0;


    // encode cycle
    for(;;){

        // read necessary bytes to fill the buffer from file_input or less (EOF)
        bytes_2_encode += fread(win.window + from_where , 1, quanti , file_input);
	
        if ( feof(file_input) ){
	    //printf("EOF\n");
            flag_EOF = 1;
	}
     
        if (ferror(file_input) ){
            // remove the file_output isn't complete
            remove(opt.file_out);
            break;
        }

        while(bytes_2_encode > 0){

            // find a match
            match = find_match(tree, win);

	    // is it convenient ?
	    if((break_event == match.len) && !match.type )
		match.len = 0;	    

	    // write in the file output
	    if((match.len == 0) ){ // no match
		// write 0	
		ret = bit_write(file_out,(char*)(&match.len),bits_length ,0);
		if(ret == -1)
			break;

		// write the char
		ret = bit_write(file_out,(char*)(&win.window[win.data_position]), 8, 0);
		if(ret == -1)
			break;

		match.len = 1;

	    }else{ // match
		// we must consider the offset
		int position = match.position - win.dict_position;
	
		if(match.type){
			ret = bit_write(file_out,(char*)(&forward_code),bits_length,0);		
			if(ret == -1)
				break;			
		}
		
		//ret = bit_write(file_out,(char*)(&match.len),bits_length,0);
		ret = bit_write(file_out,(char*)(&match.len),bits_length,0);
		if(ret == -1)
			break;	

		ret = bit_write(file_out,(char*)(&position), bits_position, 0);
		if(ret == -1)
			break;			

	    }
	
	    // update the tree
	    for(i = 0; i < match.len; i++ ){
		delete_node(tree, win.dict_position,win.window_length);
		win.dict_position++;
		add_node(tree,win.data_position+i,&win);
	    }

	    win.data_position += match.len;
	    bytes_2_encode-=match.len;	

     
	    if( bytes_2_encode < win.look_ah_length){
		if( flag_EOF == 1 ) // adjust the look_ahead length for last bytes (EOF reached)
			win.look_ah_length = bytes_2_encode;
		else // data less than look_ah_lenght so go to refill the win.window with data
			break;    
		
	    }

        }//end while(bytes_2_encode)

	if(ret == -1) // some error in encode loop
		break;
	

        if(flag_EOF == 1){
            // write the special code to eof
	    ret = bit_write(file_out,(char*)(&eof_code),bits_length,0);
            break;
        }

        // I must reload the window so I've to copy the dictionary (and data if there are some) at the beggining
	// memmove is necessary because the memory areas can be overlap 
	memmove( win.window, win.window + win.dict_position , (win.window_length + bytes_2_encode) );

        win.dict_position = 0;
	win.data_position = win.window_length;
        from_where = win.data_position + bytes_2_encode;
        quanti = win.window_length*K - (win.window_length + bytes_2_encode);


	// clear all informations about nodes
	empty_tree(tree, win.window_length);


	// add the dictionary in the tree
    	for(i = 0; i < win.window_length ; i++)
		add_node(tree,win.dict_position+i,&win);

    }// end for(;;)

    // free memory
    free(tree);
    free(win.window);

    // close files
    bit_close(file_out);
    if(opt.file_in != NULL)
	    fclose(file_input);

    if(ret == -1){
	remove(opt.file_out);
	return ret;
    }


    return 0;
}



/**
 * Search the number of character in common between s1 and s2
 *
 * @param tree		: tree structure
 * @param w  		: window structure
 *
 * @return		: the length of the match
 */
struct match find_match(struct Node *tree, struct window w)
{
   	struct match match;
	int i;
	int node;
	int string_head;
	int diff;
	int count;
	int type;

	node = tree[ROOT].greater;
	string_head = tree[node].position;
	match.len = 0;
	match.position = 0;
	match.type = 0;

	// the search is stopped if the match is the longest as possible or we're in a leaf
	for(;;){

		diff = w.window[ string_head ] - w.window[ w.data_position ];

		count = 0;
		i = 0;
		type = 0;

       		// stay in this node and check for a longest match
		while(  (diff == 0) && (count != w.look_ah_length) && (w.data_position + i < w.window_length*K) ){
			count++;
			i++;
			diff = w.window[ wrap(string_head + i, w.window_length + w.dict_position , w.dict_position)] - w.window[ w.data_position + i ];
		}

		// to search a match looking in the look ahead we've 2 condictions .
		// 1 - the string_head must be in the "Forward Zone"
		// 2 - the match found in wrap mode must be equal or greater to the distance between string head and data_position
		if( (string_head > (w.data_position - w.look_ah_length) ) && (count >= ( w.data_position - string_head) ) ){
			int count_forward;

			// I know that the first border - string_head chars are good
			count_forward = w.data_position - string_head;
			diff = w.window[string_head + count_forward] - w.window[ w.data_position + count_forward];

			while( (diff == 0) && (count_forward != w.look_ah_length) && (w.data_position + count_forward < w.window_length*K) ){
				count_forward++;
				diff = w.window[string_head + count_forward ] - w.window[ w.data_position + count_forward ];
			}

			// equal isn't good because we send 2*L+W against L+W
			if(count_forward > count){
				count = count_forward;
				type = 1;
			}

		}

        	// if this match (count) is longest than the previous then save it
		if( match.len < count ){
			match.len = count;
			match.position = string_head;
			match.type = type;
		}

		// maybe we can have a longest match if we check on right son
		if(diff < 0)
			node = tree[node].greater;
		else
			node = tree[node].smaller;
		
	        // if we've see all possible strings or found the longest match possible then exit from while
	        if (( node == UNUSED ) || ( match.len == w.look_ah_length ) )
			break;

        	string_head = tree[node].position;	

	}

	return match;
}

