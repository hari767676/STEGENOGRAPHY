#include <stdio.h>
#include "encode.h"
#include "decode.h"
#include "types.h"

int main(int argc, char  *argv[]) 
{
    if(argc<4)
    {
        printf("Error : Insufficient arguments\n");
        printf("For Encoding -> ./a.out -e  <.bmp file> secret file>\nFor Decoding  -> ./a.out -d <.bmp file> <output file>\n");
        return e_failure;
    }
    else
    {
        if(check_operation_type(argv) == e_encode)
        {
            printf("INFO: Selected Encoding\n");
            EncodeInfo encInfo;
            if(read_and_validate_encode_args(argv, &encInfo) == e_success)
            {
                printf("INFO: Read and validation successfull\n");
                if(do_encoding(&encInfo) == e_success)
                {
                    //printf("Encoding is completed\n");
                }
                else
                {
                    printf("INFO: Error in encoding\n");
                }
            }
            else
            {
                printf("INFO: Error in reading and validation for encoding\n");
            }
        }
       else if(check_operation_type(argv) == e_decode)
       {
            printf("INFO: Selected decoding\n");
            DecodeInfo decInfo;
            if(read_and_validate_decode_args(argv, &decInfo) == e_success)
            {
                printf("INFO: Read and validation Successfull\n");
                if(do_decoding(&decInfo) == e_success)
                {
                    printf("INFO: ##Decoding Done Successfully##\n");
                }
                else
                {
                    printf("INFO: Error in decoding\n");
                }
            }
            else
            {
                printf("INFO: Error in read and validation\n");
            }
        }
        else
        {
            printf("Error : Invalid operation type\n");
            printf("For Encoding -> ./a.out -e  <.bmp file> <secret file>\n");
            printf("For Decoding  -> ./a.out -d <.bmp file> <output file>\n");
            return e_failure;
        }
    }
 return 0;
}