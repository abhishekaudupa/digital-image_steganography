#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "encode.h"
#include "types.h"
#include "error.h"

/* Function Definitions */

/*
 * Function to sequentially call other functions to conduct the
 * encoding of secret message into the BMP image file.
 *
 * This function calls the following checks and functions to do 
 * the encoding:
 * 1. Checks if the input is NULL.
 *	a. If NULL, prints error message and return failure flag.
 *	b. If not NULL, continues.
 *
 * 2. Opens the source image file, secret data file and a new image
 *    file to encode secret data.
 *	a. If opening any of the above fails, prins error message
 *	   and returns failure flag.
 *	b. Otherwise, continues.
 *
 * 3. Checks for the size of secret message.
 *	a. If the size is 0, prints error message and return failure
 *	   flag.
 *	b. Otherwise, continues.
 *
 * 4. Checks if the source image file can accomodate the secret
 *    message.
 *	a. If it can't, prints error message and returns failure flag.
 *	b. Otherwise, continues.
 *
 * 5. Copies header info from source image to destination image.
 *	a. If copy fails, prints error message and returns failure flag.
 *	b. Otherwise, continues.
 *
 * 6. Encodes magic string in the destination image.
 *	a. If this fails, prints error message and returns failure flag.
 *	b. Otherwise, continues.
 *
 * 7. Encodes secret data file extension in the destination image.
 *	a. If this fails, prints error message and returns failure flag.
 *	b. Otherwise, continues.
 *
 * 8. Encodes secret data file size in the destination image.
 *	a. If this fails, prints error message and returns failure flag.
 *	b. Otherwise, continues.
 *
 * 9. Encodes secret data in the destination image.
 *	a. If this fails, prints error message and returns failure flag.
 *	b. Otherwise, continues.
 *
 * 10. Copies the remaining data from the source image to the
 *     destination image.
 *	a. If this fails, prints error message and returns failure flag.
 *	b. Otherwise, continues.
 *
 * Note that all the above operations are done by other functions, which
 * are called by this function.
 *
 * INPUTS: Pointer to EncodeInfo object.
 *
 * RETURNS: Operation status: e_success or e_failure.
 */
Status do_encoding(EncodeInfo *encInfo)
{
    if(!encInfo)
    {
	FATAL_ERR_MSG;
	return e_failure;
    }

    //Open files.
    if(open_files(encInfo) == e_failure)
    {
	fprintf(stderr, "File error.\n");
	return e_failure;
    }
    printf("Files opened.\n");

    //Check secret data size.
    uint secret_msg_byte_size = get_file_size(encInfo->fptr_secret) + 1;
    if(!secret_msg_byte_size)
    {
	fprintf(stderr, "The data file contains no data to encode. Encoding failed.\n");
	return e_failure;
    }
    printf("Secret message size check complete: %u bytes\n", secret_msg_byte_size);

    //Check the image file can accomodate the secret data.
    uint image_byte_size = get_image_size_for_bmp(encInfo->fptr_src_image);
    uint magic_str_byte_size = strlen(MAGIC_STRING);
    uint file_ext_byte_size = MAX_FILE_SUFFIX;
    uint file_size_str_byte_size = sizeof(long) + 1;
    uint total_encoded_msg_byte_size = secret_msg_byte_size + magic_str_byte_size + file_ext_byte_size + file_size_str_byte_size;
    if(!(image_byte_size >= total_encoded_msg_byte_size * 8))
    {
	fprintf(stderr, "Image file not large enough to hold the encoded data.\n");
	return e_failure;
    }
    printf("File size check complete.\n");

    //Copy header.
    Status header_copy_staus = copy_bmp_header(encInfo->fptr_src_image, encInfo->fptr_stego_image);
    if(header_copy_staus == e_failure)
    {
	fprintf(stderr, "BMP file header copy failed.\n");
	return e_failure;
    }
    printf("Header copied.\n");

    //Encode magic string.
    Status magic_string_encode_status = encode_magic_string(MAGIC_STRING, encInfo);
    if(magic_string_encode_status == e_failure)
    {
	fprintf(stderr, "Magic String encoding failed.\n");
	return e_failure;
    }
    printf("Message encoding started.\n");
    printf("Magic string encoded.\n");

    //Get file extension from secret data filename.
    char file_extn[MAX_FILE_SUFFIX];
    Status file_extension_acquisition_status = get_file_extension(encInfo->secret_fname, file_extn);
    if(file_extension_acquisition_status == e_failure)
    {
	fprintf(stderr, "File extension acquisition failed.\n");
	return e_failure;
    }
    printf("File extension acquired.\n");

    //Encode secret data file extension.
    Status sec_file_extn_encode_status = encode_secret_file_extn(file_extn, encInfo);
    if(sec_file_extn_encode_status == e_failure)
    {
	fprintf(stderr, "Secret file extension encoding failed.\n");
	return e_failure;
    }
    printf("Secret file extension encoded.\n");

    //Encode secret file size.
    Status file_size_encode_status = encode_secret_file_size(secret_msg_byte_size, encInfo);
    if(file_size_encode_status == e_failure)
    {
	fprintf(stderr, "Secret file size encoding failed.\n");
	return e_failure;
    }
    printf("Secret file size encoded.\n");

    //Encode secret data.
    Status secret_data_encode_status = encode_secret_file_data(encInfo);
    if(secret_data_encode_status == e_failure)
    {
	fprintf(stderr, "Secret data encoding failed.\n");
	return e_failure;
    }
    printf("Secret data encoded.\n");

    //Copy remaining data.
    Status cpy_remaining_data_status = copy_remaining_img_data(encInfo->fptr_src_image, encInfo->fptr_stego_image);
    if(cpy_remaining_data_status == e_failure)
    {
	fprintf(stderr, "Remaining data encoding failed.\n");
	return e_failure;
    }
    printf("Remaining data encoded.\n");

    printf("Output file: %s\n", encInfo->stego_image_fname);

    cleanup(encInfo);
    return e_success;
}

/* Get image size
 * Input: Image file ptr
 * Output: width * height * bytes per pixel (3 in our case)
 * Description: In BMP Image, width is stored in offset 18,
 * and height after that. size is 4 bytes
 */
uint get_image_size_for_bmp(FILE *fptr_image)
{
    if(fptr_image)
    {
	uint width, height;

	rewind(fptr_image);
	// Seek to 18th byte
	fseek(fptr_image, 18, SEEK_SET);

	// Read the width (an int)
	fread(&width, sizeof(int), 1, fptr_image);
	//printf("width = %u\n", width);

	// Read the height (an int)
	fread(&height, sizeof(int), 1, fptr_image);
	//printf("height = %u\n", height);

	// Return image capacity
	return width * height * 3;
    }
    else
    {
	FATAL_ERR_MSG;
	return 0;
    }
}

/* 
 * Get File pointers for i/p and o/p files
 * Inputs: Src Image file, Secret file and
 * Stego Image file
 * Output: FILE pointer for above files
 * Return Value: e_success or e_failure, on file errors
 */
Status open_files(EncodeInfo *encInfo)
{
    if(!encInfo)
    {
	FATAL_ERR_MSG;
	return e_failure;
    }

    // Src Image file
    encInfo->fptr_src_image = fopen(encInfo->src_image_fname, "rb");
    // Do Error handling
    if (encInfo->fptr_src_image == NULL)
    {
	perror("fopen");
	fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->src_image_fname);

	return e_failure;
    }

    // Secret file
    encInfo->fptr_secret = fopen(encInfo->secret_fname, "rb");
    // Do Error handling
    if (encInfo->fptr_secret == NULL)
    {
	perror("fopen");
	fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->secret_fname);

	return e_failure;
    }

    // Stego Image file
    encInfo->fptr_stego_image = fopen(encInfo->stego_image_fname, "wb");
    // Do Error handling
    if (encInfo->fptr_stego_image == NULL)
    {
	perror("fopen");
	fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->stego_image_fname);

	return e_failure;
    }

    // No failure return e_success
    return e_success;
}

/*
 * Function to check the type of operation argument input by the user
 * and return the operation type.
 *
 * INPUTS: The argument vector from the main() function.
 *
 * RETURNS: The operation type enum: e_encode, e_decode or e_unsupported.
 */
OperationType check_operation_type(char *argv[])
{
    if(!argv)
    {
	FATAL_ERR_MSG;
	return e_unsupported;
    }

    if(argv[1])
    {
	if(!strcmp(argv[1], ENCODE_ARG))
	    return e_encode;
	if(!strcmp(argv[1], DECODE_ARG))
	    return e_decode;
	return e_unsupported;
    }
    return e_unsupported;
}

/*
 * Function to check the validity of arguments input by the user and
 * return the status.
 *
 * This function check for input files names in the argument list
 * provided by the user that suits the type of operation to be
 * performed on the inputs: encoding/decoding.
 * 
 * Encoding requires an input image file name, name of the file 
 * containing the secret data and an optional argument for the output
 * file name, in that order. Similarly for decoding, except it does
 * not require the file containing secret data.
 *
 * If all inputs are valid, this function initialized the names of the
 * input files in the appropriate fields of the EncodeInfo object that
 * is passed by reference into this function. If no output file name is
 * provided by the user, a default name is given.
 *
 * INPUTS: Argument vector from the main() function and the EncodeInfo
 *         variable pointer.
 *
 * RETURNS: The check status enum: e_success or e_failure.
 */
Status read_and_validate_encode_args(char *argv[], EncodeInfo *encInfo)
{
    if(!argv || !encInfo)
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

    if(!argv[3])
    {
	fprintf(stderr, "Error: Please input a file to be encoded as the third argument:\n%s <%s/%s> <image%s> <secret_msg_file>\n", argv[0], ENCODE_ARG, DECODE_ARG, IMG_FILE_EXTN);
	return e_failure;
    }

    encInfo->src_image_fname = argv[2];
    encInfo->secret_fname = argv[3];
    encInfo->stego_image_fname = get_default_stegged_output_filename(argv[4]);
    if(!encInfo->stego_image_fname)
    {
	FATAL_ERR_MSG;
	return e_failure;
    }
    return e_success;
}

/*
 * Function to get a default filename for the stegged image file, depending on the user given name.
 *
 * This function either uses the user given file name for the output image file or it gives the output 
 * file a default name that bears DEFAULT_DECODED_FILE_PREFIX, __TIME__ and DEFAULT_ENCODED_FILE_SUFFIX 
 * along with the IMG_FILE_EXTN.
 *
 * CAUTION: This function dynamically allocates memory for the output character array and must be 
 * deallocated at a later point in time.
 *
 * INPUTS: User suggested output filename pointer.
 *
 * RETURNS: A character pointer to the output file name character array.
 */
char *get_default_stegged_output_filename(const char* user_given_name)
{
    char *ofile_name = NULL;

    if(user_given_name)
    {
	ofile_name = malloc(strlen(user_given_name + 1));
	strcpy(ofile_name, user_given_name);
	return ofile_name;
    }

    ofile_name = malloc(strlen(DEFAULT_ENCODED_FILE_PREFIX) + strlen(__TIME__) + strlen(IMG_FILE_EXTN) + strlen(DEFAULT_ENCODED_FILE_SUFFIX) + 1);
    strcpy(ofile_name, DEFAULT_ENCODED_FILE_PREFIX);
    strcat(ofile_name, __TIME__);
    strcat(ofile_name, DEFAULT_ENCODED_FILE_SUFFIX);
    strcat(ofile_name, IMG_FILE_EXTN);

    return ofile_name;
}

/*
 * Function to cleanup resources after finishing encoding.
 *
 * This function closes opened files: source image, destination image
 * secret file and frees dynamically allocated memory.
 *
 * INPUTS: The EncodeInfo object.
 *
 * RETURNS: Nothing.
 */
void cleanup(EncodeInfo *encInfo)
{
    if(!encInfo)
    {
	FATAL_ERR_MSG;
	return;
    }

    if(encInfo->stego_image_fname)
	free(encInfo->stego_image_fname);
    fclose(encInfo->fptr_src_image);
    fclose(encInfo->fptr_secret);
    fclose(encInfo->fptr_stego_image);
}

/*
 * Function to return the size of the file pointed by a file pointer
 *
 * This function needs a file pointer refering to a file opened in binary
 * read mode. It reads through each byte and returns the number of bytes
 * read, which is the size of the file in bytes.
 *
 * If null file pointer is input, it throws an error message. This is a
 * fatal situation and indicates a semantic error in the program.
 *
 * INPUTS: File pointer of file whose size is required.
 *
 * RETURNS: The size of the file in bytes.
 */
uint get_file_size(FILE *fptr)
{    
    if(fptr)
    {
	char c;
	rewind(fptr);
	uint size = 0;
	while(1)
	{
	    fread(&c, sizeof(c), 1, fptr);
	    if(ferror(fptr))
	    {
		fprintf(stderr, "File read error.\n");
		return 0;
	    }
	    if(feof(fptr))
		break;
	    ++size;
	}
	//uint size = ftell(fptr);
	//return size? size - 1: 0;
	return size - 1;
    }
    FATAL_ERR_MSG;
    return 0;
}

/*
 * Function to copy the header information of source BMP file to the 
 * destination BMP file.
 *
 * This function simply copies 54 initial bytes (the size of header in
 * .bmp files) in the source file to the destination file. So, obviously
 * this works only for standard BMP image files.
 *
 * INPUTS: Two file pointers: Source and destination .bmp image files.
 * 
 * RETURNS: The status enum for the operation: e_success or e_failure.
 */
Status copy_bmp_header(FILE *fptr_src_image, FILE *fptr_dest_image)
{
    if(!fptr_src_image || !fptr_dest_image)
    {
	FATAL_ERR_MSG;
	return e_failure;
    }

    int bmp_pixel_data_offset = get_image_size_for_bmp(fptr_src_image);
    void *buffer = malloc(bmp_pixel_data_offset/*BMP_HEADER_SIZE*/);
    while(!buffer)
	buffer = malloc(bmp_pixel_data_offset/*BMP_HEADER_SIZE*/);

    rewind(fptr_src_image);
    rewind(fptr_dest_image);

    fread(buffer, bmp_pixel_data_offset/*BMP_HEADER_SIZE*/, 1, fptr_src_image);
    if(ferror(fptr_src_image))
    {
	FILE_READ_ERR;
	free(buffer);
	return e_failure;
    }

    fwrite(buffer, bmp_pixel_data_offset/*BMP_HEADER_SIZE*/, 1, fptr_dest_image);
    if(ferror(fptr_dest_image))
    {
	FILE_READ_ERR;
	free(buffer);
	return e_failure;
    }

    free(buffer);
    return e_success;
}

/*
 * Function to encode a data byte into 8 successive image buffer bytes.
 *
 * This function takes a data byte and encodes each bit to each of the 8 
 * bytes of the image buffer. It starts at the LSB of the data byte and 
 * encodes each bit into the LSB of each successive byte of the image 
 * buffer starting at index 0 of the image buffer. At the end of the 
 * operation, each bit of the data byte from LSB side could be found in 
 * the LSB of each successive byte of the image buffer starting at the 
 * 0th position.
 *
 * If the image buffer is NULL, the function displays an error message and
 * returns the failure flag.
 *
 * INPUTS: The data byte and the image buffer.
 *
 * CAUTION: Ensure the image buffer has 8 bytes in it.
 *
 * RETURNS: Operation status: e_success or e_failure.
 * 
 */
Status encode_byte_to_lsb(char data, char *image_buffer)
{
    if(!image_buffer)
    {
	FATAL_ERR_MSG;
	return e_failure;
    }

    for(int i = 0; i < MAX_IMAGE_BUF_SIZE; ++i)
	image_buffer[i] = ((data >> i) & 1) | ((image_buffer[i] >> 1) << 1);
    return e_success;
}

/*
 * Function to encode a string into a destination BMP image file.
 *
 * This function takes in a string, the source BMP image pointer and the destination 
 * BMP image file pointer. It takes each character from the string, copies (no. of 
 * characters * 8 bytes) of data from the source image file at the position pointed 
 * to by the position indicator of the source file stream. It then encodes each bit 
 * of the character into consecutive bytes of the copied bytes and puts them into the 
 * same location as in the source file into the destination file. It then returns a
 * success flag.
 *
 * If even one of the input data is NULL, it displays error message and returns a 
 * failure flag.
 * 
 * CAUTION: This function simply copies and encodes the data at position pointed to by
 * the file stream position indicators. Ensure they are pointing at the right index
 * before calling this function.
 * 
 * INPUTS: The string to be encoded, the source and destination image file pointers.
 *
 * RETURNS: Operation status: e_success or e_failure.
 */
Status encode_string_to_image(const char *string, FILE *fptr_src_image, FILE *fptr_dest_image)
{
    if(!string || !fptr_src_image || !fptr_dest_image)
    {
	FATAL_ERR_MSG;
	return e_failure;
    }

    for(int i = 0; i < strlen(string); ++i)
    {
	char image_buffer[MAX_IMAGE_BUF_SIZE];
	fread(image_buffer, MAX_IMAGE_BUF_SIZE, 1, fptr_src_image);
	if(ferror(fptr_dest_image))
	{
	    FILE_READ_ERR;
	    return e_failure;
	}

	encode_byte_to_lsb(string[i], image_buffer);

	fwrite(image_buffer, MAX_IMAGE_BUF_SIZE, 1, fptr_dest_image);
	if(ferror(fptr_dest_image))
	{
	    FILE_WRITE_ERR;
	    return e_failure;
	}
    }
    return e_success;
}

/*
 * Function to encode the magic string to the destination BMP image file.
 *
 * This function first sets the position indicator to the BMP_HEADER_SIZEth
 * byte in both the source image file and the stegged image file. From there,
 * it will encode each character of the magic string into 8 consecutive bytes
 * of the source image file starting at positon BMP_HEADER_SIZE. It returns
 * the success flag if this operation was successful otherwise it will stop
 * operation at the first failure and return failure flag.
 *
 * INPUTS: The magic string pointer and pointer to EncodeInfo object.
 *
 * RETURNS: The operation status enum: e_success or e_failure.
 */
Status encode_magic_string(const char *magic_string, EncodeInfo *encInfo)
{
    if(!magic_string || !encInfo)
    {
	FATAL_ERR_MSG;
	return e_failure;
    }

    FILE *fptr_dest_image = encInfo->fptr_stego_image;
    FILE *fptr_src_image = encInfo->fptr_src_image;

    int bmp_pixel_data_offset = get_image_data_offset(fptr_src_image);

    fseek(fptr_src_image, bmp_pixel_data_offset/*BMP_HEADER_SIZE*/, SEEK_SET);
    fseek(fptr_dest_image, bmp_pixel_data_offset/*BMP_HEADER_SIZE*/, SEEK_SET);
    if(ferror(fptr_src_image) || ferror(fptr_dest_image))
    {
	FILE_SEEK_ERR;
	return e_failure;
    }

    return encode_string_to_image(MAGIC_STRING, fptr_src_image, fptr_dest_image);
}

/*
 * Function to encode the file extension of secret message file to the destination 
 * BMP image file.
 *
 * This function first will encode each character of 
 * the file extn and a terminator character '*' into 8 consecutive bytes of the 
 * source image file starting at positon previously set. It returns the success 
 * flag if this operation was successful otherwise it will stop operation at the 
 * first failure and return failure flag.
 *
 * CAUTION: This function assumes that the file position indicators are at the
 * correct position at the time of calling this function and starts reading and 
 * encoding from the indicated position.
 *
 * INPUTS: The file extn  string pointer and pointer to EncodeInfo object.
 *
 * RETURNS: The operation status enum: e_success or e_failure.
 */
Status encode_secret_file_extn(const char *file_extn, EncodeInfo *encInfo)
{
    if(!file_extn || !encInfo)
    {
	FATAL_ERR_MSG;
	return e_failure;
    }

    strcpy(encInfo->extn_secret_file, file_extn);

    FILE *fptr_src_image = encInfo->fptr_src_image;
    FILE *fptr_dest_image = encInfo->fptr_stego_image;

    if(ferror(fptr_dest_image) || ferror(fptr_src_image))
    {
	FILE_SEEK_ERR;
	return e_failure;
    }

    Status encode_file_extn_status = encode_string_to_image(file_extn, fptr_src_image, fptr_dest_image);
    Status encode_file_extn_terminator_status = encode_string_to_image(ENC_DATA_SEPARATOR_STRING, fptr_src_image, fptr_dest_image);

    return (encode_file_extn_terminator_status && encode_file_extn_status);
}

/*
 * Function to convert a given number into a string.
 *
 * This function takes a number and a pointer to a char array. It converts 
 * each digit of the number into a character that represents the same digit, 
 * puts it into the appropriate position in the input char array, appends a 
 * null terminator in the end.
 *
 * INPUTS: The number and the character array pointer.
 * 
 * Caution: Ensure the char array has enough places to store the digits of the number.
 *
 * RETURNS: Nothing
 */
void itoa(long num, char str[])
{
    if(!str || (num < 0))
    {
	FATAL_ERR_MSG;
	return;
    }

    if(!num)
    {
	str[0] = '0';
	str[1] = '\0';
    }

    int i = 0;
    //copy each digit    
    while(num != 0)
    {
	str[i] = (num % 10) + '0';
	num = num / 10;
	++i;
    }
    str[i] = '\0';

    int str_len = strlen(str);
    //reverse str array
    for(i = 0; i < str_len / 2; ++i)
    {
	char temp = str[i];
	str[i] = str[str_len - i - 1];
	str[str_len - i - 1] = temp;
    }
}

/*
 * Function to encode the file size of secret message file to the destination 
 * BMP image file.
 *
 * This function will encode each character of the file size string and a terminator 
 * character '*' into 8 consecutive bytes of the source image file starting at positon 
 * previously set. It returns the success flag if this operation was successful 
 * otherwise it will stop operation at the first failure and return failure flag.
 *
 * CAUTION: This function assumes that the file position indicators are at the
 * correct position at the time of calling this function and starts reading and 
 * encoding from the indicated position.
 *
 * INPUTS: The file extn  string pointer and pointer to EncodeInfo object.
 *
 * RETURNS: The operation status enum: e_success or e_failure.
 */
Status encode_secret_file_size(long file_size, EncodeInfo *encInfo)
{
    if(!encInfo || (file_size <= 0))
    {
	FATAL_ERR_MSG;
	return e_failure;
    }

    encInfo->size_secret_file = file_size;

    FILE *fptr_src_image = encInfo->fptr_src_image;
    FILE *fptr_dest_image = encInfo->fptr_stego_image;

    if(ferror(fptr_dest_image) || ferror(fptr_src_image))
    {
	FILE_SEEK_ERR;
	return e_failure;
    }

    char file_size_as_str[sizeof(file_size) + 1];
    itoa(file_size, file_size_as_str);
    strcat(file_size_as_str, ENC_DATA_SEPARATOR_STRING);
    return encode_string_to_image(file_size_as_str, fptr_src_image, fptr_dest_image);
}

/*
 * Function to encode the secret message to the destination BMP image file.
 *
 * This function first copies the secret message in the secret data file into a 
 * dynamic character array. It then appends the ENC_DATA_SEPARATOR_STRING and the
 * null terminator thereby constructing the secret data string.
 * 
 * It  will then encode each character of the secret message string and a terminator 
 * character '*' into 8 consecutive bytes of the source image file starting at positon 
 * previously set. It returns the success flag if this operation was successful 
 * otherwise it will stop operation at the first failure and return failure flag.
 *
 * CAUTION: This function assumes that the file position indicators are at the
 * correct position at the time of calling this function and starts reading and 
 * encoding from the indicated position.
 *
 * INPUTS: The file extn  string pointer and pointer to EncodeInfo object.
 *
 * RETURNS: The operation status enum: e_success or e_failure.
 */
Status encode_secret_file_data(EncodeInfo *encInfo)
{
    if(!encInfo)
    {
	FATAL_ERR_MSG;
	return e_failure;
    }

    FILE *fptr_src_image = encInfo->fptr_src_image;
    FILE *fptr_dest_image = encInfo->fptr_stego_image;
    FILE *fptr_secret_data = encInfo->fptr_secret;

    char *secret_data = malloc(encInfo->size_secret_file + 2);
    rewind(fptr_secret_data);
    fread(secret_data, encInfo->size_secret_file, 1, fptr_secret_data);
    if(ferror(fptr_secret_data))
    {
	FILE_READ_ERR;
	free(secret_data);
	return e_failure;
    }

    secret_data[encInfo->size_secret_file] = ENC_DATA_SEPARATOR_STRING[0];
    secret_data[encInfo->size_secret_file + 1] = '\0';
    Status secret_data_encode_status = encode_string_to_image(secret_data, fptr_src_image, fptr_dest_image);
    free(secret_data);

    return secret_data_encode_status;
}

/*
 * Function to copy the remaining bytes from source imag BMP file to
 * destination image BMP file after encoding secret data.
 *
 * This function simply copies the bytes from the indicator position of
 * the source image file to the destination file till the end of the file
 * for the source image is reached.
 *
 * CAUTION: This function assumes that the file position indicators are at the
 * correct position at the time of calling this function and starts reading and 
 * encoding from the indicated position.
 *
 * INPUTS: File pointers for source and destination image files.
 *
 * RETURNS: Operation status: e_success or e_failure.
 */
Status copy_remaining_img_data(FILE *fptr_src, FILE *fptr_dest)
{
    if(!fptr_dest || !fptr_src)
    {
	FATAL_ERR_MSG;
	return e_failure;
    }

    while(1)
    {
	char c;
	fread(&c, 1, 1, fptr_src);
	if(ferror(fptr_src))
	{
	    FILE_READ_ERR;
	    return e_failure;
	}
	if(feof(fptr_src))
	    break;

	fwrite(&c, 1, 1, fptr_dest);
	if(ferror(fptr_dest))
	{
	    FILE_READ_ERR;
	    return e_failure;
	}
    }
    return e_success;
}
