#ifndef DECODE_H
#define DECODE_H

#include "types.h" 


#define MAX_SECRET_BUF_SIZE 1
#define MAX_IMAGE_BUF_SIZE (MAX_SECRET_BUF_SIZE * 8)
#define MAX_FILE_SUFFIX 4
#define MAGIC_STR_SIZE 2

typedef struct _DecodeInfo
{
    char *stego_image_fname;		  		
    FILE *fptr_stego_image;		  		
    char image_data[20];                
    
    char *decoded_file_fname;				
    FILE *fptr_decoded_file;				
    char magic_str[20];			
    char single_byte;					
    char *decoded_byte;					
    int data;					
    int extn_size;					
    char extn_decoded_file[20];
    long size_decoded_file;				

} DecodeInfo;


/* Read and validate*/
Status read_and_validate_decode_args(char *argv[], DecodeInfo *decInfo);

/* main decoding */
Status do_decoding(DecodeInfo *decInfo);
//opening files
Status Open_files(DecodeInfo *decInfo);

/* decode Magic String */
Status decode_magic_string(FILE *fptr, char *magic_string, DecodeInfo *decInfo); 

/* Decode secret file extension size */
Status decode_extn_size(DecodeInfo *decInfo);

/* Decode secret file extenstion */
Status decode_secret_file_extn(DecodeInfo *decInfo);

/* Decode secret file size */
Status decode_secret_file_size(DecodeInfo *decInfo);

/* Decode secret file data*/
Status decode_secret_file_data(DecodeInfo *decInfo);

/* Decode data from image */
Status decode_data_from_image(int size, DecodeInfo *decInfo);

/* Decode byte from lsb */
Status decode_byte_from_lsb(char *image_buffer, DecodeInfo *decInfo);

/* decode size from lsb */
Status decode_size_from_lsb(DecodeInfo *decInfo);

#endif