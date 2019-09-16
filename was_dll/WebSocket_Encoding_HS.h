#ifndef _WebSocket_Encoding_HS_H
#define _WebSocket_Encoding_HS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define WEBSOCKET_ENCODING_OUTPUT_DATA_LENGTH 200

#ifndef uint8 
#define uint8  unsigned char 
#endif 

#ifndef uint32 
#define uint32 unsigned long int 
#endif 

typedef struct
{
	uint32 total[2];
	uint32 state[5];
	uint8 buffer[64];
}
sha1_context;


#define GET_UINT32(n,b,i)  {(n) = ((uint32)(b)[(i)] << 24) | ((uint32)(b)[(i)+1] << 16) | ((uint32)(b)[(i)+2] << 8) | ((uint32)(b)[(i)+3]);}

#define PUT_UINT32(n,b,i)  {(b)[(i)] = (uint8)((n) >> 24); (b)[(i)+1] = (uint8)((n) >> 16);(b)[(i)+2] = (uint8)((n) >> 8); (b)[(i)+3] = (uint8)((n));}

void sha1_starts(sha1_context * ctx)
{
	ctx->total[0] = 0;
	ctx->total[1] = 0;

	ctx->state[0] = 0x67452301;
	ctx->state[1] = 0xEFCDAB89;
	ctx->state[2] = 0x98BADCFE;
	ctx->state[3] = 0x10325476;
	ctx->state[4] = 0xC3D2E1F0;
}

void sha1_process(sha1_context * ctx, uint8 data[64])
{
	uint32 temp, W[16], A, B, C, D, E;

	GET_UINT32(W[0], data, 0);
	GET_UINT32(W[1], data, 4);
	GET_UINT32(W[2], data, 8);
	GET_UINT32(W[3], data, 12);
	GET_UINT32(W[4], data, 16);
	GET_UINT32(W[5], data, 20);
	GET_UINT32(W[6], data, 24);
	GET_UINT32(W[7], data, 28);
	GET_UINT32(W[8], data, 32);
	GET_UINT32(W[9], data, 36);
	GET_UINT32(W[10], data, 40);
	GET_UINT32(W[11], data, 44);
	GET_UINT32(W[12], data, 48);
	GET_UINT32(W[13], data, 52);
	GET_UINT32(W[14], data, 56);
	GET_UINT32(W[15], data, 60);

#define S(x,n) ((x << n) | ((x & 0xFFFFFFFF) >> (32 - n))) 

#define R(t)  (temp = W[(t - 3) & 0x0F] ^ W[(t - 8) & 0x0F] ^ W[(t - 14) & 0x0F] ^ W[t & 0x0F], (W[t & 0x0F] = S(temp, 1)) )

#define P(a,b,c,d,e,x)  {e += S(a, 5) + F(b, c, d) + K + x; b = S(b, 30);}

	A = ctx->state[0];
	B = ctx->state[1];
	C = ctx->state[2];
	D = ctx->state[3];
	E = ctx->state[4];

#define F(x,y,z) (z ^ (x & (y ^ z))) 
#define K 0x5A827999 

	P(A, B, C, D, E, W[0]);
	P(E, A, B, C, D, W[1]);
	P(D, E, A, B, C, W[2]);
	P(C, D, E, A, B, W[3]);
	P(B, C, D, E, A, W[4]);
	P(A, B, C, D, E, W[5]);
	P(E, A, B, C, D, W[6]);
	P(D, E, A, B, C, W[7]);
	P(C, D, E, A, B, W[8]);
	P(B, C, D, E, A, W[9]);
	P(A, B, C, D, E, W[10]);
	P(E, A, B, C, D, W[11]);
	P(D, E, A, B, C, W[12]);
	P(C, D, E, A, B, W[13]);
	P(B, C, D, E, A, W[14]);
	P(A, B, C, D, E, W[15]);
	P(E, A, B, C, D, R(16));
	P(D, E, A, B, C, R(17));
	P(C, D, E, A, B, R(18));
	P(B, C, D, E, A, R(19));

#undef K 
#undef F 

#define F(x,y,z) (x ^ y ^ z) 
#define K 0x6ED9EBA1 

	P(A, B, C, D, E, R(20));
	P(E, A, B, C, D, R(21));
	P(D, E, A, B, C, R(22));
	P(C, D, E, A, B, R(23));
	P(B, C, D, E, A, R(24));
	P(A, B, C, D, E, R(25));
	P(E, A, B, C, D, R(26));
	P(D, E, A, B, C, R(27));
	P(C, D, E, A, B, R(28));
	P(B, C, D, E, A, R(29));
	P(A, B, C, D, E, R(30));
	P(E, A, B, C, D, R(31));
	P(D, E, A, B, C, R(32));
	P(C, D, E, A, B, R(33));
	P(B, C, D, E, A, R(34));
	P(A, B, C, D, E, R(35));
	P(E, A, B, C, D, R(36));
	P(D, E, A, B, C, R(37));
	P(C, D, E, A, B, R(38));
	P(B, C, D, E, A, R(39));

#undef K 
#undef F 

#define F(x,y,z) ((x & y) | (z & (x | y))) 
#define K 0x8F1BBCDC 

	P(A, B, C, D, E, R(40));
	P(E, A, B, C, D, R(41));
	P(D, E, A, B, C, R(42));
	P(C, D, E, A, B, R(43));
	P(B, C, D, E, A, R(44));
	P(A, B, C, D, E, R(45));
	P(E, A, B, C, D, R(46));
	P(D, E, A, B, C, R(47));
	P(C, D, E, A, B, R(48));
	P(B, C, D, E, A, R(49));
	P(A, B, C, D, E, R(50));
	P(E, A, B, C, D, R(51));
	P(D, E, A, B, C, R(52));
	P(C, D, E, A, B, R(53));
	P(B, C, D, E, A, R(54));
	P(A, B, C, D, E, R(55));
	P(E, A, B, C, D, R(56));
	P(D, E, A, B, C, R(57));
	P(C, D, E, A, B, R(58));
	P(B, C, D, E, A, R(59));

#undef K 
#undef F 

#define F(x,y,z) (x ^ y ^ z) 
#define K 0xCA62C1D6 

	P(A, B, C, D, E, R(60));
	P(E, A, B, C, D, R(61));
	P(D, E, A, B, C, R(62));
	P(C, D, E, A, B, R(63));
	P(B, C, D, E, A, R(64));
	P(A, B, C, D, E, R(65));
	P(E, A, B, C, D, R(66));
	P(D, E, A, B, C, R(67));
	P(C, D, E, A, B, R(68));
	P(B, C, D, E, A, R(69));
	P(A, B, C, D, E, R(70));
	P(E, A, B, C, D, R(71));
	P(D, E, A, B, C, R(72));
	P(C, D, E, A, B, R(73));
	P(B, C, D, E, A, R(74));
	P(A, B, C, D, E, R(75));
	P(E, A, B, C, D, R(76));
	P(D, E, A, B, C, R(77));
	P(C, D, E, A, B, R(78));
	P(B, C, D, E, A, R(79));

#undef K 
#undef F 

	ctx->state[0] += A;
	ctx->state[1] += B;
	ctx->state[2] += C;
	ctx->state[3] += D;
	ctx->state[4] += E;
}

void sha1_update(sha1_context * ctx, uint8 * input, uint32 length)
{
	uint32 left, fill;

	if (!length) return;

	left = ctx->total[0] & 0x3F;
	fill = 64 - left;

	ctx->total[0] += length;
	ctx->total[0] &= 0xFFFFFFFF;

	if (ctx->total[0] < length)
		ctx->total[1]++;

	if (left && length >= fill)
	{
		memcpy((void*)(ctx->buffer + left),
			(void*)input, fill);
		sha1_process(ctx, ctx->buffer);
		length -= fill;
		input += fill;
		left = 0;
	}

	while (length >= 64)
	{
		sha1_process(ctx, input);
		length -= 64;
		input += 64;
	}

	if (length)
	{
		memcpy((void*)(ctx->buffer + left),
			(void*)input, length);
	}
}

static uint8 sha1_padding[64] =
{
 0x80, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

void sha1_finish(sha1_context * ctx, uint8 digest[20])
{
	uint32 last, padn;
	uint32 high, low;
	uint8 msglen[8];

	high = (ctx->total[0] >> 29)
		| (ctx->total[1] << 3);
	low = (ctx->total[0] << 3);

	PUT_UINT32(high, msglen, 0);
	PUT_UINT32(low, msglen, 4);

	last = ctx->total[0] & 0x3F;
	padn = (last < 56) ? (56 - last) : (120 - last);

	sha1_update(ctx, sha1_padding, padn);
	sha1_update(ctx, msglen, 8);

	PUT_UINT32(ctx->state[0], digest, 0);
	PUT_UINT32(ctx->state[1], digest, 4);
	PUT_UINT32(ctx->state[2], digest, 8);
	PUT_UINT32(ctx->state[3], digest, 12);
	PUT_UINT32(ctx->state[4], digest, 16);
}

typedef struct SHA_1_hashValue
{
	char value[41];
	int len;
}sha_1_hashValue;

void sha_1_hexa(char* message, sha_1_hashValue * output_value)
{
	int i = 0, j;
	char output[41] = { 0, };
	sha1_context ctx;
	unsigned char sha1sum[20];

	sha1_starts(&ctx);
	sha1_update(&ctx, (uint8*)message, strlen(message));
	sha1_finish(&ctx, sha1sum);
	for (j = 0; j < 20; j++)
	{
		sprintf_s(output + j * 2, 41, "%02x", sha1sum[j]);
	}

	//printf("출력될 값 : %s\n", output);

	strcpy_s(output_value->value, 41, output);
	output_value->len = strlen(output);

}

void sha_1_bin(const char* message, sha_1_hashValue * output_value)
{
	int i = 0;
	char output[41] = { 0, };
	sha1_context ctx;
	unsigned char sha1sum[20];

	sha1_starts(&ctx);
	sha1_update(&ctx, (uint8*)message, strlen(message));
	sha1_finish(&ctx, sha1sum);


	//printf("출력될 값 : %s\n", output);

	for (i = 0; i < 20; i++)
		output_value->value[i] = sha1sum[i];
	output_value->value[i] = '\0';
	output_value->len = 20;

}

// BASE64
static const char __base64_table[] = {
   'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M',
   'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
   'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm',
   'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z',
   '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '+', '/', '\0'
};

#define BASE64_PAD '='


unsigned char* __base64_encode(const char* message, int length, int* ret_length) {
	const unsigned char* current = (const unsigned char* )message;
	int i = 0;
	unsigned char* result = (unsigned char*)malloc(((length + 3 - length % 3) * 4 / 3 + 1) * sizeof(char));

	while (length > 2) { /* keep going until we have less than 24 bits */
		result[i++] = __base64_table[current[0] >> 2];
		result[i++] = __base64_table[((current[0] & 0x03) << 4) + (current[1] >> 4)];
		result[i++] = __base64_table[((current[1] & 0x0f) << 2) + (current[2] >> 6)];
		result[i++] = __base64_table[current[2] & 0x3f];

		current += 3;
		length -= 3; /* we just handle 3 octets of data */
	}

	/* now deal with the tail end of things */
	if (length != 0) {
		result[i++] = __base64_table[current[0] >> 2];
		if (length > 1) {
			result[i++] = __base64_table[((current[0] & 0x03) << 4) + (current[1] >> 4)];
			result[i++] = __base64_table[(current[1] & 0x0f) << 2];
			result[i++] = BASE64_PAD;
		}
		else {
			result[i++] = __base64_table[(current[0] & 0x03) << 4];
			result[i++] = BASE64_PAD;
			result[i++] = BASE64_PAD;
		}
	}
	if (ret_length) {
		*ret_length = i;
	}
	result[i] = '\0';
	return result;
}

typedef struct WebSocket_Encodeing_Data
{
	unsigned char* result;
	int ret_length;
}Websocket_encoding_data;

char* getWebsocketKey(char* Sec_WebSocket_Key)
{
	int i, j, k;
	char output[100];

	for (i = 0; i < strlen(Sec_WebSocket_Key); i++)
		if (Sec_WebSocket_Key[i] == 'S')
			if (/*Sec_WebSocket_Key[i]=='S' && */Sec_WebSocket_Key[i + 1] == 'e' && Sec_WebSocket_Key[i + 2] == 'c' && Sec_WebSocket_Key[i + 3] == '-')
				if (Sec_WebSocket_Key[i + 4] == 'W' && Sec_WebSocket_Key[i + 5] == 'e' && Sec_WebSocket_Key[i + 6] == 'b' && Sec_WebSocket_Key[i + 7] == 'S' && Sec_WebSocket_Key[i + 8] == 'o' && Sec_WebSocket_Key[i + 9] == 'c' && Sec_WebSocket_Key[i + 10] == 'k' && Sec_WebSocket_Key[i + 11] == 'e' && Sec_WebSocket_Key[i + 12] == 't')
					if (Sec_WebSocket_Key[i + 13] == '-' && Sec_WebSocket_Key[i + 14] == 'K' && Sec_WebSocket_Key[i + 15] == 'e' && Sec_WebSocket_Key[i + 16] == 'y')
					{

						for (j = i + 19, k = 0; Sec_WebSocket_Key[j] != '\r'; j++, k++)
							output[k] = Sec_WebSocket_Key[j];
						output[k] = '\0';
						break;
					}
	return output;
}

int WebSocket_Encoding_For_HandShake(const char* key, char *output_data)
{
	const char id[] = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
	sha_1_hashValue hash;
	int len = 100;
	char* message[200];
	int i = 0, j;
	sha1_context ctx;
	unsigned char sha1sum[20];
	//unsigned char output_data[WEBSOCKET_ENCODING_OUTPUT_DATA_LENGTH];

#ifdef _WebSocket_Encoding_HS_DEBUG
	printf("Key : %s, id : %s\n", key, id);
#endif


	strcpy_s((char*)message, len, key);
	strcat_s((char*)message, len, id);
	message[strlen((const char*)message)] = '\0';


#ifdef _WebSocket_Encoding_HS_DEBUG
	printf("합처진 String : %s %d\n", message, strlen(message));
#endif

	// Calculating sha-1
	{

		sha1_starts(&ctx);
		sha1_update(&ctx, (uint8*)message, strlen((const char*)message));
		sha1_finish(&ctx, sha1sum);


#ifdef _WebSocket_Encoding_HS_DEBUG
		printf("sha-1 : %s\n", sha1sum);
		printf("원본데이터 : (%s)%d, sha-1데이터 : %d\n", message, strcmp(message, "dGhlIHNhbXBsZSBub25jZQ==258EAFA5-E914-47DA-95CA-C5AB0DC85B11"), strcmp((const char*)sha1sum, "B37A4F2CC0624F1690F64606CF385945B2BEC4EA"));
		//printf("sha-1 데이터길이 : %d, %d\n", strlen(output), strlen("357C4600911F2B8D569B9C0EBA7E7B2CC7D7683D"));
#endif
	}

	// Calculating base64

	{
		const unsigned char* current = (const unsigned char*)sha1sum;
		hash.len = 20;
		i = 0;

		while (hash.len > 2) { /* keep going until we have less than 24 bits */
			output_data[i++] = __base64_table[current[0] >> 2];
			output_data[i++] = __base64_table[((current[0] & 0x03) << 4) + (current[1] >> 4)];
			output_data[i++] = __base64_table[((current[1] & 0x0f) << 2) + (current[2] >> 6)];
			output_data[i++] = __base64_table[current[2] & 0x3f];

			current += 3;
			hash.len -= 3; /* we just handle 3 octets of data */
		}

		/* now deal with the tail end of things */
		if (hash.len != 0) {
			output_data[i++] = __base64_table[current[0] >> 2];
			if (hash.len > 1) {
				output_data[i++] = __base64_table[((current[0] & 0x03) << 4) + (current[1] >> 4)];
				output_data[i++] = __base64_table[(current[1] & 0x0f) << 2];
				output_data[i++] = BASE64_PAD;
			}
			else {
				output_data[i++] = __base64_table[(current[0] & 0x03) << 4];
				output_data[i++] = BASE64_PAD;
				output_data[i++] = BASE64_PAD;
			}
		}


		output_data[i] = '\0';
	}

#ifdef _WebSocket_Encoding_HS_DEBUG
	printf("base64 : %s\n", output_data);
	printf("base64데이터 : %d\n", strcmp((char*)output_data, "MzU3QzQ2MDA5MTFGMkI4RDU2OUI5QzBFQkE3RTdCMkNDN0Q3NjgzRA=="));



#endif
	
	return i;
}

#endif
