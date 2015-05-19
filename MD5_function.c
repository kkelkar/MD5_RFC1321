#include<stdio.h>
#include<stdint.h>
#include<stdlib.h>
#include<string.h>
#include<limits.h>
#include<inttypes.h>

#define WordSize 32
#define TotalOctets 64
#define TotalBits 512

const uint32_t T[TotalOctets]={0xd76aa478, 0xe8c7b756, 0x242070db, 0xc1bdceee,
								0xf57c0faf, 0x4787c62a, 0xa8304613, 0xfd469501,
								0x698098d8, 0x8b44f7af, 0xffff5bb1, 0x895cd7be,
								0x6b901122, 0xfd987193, 0xa679438e, 0x49b40821,
								0xf61e2562, 0xc040b340, 0x265e5a51, 0xe9b6c7aa,
								0xd62f105d, 0x02441453, 0xd8a1e681, 0xe7d3fbc8,
								0x21e1cde6, 0xc33707d6, 0xf4d50d87, 0x455a14ed,
								0xa9e3e905, 0xfcefa3f8, 0x676f02d9, 0x8d2a4c8a,
								0xfffa3942, 0x8771f681, 0x6d9d6122, 0xfde5380c,
								0xa4beea44, 0x4bdecfa9, 0xf6bb4b60, 0xbebfbc70,
								0x289b7ec6, 0xeaa127fa, 0xd4ef3085, 0x04881d05,
								0xd9d4d039, 0xe6db99e5, 0x1fa27cf8, 0xc4ac5665,
								0xf4292244, 0x432aff97, 0xab9423a7, 0xfc93a039,
								0x655b59c3, 0x8f0ccc92, 0xffeff47d, 0x85845dd1,
								0x6fa87e4f, 0xfe2ce6e0, 0xa3014314, 0x4e0811a1,
								0xf7537e82, 0xbd3af235, 0x2ad7d2bb, 0xeb86d391};

uint64_t ceiling(uint64_t x)
{
	uint64_t res=(x%TotalBits==0) ? (x/TotalBits) : (x/TotalBits)+1;
	return(res);
}

uint32_t rotl(uint64_t val, uint32_t pos)
{
	uint32_t res=val%4294967296;
	return((res<<pos) | (res>>(sizeof(uint32_t)*CHAR_BIT-pos)));
}

uint32_t F(uint32_t x, uint32_t y, uint32_t z)
{
	return((x&y) | ((~x)&z));
}

uint32_t G(uint32_t x, uint32_t y, uint32_t z)
{
	return((x&z) | (y&(~z)));
}

uint32_t H(uint32_t x, uint32_t y, uint32_t z)
{
	return(x^y^z);
}

uint32_t I(uint32_t x, uint32_t y, uint32_t z)
{
	return(y ^ (x|(~z)));
}

void compute_digest(uint32_t *d, uint32_t *m)
{
	/*pass 1*/
	int i;
	uint32_t s1[4]={7,12,17,22};
	for(i=0;i<16;i++)
	{
		d[(-i)&3]=d[(1-i)&3]+rotl(d[(-i)&3]+F(d[(1-i)&3],d[(2-i)&3],d[(3-i)&3])+m[i]+T[i],s1[i%4]);
	}
	
	/*pass 2*/
	uint32_t s2[4]={5,9,14,20};
	for(i=0;i<16;i++)
	{
		d[(-i)&3]=d[(1-i)&3]+rotl(d[(-i)&3]+G(d[(1-i)&3],d[(2-i)&3],d[(3-i)&3])+m[((5*i)+1)&15]+T[i+16],s2[i%4]);
	}
	
	/*pass 3*/
	uint32_t s3[4]={4,11,16,23};
	for(i=0;i<16;i++)
	{
		d[(-i)&3]=d[(1-i)&3]+rotl(d[(-i)&3]+H(d[(1-i)&3],d[(2-i)&3],d[(3-i)&3])+m[((3*i)+5)&15]+T[i+32],s3[i%4]);
	}
	
	/*pass 4*/
	uint32_t s4[4]={6,10,15,21};
	for(i=0;i<16;i++)
	{
		d[(-i)&3]=d[(1-i)&3]+rotl(d[(-i)&3]+I(d[(1-i)&3],d[(2-i)&3],d[(3-i)&3])+m[(7*i)&15]+T[i+48],s4[i%4]);
	}
	
}

void main(int argc, char *argv[])
{
	int i;
	uint8_t *message=argv[1];
	uint64_t MsgLength=strlen(message);
	printf("Input string:\n%s\n",message);	

	/*message padding*/
	uint64_t NumBlocks=(MsgLength*sizeof(uint8_t)*CHAR_BIT)+64;
	NumBlocks=(NumBlocks%TotalBits==0) ? ceiling(NumBlocks)+1 : ceiling(NumBlocks);
	uint8_t *OldMsg=(uint8_t *)calloc((TotalOctets*NumBlocks),sizeof(uint8_t));
	strcpy(OldMsg,message);	
	OldMsg[MsgLength]=0x80;
	MsgLength=strlen(message)*sizeof(uint8_t)*CHAR_BIT;
	for(i=0;i<8;i++)
	{
		OldMsg[TotalOctets*NumBlocks-8+i]=(MsgLength>>(i*8)) & 0xff;
	}

	/*converting to 32 bit array*/
	MsgLength=TotalOctets*NumBlocks*CHAR_BIT/WordSize;
	uint32_t *PaddedMsg=(uint32_t *)calloc(MsgLength,sizeof(uint32_t));
	int j;
	for(i=0,j=0;i<MsgLength;i++,j+=4)
	{
		PaddedMsg[i]=(PaddedMsg[i]) | OldMsg[j+3];
		PaddedMsg[i]=(PaddedMsg[i]<<8) | OldMsg[j+2];
		PaddedMsg[i]=(PaddedMsg[i]<<8) | OldMsg[j+1];
		PaddedMsg[i]=(PaddedMsg[i]<<8) | OldMsg[j];
	}
	
	/*initialize variables*/
	uint32_t digest[4]={0x67452301, 0xefcdab89, 0x98badcfe, 0x10325476};
	uint32_t dd[4]={0x00000000, 0x00000000, 0x00000000, 0x00000000};

	/*digest 16 words at a time*/
	for(i=0;i<MsgLength;i+=16)
	{
		/*get previous result*/
		dd[0]=digest[0];
		dd[1]=digest[1];
		dd[2]=digest[2];
		dd[3]=digest[3];
		uint32_t *CurrentBlock=(uint32_t *)calloc(TotalBits/WordSize,sizeof(uint32_t));
		for(j=0;j<16;j++)
		{
			CurrentBlock[j]=PaddedMsg[j+i];
		}
		compute_digest(digest,CurrentBlock);
		
		digest[0]+=dd[0];
		digest[1]+=dd[1];
		digest[2]+=dd[2];
		digest[3]+=dd[3];
	}

	/*reverse word order*/
	digest[0]=ntohl(digest[0]);
	digest[1]=ntohl(digest[1]);
	digest[2]=ntohl(digest[2]);
	digest[3]=ntohl(digest[3]);

	printf("Hash:\n%"PRIx32"%"PRIx32"%"PRIx32"%"PRIx32"\n", digest[0],digest[1],digest[2],digest[3]);
}
