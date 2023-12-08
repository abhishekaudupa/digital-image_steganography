#ifndef DECODE_H
#define DECODE_H

#include "types.h" 	// Contains user defined types
#include "common.h"	// Contains common strings
#include "error.h"	// Contains standard error messages

/* 
 * Structure to store information required for
 * encoding secret file to source Image
 * Info about output and intermediate data is
 * also stored
 */

typedef struct _DecodeInfo
{
    /* Secret File Info */
    char *secret_fname;
    FILE *fptr_secret;
    char extn_secret_file[MAX_FILE_SUFFIX];

    /* Stego Image Info */
    char *stego_image_fname;
    FILE *fptr_stego_image;

} DecodeInfo;

/* Decoding function prototypes */

/* Read and validate decode args from argv */
Status read_and_validate_decode_args(char *argv[], DecodeInfo *decInfo);

/* Perform the encoding */
Status do_decoding(char *user_given_destgged_file_name, DecodeInfo *decInfo);

/* Get File pointers for i/p and o/p files */
Status open_files_for_decoding(DecodeInfo *decInfo);

/* Find the magic string in the image file */
Status find_magic_string(FILE *fptr_steg_img);

/* Get the encoded data inside a byte array of 8 bytes */
Status get_data_from_byte_array(char *data, char *byte_buffer);

/* Check if the given string is the magic string */
Status is_magic_string(const char *str);

/* Get the secret data file extension */
Status get_secret_data_file_extn(DecodeInfo *decInfo);

/* Create the secret data file */
Status create_secret_data_file(DecodeInfo *decInfo, const char* user_given_name);

/* Copy the secret data to the secret data file */
Status copy_data_to_secret_data_file(DecodeInfo *decInfo);

/* Function to get a default output file name */
//char *get_default_destegged_output_filename(const char* input_filename, const char* user_given_name, const char *file_extn);
char *get_default_destegged_output_filename(const char* user_given_name, const char *file_extn);

#endif
