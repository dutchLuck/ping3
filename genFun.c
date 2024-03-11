/*
 * G E N F U N . C
 *
 * genFun.c last edited Sat Dec  2 21:30:25 2023
 * 
 */

#include <stdio.h>		/* printf() */
#include <stdlib.h>		/* atoi() malloc() free() atol() strtod() strtol() random() */
#include <limits.h>		/* LONG_MIN LONG_MAX INT_MIN strtol() */
#include <ctype.h>		/* touppper() */
#include "genFun.h"


void  clearByteArray( unsigned char *  ptr, int  sizeOfArray )  {
	for( ; sizeOfArray > 0; --sizeOfArray )  *ptr++ = ( unsigned char ) 0;
}


void  fillByteArray( unsigned char  dataByte,  unsigned char *  ptr, int  sizeOfArray )  {
	for( ; sizeOfArray > 0; --sizeOfArray )  *ptr++ = dataByte;
}


void  fillByteArrayWithIncByOne( unsigned char  startDataByte,  unsigned char *  ptr, int  sizeOfArray )  {
	for( ; sizeOfArray > 0; --sizeOfArray )  *ptr++ = startDataByte++;
}


/* fillFirstByteArrayByReplicatingSecondByteArray( icmpDataPtr, icmpMsgSize - 8, icmpPayloadPattern, icmpPayloadPatternSize ) */
void  fillFirstByteArrayByReplicatingSecondByteArray( unsigned char *  firstArray, int  firstArraySize,
	unsigned char *  secondArray, int  secondArraySize )  {
	int  cnt;
	unsigned char *  ptr1;
	unsigned char *  ptr2;

	if( secondArraySize == 1 )  fillByteArray( *secondArray, firstArray, firstArraySize );	/* Special case of all set the same */
	else  {
		ptr1 = firstArray;
		ptr2 = secondArray;
		for( cnt = 0; cnt < firstArraySize; cnt++ )  {
			if(( cnt % secondArraySize ) == 0 )  ptr2 = secondArray;	/* set the pointer back to the start of the 2nd Array */
			*ptr1++ = *ptr2++;
		}
	}
}


/* fillByteArrayWithPseudoRandomData( icmpDataPtr, icmpMsgSize ) */
void  fillByteArrayWithPseudoRandomData( unsigned char *  array, int  arraySize )  {
	int  cnt, pseudoRandomInt;
	unsigned char *  chrPtr;
	int *  intPtr;

	intPtr = ( int * ) array;
	for( cnt = 0; cnt < arraySize; )  {
		pseudoRandomInt = ( int )( random() & 0xffffffff );	/* use of random() is far from perfect  */
		if(( cnt + sizeof( int ) ) <= arraySize )  {	/* will a whole int fit or are we too close to full */
			*intPtr++ = pseudoRandomInt;	/* put an pseudo random integer into the array */
			cnt += sizeof( int );
		}
		else  {		/* fill last part of array if the array wasn't an exact multiple of integer in size */
			chrPtr = ( unsigned char * ) intPtr;
			for( ; cnt < arraySize; cnt++ )  {
				*chrPtr++ = ( unsigned char )( pseudoRandomInt & 0xff );
				pseudoRandomInt = pseudoRandomInt >> 8;
			}
		}
	}
}


void  printByteArray( unsigned char *  ptr, int  sizeOfArray, int  charsPerLine )  {
	int  cnt, printIndex;

	printIndex = ( sizeOfArray > ( charsPerLine * 2 ));		/* Start each line with an Index value if there will be more than two lines of output */
	for( cnt = 0; cnt < sizeOfArray; cnt++ )  {
		if(( cnt % charsPerLine ) == 0 )  {
			printf( "\n" );
			if( printIndex )  printf( "%4d - ", cnt );
		}
		printf( " %02x", *ptr++ & 0xff );
	}
	if(( cnt % charsPerLine ) != 1 )  printf( "\n" );
}


double  limitDoubleValueToEqualOrMoreNegetiveThanBoundary( double  value, double  boundary )  {
	return(( value > boundary ) ? boundary : value );
}


double  limitDoubleValueToEqualOrMorePositiveThanBoundary( double  value, double  boundary )  {
	return(( value < boundary ) ? boundary : value );
}


double  limitDoubleValueToEqualOrWithinRange( double  value, double  loBoundary, double  hiBoundary )  {
	return(( value < loBoundary ) ? loBoundary : (( value > hiBoundary ) ? hiBoundary : value ));
}


long  limitLongValueToEqualOrMoreNegetiveThanBoundary( long  value, long  boundary )  {
	return(( value > boundary ) ? boundary : value );
}


long  limitLongValueToEqualOrMorePositiveThanBoundary( long  value, long  boundary )  {
	return(( value < boundary ) ? boundary : value );
}


unsigned long  limitUnsignedLongValueToEqualOrWithinRange( unsigned long  value, unsigned long  loBoundary, unsigned long  hiBoundary )  {
	return(( value < loBoundary ) ? loBoundary : (( value > hiBoundary ) ? hiBoundary : value ));
}


long  limitLongValueToEqualOrWithinRange( long  value, long  loBoundary, long  hiBoundary )  {
	return(( value < loBoundary ) ? loBoundary : (( value > hiBoundary ) ? hiBoundary : value ));
}


unsigned int  limitUnsignedIntegerValueToEqualOrWithinRange( unsigned int  value, unsigned int  loBoundary, unsigned int  hiBoundary )  {
	return(( value < loBoundary ) ? loBoundary : (( value > hiBoundary ) ? hiBoundary : value ));
}


int  limitIntegerValueToEqualOrWithinRange( int  value, int  loBoundary, int  hiBoundary )  {
	return(( value < loBoundary ) ? loBoundary : (( value > hiBoundary ) ? hiBoundary : value ));
}


unsigned char  limitUnsignedCharValueToEqualOrWithinRange( unsigned char  value, unsigned char  loBoundary, unsigned char  hiBoundary )  {
	return(( value < loBoundary ) ? loBoundary : (( value > hiBoundary ) ? hiBoundary : value ));
}


char  limitCharValueToEqualOrWithinRange( char  value, char  loBoundary, char  hiBoundary )  {
	return(( value < loBoundary ) ? loBoundary : (( value > hiBoundary ) ? hiBoundary : value ));
}


void  resetFlagAndPrintParameterWarning( int *  flag, char *  flagName, char *  warningDetail )  {
	/* reset the flag to inactive and output a Warning */
	*flag = FALSE;
    printf( "Warning: Ignoring option %s : Parameter value for option %s.\n", flagName, warningDetail );
}


void  resetFlagAndPrintConversionWarning( int *  flag, char *  flagName, char *  strng, char *  warningDetail )  {
	/* reset the flag to inactive and output a Warning */
	*flag = FALSE;
    printf( "Warning: Ignoring option %s : Unable to convert \"%s\" %s.\n", flagName, strng, warningDetail );
}


long long  convertOptionStringToLongLong( long long  defltValue, char *  strng, char *  flgName, int *  flgActive, int  strictFlag )  {
	long long  result;
	char *  endPtrStore;

	result = defltValue;	/* Set default value if flag is not active */
	if( *flgActive )  {
		if( strng == ( char * ) NULL )  {
			if( strictFlag )  resetFlagAndPrintParameterWarning( flgActive, flgName, "is uninitialised" );
		}
		else if( *strng == '\0' )  {	/* Assume "" used as option value so return default */
			if( strictFlag )  resetFlagAndPrintParameterWarning( flgActive, flgName, "contains no information" );
		}
		else  {
#ifdef DEBUG  
			printf( "Debug: String for option %s is \"%s\"\n", flgName, strng );
#endif
		 /* Convert option string specified to signed long, if possible */
    		result = strtoll( strng, &endPtrStore, 10 );		/* base 10 conversion of strng to long integer */
		 /* Check on strtol output - did any characters get converted? */
    		if( endPtrStore == strng )  {
				if( strictFlag )  resetFlagAndPrintConversionWarning( flgActive, flgName, strng, "into an integer number" );
    			result = defltValue;
			}
#ifdef DEBUG  
			printf( "Debug: The conversion of \"%s\" for option %s resulted in a value of %lld\n", strng, flgName, result );
#endif
		}
	}
	return( result );
}


long  convertOptionStringToLong( long  defltValue, char *  strng, char *  flgName, int *  flgActive, int  strictFlag )  {
	long  result;
	char *  endPtrStore;

	result = defltValue;	/* Set default value if flag is not active */
	if( *flgActive )  {
		if( strng == ( char * ) NULL )  {
			if( strictFlag )  resetFlagAndPrintParameterWarning( flgActive, flgName, "is uninitialised" );
		}
		else if( *strng == '\0' )  {	/* Assume "" used as option value so return default */
			if( strictFlag )  resetFlagAndPrintParameterWarning( flgActive, flgName, "contains no information" );
		}
		else  {
#ifdef DEBUG  
			printf( "Debug: String for option %s is \"%s\"\n", flgName, strng );
#endif
		 /* Convert option string specified to signed long, if possible */
    		result = strtol( strng, &endPtrStore, 10 );		/* base 10 conversion of strng to long integer */
		 /* Check on strtol output - did any characters get converted? */
    		if( endPtrStore == strng )  {
				if( strictFlag )  resetFlagAndPrintConversionWarning( flgActive, flgName, strng, "into an integer number" );
    			result = defltValue;
			}
#ifdef DEBUG  
			printf( "Debug: The conversion of \"%s\" for option %s resulted in a value of %ld\n", strng, flgName, result );
#endif
		}
	}
	return( result );
}


int  convertOptionStringToInteger( int  defltValue, char *  strng, char *  flgName, int *  flgActive, int  strictFlg )  {
	long  tempResult;
	int  result;

	tempResult = convertOptionStringToLong(( long ) defltValue, strng, flgName, flgActive, strictFlg );	/* use long int code */
	result = ( int ) limitLongValueToEqualOrWithinRange( tempResult, ( long ) INT_MIN, ( long ) INT_MAX );	/* ensure result fits in an integer */
	return( result );
}


double  convertOptionStringToDouble( double  defltValue, char *  strng, char *  flgName, int *  flgActive, int  strictFlag )  {
	double  result;
	char *  endPtrStore;

	result = defltValue;	/* Ensure the return the default value if flag is not active */
	if( *flgActive )  {		/* Attempt to convert user input if flag is active */
		if( strng == ( char * ) NULL )  {	/* No string to convert so return default value */
			if( strictFlag )  resetFlagAndPrintParameterWarning( flgActive, flgName, "is uninitialised" );
		}
		else if( *strng == '\0' )  {	/* Assume "" used as option value so return default value */
			if( strictFlag )  resetFlagAndPrintParameterWarning( flgActive, flgName, "contains no information" );
		}
		else  {
#ifdef DEBUG  
			printf( "Debug: String for option %s is \"%s\"\n", flgName, strng );
#endif
		 /* Convert option string specified to double, if possible */
    		result = strtod( strng, &endPtrStore );
		 /* Check on strtod output - No conversion acheived if endptr == nptr for strtod() */
    		if( endPtrStore == strng )  {
				/* If strictFlag set then reset the flag to inactive and output a Warning */
				if( strictFlag )
					resetFlagAndPrintConversionWarning( flgActive, flgName, strng, "into a double precision floating point number" );
    			result = defltValue;
			}
#ifdef DEBUG  
			printf( "Debug: The conversion of \"%s\" for option %s resulted in a value of %lg\n", strng, flgName, result );
#endif
		}
	}
	return( result );
}


int  convertHexChrNibbleToInt( char *  hexChr )  {
	int  intChr;

	intChr = toupper( *hexChr );
	if(( intChr >= '0' ) && ( intChr <= '9' ))  return( intChr - ( int ) '0' );
	else if(( intChr >= 'A') && ( intChr <= 'F' ))  return( 10 + ( intChr - ( int ) 'A' ));
	else  return( -1 );
}


/*		icmpPayloadPatternSize = convertHexByteStringToByteArray( p_Strng, icmpPayloadPattern ); */
int  convertHexByteStringToByteArray( char *  hexStrng, unsigned char *  storageBuffer, int  storageBufferSize )  {
	int  byteCount = 0;
	int  upperNibble, lowerNibble;

	if(( hexStrng != ( char * ) NULL ) && ( *hexStrng != '\0' ))  {
		while(( byteCount < storageBufferSize ) && ( *hexStrng != '\0' ))  {
			upperNibble = convertHexChrNibbleToInt( hexStrng++ );
			if(( upperNibble != -1 ) && ( *hexStrng != '\0' ))  {
				lowerNibble = convertHexChrNibbleToInt( hexStrng++ );
				if( lowerNibble != -1 )  {
					*storageBuffer++ = ( unsigned char ) ((( upperNibble << 4 ) + lowerNibble ) & 0xff );
					byteCount += 1;
				}
			}
		}
	}
	return( byteCount );
}
