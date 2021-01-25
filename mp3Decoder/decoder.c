
#include <string.h>
#include <stdbool.h> 
#include <stdio.h>
#include "decoder.h"
#include  "../helix/pub/mp3dec.h"

#define FRAME_BYTES 5000
#define MAX_DEPTH       5
#define DECODER_NORMAL_MODE 0

#define ErrorGettingNextFrameInfo 0

typedef struct
{
	// Helix structures
	HMP3Decoder   helixDecoder;                                   // Helix MP3 decoder instance 
	MP3FrameInfo  lastFrameInfo;                                  // current MP3 frame info

	// MP3 file
	FILE* mp3File;												  // MP3 file object
	uint32_t      fileSize;                                       // file size
	uint32_t      bytesRem;									      // Encoded MP3 bytes remaining to be processed by either offset or decodeMP3
	bool          fileOpened;                                     // true if there is a loaded file
	uint32_t      lastFrameLength;                                // Last frame length

	// MP3-encoded buffer
	uint8_t       mp3FrameBuffer[FRAME_BYTES];							  // buffer for MP3-encoded frames
	uint32_t      topPosition;                                            // current position in frame buffer (points to top)
	uint32_t      bottomPosition;                                         // current position at info end in frame buffer

	//// ID3 tag
	//bool                  ID3Tag;									// True if the file has valid ID3 tag
	//mp3decoder_tag_data_t ID3Data;                                // Parsed data from ID3 tag


}decoder_t;


/*
* @brief  calculate the number of bytes of the file
* @returns  the number of bytes of the file
*/
static size_t fileSize(void);


static void flushFileToBuffer(void);
static bool openFile(const char* filename);
static void copyFrameInfo(decoder_data_t* mp3Data, MP3FrameInfo* helixData);
static size_t readFile(void* buf, size_t count);
static void ToBuffer(void);
size_t fileSize(void);

static decoder_t decoder;

void MP3DecoderInit(void)
{
	decoder.helixDecoder = MP3InitDecoder();
	decoder.mp3File = NULL;
	decoder.fileOpened = false;
	decoder.bottomPosition = 0;
	decoder.topPosition = 0;
	decoder.fileSize = 0;
	decoder.bytesRem = 0;
	//decoder.hasID3Tag = false;
}



decoder_return_t MP3DecodedFrame(int* outBuffer, uint16_t bufferSize, uint16_t* Decodedsamples, uint8_t depth)
{
    //! someone is innocent until proven guilty. 
    decoder_return_t check = DECODER_WORKED;

    //! MAX_DEPTH to avoid problems with recursivity
    if (depth < MAX_DEPTH)
    {
        if (!decoder.fileOpened)
        {
            check = DECODER_NO_FILE;
        }
        //! checks if there is still a part of the file to be decoded
        else if (decoder.bytesRem) 
        {
            //! checks if the conditions are fine
            if ((decoder.topPosition > 0) && ((decoder.bottomPosition - decoder.topPosition) > 0) && (decoder.bottomPosition - decoder.topPosition < FRAME_BYTES))
            {
                memmove(decoder.mp3FrameBuffer, decoder.mp3FrameBuffer + decoder.topPosition, decoder.bottomPosition - decoder.topPosition);
                decoder.bottomPosition = decoder.bottomPosition - decoder.topPosition;
                decoder.topPosition = 0;
            }

            //! Read data from file
            flushFileToBuffer();

            //! search the mp3 header
            int offset = MP3FindSyncWord(decoder.mp3FrameBuffer + decoder.topPosition, decoder.bottomPosition);

            if (offset >= 0)
            {
                //! check errors in searching for sync words 
                decoder.topPosition += offset;
                decoder.bytesRem -= offset;  //! subtract garbage 
            }
            //! check samples in next frame
            MP3FrameInfo nextFrameInfo;
            //! with this function we store the nextFrameInfo data in our struct
            int err = MP3GetNextFrameInfo(decoder.helixDecoder, &nextFrameInfo, decoder.mp3FrameBuffer + decoder.topPosition);
            if (err == ErrorGettingNextFrameInfo)
            {
                if (nextFrameInfo.outputSamps > bufferSize)
                {
                    return DECODER_OVERFLOW;
                }
            }
            //! we can now decode a frame
            uint8_t* decPointer = decoder.mp3FrameBuffer + decoder.topPosition;
            int bytesLeft = decoder.bottomPosition - decoder.topPosition;
            //! the next funtion autodecrements fileSize with bytes decoded and updated bytesLeft
            int res = MP3Decode(decoder.helixDecoder, &decPointer, &(bytesLeft), outBuffer, DECODER_NORMAL_MODE); 

            //! if everithing worked okey
            if (res == ERR_MP3_NONE)
            {
                //! we calculate the bytes decoded
                uint16_t decodedBytes = decoder.bottomPosition - decoder.topPosition - bytesLeft;
                decoder.lastFrameLength = decodedBytes;

                //! update pointers and the numb of bytes that left to decode.
                decoder.topPosition += decodedBytes;
                decoder.bytesRem -= decodedBytes;

                //! update last frame decoded data
                MP3GetLastFrameInfo(decoder.helixDecoder, &(decoder.lastFrameInfo));

                //! update num of samples decoded
                *Decodedsamples = decoder.lastFrameInfo.outputSamps;

                check = DECODER_WORKED;
            }
            else if (res == ERR_MP3_INDATA_UNDERFLOW || res == ERR_MP3_MAINDATA_UNDERFLOW)
            {
                printf("buffer underflow error");

                //// If there weren't enough bytes on the buffer, try again
                ////return MP3GetDecodedFrameRec(outBuffer, bufferSize, samplesDecoded, depth + 1); //! H-quearlo
            }
            else
            {
                if (decoder.bytesRem <= decoder.lastFrameLength)
                {
                    //if you are here it means that you ended the file
                    return DECODER_END_OF_FILE;
                }
                else
                {
                    ////decoder.topPosition++;
                    ////decoder.bytesRem--;

                    printf("Error");

                    //// If invalid header, try with next frame
                    ////return MP3GetDecodedFrameRec(outBuffer, bufferSize, samplesDecoded, depth + 1); //! H-quearlo
                }
            }
        }
        else
        {
            //! if you are here it means that you ended the file
            check = DECODER_END_OF_FILE;
        }
    }
    else
    {
        //! if you are here it means that you overpass the max number of depth
        check = DECODER_ERROR;
    }
    return check;
}



void flushFileToBuffer()
{
    uint16_t bytesRead;

    //! Fill buffer with info encoded in mp3 file
    uint8_t* dest = decoder.mp3FrameBuffer + decoder.bottomPosition;
    bytesRead = readFile(dest, (FRAME_BYTES - decoder.bottomPosition));
    //! Update bottomPosition
    decoder.bottomPosition += bytesRead;
}



bool MP3GetLastFrameData(decoder_data_t* data)
{
    //! we assume that there are no last frame.
    bool ret = false;
    if (decoder.bytesRem < decoder.fileSize)
    {
        copyFrameInfo(data, &decoder.lastFrameInfo);
        ret = true;
    }
    return ret;
}


void copyFrameInfo(decoder_data_t* mp3Data, MP3FrameInfo* helixData)
{
    mp3Data->bitRate = helixData->bitrate;
    mp3Data->binitsPerSample = helixData->bitsPerSample;
    mp3Data->channelCount = helixData->nChans;
    mp3Data->sampleRate = helixData->samprate;
    mp3Data->sampleCount = helixData->outputSamps;
}




bool MP3LoadFile(const char* filename)
{
    //! he is gulty I don't have any proof but I don´t have any doubts either 
	bool check = false;

    //! if the file was open
	if (decoder.fileOpened)
	{
		//! Close the file 
		fclose(decoder.mp3File);

		//! Reset pointers and variables 
		decoder.fileOpened = false;
		decoder.bottomPosition = 0;
		decoder.topPosition = 0;
		decoder.fileSize = 0;
		decoder.bytesRem = 0;
		//decoder.hasID3Tag = false;
	}

	//! try to open the file an if it can modify the variables inside decoder.
	if (openFile(filename))
	{
		decoder.fileOpened = true;
		decoder.fileSize = fileSize();
		//! because is the first time there are fileSize numbers of bytes remaining for the decoder
		decoder.bytesRem = decoder.fileSize;

        // flush file to buffer
        ToBuffer();

		//readID3Tag();

        //! every worked okey
		check = true;
	}
	return check;
}


//! Fill buffer with the info of the mp3
void ToBuffer()
{
    uint16_t bytesRead;

    uint8_t* dest = decoder.mp3FrameBuffer + decoder.bottomPosition;
    bytesRead = readFile(dest, (FRAME_BYTES - decoder.bottomPosition));
    // Update bottom pointer with the bytes read
    decoder.bottomPosition += bytesRead;
}


size_t readFile(void * buf, size_t count)
{
    if (decoder.fileOpened)
    {
        return fread(buf, 1, count, decoder.mp3File);
    }
    return 0;
}



bool openFile(const char* filename)
{
	decoder.mp3File = fopen(filename, "rb");
	return (decoder.mp3File != NULL);
}


size_t fileSize()
{
	size_t result = 0;
	if (decoder.fileOpened)
	{
		//! move the poiner to the end of the file
		fseek(decoder.mp3File, 0L, SEEK_END);
		//! get the position of the pointer
		result = ftell(decoder.mp3File);
		//! sets the file position to the beginning of the file
		rewind(decoder.mp3File);
	}
	return result;
}





