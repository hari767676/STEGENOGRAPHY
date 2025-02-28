#include <stdio.h>
#include "encode.h"
#include "types.h"


//-----------------------------------------------------------------------------------------------------
OperationType check_operation_type(char *argv[])
{
    if (strcmp(argv[1], "-e") == 0)
        return  e_encode;
    else if(strcmp(argv[1],"-d")==0)
        return e_decode;
    else
        return e_unsupported;
}

//-------------------------------------------------------------------------------------------------------

Status read_and_validate_encode_args(char *argv[], EncodeInfo *encInfo)
{
    if (strstr(argv[2], ".bmp"))
    {
        encInfo->src_image_fname = argv[2];
        if (strstr(argv[3], ".txt") || strstr(argv[3], ".c") || strstr(argv[3], ".sh"))
        {
            encInfo->secret_fname = argv[3];
            encInfo->extn_secret_file = strstr(argv[3], ".");
            if (argv[4])
            {
                if (strstr(argv[4], ".bmp"))
                {
                    encInfo->stego_image_fname = argv[4];
                    return e_success;
                }
                else
                {
                    printf("For Encoding -> ./a.out -e <.bmp file> <secret file>\n");
                    return e_failure;
                }
            }
            encInfo->stego_image_fname = "stego.bmp";
            return e_success;
        }
        else
        {
            printf("For Encoding -> ./a.out -e <.bmp file> <secret file>\n");
            return e_failure;
        }
    }
    printf("For Encoding -> ./a.out -e <.bmp file> <secret file>\n");
    return e_failure;
}

//-----------------------------------------------------------------------------------------------------

Status open_files(EncodeInfo *encInfo)
{
    encInfo->fptr_src_image = fopen(encInfo->src_image_fname, "r");
    if (encInfo->fptr_src_image == NULL)
    {
    	perror("fopen"); 
    	fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->src_image_fname);
        return e_failure;
    }
    encInfo->fptr_secret = fopen(encInfo->secret_fname, "r");
    if (encInfo->fptr_secret == NULL)
    {
    	perror("fopen");
    	fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->secret_fname);
        return e_failure;
    }
    encInfo->fptr_stego_image = fopen(encInfo->stego_image_fname, "w");
    if (encInfo->fptr_stego_image == NULL)
    {
    	perror("fopen");
    	fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->stego_image_fname);

    	return e_failure;
    }
    return e_success;
}
//---------------------------------------------------------------------------------------------------
Status check_capacity(EncodeInfo *encInfo)
{
    encInfo->image_capacity=get_image_size_for_bmp(encInfo -> fptr_src_image);
    encInfo->size_secret_file=get_file_size(encInfo->fptr_secret);
    if(encInfo->image_capacity > (54 + (2 + 4 + 4 + 4 + encInfo->size_secret_file)*8))
        return e_success;
    return e_failure;
}

//---------------------------------------------------------------------------------------------------
uint get_file_size(FILE  *fptr)
{
    fseek(fptr,0,SEEK_END);
    return ftell(fptr);
}
//-------------------------------------------------------------------------------------------------
uint get_image_size_for_bmp(FILE *fptr_image)
{
    uint width, height;
    // Seek to 18th byte

    fseek(fptr_image, 18, SEEK_SET);
    // Read the width (an int)
    fread(&width, sizeof(int), 1,fptr_image);
    //printf("INFO: Checking capacity\n");
    printf("INFO: Width = %u\n", width);

    // Read the height (an int)
    fread(&height, sizeof(int), 1,fptr_image);
    printf("INFO: Height = %u\n", height);
    // Return image capacity
    return width * height * 3; //3 is for converting pixel to bytes
}
//------------------------------------------------------------------------------------------------------
Status copy_remaining_img_data(FILE *fptr_src, FILE *fptr_dest)
{
    char ch;
    while(fread(&ch, 1, 1, fptr_src) != 0)
	    fwrite(&ch, 1, 1, fptr_dest);
    return e_success;
}


//----------------------------------------------------------------------------------------------------
Status encode_secret_file_data(EncodeInfo *encInfo)
{
    fseek(encInfo -> fptr_secret, 0, SEEK_SET);		
    char buffer[encInfo->size_secret_file];
    fread(buffer,encInfo->size_secret_file,1,encInfo->fptr_secret);

    if(encode_data_to_image(buffer,encInfo->size_secret_file,encInfo) ==  e_success)
    {
        return e_success;
    }
    return e_failure;
}
//-------------------------------------------------------------------------------------------------
Status encode_secret_file_size(int file_size, EncodeInfo *encInfo)
{
    if(encode_size_to_lsb(file_size,encInfo) == e_success)
    {
        return e_success;
    }
    return e_failure;
}

Status encode_secret_file_extn(const char *file_extn, EncodeInfo *encInfo)
{
    if(encode_data_to_image(file_extn,strlen(file_extn),encInfo) ==  e_success)
    {
        return e_success;
    }
    return e_failure;
}

Status encode_secret_file_extn_size(int extn_size, EncodeInfo *encInfo)
{
    if(encode_size_to_lsb(extn_size,encInfo) == e_success)
    {
        return e_success;
    }
    return e_failure;
}

Status encode_byte_to_lsb(char data, char *image_buffer)
{
    for (int i = 0; i < 8; i++)
    {
        image_buffer[i] = image_buffer[i] & 0xFE;
        image_buffer[i] = image_buffer[i] | ((data >> (7 - i)) & 1);
    }
    return e_success;
}


Status  encode_data_to_image(const char *data, int size, EncodeInfo *encInfo)
{
    for(int i = 0; i < size; i++)				
    {
	    fread(encInfo -> image_data, 8, 1, encInfo -> fptr_src_image);		
	    if(encode_byte_to_lsb(data[i], encInfo -> image_data) == e_success)
            fwrite(encInfo -> image_data, 8, 1, encInfo -> fptr_stego_image);		
        else
        {
            printf("error encoding byte to lsb\n");
            return  e_failure;
        }
    }
    return e_success;
}
//----------------------------------------------------------------------------------------------------
Status encode_magic_string(char *magic_string, EncodeInfo *encInfo)
{
    fseek(encInfo->fptr_src_image,54,SEEK_SET);
    if(encode_data_to_image(magic_string, strlen(magic_string), encInfo) ==  e_success)
        return e_success;
    return e_failure;
}


Status encode_size_to_lsb(int size, EncodeInfo *encInfo)
{
    char buffer[32];                                
    fread(buffer, 32, 1, encInfo -> fptr_src_image);  
    for(int i = 0; i < 32; i++)
    {
        buffer[i] = buffer[i] & 0xFE;
        buffer[i] = buffer[i] | ((size >> (31 - i)) & 1);
    }
    fwrite(buffer, 32, 1, encInfo -> fptr_stego_image);            
    return e_success;
}

//-----------------------------------------------------------------------------------------------------
Status copy_bmp_header(FILE *fptr_src_image, FILE *fptr_dest_image)
{
    //function to copy first 54 bytes from beautigul to stego image
    char str[54];
    fseek(fptr_src_image,0,SEEK_SET); 
    fread(str,1,54,fptr_src_image);
    fwrite(str,1,54,fptr_dest_image);
    return e_success;
}

Status do_encoding(EncodeInfo *encInfo)
{
    printf("INFO: Opening required files\n");
    if(open_files(encInfo) == e_success)
    {
        printf("INFO: Opened %s\n", encInfo->src_image_fname);
        printf("INFO: Opened %s\n", encInfo->secret_fname);
        printf("INFO: Opened %s\n", encInfo->stego_image_fname);
        printf("INFO: Done\n");
        printf("INFO: ## Encoding Procedure Started ##\n");
        if(copy_bmp_header(encInfo-> fptr_src_image, encInfo-> fptr_stego_image) == e_success)
        { 
            printf("INFO: Copying BMP Header\n");
            printf("INFO: Done\n");
            if(check_capacity(encInfo) == e_success)
            {
                printf("INFO: Checked capacity\n");
                printf("INFO: Done\n");
                printf("INFO: Encoding Magic String Signature\n");
                if(encode_magic_string(MAGIC_STRING,encInfo) == e_success)
                {
                    printf("INFO: Done\n");
                    printf("INFO: Encoding %s File Extension size\n", encInfo->secret_fname);
                    if(encode_secret_file_extn_size(strlen(encInfo->extn_secret_file),encInfo) == e_success)
                    {
                        printf("INFO: Done\n");
                        printf("INFO: Encoding %s File Extn\n", encInfo->secret_fname);
                        if(encode_secret_file_extn(encInfo->extn_secret_file,encInfo) ==  e_success)
                        {
                            printf("INFO: Done\n");
                            printf("INFO: Encoding %s File size\n", encInfo->secret_fname);
                            if(encode_secret_file_size(encInfo->size_secret_file,encInfo)==  e_success)
                            {
                                printf("INFO: Done\n");
                                printf("INFO: Encoding %s File Data\n", encInfo->secret_fname);
                                if(encode_secret_file_data(encInfo) == e_success)
                                {
                                    printf("INFO: Done\n");
                                    printf("INFO: Copying Left over Data\n");
                                    if(copy_remaining_img_data(encInfo->fptr_src_image, encInfo->fptr_stego_image) == 0)
                                    {
                                        printf("INFO: Done\n");
                                        printf("INFO: ## Encoding Done Successfully ##\n");
                                        return e_success;
                                    }
                                    else
                                    {
                                        printf("INFO: Failed to copy remaining image data\n");
                                        return e_failure;
                                        }
                                    }
                                else
                                {
                                    printf("INFO: Failed to encode secret file size\n");
                                    return e_failure;
                                }
                            }
                        else
                            {
                                printf("INFO: Failed to encode secret file extension size\n");
                                return  e_failure;
                            }
                        }
                    else
                    {
                        printf("INFO: Failed to encode secret file data\n");
                        return  e_failure;
                    }
                }
            else
                {
                    printf("INFO: Failed to encode secret file extension\n");
                    return  e_failure;
                }
            }
        else
            {
                 printf("INFO: Failed to encode magic string\n");
                return  e_failure;
            }
        }   
    else
        {
            printf("INFO: Failed to check capacity\n");
            return  e_failure;
        }
    }
    else
        {
            printf("INFO: Failed to open stego image file\n");
            return  e_failure;
        }
    }
else
    {
        printf("INFO: Failed to open files\n");
        return  e_failure;
    }
}




