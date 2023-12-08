#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "decode.h"
#include "types.h"
#include "error.h"
#include "common.h"

/*
 * Function to check the validity of arguments input by the user and
 * return the status.
 *
 * This function check for input files names in the argument list
 * provided by the user that suits the type of operation to be
 * performed on the inputs: decoding.
 * 
 * Decoding requires an input image file name and an optional argument for the output
 * file name, in that order.
 *
 * If all inputs are valid, this function initialized the names of the
 * input files in the appropriate fields of the DecodeInfo object that
 * is passed by reference into this function. If no output file name is
 * provided by the user, a default name is given.
 *
 * INPUTS: Argument vector from the main() function and the DecodeInfo
 *         variable pointer.
 *
 * RETURNS: The check status enum: e_success or e_failure.
 */
Status read_and_validate_decode_args(char *argv[], DecodeInfo *decInfo)
{
    if(!argv || !decInfo)
    {
	FATAL_ERR_MSG;
	return e_failure;
    }

    if(!argv[2])
    {
	fprintf(stderr, "Error: Please input a %s file as the second argument:\n%s <%s/%s> <image%s>\n", IMG_FILE_EXTN, argv[0], ENCODE_ARG, DECODE_ARG, IMG_FILE_EXTN);
	return e_failure;
    }

    if(!strstr(argv[2], ".bmp"))
    {
	fprintf(stderr, "Error: Please input a %s file as the second argument:\n%s <%s/%s> <image%s>\n", IMG_FILE_EXTN, argv[0], ENCODE_ARG, DECODE_ARG, IMG_FILE_EXTN);
	return e_failure;
    }

    decInfo->stego_image_fname = argv[2];
    return e_success;
}

/* Function to do the decoding of encoded message in the image file.
 *
 * This function acts as a driver and calls other functions in succession 
 * to do the decoding of the encoded message in the input image file, if 
 * such a message exists in it. While it successively calls functions to 
 * carry out various steps of the decoding process, it checks for 
 * success/failure status of the operation. Upon success of each operation, 
 * it proceeds to the next step, otherwise, it exits the program.
 *
 * This functon calls functions to carry out below operations in succession
 *	- Open the image file
 *	- Locate the magic string
 *	- Extract the file extension of the encoded data
 *	- Create the output file
 *	- Copy the encoded data to the output file.
 *
 * Failure of any one of the above operation leads to the termination of 
 * the program.
 *
 * INPUTS: The DecodeInfo object and the user given output file name.
 *
 * RETURNS: Operation status enum: e_success or e_failure.
 */
Status do_decoding(char *user_given_destegged_file_name, DecodeInfo *decInfo)
{
    if(!decInfo)
    {
	FATAL_ERR_MSG;
	return e_failure;
    }

    Status file_opening_status = open_files_for_decoding(decInfo);
    if(file_opening_status == e_failure)
    {
	fprintf(stderr, "File opening failed.\n");
	return e_failure;
    }
    printf("Image file opening succeeded.\n");

    Status find_magic_string_status = find_magic_string(decInfo->fptr_stego_image);
    if(find_magic_string_status == e_failure)
    {
	fprintf(stderr, "The input image file contains no data encoded/stegged\n");
	return e_failure;
    }
    printf("Magic string detected.\n");

    Status get_secret_data_file_extn_status = get_secret_data_file_extn(decInfo);
    if(get_secret_data_file_extn_status == e_failure)
    {
	fprintf(stderr, "Secret data file extension acquisition failed\n");
	return e_failure;
    }
    printf("Encoded data file extension acquired.\n");

    Status create_secret_data_file_status = create_secret_data_file(decInfo, user_given_destegged_file_name);
    if(create_secret_data_file_status == e_failure)
    {
	fprintf(stderr, "Secret data file creation failed\n");
	return e_failure;
    }
    printf("Output file created.\n");

    Status copy_secret_data_to_secret_data_file_status = copy_data_to_secret_data_file(decInfo);
    if(copy_secret_data_to_secret_data_file_status == e_failure)
    {
	fprintf(stderr, "Secret data copy failed.\n");
	return e_failure;
    }
    printf("Encoded data copied to output file: %s\n", decInfo->secret_fname);

    free(decInfo->secret_fname);
    fclose(decInfo->fptr_stego_image);
    fclose(decInfo->fptr_secret);
    return e_success;
}

/*
 * Function to open files for decoding.
 *
 * This function opens one file: the image file with encoded information for further
 * processing.
 *
 * INPUTS: The DecodeInfo object.
 *
 * RETURNS: Operation status enum: e_success or e_failure.
 */
Status open_files_for_decoding(DecodeInfo *decInfo)
{
    if(!decInfo)
    {
	FATAL_ERR_MSG;
	return e_failure;
    }

    decInfo->fptr_stego_image = fopen(decInfo->stego_image_fname, "rb");
    if (decInfo->fptr_stego_image == NULL)
    {
	perror("fopen");
	fprintf(stderr, "ERROR: Unable to open file %s\n", decInfo->stego_image_fname);

	return e_failure;
    }
    return e_success;
}

/*
 * Function to convert a given numeric string into an integer.
 *
 * This function takes a numeric string and it converts it into an integer 
 * represented by the input string.
 *
 * INPUTS: The numberic string character array pointer.
 * 
 * RETURNS: The integer form of the numeric string input by the user.
 */

int atoi(const char str[])
{
    int strToNum = 0;							
    int i = 0;							
    int startCharNumFlag = (str[0] >= '0' && str[0] <= '9');		
    int startCharSignFlag = (str[0] == '+' || str[0] == '-');		


    if(!startCharNumFlag && !startCharSignFlag)				
	return 0;

    int sign = (str[0] == '-')? 1: 0;					
    i = (startCharNumFlag)? 0: 1;					
    while(str[i])							
    {
	if(str[i] >= '0' && str[i] <= '9')				
	    strToNum = strToNum * 10 + (str[i] - '0');			
	else
	    break;
	++i;
    }
    return (sign)? -strToNum: strToNum;	
}

/*
 * Function to read encoded data from a byte array of MAX_IMAGE_BUF_SIZE
 * bytes.
 *
 * This function takes in a char buffer of 8 bytes, takes the LSB of each
 * consecutive byte and puts it into successive bits of the output char,
 * starting from the LSB of the output char.
 *
 * INPUTS: The data char to which data has to be read and the pointer to
 * the buffer from which the data has to be extracted.
 *
 * RETURNS: Operation status enum: e_success or e_failure.
 */
Status get_data_from_byte_array(char *data, char *byte_buffer)
{
    if(!data || !byte_buffer)
    {
	FATAL_ERR_MSG;
	return e_failure;
    }

    *data = 0;
    for(int i = 0; i < MAX_IMAGE_BUF_SIZE; ++i)
	*data = *data | ((byte_buffer[i] & 1) << i);
    return e_success;
}

/*
 * Function to check if the input string is magic string.
 *
 * INPUTS: The pointer to the string to be checked.
 *
 * REUTRNS: e_success if there is a match, e_failure otherwise.
 */
Status is_magic_string(const char *str)
{
    if(!str)
    {
	FATAL_ERR_MSG;
	return e_failure;
    }
    return strncmp(str, MAGIC_STRING, strlen(MAGIC_STRING))? e_failure: e_success;
}

/*
 * Function to check if the given .bmp file has MAGIC_STRING encoded in it.
 *
 * This function seeks to the BMP_HEADER_SIZEth position in the input file and
 * checks if the first bytes have the MAGIC_STRING encoded in it. This signifies
 * that a message has been encoded in the image file.
 *
 * INPUTS: The image file pointer.
 *
 * RETURNS: e_success if MAGIC_STRING is found, e_failure otherwise.
 */
Status find_magic_string(FILE *fptr_steg_img)
{
    if(!fptr_steg_img)
    {
	FATAL_ERR_MSG;
	return e_failure;
    }

    int bmp_image_data_offset = get_image_data_offset(fptr_steg_img);
    fseek(fptr_steg_img, bmp_image_data_offset/*BMP_HEADER_SIZE*/, SEEK_SET);
    if(ferror(fptr_steg_img))
    {
	FILE_SEEK_ERR;
	return e_failure;
    }

    char magic_str[strlen(MAGIC_STRING)];
    for(int i = 0; i < strlen(MAGIC_STRING); ++i)
    {
	char byte_arr[MAX_IMAGE_BUF_SIZE];
	fread(byte_arr, MAX_IMAGE_BUF_SIZE, 1, fptr_steg_img);

	if(feof(fptr_steg_img) || ferror(fptr_steg_img))
	    break;

	Status get_data_status = get_data_from_byte_array(magic_str + i, byte_arr);
	if(get_data_status == e_failure)
	{
	    fprintf(stderr, "Data fetch failed while searching for magic string.\n");
	    return e_failure;
	}
    }
    return is_magic_string(magic_str);
}

/*
 * Function to read the file extension of the data encoded in image file.
 *
 * This function expects the file indicator to be at the position immediately 
 * after the MAGIC_STRING string and from which point it reads data until it 
 * encounters the ENC_DATA_SEPARATOR_STRING. At that point it stops reading 
 * and considers the data read till the ENC_DATA_SEPARATOR_STRING to be the 
 * file extension of the data encoded. It then copies this extension into
 * the relevant member of the DecodeInfo object that is passed as an input.
 *
 * INPUTS: The DecodeInfo object
 *
 * RETURNS: Operaton status enum: e_success or e_failure.
 */
Status get_secret_data_file_extn(DecodeInfo *decInfo)
{
    if(!decInfo)
    {
	FATAL_ERR_MSG;
	return e_failure;
    }

    FILE *fptr_steg_img = decInfo->fptr_stego_image;
    if(!fptr_steg_img)
    {
	FATAL_ERR_MSG;
	return e_failure;
    }

    int i = 0;
    while(1)
    {
	char byte_arr[MAX_IMAGE_BUF_SIZE];
	fread(byte_arr, MAX_IMAGE_BUF_SIZE, 1, fptr_steg_img);

	if(feof(fptr_steg_img) || ferror(fptr_steg_img))
	    break;

	Status get_data_status = get_data_from_byte_array(decInfo->extn_secret_file + i, byte_arr);
	if(get_data_status == e_failure)
	{
	    fprintf(stderr, "Data fetch failed while fetching file extension.\n");
	    return e_failure;
	}
	if(decInfo->extn_secret_file[i] == '*')
	    break;
	++i;
    }
    decInfo->extn_secret_file[i] = '\0';
    return e_success;
}

/*
 * Function to create the file where the decoded data is saved and given to the user.
 *
 * This function creates a file with 
 *	- either user given name
 *	- or a default name
 * along with the file extension extracted from the encoded data, creates the file, 
 * opens the file handle on the relevant member of the DecodeInfo object that is 
 * passed as an input to this function. This file will be used to save the decoded 
 * info further down the line.
 * 
 * INPUTS: The DecodeInfo object and the user given name for the output file.
 *
 * RETURNS: Operaton status enum: e_success or e_failure.
 */
Status create_secret_data_file(DecodeInfo *decInfo, const char* user_given_name)
{
    if(!decInfo)
    {
	FATAL_ERR_MSG;
	return e_failure;
    }

    //char *secret_data_file_name = get_default_destegged_output_filename(user_given_name, decInfo->extn_secret_file);
    decInfo->secret_fname = get_default_destegged_output_filename(user_given_name, decInfo->extn_secret_file);

    //decInfo->fptr_secret = fopen(secret_data_file_name, "wb");
    decInfo->fptr_secret = fopen(decInfo->secret_fname, "wb");
    if(!decInfo->fptr_secret)
    {
	perror("fopen");
	fprintf(stderr, "ERROR: Unable to open file %s\n", decInfo->stego_image_fname);
	return e_failure;
    }
    return e_success;
}

/*
 * Function to copy the secret data into the output file.
 *
 * This function copies the encoded data into the output file in these steps
 *	- First, it reads the size of the secret data that is encoded in the image file. 
 *	  It expects this size to be encoded as a numeric string, which it then proceeds 
 *	  to convert to an integer type.
 *	- It then allocates the necessary amount of dynamic memory and reads the secret 
 *	  data encoded in the image file based on the size of the data it fetched in the 
 *	  previous step.
 *	- It then writes the contents of this buffer into the output file, which is then 
 *	  seen by the user.
 *
 * INPUTS: The DecodeInfo object.
 *
 * RETURNS: Operaton status enum: e_success or e_failure.
 */
Status copy_data_to_secret_data_file(DecodeInfo *decInfo)
{
    if(!decInfo)
    {
	FATAL_ERR_MSG;
	return e_failure;
    }

    FILE *fptr_steg_img = decInfo->fptr_stego_image;

    //read the encoded file size
    int i = 0;
    char msg_size[10];
    while(1)
    {
	char byte_arr[MAX_IMAGE_BUF_SIZE];
	fread(byte_arr, MAX_IMAGE_BUF_SIZE, 1, fptr_steg_img);

	if(feof(fptr_steg_img) || ferror(fptr_steg_img))
	    break;

	Status get_data_status = get_data_from_byte_array(msg_size + i, byte_arr);
	if(get_data_status == e_failure)
	{
	    fprintf(stderr, "Data fetch failed while fetching secret data size.\n");
	    return e_failure;
	}
	if(msg_size[i] == '*')
	    break;
	++i;
    }

    //append null terminator
    msg_size[i] = 0;

    //convert the numeric string to integer
    int msg_size_i = atoi(msg_size);

    i = 0;
    //allocate memory to store the secret message
    char *secret_msg = malloc(msg_size_i + 1);

    //read the encoded secret message.
    for(int j = 0; j < msg_size_i; ++j)
    {
	char byte_arr[MAX_IMAGE_BUF_SIZE];
	fread(byte_arr, MAX_IMAGE_BUF_SIZE, 1, fptr_steg_img);

	if(feof(fptr_steg_img) || ferror(fptr_steg_img))
	    break;

	Status get_data_status = get_data_from_byte_array(secret_msg + i, byte_arr);
	if(get_data_status == e_failure)
	{
	    fprintf(stderr, "Data fetch failed while fetching secret data.\n");
	    return e_failure;
	}
	++i;
    }

    //append null terminator
    secret_msg[i] = 0;

    //write decoded data to output file
    FILE *fptr_sec_data_file = decInfo->fptr_secret;
    fwrite(secret_msg, strlen(secret_msg), 1, fptr_sec_data_file);
    if(ferror(fptr_sec_data_file))
    {
	FILE_WRITE_ERR;
	free(secret_msg);
	return e_failure;
    }

    //free memory
    free(secret_msg);
    return e_success;
}

/*
 * Function to get a default filename for the destegged file, depending on the user given name.
 *
 * This function either uses the user given file name for the output file or it gives the output 
 * file a default name that bears DEFAULT_DECODED_FILE_PREFIX, __TIME__ and DEFAULT_DECODED_FILE_SUFFIX 
 * along with the input file extension string.
 *
 * CAUTION: This function dynamically allocates memory for the output character array and must be 
 * deallocated at a later point in time.
 *
 * INPUTS: User suggested output filename pointer.
 *
 * RETURNS: A character pointer to the output file name character array.
 */
char *get_default_destegged_output_filename(const char* user_given_name, const char *file_extn)
{
    char *ofile_name = NULL;

    if(user_given_name)
    {
	ofile_name = malloc(strlen(user_given_name + 1));
	strcpy(ofile_name, user_given_name);
	return ofile_name;
    }

    ofile_name = malloc(strlen(DEFAULT_DECODED_FILE_PREFIX) + strlen(__TIME__) + strlen(DEFAULT_DECODED_FILE_SUFFIX) + strlen(file_extn) + 1);
    strcpy(ofile_name, DEFAULT_DECODED_FILE_PREFIX);
    strcat(ofile_name, __TIME__);
    strcat(ofile_name, DEFAULT_DECODED_FILE_SUFFIX);
    strcat(ofile_name, file_extn);

    return ofile_name;
}

