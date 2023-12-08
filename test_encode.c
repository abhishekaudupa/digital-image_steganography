#include <stdio.h>
#include "encode.h"
#include "decode.h"
#include "error.h"
#include <string.h>
#include <stdlib.h>

int main(int argc, char **argv)
{
    OperationType opr = check_operation_type(argv);

    switch(opr)
    {
	case e_encode:
	    EncodeInfo encInfo;
	    if(read_and_validate_encode_args(argv, &encInfo) == e_success)
	    {
		Status encode_success = do_encoding(&encInfo);
		if(encode_success == e_failure)
		    fprintf(stderr, "Encoding failed.\n");
		else
		    fprintf(stdout, "Encoding complete.\n");
	    }
	    break;
	case e_decode:
	    DecodeInfo decInfo;
	    if(read_and_validate_decode_args(argv, &decInfo) == e_success)
	    {
		Status decode_success = do_decoding(argv[3], &decInfo);
		if(decode_success == e_failure)
		    fprintf(stderr, "Decoding failed.\n");
		else
		    fprintf(stdout, "Decoding complete.\n");
	    }
	    break;
	default:
	    fprintf(stderr, "Error. Please input the encode/decode argument:\n%s <%s/%s>\n", argv[0], ENCODE_ARG, DECODE_ARG);
	    break;
    }
}
