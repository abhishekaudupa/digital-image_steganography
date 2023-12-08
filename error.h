#ifndef ERROR_H
#define ERROR_H

/* Fatal error message indicating a serious bug in the program. */
#define FATAL_ERR_MSG fprintf(stderr, "In the file %s and function %s: Something went wrong!!\n", __FILE__, __func__)

/* Error message triggered by fopen */
#define FILE_OPEN_ERR fprintf(stderr, "%s: %s: File open error.\n", __FILE__, __func__)

/* Error message triggered by ferror(fptr) after file read operation */
#define FILE_READ_ERR fprintf(stderr, "%s: %s: File read error.\n", __FILE__, __func__)

/* Error message triggered by ferror(fptr) after file write operation */
#define FILE_WRITE_ERR fprintf(stderr, "%s: %s: File write error.\n", __FILE__, __func__)

/* Error message triggered by ferror(fptr) after file seek operation */
#define FILE_SEEK_ERR fprintf(stderr, "%s: %s: File seek error.\n", __FILE__, __func__)

#endif
