/*******************************************************************************
  @file     decoder.c
  @brief    file with the decoder function to use the library helix
  @author   M. Francois
 ******************************************************************************/

 /*******************************************************************************
  *							INCLUDE HEADER FILES
  ******************************************************************************/

#include <string.h>
#include <stdbool.h> 
#include <stdio.h>
#include "decoder.h"
#include  "../helix/pub/mp3dec.h"
#include "../id3/read_id3.h"

#include "loading.h"

 /*******************************************************************************
 *					CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/

#define FRAME_BYTES 5000
#define MAX_DEPTH       5
#define DECODER_NORMAL_MODE 0

#define ErrorGettingNextFrameInfo 0

#define DEFAULT_ID3 "Unknown"

 /*******************************************************************************
 *                 ENUMERATIONS, STRUCTURES AND TYPEDEFS
 ******************************************************************************/

typedef struct
{
	//! Helix data
	HMP3Decoder   helixDecoder;                                   // Helix MP3 decoder instance 
	MP3FrameInfo  lastFrameInfo;                                  // MP3 frame info

	//! MP3 file data
	FILE* mp3File;												  // MP3 file object
	uint32_t      fileSize;                                       // Size of the file used
	uint32_t      bytesRem;									      // Encoded MP3 bytes remaining to be decoded
	bool          fileOpened;                                     // true if there is an open file, false if is not
	uint32_t      lastFrameLength;                                // Last frame length

	//! MP3-encoded buffer data
	uint8_t       mp3FrameBuffer[FRAME_BYTES];					  // buffer for MP3-encoded frames
	uint32_t      topPosition;                                    // current position in frame buffer (points to top)
	uint32_t      bottomPosition;                                 // current position at info end in frame buffer

	//! ID3 tag data
	bool                  hasID3;								  // True if the file has valid ID3 tag
    decoder_tag_t         ID3Data;                                // Parsed data from ID3 tag

}decoder_t;

/*******************************************************************************
 *      FUNCTION PROTOTYPES FOR PRIVATE FUNCTIONS WITH FILE LEVEL SCOPE
 ******************************************************************************/

 /*
 * @brief  Open the file to read the information inside
 * @returns  the number of bytes of the file
 */
static bool openFile(const char* filename);
/*
* @brief  calculate the number of bytes of the file
* @returns  the number of bytes of the file
*/
static size_t fileSize(void);

/*
* @brief  This function fills buffer with info encoded in mp3 file and update the pointers
*/


/**
 * @brief store the data of the helixData in the mp3Data structure.
 * @params mp3Data: Here we will store the data of the helixData structure passed as a second parameter
 * @params helixData: Here are the information to be copy
 */
static void copyFrameInfo(decoder_data_t* mp3Data, MP3FrameInfo* helixData);

/**
 * @brief check if the file is open and read as much data as passed in count parameter
 * @params buf: here we store the data 
 * @params count: the number of data to be readed
 */
static size_t readFile(void* buf, size_t count);

/*
* @brief  This function fills buffer with info encoded in mp3 file and update the pointers
*/
static void ToBuffer(void);

/*
* @brief  This function reads the ID3 Tag of the file if it has and move the read position according to the size of the ID3Tag to avoid read it twice
*/
void readID3Tag(void);

/*****************************************************************************
 *  					VARIABLES WITH LOCAL SCOPE
 *****************************************************************************/

static decoder_t decoder;

/*******************************************************************************
                        GLOBAL FUNCTION DEFINITIONS
 ******************************************************************************/

void MP3DecoderInit(void)
{
    decoder.helixDecoder = MP3InitDecoder();
    decoder.mp3File = NULL;
    decoder.fileOpened = false;
    decoder.bottomPosition = 0;
    decoder.topPosition = 0;
    decoder.fileSize = 0;
    decoder.bytesRem = 0;
    decoder.hasID3 = false;
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
        decoder.hasID3 = false;
    }

    //! try to open the file an if it can modify the variables inside decoder.
    if (openFile(filename))
    {
        decoder.fileOpened = true;
        decoder.fileSize = fileSize();
        //! because is the first time there are fileSize numbers of bytes remaining for the decoder
        decoder.bytesRem = decoder.fileSize;

        //! We read the data stored in the tag of the mp3 file
        readID3Tag();

        //! flush file to buffer
        ToBuffer();


        //! every worked okey
        check = true;
    }
    return check;
}

decoder_return_t MP3DecodedFrame(short* outBuffer, uint16_t bufferSize, uint16_t* Decodedsamples)
{
    //! someone is innocent until proven guilty. 
    decoder_return_t check = DECODER_WORKED;

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
        ToBuffer();
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
            if (decoder.bytesRem == 0)
            {
                printf("buffer underflow error");
                return DECODER_END_OF_FILE;
            }
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
                printf("Error");
            }
        }
    }
    else
    {
        //! if you are here it means that you ended the file
        check = DECODER_END_OF_FILE;
    }
    return check;
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

bool MP3GetTagData(decoder_tag_t* data)
{
    bool ret = false;
    if (decoder.hasID3)
    {
        strcpy(data->album, decoder.ID3Data.album);
        strcpy(data->artist, decoder.ID3Data.artist);
        strcpy(data->title, decoder.ID3Data.title);
        strcpy(data->trackNum, decoder.ID3Data.trackNum);
        strcpy(data->year, decoder.ID3Data.year);
        ret = true;
    }

    return ret;
}

void printSize(void) 
{
    printf("size: %d \n", decoder.fileSize);
}

void printPercentage(void) 
{
    loadingBar(100.0 - ((float)decoder.bytesRem / (float)decoder.fileSize) * 100.0);
}

void print100Percentage(void)
{
    loadingBar(100);
}

/*******************************************************************************
                        LOCAL FUNCTION DEFINITIONS
 ******************************************************************************/

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

void ToBuffer(void)
{
    //! Fill buffer with the info of the mp3
    uint16_t bytesRead;

    uint8_t* dest = decoder.mp3FrameBuffer + decoder.bottomPosition;
    bytesRead = readFile(dest, (FRAME_BYTES - decoder.bottomPosition));
    // Update bottom pointer with the bytes read
    decoder.bottomPosition += bytesRead;
}


size_t readFile(void* buf, size_t count)
{
    if (decoder.fileOpened)
    {
        return fread(buf, 1, count, decoder.mp3File);
    }
    return 0;
}

void copyFrameInfo(decoder_data_t* mp3Data, MP3FrameInfo* helixData)
{
    mp3Data->bitRate = helixData->bitrate;
    mp3Data->binitsPerSample = helixData->bitsPerSample;
    mp3Data->channelCount = helixData->nChans;
    mp3Data->sampleRate = helixData->samprate;
    mp3Data->sampleCount = helixData->outputSamps;
}

void readID3Tag(void)
{
    //! checks if the file has ID3 Tag inside with the ID3 library
    if (has_ID3_tag(decoder.mp3File))
    {
        //! if you are here it means that the file has ID3
        decoder.hasID3 = true;

        if (!read_ID3_info(TITLE_ID3, decoder.ID3Data.title, ID3_MAX_SIZE, decoder.mp3File))
            strcpy(decoder.ID3Data.title, DEFAULT_ID3);

        if (!read_ID3_info(ALBUM_ID3, decoder.ID3Data.album, ID3_MAX_SIZE, decoder.mp3File))
            strcpy(decoder.ID3Data.album, DEFAULT_ID3);

        if (!read_ID3_info(ARTIST_ID3, decoder.ID3Data.artist, ID3_MAX_SIZE, decoder.mp3File))
            strcpy(decoder.ID3Data.artist, DEFAULT_ID3);

        if (!read_ID3_info(YEAR_ID3, decoder.ID3Data.year, 10, decoder.mp3File))
            strcpy(decoder.ID3Data.year, DEFAULT_ID3);

        if (!read_ID3_info(TRACK_NUM_ID3, decoder.ID3Data.trackNum, 10, decoder.mp3File))
            strcpy(decoder.ID3Data.trackNum, DEFAULT_ID3);

        //! here we get the size of the tag
        unsigned int tagSize = get_ID3_size(decoder.mp3File);
        
        //! we moves the position according to the tagSize
        fseek(decoder.mp3File, tagSize, SEEK_SET);
        decoder.bytesRem -= tagSize;

    }
    else
    {
        //! if you are here the mp3 file has not ID3 Tag
        rewind(decoder.mp3File);
    }
}