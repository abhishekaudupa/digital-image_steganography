#include <stdio.h>
#include <string.h>
#include "common.h"
#include "types.h"

/*
 * Function to find the file extension of a given filename.
 *
 * This function takes in the pointers to character arrays of existing file
 * name and an empty array for storing the extension characters. If the extn
 * is greater than or equal to MAX_FILE_SUFFIX or if an extension is not found,
 * it displays error and returns a failure flag. Otherwise it copies the 
 * extension to the empty array and returns a success flag.
 *
 * INPUTS: The file name pointer and the file extension pointer.
 *
 * CAUTION: The file pointer has to point to a character array that is 
 * MAX_FILE_SUFFIX characters long.
 *
 * RETURNS: Operation success flag: e_success or e_failure.
 */
Status get_file_extension(const char *file_name, char *file_extension)
{
    if(!file_name || !file_extension)
    {
	FATAL_ERR_MSG;
	return e_failure;
    }

    char *extn_start = strstr(file_name, ".");
    if(!extn_start)
    {
	fprintf(stderr, "No file extension found.\n");
	return e_failure;
    }

    ++extn_start;
    if(strlen(extn_start) >= MAX_FILE_SUFFIX)
    {
	fprintf(stderr, "File extension too long. It could be max %hhd characters long.\n", MAX_FILE_SUFFIX - 1);
	return e_failure;
    }

    strcpy(file_extension, extn_start);
    return e_success;
}

/*
 * Function to get the image pixel data offset in bmp files.
 *
 * This function reads the data offset information bytes in the bmp
 * header and returns it.
 *
 * INPUT: The bmp image file pointer.
 *
 * RETURNS: Pixel data offset if fptr exists, -1 otherwise.
 */
int get_image_data_offset(FILE *fptr_bmp_image)
{
    if(!fptr_bmp_image)
    {
	FATAL_ERR_MSG;
	return -1;
    }

    int size;
    fseek(fptr_bmp_image, 10, SEEK_SET);
    fread(&size, sizeof(int), 1, fptr_bmp_image);
    return size;
}
