/*******************************************************************************
  @file     loading.h
  @brief    Header for loading.c file that allow the user to use the loadingBar function
  @author   M. Francois
 ******************************************************************************/

 /*******************************************************************************
 *							INCLUDE HEADER FILES
 ******************************************************************************/

#pragma once
#include <stdint.h>

 /*******************************************************************************
  *					FUNCTION PROTOTYPES WITH GLOBAL SCOPE
  ******************************************************************************/

  /**
   * @brief This function is called to update the percentaje showed on
			the loading bar in your console application.
   * @params percentaje: The percentaje to show in the console
			 (an unsigned int with 8 bits (from 0 to 256) value between 0 and 100)
   */
void loadingBar(uint8_t percentaje);