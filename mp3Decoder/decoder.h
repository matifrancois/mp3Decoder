/***************************************************************************//**
  @file     decoder.h
  @brief    decoder header
  @author   M. Francois
 ******************************************************************************/

/*******************************************************************************
*							INCLUDE HEADER FILES
******************************************************************************/

#ifndef _DECODER_H_
#define _DECODER_H_

#include <stdbool.h> 
#include <stdint.h>
#include <string.h>

#define ID3_MAX_SIZE      50
#define TRACK_MAX_SIZE    10
#define YEAR_MAX_SIZE     10



/*******************************************************************************
*				  CONSTANT AND MACRO DEFINITIONS USING #DEFINE
******************************************************************************/

#define DECODED_BUFFER_SIZE 4608  

/*******************************************************************************
 *					ENUMERATIONS, STRUCTURES AND TYPEDEFS
 ******************************************************************************/

typedef enum
{
	DECODER_WORKED,
	DECODER_ERROR,
	DECODER_NO_FILE,
	DECODER_END_OF_FILE,
	DECODER_OVERFLOW
} decoder_return_t;

typedef struct
{
	uint16_t    bitRate;
	uint8_t     channelCount;
	uint16_t	sampleRate;
	uint16_t    binitsPerSample;
	uint16_t    sampleCount;
} decoder_data_t;

typedef struct
{
	uint8_t title[ID3_MAX_SIZE];
	uint8_t artist[ID3_MAX_SIZE];
	uint8_t album[ID3_MAX_SIZE];
	uint8_t trackNum[TRACK_MAX_SIZE];
	uint8_t year[YEAR_MAX_SIZE];

} decoder_tag_t;

/*******************************************************************************
 *					FUNCTION PROTOTYPES WITH GLOBAL SCOPE
 ******************************************************************************/

/*
* @brief Initialize the values inside the decoder_t decoder struct
*/
void MP3DecoderInit(void);

/**
 * @brief This function open the encoded mp3 file, if it was open 
		  close it reset the values and open again
 * @return true if it can open the mp3 file and false if it can not.
 */
bool MP3LoadFile(const char* filename);

/**
 * @brief If the depth is not bigger than the MAX_DEPTH value 
          and if the file is open and there are bytes to decoder
		  in that case read the mp3 file and find the sync word and
		  want store the next frame info and decode it.
		  if there were no errors decoding, then update the pointers
		  to use in the next decoder instance.
 * @params outBuffer: the buffer were we will store the de decoded data. this has to be short because 
					  with that we insure that it uses 16 bits (if it had been int we can not assure 
					  that because int could be 16 or 32 bits). Anyway it could be uint_16 but with that 
					  we would be ensuring that the data is an unsigned data and we dont know that
 * @params bufferSize: the size of the buffer were we will store the decoded data.
 * @params decodedsamples: The pointer to an uint_16 data to store there the number of samples decoded
 * @return true if it can open the mp3 file and false if it can not.
 */
decoder_return_t MP3DecodedFrame(short* outBuffer, uint16_t bufferSize, uint16_t* Decodedsamples);


/**
 * @brief get the info of the last frame data and store it inside 
		  the pointers data passed as an argument
 * @params data: pointer to a variable where we can store the data of the frame 
 * @return true if the function can get the info of the last frame 
		   and false if it can not (usually because there was no last frame)
 */
bool MP3GetLastFrameData(decoder_data_t* data);


bool MP3GetTagData(decoder_tag_t* data);

/**
 * @brief Prints the size of the file to decode
 */
void printSize(void);

/**
 * @brief create and update the loading bar with the percentaje completed.
 */
void printPercentage(void);

/**
 * @brief create and update the loading bar with 100% percentaje completed that is to end with 100% and not with 99%.
 */
void print100Percentage();


#endif /* _DECODER_H_ */