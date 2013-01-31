#include "../include/option.h"




// This function describe how to use this program
void usage()
{
	printf("LZ77 Help\n");
	printf("Usage: ./lz77 [options]\n\n");
	printf("options \n");
	printf("  -c\tSet compression mode\n");
	printf("  -d\tSet decompression mode\n");
	printf("  -i FILE\n\tFilename input (source).\n\tIn compression mode if is omitted the software use the standard input,\n\tinstead in decompression mode you must specify it.\n");
	printf("  -o FILE\n\tFile name output (destination).\n\tIn decompression mode if is omitted the software use the standard output,\n\tinstead in compression mode you must specify it.\n");
	printf("  -t DICTIONARY\n\tSpecify which dictionary you want to use\n\tThe default one is 'it'. The name must be of 2 characters.\n");
	printf("  -l VALUE\n\tSet look-ahead length, must specify a positive value.\n\tMin value is 8 and the max value is 255\n");
	printf("  -w VALUE\n\tSet window length, must specify a positive value.\n\tMin value must be equal to look ahead length the max value is 32767.\n");
	printf("  -v\tSet verbose mode\n");
	printf("\nEXAMPLES\n");
	printf("  Using files  ./lz77 -c -o compress_file  -i original_file -w 1024 -l 16 -t it\n");
	printf("  Using STDIN  ./lz77 -c -o compress_file -w 1024 -l 16 -t it\n");
	printf("  Using STDOUT ./lz77 -i compress_file -w 1024 -l 16 -t it\n");
	printf("\nNOTE\n");
	printf("  If you're using the STDIN typing the text manually use Ctrl+D to do EOF.\n");
	printf("\nDEFAULT VALUES\n");
	printf("  Window length : 2048 bytes\n");
	printf("  Look ahead length : 64 bytes\n");
	printf("  Dictionary : it\n\n");
	exit(0);

}


int init_opt(struct options *opt)
{
	bzero(opt, sizeof(struct options));
	if(opt == NULL){
		errno = EINVAL;
		return -1;
	}

	opt->mode = NONE;
	opt->window_len = 1024;
	opt->look_ahead_len = 64;
	opt->verbose = 0;
	opt->file_in = NULL;
	opt->file_out = NULL;
	opt->dict = calloc(2, sizeof(char));
	if(opt->dict == NULL)
		return -1;
	strncpy(opt->dict,"it",2);

	return 0;
}


int handle_options(struct options *opt, int argc, char* argv[])
{
	char c;
	
	while ((c = getopt (argc, argv, "hvcdi:o:w:l:t:")) != -1){
	 	switch(c) {
	 		case 'c':
				opt->mode = COMPRESSION;
				break;
			case 'd':
				opt->mode = DECOMPRESSION;
				break;

           		case 'i':
                		// check if exist and the read permission
                		if(file_check(optarg,F_OK|R_OK) == -1){
					printf("ERROR!!! The input file %s not exists or you haven't the read permission\n",optarg);
                    			return -1;
				}
				opt->file_in = malloc( strlen( optarg ) );
                		strcpy( opt->file_in, optarg );
                		break;

			case 'o':
				
				// check the write permission on the directory and his existence
				if(file_check(optarg,W_OK) == -1){
					printf("ERROR!!! The output file %s not exist or you haven't write permission on it.\n",optarg);
					return -1;
                		}
				opt->file_out = malloc( strlen( optarg ) );
				strcpy( opt->file_out, optarg );
				break;


			case 'v':
				opt->verbose = 1;
				break;

            		case 't':
				// dictionary name must be of two chars
                		if ( strlen(optarg) != 2){
                    			printf("Error : The dictionary will be only of 2 characters\n");
			                return -1;
                		}
                		// check if exist and the read permission
		                if(file_check(optarg,F_OK|R_OK) == -1){
					printf("Error : The dictionary %s doesnt exists or you haven't the read permission\n",optarg);
                			return -1;
				}
                		//opt->dict = malloc(strlen(optarg));
		                strcpy( opt->dict, optarg );

		                break;
			case 'w':
				opt->window_len = atoi(optarg);
				if (opt->window_len < 0){
				        printf("Error : window parameter must be positive\n");
                			return -1;
	                	}
				if (opt->window_len >= 32768){
				        printf("Error : window parameter must be at least 32767\n");
                			return -1;
	                	}	
				break;

			case 'l':
				opt->look_ahead_len = atoi(optarg);
				if (opt->look_ahead_len < 8){
				       	printf("Error : look ahead parameter must be greater or equal to 8\n");
					return -1;
			   	}
				if(opt->look_ahead_len > 255){
				       	printf("Error : look ahead parameter must be almost 255\n");
					return -1;				
				}
				break;

			case 'h': // help
				usage();
				break;

			case '?': // check unknow options or missing parameter
     				if (isprint (optopt))
     			   		fprintf (stderr, "Unknown option `-%c'.\n", optopt);
		        	else
                    			fprintf (stderr, "Unknown character `\\x%x'.\n", optopt);

                		printf("Use -h to see hot to use this program.\n");

                		return -1;

	 	} // end switch

	 } // end getopt

	if( opt->mode == NONE){
		printf("Missing argument -c or -d\n");
		printf("Use -h to see hot to use this program.\n");
		return -1;
       	}
	if(( opt->file_in == NULL) & (opt->mode == DECOMPRESSION)){
		printf("In decompression mode the file input can't be the standard input.\n");
		return -1; 
	}
	
	if(( opt->file_out == NULL) & (opt->mode == COMPRESSION)){
		printf("In compression mode the file output can't be the standard output\n");
		return -1; 
	}


	if( opt->window_len < opt->look_ahead_len ){
		printf("Error : window length must be grater or equal to the look ahead length.\n");
		return -1;
	}

	return 0;

}

/**
 * Print the fields contained in the option structure
 * 
 * @param opt : option structure that contain the informations to be show
 */
void print_options(struct options opt){
	
	if(opt.verbose){
		if(opt.mode){
			printf("Mode : Compression\n");
			printf("Dictionary : %s\n",opt.dict);
		}
		else
			printf("Mode : Decompression\n");
		if(opt.verbose)
			printf("Verbose : ON\n");
		printf("Window size : %d\n",opt.window_len);
		printf("Look ahead buffer size : %d\n",opt.look_ahead_len);		
	}	

	if(opt.file_in == NULL)
		printf("Input : Standard Input\n");	
	else
		printf("File Input : %s\n",opt.file_in);
	if(opt.file_out == NULL)
		printf("Output : Standard Output\n");
	else
		printf("File Output : %s\n",opt.file_out);
	printf("\n");

}


/**
 * overwrite() function is used by file_check(), is used only when you must create a file.
 * Advise the user if a file already exist and in that case ask him if he want to overwrite the
 * file or choose a new name.
 *
 * @param filename	array that contain the file name to check or the new file name to use
 * @return 		0 for success or -1 in case of error
 */
int overwrite(char* filename){
	unsigned char answer;
	int ret ;
	for(;;){
		// check if there is a file with this name
		if(access(filename, R_OK)==0){
			printf("\n%s already exist, do you want overwrite it? [y/n] : ",filename);
			for(;;){
				//ret = scanf("%c",&answer);
				answer = getchar();

				if(answer == '\n') //ignore it
					continue;
				if(answer!='y' && answer!= 'n'){
					printf("\n%c isn't supported, please type 'y' or 'n' : ",answer);
					continue;
				}
				if(answer == 'n')
					break;
				else
					return 0;
			}//end internal for
		}else{
			break;
		}
		
		printf("Insert the new output filename : ");
		ret = scanf("%s",filename);
		if( ret !=1 ){ // the errno is already set by scanf()
			return -1;
		}
	}

	return 0;
}

int file_check(char* filename, int check_type){

	if(check_type == W_OK){
		FILE *file;
		
	 	if(overwrite(filename)!=0)
			return -1;
		file = fopen(filename,"w+");
		if(file == NULL) 
			return -1;
		fclose(file);
		remove(filename);
		return 1;
	}
		
	if( access(filename, check_type)==0 )
		return 1;

	return -1;
}

