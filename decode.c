#include <stdio.h>
#include "decode.h"
#include "types.h"
#include <string.h>
#include "common.h"


Status read_and_validate_decode_args(char *argv[], DecodeInfo *decInfo)
{
    // Check if the stego image file is a .bmp file
    if (strstr(argv[2], ".bmp"))
    {
        decInfo->stego_image_fname = argv[2];

        // Check if the output file has a valid extension (.txt, .c, or .sh)
        if (strstr(argv[3], ".txt") || strstr(argv[3], ".c") || strstr(argv[3], ".sh"))
        {
            decInfo->decoded_file_fname = strtok(argv[3], ".");
            return e_success;
        }
    }
    printf("For Decoding -> ./a.out -d <.bmp file> <output file>\n");
    return e_failure;
}

Status Open_files(DecodeInfo *decInfo)
{
    decInfo->fptr_stego_image = fopen(decInfo->stego_image_fname,"r");
    if(decInfo->fptr_stego_image == NULL)
    {
        printf("Error in opening stego image file\n");
        return e_failure;
    }
    return e_success;
}



Status do_decoding(DecodeInfo *decInfo)
{
    printf("INFO: ## Decoding Procedure Started ##\n");
    printf("INFO: Opening required files\n");
    if(Open_files(decInfo) ==  e_success)
    {
        printf("INFO: Opened %s\n", decInfo->stego_image_fname);
        printf("INFO: Done. Opened all required files\n");
        if(decode_magic_string(decInfo -> fptr_stego_image,MAGIC_STRING, decInfo) == e_success)
        {
            printf("INFO: Decoding Magic String Signature\n");
            printf("INFO: Done\n");
            printf("INFO: Decoding Output File Extension Size\n");
            if(decode_extn_size(decInfo) ==  e_success)
            {
                printf("INFO: Done\n");
                printf("INFO: Decoding Output File Extension\n");
                if(decode_secret_file_extn(decInfo) ==   e_success)
                {
                    printf("INFO: Done\n");
                    printf("INFO: Decoding File Size\n");
                    if(decode_secret_file_size(decInfo) == e_success)
                    {
                        printf("INFO: Done\n");
                        printf("INFO: Decoding File Data\n");
                        if(decode_secret_file_data(decInfo) == e_success)
                        {
                           printf("INFO: Done\n");
                           return e_success;
                        }
                        else
                        {
                            printf("EFailed to decode data\n");
                            return e_failure;
                        }
                    }    
                    else
                    {
                        printf("Failed to decode file size\n");
                        return e_failure;
                    }
                }
                else
                {
                     printf("Failed to decode output file extension\n");
                     return e_failure;
                }
            }
            else
            {
                printf("Failed to decode output file extension size\n");
                return e_failure;
            }
        }
        else
        {
            printf("Failed to decode magic string\n");
            return e_failure;
        }
    }
    else
    {
        printf("Failed to open files\n");
        return e_failure;
    }
}

Status decode_magic_string(FILE *fptr, char *magic_string, DecodeInfo *decInfo)
{
    fseek(fptr, 54, SEEK_SET);								
    int size = strlen(magic_string);
    for(int i = 0; i < size; i++)
    {
	    fread(decInfo -> image_data, 8, 1, decInfo -> fptr_stego_image);		
	    decode_byte_from_lsb(decInfo -> image_data, decInfo);				
	    decInfo -> magic_str[i] = decInfo -> single_byte;					
	    decInfo -> magic_str[i+1] = '\0'; //null at end
    }
    //checking
    if(strcmp(magic_string, decInfo -> magic_str) == 0)
        return e_success;			
    else
        return  e_failure;
}


Status decode_extn_size(DecodeInfo *decInfo)
{
    decode_size_from_lsb(decInfo);				
    decInfo -> extn_size = decInfo -> data;			
    return e_success;
}

Status decode_secret_file_extn(DecodeInfo *decInfo)
{
    int size = decInfo -> extn_size;
    for(int i = 0; i < size; i++)
    {
	    fread(decInfo -> image_data, 8, 1, decInfo -> fptr_stego_image);			
	    if(decode_byte_from_lsb(decInfo -> image_data, decInfo) == e_success)
        {    
	        decInfo -> extn_decoded_file[i] = decInfo -> single_byte;					
            decInfo -> extn_decoded_file[i+1] = '\0';	
        }
        else
        {
            printf("error  in decoding extension\n");
            return e_failure;
        }					
    }

    decInfo->decoded_file_fname = strcat(decInfo->decoded_file_fname,decInfo -> extn_decoded_file);
    decInfo->fptr_decoded_file=fopen(decInfo->decoded_file_fname,"w");
    if(decInfo->fptr_decoded_file == NULL)
    {
        printf("Error in opening output file\n");
        return e_failure;
    }
    return e_success;
} 


Status decode_secret_file_size(DecodeInfo *decInfo)
{
    decode_size_from_lsb(decInfo);						
    decInfo -> size_decoded_file = decInfo -> data;				
    return e_success;
}

Status decode_data_from_image(int size, DecodeInfo *decInfo)
{
    for(int i = 0; i < size; i++)							
    {
	    fread(decInfo -> image_data, 8, 1, decInfo -> fptr_stego_image);		
	    decode_byte_from_lsb(decInfo -> image_data, decInfo);				
	    decInfo -> decoded_byte = &(decInfo -> single_byte);
	    fwrite(decInfo -> decoded_byte, 1, 1, decInfo -> fptr_decoded_file);		
    }
    return e_success;
}

Status decode_byte_from_lsb(char *image_buffer, DecodeInfo *decInfo)
{
    char data = 0x00;
    for( int i = 0; i < 8; i++)
    {
	    data = data << 1;
	    data = data | image_buffer[i] & 0x01;			
    }
    decInfo -> single_byte = data;					
	return e_success;
}

Status decode_secret_file_data(DecodeInfo *decInfo)
{
    decode_data_from_image(decInfo -> size_decoded_file, decInfo);
    return e_success;
}


Status decode_size_from_lsb(DecodeInfo *decInfo)
{
    char str[32];							
    int data = 0x00;
    fread(str, 32, 1, decInfo -> fptr_stego_image);				

    for(int i = 0; i < 32; i++)
    {
	    data = data << 1;
	    data = data | str[i] & 0x01;							
    }

    decInfo -> data = data;								
	return e_success;
}