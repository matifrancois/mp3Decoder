/***************************************************************************//**
  @file     main.c
  @brief    main file of the mp3 decoder
  @author   M. Francois
 ******************************************************************************/

 /*******************************************************************************
  *							INCLUDE HEADER FILES
  ******************************************************************************/

#include <stdint.h>
#include <stdio.h>
#include "../wav.h"
#include "../mp3Decoder/decoder.h"


 /*******************************************************************************
 *					CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/

#define HAYDN

 // #define FLOATBUF
#define INTBUF

#ifdef FLOATBUF
#define SAMPLE_FORMAT WAV_FORMAT_IEEE_FLOAT
#endif
#ifdef INTBUF
#define SAMPLE_FORMAT WAV_FORMAT_PCM
#endif

#ifdef HAYDN
#define FILEPATH		"C:/Users/Usuario/Documents/ITBA/Haydn_Cello_Concerto_D-1.mp3"
#define FILEPATH_WAV	"C:/Users/Usuario/Documents/ITBA/Haydn_Cello_Concerto_D-1.wav"
#define SAMPLE_RATE		11025
#endif

#define NUM_CHANNELS	1

/*******************************************************************************
*		FUNCTION PROTOTYPES FOR PRIVATE FUNCTIONS WITH FILE LEVEL SCOPE
******************************************************************************/
/*****************************************************************************
 *  					VARIABLES WITH LOCAL SCOPE
 *****************************************************************************/

static short buffer[MP3_DECODED_BUFFER_SIZE];


/*******************************************************************************
*                       LOCAL FUNCTION DEFINITIONS
******************************************************************************/

int main(void)
{
	// Title
	printf("  MP3 DECODER  \n");

	uint16_t sampleCount;
	uint32_t sr = 0;
	uint8_t j = 0;
	WavFile * wav_file;
	mp3decoder_frame_data_t frameData;
	mp3decoder_tag_data_t ID3Data;

	// initialize the wav part of the program
	#ifndef SAMPLE
	wav_file = wav_open(FILEPATH_WAV, "wb");
	wav_set_format(wav_file, SAMPLE_FORMAT);
	wav_set_sample_rate(wav_file, SAMPLE_RATE);
	wav_set_num_channels(wav_file, 1);
	#endif // SAMPLE

	//	#ifdef SAMPLE
	//	// Open read and write file
	//	wavIn = wav_open(FILEPATH_SRC, "rb");
	//	WavU16 format = wav_get_format(wavIn);
	//	wavOut = wav_open(FILEPATH_WAV, "wb");
	//	wav_set_format(wavOut, SAMPLE_FORMAT);
	//	#endif

	#ifndef SAMPLE
	MP3DecoderInit();

	if (MP3LoadFile(FILEPATH))
	{

		int i = 0;
		while (1)
		{
			#ifdef MAIN_DEBUG
			printf("\n[APP] Frame %d decoding started.\n", i);
			#endif
			mp3decoder_result_t res = MP3GetDecodedFrame(buffer, MP3_DECODED_BUFFER_SIZE, &sampleCount);
			if (res == 0)
			{
				MP3GetLastFrameData(&frameData);
				if (sr != frameData.sampleRate)
				{
					int huevo = 0;
					huevo++;
				}
				#ifdef MAIN_DEBUG
				printf("[APP] Frame %d decoded.\n", i);
				#endif
				i++;
				if (i == 4)
				{
					i++;
					i--;
				}
				sr = frameData.sampleRate;
				#ifdef MAIN_DEBUG
				printf("[APP] FRAME SAMPLE RATE: %d \n", sr);
				#endif

				int16_t auxBuffer[MP3_DECODED_BUFFER_SIZE];
				for (uint32_t j = 0; j < sampleCount / frameData.channelCount; j++)
				{
					auxBuffer[j] = buffer[frameData.channelCount * j];
				}
				wav_write(wav_file, auxBuffer, sampleCount / frameData.channelCount);
			}
			else if (res == MP3DECODER_FILE_END)
			{
				printf("[APP] FILE ENDED. Decoded %d frames.\n", i - 1);
				wav_close(wav_file);
				if (MP3GetTagData(&ID3Data))
				{
					printf("\nSONG INFO\n");
					printf("TITLE: %s\n", ID3Data.title);
					printf("ARTIST: %s\n", ID3Data.artist);
					printf("ALBUM: %s\n", ID3Data.album);
					printf("TRACK NUM: %s\n", ID3Data.trackNum);
					printf("YEAR: %s\n", ID3Data.year);
				}
				break;
			}
			else
			{
				int huevo = 0;
				huevo++;
			}
		}
	}
	else
	{
		printf("Couldnt open file\n");
	}
	#endif

	//	#ifdef SAMPLE
	//	uint16_t readBytes;

	//	// Read and write files
	//	while (!wav_eof(wavIn))
	//	{
	//		static int i = 0;
	//		i++;
	//		readBytes = wav_read(wavIn, readBuffer, BLOCK_SIZE);
	//		if (readBytes == 0)
	//		{
	//			i++;
	//		}
	//		wav_write(wavOut, readBuffer, readBytes);
	//	}
	//	wav_close(wavOut);
	//	wav_close(wavIn);
	//	#endif

	return 0;
}
