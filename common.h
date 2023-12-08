#ifndef COMMON_H
#define COMMON_H

#include "types.h"
#include "error.h"

/* Magic string to identify whether stegged or not */
#define MAGIC_STRING "#*"

/* Encoded data separator string */
#define ENC_DATA_SEPARATOR_STRING "*"

#define MAX_SECRET_BUF_SIZE 1
#define MAX_IMAGE_BUF_SIZE (MAX_SECRET_BUF_SIZE * 8)
#define MAX_FILE_SUFFIX 4

/* Encode and decode arguments from the user */
#define ENCODE_ARG "-e"
#define DECODE_ARG "-d"

/* The default prefix for encoded .bmp file  */
#define DEFAULT_ENCODED_FILE_PREFIX "stegged_"

/* The default suffix for encoded .bmp file  */
#define DEFAULT_ENCODED_FILE_SUFFIX "_default"

/* The default suffix for decoded file  */
#define DEFAULT_DECODED_FILE_SUFFIX "_default."

/* The default suffix for decoded file  */
#define DEFAULT_DECODED_FILE_PREFIX "destegged_"

/* File extension of the image file  */
#define IMG_FILE_EXTN ".bmp"

//#define BMP_HEADER_SIZE 54

/* Function to get file extension */
Status get_file_extension(const char *file_name, char *file_extension);

/* Function to get the image pixel data offset */
int get_image_data_offset(FILE *fptr_bmp_image);

#endif
