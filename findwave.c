#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <string.h>
#include <io.h>
#include <fcntl.h>
#include "bladedll.h"
#include "wipewave.h"

FILE			*fd;
BEINITSTREAM	beInitStream;
BEENCODECHUNK	beEncodeChunk;
BEDEINITSTREAM	beDeinitStream;
BECLOSESTREAM	beCloseStream;
BEVERSION		beVersion;
HINSTANCE		hBladeDLL;
BE_CONFIG		beConfig;
DWORD			dwSamples, dwMP3Buffer;
HBE_STREAM		hbeStream;
BE_ERR			err;
char			*pBuffer;
char			*pMP3Buffer;

#define			MAXBUF	7500

int				ExtractWipeWav(char *, char *, long, long, int);
int				my_strncmp(char *, char *, unsigned int);
int				Mp3EncInit();
int				Mp3iT(unsigned long, char *);
unsigned long			WipeVersion(char *);


int	main(int argc, char **argv)
{
	//For debug purposes only
	//if (WipeVersion("j:\\WOF\\FILESYS.WAD") == WIPEFUSIONPALSIZE)
	//	ExtractWipeWav(argv[1], argv[2], 418637824, 0, 1);418637824
	//else
	//	return (-1);
	printf("Lovekenny <noyork2000@yahoo.fr> present you :\n");
	printf("Wipewave Beta 1.1 : Wipeout Fusion Music Wave Files Extractor\n");
	
	if (argc >= 3)
	{
		if (WipeVersion(argv[1]) == WIPEFUSIONPALSIZE)
		{
			printf("\nWipeOut Fusion PAL (NTSC?) is detected.. using apropriate filename.\n");
			if (argc >= 5) 
			{
				if (ExtractWipeWav(argv[1], argv[2], 325261312, atol(argv[4]), 1) != 0)
					printf("Error : Process not succefully done...\n");
			
			}
			else
				if (ExtractWipeWav(argv[1], argv[2], 325261312, 0, 1) != 0)
					printf("Error : Process not succefully done...\n");
		}
		else
		{
			if (argc >= 6)
			{
				if (ExtractWipeWav(argv[1], argv[2], atol(argv[3]), atol(argv[4]), atoi(argv[5])) != 0)
					printf("Error : Process not succefully done...\n");
			}
			else
			{
				printf("Incomplete or No arguments passed in option. Set to default Values\n");
				if (ExtractWipeWav(argv[1], argv[2], 0, 0, 0) != 0)
					printf("Error : Process not succefully done...\n");
			}
		}
	}
	else
		printf("Use : wipewave [filesys.wad] [dest dir] optional: [file position def:0] [Mp3 kbps def:192] [Force WipeOut Fusion:1 def:0]");
	return (0);
}

unsigned long WipeVersion(char *filename)
{
	unsigned long	fSize;
	struct			stat filestat;
	
	memset(&filestat, 0, sizeof(filestat));
	stat(filename, &filestat);
	fSize = (unsigned long) filestat.st_size;
	
	return (fSize);
}

int ExtractWipeWav(char *filename, char *file, long filepos, long kbps, int wipever)
{
	FILE			*fd2;
	//char			*debug2 = "j:\\WOF\\filesys.wad";
	//char			*debug3 = "d:\\tmp\\";
	char			*filewav = 0;
	char			*minbuf;
	char			*backbuf;
	char			*backwav;
	char			nowav = '1';
	char			nowav2 = '0';
	long			pos;
	long			trackpos;
	unsigned long	length;
	int				j;
	int				len;
	int				tmp;
	int				nowavint = 0;

	// Initialisation de l'encoder
	if (Mp3EncInit(kbps) != 0)
	{
		printf("Error : Blade Mp3 Encoder Fail to Initialize...\n");
		return (-1);
	}
	printf("Blade Mp3 Initialized...\n");
	// For debug purposes
	//filename = debug2;
	//file = debug3;
	
	trackpos = 1;
	len = 1;
	fd2 = 0;
	pos = 0;

	// Open the ressource file for wav finding....
	fd = fopen(filename, "rb");
	if (fd == 0)
	{
		printf("Error : Erreur d'ouverture de fichier\n");
		return (-1);
	}
	
	// Allocate memory +  set to correct position...
	minbuf = (char *) malloc((MAXBUF + 1) * sizeof(char));
	backbuf = minbuf;
	backwav = filewav;
	tmp = fseek(fd, filepos, sizeof(char));
	pos = filepos;
	
	// Search for wav file in open file 
	while (len != 0)
	{
		len = fread(minbuf, sizeof(char), MAXBUF, fd);
		if (len <= 0)
			return (-1);
		for (j = 0; j != len; j++)
		{
			if (my_strncmp("RIFF", minbuf, 4) == 0)
			{
				if (my_strncmp("WAVE", minbuf + 8, 4) == 0)
				{
					if (wipever == 0)
					{
						filewav = (char *) malloc((strlen(file) + 12) * sizeof(char));
						filewav = strcpy(filewav, file);
						filewav = strcat(filewav, "Track");
						*(filewav + strlen(file) + 5) = nowav2;
						*(filewav + strlen(file) + 6) = nowav;
						*(filewav + strlen(file) + 7) = 0;
					}
					else
					{
						filewav = (char *) malloc((strlen(file) + strlen(wipefusion[nowavint]) + 5) * sizeof(char));
						filewav = strcpy(filewav, file);
						filewav = strcat(filewav, wipefusion[nowavint++]);
					}
					filewav = strcat(filewav, ".mp3");
					while (strncmp("data", minbuf, 4) != 0)
					{
						j++;
						minbuf++;
					}
					tmp = fseek(fd, -(len - (j + 4)), sizeof(char));
					tmp = fread(&length, sizeof(DWORD), 1, fd);
					printf("\nTrack %c%c find at position %d bytes...\n", nowav2, nowav, pos);
					printf("Ready to grab it to %s....\n", filewav);
					if (Mp3iT(length, filewav) != 0)
					{
						printf("Error: Mp3 processing Fail...\n");
						return(-1);
					}
					
					if (nowav == '9')
					{
						nowav = ('0' - 1);
						nowav2++;
					}
					nowav++;
					pos = pos + ((len - (j + 8)) + length);
					break;
				}
			}
			minbuf++;
			pos++;
		}
		minbuf = backbuf;
	}
	printf("Fin du fichier...\n");
	fclose(fd);
	//free(minbuf);
	//free(filewav);
	beCloseStream(hbeStream);
	FreeLibrary(hBladeDLL);
	return(0);
}

int		Mp3EncInit(int kbps)
{
	// Load BladeEnc.DLL
	hBladeDLL = LoadLibrary("BLADEENC.DLL");

	if(!hBladeDLL)
	{
		printf("Error loading BLADEENC.DLL");
		return (-1);
	}

	// Get Interface
	beInitStream	= (BEINITSTREAM) GetProcAddress(hBladeDLL, TEXT_BEINITSTREAM);
	beEncodeChunk	= (BEENCODECHUNK) GetProcAddress(hBladeDLL, TEXT_BEENCODECHUNK);
	beDeinitStream	= (BEDEINITSTREAM) GetProcAddress(hBladeDLL, TEXT_BEDEINITSTREAM);
	beCloseStream	= (BECLOSESTREAM) GetProcAddress(hBladeDLL, TEXT_BECLOSESTREAM);

	

	if(!beInitStream || !beEncodeChunk || !beDeinitStream || !beCloseStream)
	{
		printf("Unable to get BladeEnc interface");
		return -1;
	}

	beConfig.dwConfig = BE_CONFIG_MP3;

	if (kbps == 0 || kbps != 128 || kbps != 160 || kbps != 192 || kbps != 256 || kbps != 320)
		beConfig.format.mp3.wBitrate = 192;
	else
		beConfig.format.mp3.wBitrate = kbps;

	beConfig.format.mp3.dwSampleRate	= 48000;
	beConfig.format.mp3.byMode			= BE_MP3_MODE_STEREO;
	beConfig.format.mp3.bCopyright		= FALSE;
	beConfig.format.mp3.bCRC			= FALSE;
	beConfig.format.mp3.bOriginal		= FALSE;
	beConfig.format.mp3.bPrivate		= FALSE;

	err = beInitStream(&beConfig, &dwSamples, &dwMP3Buffer, &hbeStream);

	if (err != BE_ERR_SUCCESSFUL)
	{
		printf("Error opening encoding stream (%lu)", err);
		return (-1);
	}

	// Allocate buffers
	pBuffer = (char *) malloc(dwSamples * sizeof(char));
	pMP3Buffer = (char *) malloc(dwMP3Buffer * sizeof(char));
	
	return (0);
}

int	Mp3iT(unsigned long length, char *filename)
{	
	int				hOut;
	int				test;
	int				i;
	unsigned long	done;
	long			toread;
	long			towrite;	
	
	test = 0;

	if(!pMP3Buffer || !pBuffer)
	{
		printf("Out of memory");
		return -1;
	}

	// Open MP3 file | O_NOINHERIT
	
	hOut = open(filename, O_WRONLY | O_BINARY | O_TRUNC | O_CREAT, S_IWRITE);
	if (hOut == -1)
	{
		printf("Error Opening Destination File\n");
		return (-1);
	}
	//printf("fopen ok !!! %d %s\n", hOut, filename);
	// Start encoding
	done = 0;
	
	while (done < length)
	{
		if(done + dwSamples * 2 < length)	
			toread = dwSamples * 2;
		else
			toread = length - done;
		
		if(fread(pBuffer, sizeof(char), toread, fd) == 0)
		{
			printf("\nRead error");
			return -1;
		}		 
		//printf("fread ok !!!\n");
		err = beEncodeChunk(hbeStream, toread/2, (short *)pBuffer, pMP3Buffer, &towrite);

		if(err != BE_ERR_SUCCESSFUL)
		{
			beCloseStream(hbeStream);
			printf("beEncodeChunk() failed (%lu)", err);
			return -1;
		}
		//printf("decode ok !!!\n");
		
		if(write(hOut, pMP3Buffer, towrite) == -1)
		{
			printf("\nWrite error");
			return -1;
		}
		//printf("fwrite ok !!!\n");
		done += toread;	
		for (i = 0; i != test; i++)
			printf("\b");
		test = printf("%dKB", (done / 1000));
	}

	close(hOut);
	return 0;
}


