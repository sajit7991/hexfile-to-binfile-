#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdint.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#define PASS 	1
#define FAIL 	-1

/**
 * \fn: getFileSize(const char *filename)
 * \brief: return file size
 * \param: const char * filename
 * \exception nothing.
 * \return: on success file's size is return else 0.
 * \bug None
 */
off_t getFileSize(const char *filename)
{
	struct stat fileinfo;
	int ret = stat(filename,&fileinfo);
	if(-1 == ret )
	{
		return -1;
	}
	return fileinfo.st_size;
}

/**
 * \fn: static uint8_t ascii2Hex(char nibble)
 * \brief: convert ascii char into hex values
 * \param: char nibble
 * \exception nothing.
 * \return: resp hex value of ascii if it's hexdigit
 * \bug None
 */
uint8_t Ascii2hex(uint8_t nibble)
{
	uint8_t hexVal = 0xff;
	if(isxdigit(nibble))
	{
		switch(nibble)
		{
			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
			case '8':
			case '9':
			case '0':
				hexVal = (nibble - '0');break;
			case 'A':
			case 'a':
				hexVal = 0x0A;break;
			case 'b':
			case 'B':
				hexVal = 0x0B;break;
			case 'c':
			case 'C':
				hexVal = 0x0C;break;
			case 'D':
			case 'd':
				hexVal = 0x0D;break;
			case 'e':
			case 'E':
				hexVal = 0x0E;break;
			case 'F':
			case 'f':
				hexVal = 0x0F;break;
		}
	}
	return hexVal;
}


/**
 * \fn: static void ascii2hexDecimal(char *data,char *databyte,size_t len)
 * \brief: convert ascii into hexa values
 * \param: char *data,char *databyte
 * \exception nothing.
 * \return: nothing
 * \bug None
 */
static void ascii2hexDecimal(char *data,char *databyte,size_t len)
{
	uint8_t idx,byteIdx = 0;
	for(idx = 0;idx < len;++idx)
	{
		databyte[idx] = ((Ascii2hex(data[byteIdx]) << 4) | Ascii2hex(data[byteIdx + 1]));
		byteIdx += 2;
	}

}

/**
 * \fn: padding(FILE* fp , uint32_t size)
 * \brief: add padding bytes into file
 * \param: FILE* fp , uint32_t size
 * \exception nothing.
 * \return: nothing
 * \bug None
 */
void padding(FILE* fp , uint32_t size)
{
	char padd = 0xff;
	unsigned int idx;
	if(!fp)
	{
		printf("file descriptor fails");
		return;
	}
#if 0
	unsigned int blockSize;
	do
	{
		blockSize = (size > 8192) ? 8192 : size;
		if( blockSize != fwrite(&padd,1,blockSize,fp))
		{
			perror("fwrite");
		}
		size -= blockSize;

	}while(size);
#else
	printf("padding size %d\n",size);
	for(idx = 0 ; idx < size ;idx++)
	{
		fwrite(&padd,1,1,fp);
	}
#endif
}

/**
 * \fn: convertHex2Bin(const char *fileName)
 * \brief: convet a hex file into bin file
 * \param: const char * fileName
 * \exception nothing.
 * \return: on success PASS else FAIL
 * \bug None
 */
uint32_t convertHex2Bin(const char *file,uint32_t maxAddress)
{

	char filename[64] = {0};
	FILE *hexFileFd = NULL,*binFileFd = NULL;
	char line[256],*data = NULL,dataLen,recordType;
	char databyte[32] = {0};
	uint16_t segmentAddr = 0;
	uint32_t baseAddr = 0x0;
	uint32_t physAddr = 0x0,prvPhysAddr = 0x0,nextPhysAddr = 0x0;
	off_t ret = 0;
	uint32_t paddingSize = 0;
	int byteRead = 0 ;
	time_t tm1, tm2;

	strcpy(filename,file);
	ret = getFileSize(filename);
	if(-1 == ret)
	{
		printf("file not found(%s)",filename);
		return FAIL;
	}

	hexFileFd = fopen(filename,"r");
	if(!hexFileFd)
	{
		printf("%s file opening fail",filename);
		return FAIL;
	}

	char* p = strrchr(filename,'.');

	sprintf(++p,"%s","bin");

	binFileFd = fopen(filename,"w");
	if(!binFileFd)
	{
		printf("%s file opening fail",filename);
		fclose(hexFileFd);
		return FAIL;
	}

	maxAddress = 0xFFFFF;

	time(&tm1);
	//fscanf(hexFileFd,"%[^\n]",line);	/** no need to process 1st line */
	while(EOF != (byteRead = fscanf(hexFileFd," %[^\n]",line)))
	{
		/** sample records :- :10EA40002DBAE511D2F5E5ED4016BFFE6207A59699
		* ':' - start of record
		* 'next 2 byte' - length
		* 'next 4 byte' - address
		* 'next 2 byte' - records type
		* 'next bytes' - data
		* 'last 2 byte' - crc
		* all are in ascii values data need to convert into bin also.
		*/

		/** 		data type
		 * 0 = actual data
		 * 1 = last line in hex
		 * 2 = extended segment address record
		 * 4 = extended linear address record
		 */

		data = &line[9];	/** data present at 9th index of line */
		dataLen = ((Ascii2hex(line[1]) << 4) | Ascii2hex(line[2]));
		recordType = ((Ascii2hex(line[7]) << 4) | Ascii2hex(line[8]));
		segmentAddr = ((Ascii2hex(line[3]) << 12) | (Ascii2hex(line[4]) << 8) | (Ascii2hex(line[5]) << 4) | (Ascii2hex(line[6])));


		/** Before extraction of data check record type and data Length. write only when record type is 0 */
		if((recordType == 0x0) && dataLen)
		{
			physAddr = baseAddr + segmentAddr;
			(physAddr >= maxAddress) ? (physAddr = maxAddress) : (physAddr );
			if((paddingSize = (physAddr - nextPhysAddr)) )	/** if there is any padding data padd with 0xff */
			{
				padding(binFileFd,paddingSize);
			}
			ascii2hexDecimal(data,databyte,dataLen);
			if(dataLen != fwrite(databyte,1,dataLen,binFileFd))
			{
				printf("data fwrite fail\n");
			}
			nextPhysAddr = physAddr + dataLen;
		}
		else if(recordType == 0x01)
		{
			break; /** recordType one means it's a last line */
		}
		else if(recordType == 0x02)
		{
			baseAddr = (((Ascii2hex(line[9]) << 12) | (Ascii2hex(line[10]) << 8) | (Ascii2hex(line[11]) << 4) | (Ascii2hex(line[12]))) << 4);
			physAddr = (baseAddr | segmentAddr);
			(physAddr >= maxAddress) ? (physAddr = maxAddress) : (physAddr );
			if((paddingSize = (physAddr - nextPhysAddr)) )	/** if there is any padding data padd with 0xff */
			{
				padding(binFileFd,paddingSize);
			}
			nextPhysAddr = physAddr;
		}
		else if (recordType == 0x04)
		{
			baseAddr = (((Ascii2hex(line[9]) << 12) | (Ascii2hex(line[10]) << 8) | (Ascii2hex(line[11]) << 4) | (Ascii2hex(line[12]))) << 16);
			physAddr = (baseAddr | segmentAddr);
			(physAddr >= maxAddress) ? (physAddr = maxAddress) : (physAddr );
			if((paddingSize = (physAddr - nextPhysAddr)) )	/** if there is any padding data padd with 0xff */
			{
				padding(binFileFd,paddingSize);
			}
			nextPhysAddr = physAddr;
		}
		else
		{
			memset(line,0,sizeof(line));
			continue;
		}

		prvPhysAddr = physAddr;
		if(physAddr >= maxAddress)
		{
			break;
		}
		memset(line,0,sizeof(line));
	}
	fclose(hexFileFd);
	fclose(binFileFd);
	time(&tm2);
	printf("converting DONE in ----> %f sec",difftime(tm2,tm1));
	return PASS;
}


/**
 * \fn: main(int ,char**)
 * \brief: start point of the program
 * \param: int , char**
 * \exception nothing.
 * \return: on success PASS else FAIL
 * \bug None
 */
int main(int argc , char** argv)
{
	if(argc != 2 ) {
		printf("usage:  %s <hexfile name>\n",argv[0]);
		return 0;
	}
	char* filename = argv[1];
	int ret = convertHex2Bin(filename,0);
	
	char* p = strrchr(filename,'.');
	sprintf(++p,"%s","bin");
	
	(ret == PASS)? printf("%s created!!!\n",filename): printf("failed to created %s !!!\n",filename);
	exit(0);
}