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

//#define HAYDN
#define ODA_A_LA_ALEGRIA

#define SAMPLE_FORMAT WAV_FORMAT_PCM

#ifdef HAYDN
#define FILEPATH		"C:/Users/Usuario/Documents/ITBA/Haydn_Cello_Concerto_D-1.mp3"
#define FILEPATH_WAV	"C:/Users/Usuario/Documents/ITBA/Haydn_Cello_Concerto_D-1.wav"
#define SAMPLE_RATE		11025
#endif

#ifdef ODA_A_LA_ALEGRIA
#define FILEPATH		"C:/Users/Usuario/Documents/ITBA/Oda-a-la-alegria.mp3"
#define FILEPATH_WAV	"C:/Users/Usuario/Documents/ITBA/Oda-a-la-alegria.wav"
#define SAMPLE_RATE		44100
#endif

////#define NUM_CHANNELS	1

/*******************************************************************************
*		FUNCTION PROTOTYPES FOR PRIVATE FUNCTIONS WITH FILE LEVEL SCOPE
******************************************************************************/
/*****************************************************************************
 *  					VARIABLES WITH LOCAL SCOPE
 *****************************************************************************/

static short buffer[DECODED_BUFFER_SIZE];


/*******************************************************************************
*                       LOCAL FUNCTION DEFINITIONS
******************************************************************************/

int main(void)
{
	printf("  MP3 DECODER  \n");
	//! it count
	uint16_t sampleCount;
	//! if there are 
	uint16_t sampleRate = 0;
	decoder_data_t frameData;
	WavFile* wav_file;
	//mp3decoder_t ID3Data;

	//! initialize the wav part of the program
	wav_file = wav_open(FILEPATH_WAV, "wb");
	wav_set_format(wav_file, SAMPLE_FORMAT);
	wav_set_sample_rate(wav_file, SAMPLE_RATE);
	wav_set_num_channels(wav_file, 1);

	//! we initializate the data to use the decoder.
	MP3DecoderInit();

	//! if we can open the mp3 file
	if (MP3LoadFile(FILEPATH))
	{
		//! variable frames to count the number of frames decoded
		int frames = 0;
		while(true)
		{
			//! with this function we update the sampleCount number
			decoder_return_t check = MP3DecodedFrame(buffer, DECODED_BUFFER_SIZE, &sampleCount, 0);
			if (check == DECODER_WORKED)
			{
				//! if there are a last frame get the data
				MP3GetLastFrameData(&frameData);
				//! we update the number of frames decoded
				frames++;
				////sampleRate = frameData.sampleRate;
				////printf("FRAME SAMPLE RATE: %d \n", sampleRate);

				int16_t auxBuffer[DECODED_BUFFER_SIZE];
				for (uint32_t index = 0; index < (sampleCount / frameData.channelCount); index++)
				{
					auxBuffer[index] = buffer[frameData.channelCount * index];
				}
				wav_write(wav_file, auxBuffer, sampleCount / frameData.channelCount);
			}
			else if (check == DECODER_END_OF_FILE)
			{
				printf("[APP] FILE ENDED. Decoded %d frames.\n", frames - 1);
				wav_close(wav_file);
				/*if (MP3GetTagData(&ID3Data))
				{
					printf("\nSONG INFO\n");
					printf("TITLE: %s\n", ID3Data.title);
					printf("ARTIST: %s\n", ID3Data.artist);
					printf("ALBUM: %s\n", ID3Data.album);
					printf("TRACK NUM: %s\n", ID3Data.trackNum);
					printf("YEAR: %s\n", ID3Data.year);
				}*/
				break;
			}
		}
	}
	else
	{
		//! if you are here the mp3 file couldnt be open
		printf("Couldnt open file\n");
	}

	////	#ifdef SAMPLE
	////	uint16_t readBytes;

	////	// Read and write files
	////	while (!wav_eof(wavIn))
	////	{
	////		static int i = 0;
	////		i++;
	////		readBytes = wav_read(wavIn, readBuffer, BLOCK_SIZE);
	////		if (readBytes == 0)
	////		{
	////			i++;
	////		}
	////		wav_write(wavOut, readBuffer, readBytes);
	////	}
	////	wav_close(wavOut);
	////	wav_close(wavIn);
	////	#endif

	return 0;
}
