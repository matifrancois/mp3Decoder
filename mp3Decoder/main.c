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
#include "../wav/wav.h"
#include "../mp3Decoder/decoder.h"


 /*******************************************************************************
 *					CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/


#define SAMPLE_FORMAT WAV_FORMAT_PCM
 
//here uncomment the others posibilities and select the right song

//#define HAYDN
//#define ODA_A_LA_ALEGRIA
#define QUEEN
//#define YOUR_SONG


// Here put the path of the song that you want to decode

//#ifdef YOUR_SONG
//#define FILEPATH		"C:/The/filepath/of/your/song.mp3"
//#define FILEPATH_WAV	"C:/The/filepath/of/your/decoded/song.wav"
//#endif

#ifdef HAYDN
#define FILEPATH		"C:/Users/Usuario/Documents/ITBA/Haydn_Cello_Concerto_D-1.mp3"
#define FILEPATH_WAV	"C:/Users/Usuario/Documents/ITBA/Haydn_Cello_Concerto_D-1.wav"
#endif

#ifdef ODA_A_LA_ALEGRIA
#define FILEPATH		"C:/Users/Usuario/Documents/ITBA/Oda-a-la-alegria.mp3"
#define FILEPATH_WAV	"C:/Users/Usuario/Documents/ITBA/Oda-a-la-alegria.wav"
#endif

#ifdef QUEEN
#define FILEPATH		"C:/Users/Usuario/Documents/ITBA/Music/i_want_to_break_free.mp3"
#define FILEPATH_WAV	"C:/Users/Usuario/Documents/ITBA/Music/i_want_to_break_free.wav"
#endif


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
	decoder_tag_t ID3Data;

	//! initialize the wav part of the program
	wav_file = wav_open(FILEPATH_WAV, "wb");
	wav_set_format(wav_file, SAMPLE_FORMAT);
	wav_set_num_channels(wav_file, 1);

	//! we initializate the data to use the decoder.
	MP3DecoderInit();

	//! if we can open the mp3 file
	if (MP3LoadFile(FILEPATH))
	{
		//! we show the info of the song
		if (MP3GetTagData(&ID3Data))
		{
			printf("\n  SONG INFO\n");
			printf("TITLE: %s\n", ID3Data.title);
			printf("ARTIST: %s\n", ID3Data.artist);
			printf("ALBUM: %s\n", ID3Data.album);
			printf("TRACK NUMBER: %s\n", ID3Data.trackNum);
			printf("YEAR: %s\n \n", ID3Data.year);
		}
		//! variable frames to count the number of frames decoded
		int frames = 0;
		int reference = 0;
		//! we just print the size of the file
		printSize();
		//! ref is to go inside an if inside the while just one time.
		bool ref = false;
		while(true)
		{
			//! with this function we update the sampleCount number
			decoder_return_t check = MP3DecodedFrame(buffer, DECODED_BUFFER_SIZE, &sampleCount);
			if (check == DECODER_WORKED)
			{
				//! if there are a last frame get the data
				MP3GetLastFrameData(&frameData);
				//! we update the number of frames decoded
				
				if (ref == false) {
					//! we inform the sample count
					printf("sample count: %d \n\n", sampleCount);
					//! configutates the wav_file to the right sampleRate according to the information inside the mp3 file.
					//! we do it statement here because we need to know the sample rate and we know that after using the MP3GetLastFrameData function
					wav_set_sample_rate(wav_file, frameData.sampleRate);
					//! set ref in true to not go back here never ever, you are banished from the kingdom of this if
					ref = true;
				}

				//! create and update the loading bar with the percentaje completed.
				printPercentage();

				//! update the counter to share the number of frames decoded at the end of the program
				frames++;

				int16_t auxBuffer[DECODED_BUFFER_SIZE];
				for (uint32_t index = 0; index < (sampleCount / frameData.channelCount); index++)
				{
					auxBuffer[index] = buffer[frameData.channelCount * index];
				}
				wav_write(wav_file, auxBuffer, sampleCount / frameData.channelCount);
			}
			else if (check == DECODER_END_OF_FILE)
			{
				print100Percentage();
				printf("\n\nFILE ENDED. Decoded %d frames.\n", frames - 1);
				wav_close(wav_file);
				break;
			}
		}
	}
	else
	{
		//! if you are here the mp3 file couldnt be open
		printf("Couldnt open file\n");
	}
	return 0;
}
