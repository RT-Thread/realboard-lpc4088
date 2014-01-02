#include <math.h>
#include <stdint.h>
#include <stdlib.h>


static char encoding_table[] = {'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',
                                'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
                                'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X',
                                'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
                                'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
                                'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
                                'w', 'x', 'y', 'z', '0', '1', '2', '3',
                                '4', '5', '6', '7', '8', '9', '+', '/'};
static char decoding_table[256];
static int mod_table[] = {0, 2, 1};


int base64_encode(const char *in_data, char* out_data, int input_length)
{
    int i = 0, j = 0;
    int output_length = 4 * (input_length + 2)/3;

    for (i = 0, j = 0; i < input_length;)
    {

        uint32_t octet_a = i < input_length ? in_data[i++] : 0;
        uint32_t octet_b = i < input_length ? in_data[i++] : 0;
        uint32_t octet_c = i < input_length ? in_data[i++] : 0;

        uint32_t triple = (octet_a << 0x10) + (octet_b << 0x08) + octet_c;

        out_data[j++] = encoding_table[(triple >> 3 * 6) & 0x3F];
        out_data[j++] = encoding_table[(triple >> 2 * 6) & 0x3F];
        out_data[j++] = encoding_table[(triple >> 1 * 6) & 0x3F];
        out_data[j++] = encoding_table[(triple >> 0 * 6) & 0x3F];
    }

    for (i = 0; i < mod_table[input_length % 3]; i++)
        out_data[output_length - 1 - i] = '=';

    return output_length;
}

void build_decoding_table() 
{

    int i;
    for (i = 0; i < 0x40; i++)
        decoding_table[encoding_table[i]] = i;
}

int base64_decode(const char *in_data, char* out_data, int input_length) 
{
    int i, j;
    int output_length = 0;
  
    if (input_length % 4 != 0) return 0;

    build_decoding_table();
    output_length = input_length / 4 * 3;
    if (in_data[input_length - 1] == '=') output_length--;
    if (in_data[input_length - 2] == '=') output_length--;

    for (i = 0, j = 0; i < input_length;) {

        uint32_t sextet_a = in_data[i] == '=' ? 0 & i++ : decoding_table[in_data[i++]];
        uint32_t sextet_b = in_data[i] == '=' ? 0 & i++ : decoding_table[in_data[i++]];
        uint32_t sextet_c = in_data[i] == '=' ? 0 & i++ : decoding_table[in_data[i++]];
        uint32_t sextet_d = in_data[i] == '=' ? 0 & i++ : decoding_table[in_data[i++]];

        uint32_t triple = (sextet_a << 3 * 6)
                        + (sextet_b << 2 * 6)
                        + (sextet_c << 1 * 6)
                        + (sextet_d << 0 * 6);

        if (j < output_length) out_data[j++] = (triple >> 2 * 8) & 0xFF;
        if (j < output_length) out_data[j++] = (triple >> 1 * 8) & 0xFF;
        if (j < output_length) out_data[j++] = (triple >> 0 * 8) & 0xFF;
    }

    return output_length;
}


