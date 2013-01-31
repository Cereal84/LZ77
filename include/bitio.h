/*
 * @file bitio.h
 * 
 * The routines and the structure contained in this file are used to perform bit I/O operations.
 *
 * @author Pischedda Alessandro
 */

#ifndef BITIO_H
#define BITIO_H

#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>
#include <stdint.h>
#include <unistd.h>

#define BIT_WR 1
#define BIT_RD 0
#define MAX_SIZE 1024	//bytes
#define MIN_SIZE 4	//bytes


/** 
 *  This structure is necessary to perform bit I/O
 *  operations.
 */
struct bitfile;



/**
 * @brief The bit_open() function opens the file whose name is the string pointed to by filename.
 *
 * @param *filename	string with file name to open
 * @param mode 		is the access mode, it can be write (BIT_WR) or read (BIT_RD)
 * @param bufsize 	is the dimension, in bits, of the buffer in bitfile structure
 *
 * @return 		BITFILE pointer if success.
 *			NULL if some error occour, and errno is set appropriately.
 *
 * ERRORS
 *	EINVAL		if some function's arguments isn't correct.
 *	Others		are all the possible error returned by open() function.
 *		
 */
struct bitfile* bit_open(const char *filename, int mode, int bufsize );


/**
 *  @brief  Read n bits from the buffer buf and put them in the buffer of bitfile structure
 *
 *  The function bit_write() read n_bits of data from the buffer buf and
 *  store them in the BITFILE structure starting from the offset ofs.
 *
 *  @param fd 		is a pointer to the bitfile structure
 *  @param buf 		buffer pointer where read from
 *  @param n_bits 	number of bit to read
 *  @param ofs 		offset where read from.
 *
 *  @return 	the number of the read bit
 *		-1 if an error is occured, in that case set errno.
 *
 * ERRORS
 *	EINVAL		if some function's arguments isn't correct.
 *	Others		are all the possible error returned by read() function.
 */
int bit_write(struct bitfile *fd, const char *buf, int n_bits, int ofs);


/**
 *  @brief Read n bits from the file descriptor fd and store them into the buffer.
 *
 *  The function bit_read() read n_bits of data from the fd (BITFILE structure) and
 *  store them in the buffer buf starting from the offset ofs.
 *
 *  @param fd 		is a pointer to the bitfile structure (source)
 *  @param buf 		buffer pointer where put the bits read from the bitfile structure (dest)
 *  @param n_bits 	number of bit to read
 *  @param ofs 		offset where write from.
 *
 *  @return	the number of the read bit
 *		-1 if an error is occured, in that case set errno.
 * ERRORS
 *	EINVAL		if some function's arguments isn't correct.
 *	Others		are all the possible error returned by write() function.
 *
 */
int bit_read(struct bitfile *fd, char *buf, int n_bits, int ofs);


/**
 * @brief Force a write operation to the file specified in the BITFILE structure. 
 * 
 * Only for output BITFILE. Force a write operation to the file specified in the BITFILE structure. 
 * ATTENCTION! Padding (zeros) can be added in order to align the data to byte.
 *
 * @param fd pointer to BITFILE structure
 *
 * @return 0 for success, -1 otherwise
 */
int bit_flush(struct bitfile *fd);


/**
 * @brief close a stream
 *
 * The bit_close() flushes the data pointed to by fp (using bit_flush()), close the
 * file descriptor and free the memory occupy by BITFILE structure.
 *
 * @param fp 	pointer to BITFILE structure
 *
 * @return 0 	for success,-1 otherwise
 *
 * ERRORS
 *	EINVAL		if some function's arguments isn't correct.
 *	Others		are all the possible error returned by close() function.
 */
int bit_close(struct bitfile *fp);

#endif
