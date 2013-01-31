/*
 * @file bitio.h
 * 
 * The routines and the structure contained in this file are used to perform bit I/O operations.
 * The structure necessary to perform this operations is bitfile structure described below.
 * This structure has the flowing fields :
 *
 * fd 		file_descriptor 
 * mode 	access mode
 * bufsize 	internal buffer size
 * w_inizio 	from which byte read or write data
 * ofs		offset of which bit in the byte read or write from
 * n_bits	how many bits of usefull data have the buffer
 * buf		internal buffer where store or read data
 *
 * The buffer size is always multiple of a byte.
 *
 * Graphical explenation of this field
 * 
 * X means usefull data. 
 *
 * READ operation
 * At the begining the buffer will be filled with data 
 * after a read operation we've consume some bits and 
 * the buffer will be 
 *                   _______ ofs 
 *  7               |      0    
 *  __ __ __ __ __ _| __ __
 * |XX_XX_XX_XX_XX_XX______| <- w_inizio
 * |XX_XX_XX_XX_XX_XX_XX_XX|
 * |XX_XX_XX_XX_XX_XX_XX_XX|
 *                        
 * 
 * The offset will be 2. So the w_inizo tell us which
 * is the byte to start to read and the ofs from which bit.
 *
 * WRITE operation
 * The buffer began empty and after some write operations 
 * the buffer will be
 *
 *  7                     0
 *  __ __ __ __ __ __ __ __
 * |XX_XX_XX_XX_XX_XX_XX_XX|
 * |XX_XX_XX_XX_XX_XX_XX_XX|
 * |____________XX_XX_XX_XX| <- w_inizio
 *            ^ 
 *            |__ofs
 *  
 *
 * In write mode isn't possible to have the following
 * situation
 *
 *  7                     0
 *  __ __ __ __ __ __ __ __
 * |XX_XX_XX_XX_XX_XX______|
 * |XX_XX_XX_XX_XX_XX_XX_XX|
 * |____________XX_XX_XX_XX| <- w_inizio
 *
 * so the data not aligned to byte is only the last. This
 * is really important for bit_flush() function because it 
 * can write the firsts n_bits/8 byte using write() function
 * and the write last bits (n_bits%8) as a byte filling with 
 * zeroes the remaining 8-last_bits. 
 *
 *
 *
 * @author Pischedda Alessandro
 */

#include "../include/bitio.h"

/** @struct bitfile
 *
 *  This structure is necessary to perform bit I/O
 *  operations.
 */
struct bitfile{

	int fd;		// File Descriptor
	int mode;	// 1 Write (BIT_WR) - 0 Read (BIT_RD)
	int bufsize;	// number of bits for the buffer
	int w_inizio;
	int ofs;	// offset where begin the data in the buffer
	int n_bits;
	char buf[0];
};

struct bitfile* bit_open(const char *filename, int mode, int bufsize ){

	int fd;
	int n_bytes;
	struct bitfile *bit_fp = NULL;

	if( filename == NULL || filename == '\0' || (mode != BIT_RD && mode != BIT_WR) ){
		errno = EINVAL;
		return NULL;
	}


	// bufsize must be a multiple of 8 and in the range MIN/MAX
	n_bytes = bufsize/8;
	if( n_bytes < MIN_SIZE )
	{
		n_bytes = MIN_SIZE;
	}else{
		if( n_bytes > MAX_SIZE )
			n_bytes = MAX_SIZE;
	}

	fd = open(filename,mode==0?O_RDONLY:(O_WRONLY|O_CREAT), S_IRWXU);

	if( fd == -1)
		return NULL;
	
	/* calloc initialize the structure with zeros */
	bit_fp = (struct bitfile*)calloc( 1, sizeof(struct bitfile) + n_bytes );

	if(bit_fp == NULL){
		close(fd);
	}
	else
	{
		// fill the bitfile structure
		bit_fp->fd = fd;
		bit_fp->mode = mode;
		bit_fp->bufsize = n_bytes*8;
		//all other parameters are initialized to zero thanks to calloc()
	}

	return bit_fp;
}



int bit_write( struct bitfile *fd, const char *buf, int n_bits, int ofs)
{

	uint8_t w_mask; // bitmask for buf 
	uint8_t mask;   // bitmask for internal buffer
	unsigned int bits_write = 0;
        const char *p;

	if( (fd == NULL ) || (buf == NULL) || (ofs < 0) || (ofs > 7) || ( fd->mode != BIT_WR ) || ( n_bits < 0) ){
		errno = EINVAL;
		return -1;
	}

	
	p = buf;
        w_mask=1<<ofs;
	mask = 1<<fd->ofs;

      	while(n_bits > 0){

                // flush the internal buffer if it's full
		if( fd->n_bits  == fd->bufsize )
			bit_flush(fd);

		if(*p & w_mask ){
			fd->buf[fd->w_inizio] |= mask;  //set
		}else{
			fd->buf[fd->w_inizio] &= ~mask;  //reset
		}


		// go to next row
		if(w_mask == 0x80){
			p++;	
			w_mask = 1;
		}else{
			w_mask <<= 1;
		}

		// same thing for the bitfile buffer
		if(mask == 0x80){
			fd->w_inizio++;	
			mask = 1;
		}else{
			mask <<= 1;
		}


                // update the info about missing bit to write and bit wrote
		n_bits--;
		bits_write++;		
		fd->ofs = (fd->ofs+1)%8; //update the ofs in bitfile structure
		fd->n_bits++;

  	} 
	return bits_write;
}



int bit_read(struct bitfile *fd,char* buf, int n_bits, int ofs){

	uint8_t w_mask, mask;
	// int pos;
	int ret, bits_read;
	char *p = buf;

	/* Check the argouments*/
	if( (fd == NULL ) || (buf == NULL) || (ofs < 0) || (ofs > 7) || ( fd->mode != BIT_RD ) || ( n_bits < 0) ){
		errno = EINVAL;
		return -1;
	}

	w_mask=1<<ofs;
	mask = 1<<fd->ofs;
	bits_read = 0;

	while(n_bits > 0){

		// is fd->buf empty (try to) fill it
		if(fd->n_bits == 0)
		{
			fd->ofs = 0;	// first usefull bit 
			fd->w_inizio = 0;
			ret = read(fd->fd, fd->buf ,fd->bufsize/8);

			mask = 1;
			// an error occur in the read operation above
			if(ret == -1 || ret == 0)
				return ret;

			fd->n_bits = ret * 8;
		}


		if(fd->buf[fd->w_inizio] & mask ){
			*p |= w_mask;  //set 
		}else{
			*p &= ~w_mask;  //reset
		}

		// going to next row
		if(w_mask == 0x80){
			p++;	
			w_mask = 1;
		}else{
			w_mask <<= 1;
		}

		// same thing for the bitfile buffer
		if(mask == 0x80){
			fd->w_inizio++;	
			mask = 1;
			
		}else{
			mask <<= 1;
		}

		fd->ofs = (fd->ofs+1)%8; //update the ofs in bitfile structure
		fd->n_bits--;
		n_bits--;
		bits_read++;

	}//fine while
	return bits_read;
}


int bit_flush(struct bitfile *fp){

	int ret, bytes_left,bits_left;

	if ( (fp == NULL) || ( fp->mode != BIT_WR ) ){
		errno = EINVAL;	
		return -1;
	}

	bytes_left = fp->n_bits/8;
	bits_left = fp->n_bits%8;

	// write bytes_left from the top of the buffer
	if(bytes_left){
		ret = write(fp->fd, fp->buf, bytes_left);
		if(ret == -1){
			return -1;
		}	
	}

	// Are there some bit/s left ? 
	if(bits_left)
	{
		int mask;
		char last_data;

		mask = (1<<bits_left)-1;
		last_data = fp->buf[fp->w_inizio] & mask;
		ret = write(fp->fd, &last_data, 1);
	}
		
	fp->ofs = 0;
	fp->w_inizio = 0;
	fp->n_bits = 0;

	return 0;
}



int bit_close(struct bitfile * fp){

	if(fp == NULL){
		errno = EINVAL;
		return -1;
	}

	if(fp->mode == BIT_WR )
		bit_flush(fp);

	close(fp->fd);
	free(fp);
	return 0;
}


