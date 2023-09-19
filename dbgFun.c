/*
 * D B G F U N . C
 *
 * dbgFun.c last edited Tue Sep 19 23:36:45 2023
 * 
 */

#include <stdio.h>		/* printf() fprintf() perror() */
#include "genFun.h"		/* printByteArray() */


void  printNamedByteArray( unsigned char *  ptr, int  sizeOfArray, int  bytesPerLine, char *  identifierString )  {
	printf( "\n--==##[[  %s  ]]##==--", identifierString );
	printByteArray( ptr, sizeOfArray, bytesPerLine );
	printf( "--==##[[  END of : %s  ]]##==--\n\n", identifierString );
}
