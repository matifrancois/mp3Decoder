/*******************************************************************************
  @file     loading.c
  @brief    loading file where exist the loadingBar function
            and the local definition
  @author   M. Francois
 ******************************************************************************/

/*******************************************************************************
*							INCLUDE HEADER FILES
******************************************************************************/

#include <stdio.h> 
#include <windows.h> 
#include "loading.h"

/*******************************************************************************
*					CONSTANT AND MACRO DEFINITIONS USING #DEFINE
******************************************************************************/

/* Use the color that you want, uncomment the selected and
   use it in the loadingBar function as you want */

#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_RESET   "\x1b[0m"
//#define ANSI_COLOR_RED     "\x1b[31m"
//#define ANSI_COLOR_YELLOW  "\x1b[33m"
//#define ANSI_COLOR_BLUE    "\x1b[34m"
//#define ANSI_COLOR_MAGENTA "\x1b[35m"
//#define ANSI_COLOR_CYAN    "\x1b[36m"
//#define ANSI_COLOR_RESET   "\x1b[0m"

/*******************************************************************************
                        GLOBAL FUNCTION DEFINITIONS
******************************************************************************/

void loadingBar(uint8_t percentaje)
{
    // Set the cursor again starting point of loading bar 
    printf("\r");
    // becaus we show the percentaje with 25 blocks we need to split the percentage 
    // value in 25 parts that is why we divide percentaje by 4 (100/4 = 25)
    int printablePercentaje = percentaje / 4;
    // Initialize char for printing loading bar 
    const char backgroundChar = 177, loadedChar = 219;

    //print the loading string again to avoid clear it
    printf(ANSI_COLOR_GREEN "Loading %d%% " ANSI_COLOR_RESET, percentaje);

    // Print loading bar progress according to the value passed as a parameter
    for (int i = 0; i < printablePercentaje; i++) {
        printf(ANSI_COLOR_GREEN "%c" ANSI_COLOR_RESET, loadedChar);
    }
    // Print the background again to avoid an empy char in the loading bar
    for (int i = 0; i < 25 - printablePercentaje; i++) {
        printf(ANSI_COLOR_GREEN "%c" ANSI_COLOR_RESET, backgroundChar);
    }
}