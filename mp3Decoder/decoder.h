#ifndef _MP3DECODER_H_
#define _MP3DECODER_H_

#include <stdbool.h> 
#include <stdint.h>
#include <string.h>


#define DECODED_BUFFER_SIZE 4608  


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


/*
* @brief Initialize the values inside the decoder_t decoder struct
*/
void MP3DecoderInit(void);

bool MP3LoadFile(const char* filename);

bool MP3GetLastFrameData(decoder_data_t* data);

decoder_return_t MP3DecodedFrame(short* outBuffer, uint16_t bufferSize, uint16_t* Decodedsamples, uint8_t depth);


#endif /* _MP3DECODER_H_ */