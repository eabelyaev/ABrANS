//Binary Adaptive rANS
//Written by Evgeny Belyaev, ITMO University, 2023
//If you are going to use this code, please refer to
//E. Belyaev, K. Liu, An adaptive binary rANS with probability estimation in reverse order, IEEE Signal Processing Letters, 2023.

#define _CRT_SECURE_NO_WARNINGS
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <memory.h>
#include "time.h"
#include <intrin.h>
#include <stdint.h>

//virtual sliding window settings
#define VSW_LEN  6
#define VSW  ((1<<VSW_LEN)-1)
#define VSW_SHIFT  (VSW_LEN+VSW_LEN-8)
#define VSW_ONE (1<<(VSW_LEN+VSW_LEN))
#define VSW_HALF (1<<(VSW_LEN+VSW_LEN-1))

//rANS settings
#define RANS_BYTE_L (1u << 23)  
#define RANS_PROB_BITS 14  
#define RANS_PROB_SCALE (1<<RANS_PROB_BITS) 
#define RANS_PROB_SCALEMINUSONE ((1<<RANS_PROB_BITS)-1) 
#define RANS_XMAX_SHIFT (23 - RANS_PROB_BITS + 8)
#define RANS_WSHIFT (RANS_PROB_BITS - VSW_LEN-VSW_LEN)

int rANSencode(char *input_file, char *bitsteam_file, int *size);
int rANSencodereciprocal(char *input_file, char *bitsteam_file, int *size);
int rANSdecode(const char *bitsteam_file, char *output_file, int in_size);
int FileCompare(char *fname1, char *fname2);

clock_t encoding_time;
double p1;
double H;

int main(int argc, char* argv[])
{
   char dec_name[512];
   int len=0;
   p1 = atof(argv[2]);
   int inputsize = atoi(argv[3]);

	FILE *fp = fopen(argv[1], "wb");
	int n1 = 0;
	for (int i = 0;i < inputsize;i++)
	{
		double rnd = rand() / (double)RAND_MAX;
		char s;
		if (rnd < p1)
		{
			s = '1';
			n1++;
		}
		else
		{
			s = '0';
		}
			fwrite(&s, 1, 1, fp);
	}
	fclose(fp);
	if ((n1 != 0) && (n1 != inputsize))
	{
		double p = (double)n1 / (double)inputsize;
		H = -p * log2(p) - (1 - p) * log2(1 - p);
	}
	else
	{
		H = 0;
	}

   memset(dec_name,0,512);
   for (int i=0;i<strlen(argv[1]);i++,len++)
   {
	dec_name[i] = argv[1][i];
   }
   dec_name[len++]='_';
   dec_name[len++]='d';
   dec_name[len++]='e';
   dec_name[len++]='c';
   dec_name[len++]='.';
   dec_name[len++]=argv[1][strlen(argv[1])-3];
   dec_name[len++]=argv[1][strlen(argv[1])-2];
   dec_name[len++]=argv[1][strlen(argv[1])-1];

   char filenamestream[128];
   sprintf(filenamestream, "stream.ans");
   rANSencode(argv[1], filenamestream, &inputsize);
   //rANSencodereciprocal(argv[1], filenamestream, &inputsize);
   rANSdecode(filenamestream, dec_name, inputsize);
   printf("differences = %d\n", FileCompare(argv[1], dec_name));
   
   return 0;
}

static uint8_t* read_file(char const* filename, size_t* out_size)
{
	FILE* f = fopen(filename, "rb");
	if (!f)
		printf("file not found: %s\n", filename);

	fseek(f, 0, SEEK_END);
	size_t size = ftell(f);
	fseek(f, 0, SEEK_SET);

	uint8_t* buf = new uint8_t[size];
	if (fread(buf, size, 1, f) != 1)
		printf("read failed\n");

	fclose(f);
	if (out_size)
		*out_size = size;

	return buf;
}

int rANSencode(char *input_file, char *bitsteam_file, int *size)
{
	size_t in_size;
	uint8_t* in_bytes = read_file(input_file, &in_size);
	*size = in_size;
	static size_t out_max_size = in_size;
	uint8_t* out_buf = new uint8_t[out_max_size];
	uint16_t* states = new uint16_t[in_size];

	uint64_t enc_start_time = __rdtsc();
	uint64_t enc_clocks;
	uint16_t state = VSW_HALF;

	for (size_t i = 0; i < in_size; i++)
	{
		states[i] = (state<< RANS_WSHIFT);
		if (in_bytes[i] == '0')
		{
			state -= (state) >> VSW_LEN;
		}
		else
		{
			state += (VSW_ONE - state) >> VSW_LEN;
		}
	}

	uint8_t *rans_begin;
	uint32_t rans = RANS_BYTE_L;
	uint8_t* ptr = out_buf + out_max_size; // *end* of output buffer
	for (size_t i = in_size; i > 0; i--)
	{
		int s = in_bytes[i-1];
		if (s == '0')
		{
			if (rans >= (RANS_PROB_SCALE - states[i-1]) << RANS_XMAX_SHIFT)
			{
				*--ptr = (uint8_t)(rans & 0xff);
				rans >>= 8;
			}
			rans = ((rans / (RANS_PROB_SCALE - states[i-1])) << RANS_PROB_BITS) + (rans%(RANS_PROB_SCALE - states[i-1]));
		}
		else
		{
			if (rans >= (states[i-1]) << RANS_XMAX_SHIFT)
			{
				*--ptr = (uint8_t)(rans & 0xff);
				rans >>= 8;
			}
			rans = ((rans/states[i-1]) << RANS_PROB_BITS) + RANS_PROB_SCALE - states[i-1]+(rans % states[i-1]);
		}
	}
	ptr -= 4;
	ptr[0] = (uint8_t)(rans >> 0);
	ptr[1] = (uint8_t)(rans >> 8);
	ptr[2] = (uint8_t)(rans >> 16);
	ptr[3] = (uint8_t)(rans >> 24);
		
	rans_begin = ptr;
	enc_clocks = __rdtsc() - enc_start_time;
	
	{
		FILE *fp = fopen("rANSencoding.txt", "at");
		double outputsize = (double)(8 * (out_buf + out_max_size - rans_begin));
		fprintf(fp, "%f\t%f\t%f\t%f\t%.4g\n", p1, H, (double)(outputsize) / (double)in_size, (double)(outputsize) / (double)in_size - H, (double)enc_clocks / (double)in_size);
		fclose(fp);
	}

	{
		FILE *fp = fopen(bitsteam_file, "wb");
		int bitstreamsize = out_buf + out_max_size - rans_begin;
		fwrite(rans_begin, sizeof(unsigned char), bitstreamsize, fp);
		fclose(fp);
	}

	delete[] states;
	delete[] out_buf;
	delete[] in_bytes;
	
	return 0;
}

int rANSencodereciprocal(char *input_file, char *bitsteam_file, int *size)
{
	size_t in_size;
	uint8_t* in_bytes = read_file(input_file, &in_size);
	///in_size = *size;
	
	*size = in_size;
	static size_t out_max_size = in_size;
	uint8_t* out_buf = new uint8_t[out_max_size];
	uint16_t* states = new uint16_t[in_size];
	uint32_t* rcp_freq = new uint32_t[RANS_PROB_SCALE + 1];
	uint16_t* rcp_shift = new uint16_t[RANS_PROB_SCALE + 1];

	for (size_t i = 0; i <= RANS_PROB_SCALE; i++)
	{
		if (i > 2)
		{
			uint32_t shift = 0;
			while (i > (1u << shift))
				shift++;
			rcp_freq[i] = (uint32_t)(((1ull << (shift + 31)) + i - 1) / i);
			rcp_shift[i] = shift - 1;
		}
		else
		{
			rcp_freq[i] = 0;
			rcp_shift[i] = 0;
		}
	}

	uint64_t enc_start_time = __rdtsc();
	uint64_t enc_clocks;
	uint16_t state = VSW_HALF;

	for (size_t i = 0; i < in_size; i++)
	{
		states[i] = (state << RANS_WSHIFT);
		if (in_bytes[i] == '0')
		{
			state -= (state) >> VSW_LEN;
		}
		else
		{
			state += (VSW_ONE - state) >> VSW_LEN;
		}
	}
	uint8_t *rans_begin;
	uint32_t rans = RANS_BYTE_L;
	uint8_t* ptr = out_buf + out_max_size; // *end* of output buffer
	for (size_t i = in_size; i > 0; i--)
	{
		int s = in_bytes[i - 1];
		if (s == '0')
		{
			if (rans >= (RANS_PROB_SCALE - states[i-1]) << RANS_XMAX_SHIFT)
			{
				*--ptr = (uint8_t)(rans & 0xff);
				rans >>= 8;
			}

			uint32_t q = (uint32_t)(((uint64_t)rans * rcp_freq[RANS_PROB_SCALE - states[i-1]]) >> 32) >> rcp_shift[RANS_PROB_SCALE - states[i-1]];
			rans = rans + q * (states[i-1]);
		}
		else
		{
			if (rans >= (states[i-1]) << RANS_XMAX_SHIFT)
			{
				*--ptr = (uint8_t)(rans & 0xff);
				rans >>= 8;
			}
			uint32_t q = (uint32_t)(((uint64_t)rans * rcp_freq[states[i-1]]) >> 32) >> rcp_shift[states[i-1]];
			rans = rans + (q+1) * (RANS_PROB_SCALE - states[i-1]);
		}
	}
	ptr -= 4;
	ptr[0] = (uint8_t)(rans >> 0);
	ptr[1] = (uint8_t)(rans >> 8);
	ptr[2] = (uint8_t)(rans >> 16);
	ptr[3] = (uint8_t)(rans >> 24);

	rans_begin = ptr;
	enc_clocks = __rdtsc() - enc_start_time;

	if (1)
	{
		FILE *fp = fopen("rANSencoding.txt", "at");
		double outputsize = (double)(8 * (out_buf + out_max_size - rans_begin));
		fprintf(fp, "%f\t%f\t%f\t%f\t%.4g\n", p1, H, (double)(outputsize) / (double)in_size, (double)(outputsize) / (double)in_size - H, (double)enc_clocks / (double)in_size);
		fclose(fp);
	}
	else
	{
		FILE *fp = fopen("rANSencoding.txt", "at");
		double outputsize = (double)(8 * (out_buf + out_max_size - rans_begin));
		fprintf(fp, "%.4g\n", (double)enc_clocks / (double)in_size);
		fclose(fp);
	}

	{
		FILE *fp = fopen(bitsteam_file, "wb");
		int bitstreamsize = out_buf + out_max_size - rans_begin;
		fwrite(rans_begin, sizeof(unsigned char), bitstreamsize, fp);
		fclose(fp);
	}

	delete[] states;
	delete[] out_buf;
	delete[] in_bytes;
	delete[] rcp_freq;
	delete[] rcp_shift;

	return 0;
}

int rANSdecode(const char *bitsteam_file, char *output_file, int in_size)
{
	FILE *fp = fopen(bitsteam_file, "rb");
	fseek(fp, 0, SEEK_END);
	int BitStreamSize = ftell(fp);
	fclose(fp);

	static const uint32_t prob_bits = 14;
	static const uint32_t prob_scale = 1 << prob_bits;
	uint8_t* dec_bytes = new uint8_t[in_size];
	uint8_t* bitstream_bytes = new uint8_t[in_size];

	memset(dec_bytes, 0xcc, in_size);

	fp = fopen(bitsteam_file, "rb");
	uint64_t dec_start_time = __rdtsc();
	uint32_t rans;
	uint8_t* ptr;
	fread(bitstream_bytes, sizeof(unsigned char), BitStreamSize, fp);
	fclose(fp);
	ptr = bitstream_bytes;

	rans = ptr[0];
	rans |= ptr[1] << 8;
	rans |= ptr[2] << 16;
	rans |= ptr[3] << 24;
	ptr += 4;
	
	uint16_t state = VSW_HALF;

	for (size_t i = 0; i < in_size; i++)
	{
		uint32_t cumcurr = rans & (RANS_PROB_SCALEMINUSONE);
		if (cumcurr < prob_scale - (state<<RANS_WSHIFT))
		{
			dec_bytes[i] = '0';
			rans = (prob_scale - (state << RANS_WSHIFT)) * (rans >> RANS_PROB_BITS) + cumcurr;
			state -= (state) >> VSW_LEN;
		}
		else
		{
			dec_bytes[i] = '1';
			rans = ((state << RANS_WSHIFT)) * (rans >> RANS_PROB_BITS) + cumcurr - prob_scale + (state << RANS_WSHIFT);
			state += (VSW_ONE - state) >> VSW_LEN;
		}
		if (rans < RANS_BYTE_L)
		{
			rans = (rans << 8) | *ptr++;
		}
	}

	uint64_t dec_clocks = __rdtsc() - dec_start_time;
	if (1)
	{
		FILE *fp = fopen("rANSdecoding.txt", "at");
		fprintf(fp, "%f\t%f\t%f\t%f\t%.4g\n", p1, H, (double)(8* BitStreamSize) / (double)in_size, (double)(8* BitStreamSize) / (double)in_size - H, (double)dec_clocks / (double)in_size);
		fclose(fp);
	}
	else
	{
		FILE *fp = fopen("rANSdecoding.txt", "at");
		fprintf(fp, "%.4g\n", (double)dec_clocks / (double)in_size);
		fclose(fp);
	}
	{
		FILE *fp = fopen(output_file, "wb");
		fwrite(dec_bytes, sizeof(unsigned char), in_size, fp);
		fclose(fp);
	}

	delete[] dec_bytes;
	delete[] bitstream_bytes;
	return 0;
}

int FileCompare
(
	char *fname1, 
	char *fname2
)
{
  FILE* fp1, *fp2;
  char symb1, symb2;
  int not_equ = 0;
  int numof_encsymbol1;
  int i;
  double p;

  fp1 = fopen(fname1, "rt");
  fp2 = fopen(fname2, "rt");

  p=0;
  numof_encsymbol1=0;
  
  for( i = 0; (!feof(fp1)) && (!feof(fp2)); i++ )
  {
	fscanf(fp1, "%c", &symb1);
	fscanf(fp2, "%c", &symb2);
	p=p+(symb1-'0');
	numof_encsymbol1++;
	if( symb1 != symb2 )
	{
	  printf( "%d: %c %c\n", i, symb1, symb2 );
	  not_equ++;
	}
  }

  if( i != numof_encsymbol1 )
  {
	printf( "files have different length\n" );
  }

  fclose(fp1);
  fclose(fp2);

  return not_equ;
}


