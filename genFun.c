/*
 * G E N F U N . C
 *
 * genFun.c last edited Mon May  5 20:47:31 2025
 * 
 */

#include <stdio.h>		/* printf() */
#include <stdlib.h>		/* atoi() malloc() free() atol() strtod() strtol() rand() */
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
		pseudoRandomInt = ( int )( rand() & 0xffffffff );	/* use of random() is far from perfect  */
		if(( cnt + __SIZEOF_INT__ ) <= arraySize )  {	/* will a whole int fit or are we too close to full */
			*intPtr++ = pseudoRandomInt;	/* put an pseudo random integer into the array */
			cnt += __SIZEOF_INT__;
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


int  convert2HexChrNibblesToInt( char *  hexChr )  {
	int  intChr1;
	int  intChr2;

	if (( intChr1 = convertHexChrNibbleToInt( hexChr )) < 0 )  return( -1 );
	else if (( intChr2 = convertHexChrNibbleToInt( hexChr + 1 )) < 0 )  return( -1 );
	return( 16 * intChr1 + intChr2 );
}


int  convert4HexChrNibblesToInt( char *  hexChr )  {
	int  intChr1;
	int  intChr2;

	if (( intChr1 = convert2HexChrNibblesToInt( hexChr )) < 0 )  return( -1 );
	else if (( intChr2 = convert2HexChrNibblesToInt( hexChr + 2 )) < 0 )  return( -1 );
	return( 256 * intChr1 + intChr2 );
}

/* The following Circular (Ring) buffer is an expedient version that sacrifices 1 data */
/* storage location to allow an easy buffer full indication, even though when "full" */
/* the buffer is actually 1 location short of completely full. The buffer must be a */
/* power of two in size (e.g. 256 or 512 or 1024 or 2048 etc) for the AND mask to work. */

int ringBffrFull( int  inIndx, int  outIndx ) {
	return( inIndx == outIndx );
}


int ringBffrEmpty( int  inIndx, int  outIndx, int  ringMask ) {
	return((( outIndx + 1 ) & ringMask ) == inIndx );
}


int  ringBffrUsed( int  inIndex, int  outIndex, int  ringMask )  {
	int  diff;

	if( inIndex == outIndex )  return( ringMask );	/* if equal then circular buffer is deemed to be full */
	else if(( diff = ( inIndex - outIndex )) > 0 )  return( diff - 1 );
	else return( diff + ringMask );
}


void  zeroRingBffr( unsigned char  data[], size_t  dataBufferSize )  {
	for (size_t i = 0; i < dataBufferSize; i++ )
		data[ i ] = ( unsigned char ) 0;
}


void  dumpRingBffr( unsigned char  data[], size_t  dataBufferSize )  {
	for (size_t i = 0; i < dataBufferSize; i++)  {
		if (( i % 32 ) == 0 )  fprintf( stdout, "\n%02x", data[ i ] );
		else  fprintf( stdout, " %02x", data[ i ]);
	}
	fprintf( stdout, "\n\n" );
}


int  loadRingBffrWithBytesFromLineOfAsciiHex( char *  linePtr, int  byteCount,
	unsigned char  ringBffr[], int *  inIndx, int *  outIndx, int  circMask ) {
	int  i;

	for ( i = 0; (! ringBffrFull( *inIndx, *outIndx )) && ( i < byteCount ); i++) {
		ringBffr[ *inIndx ] = ( unsigned char )( convert2HexChrNibblesToInt( linePtr + i * 2 ) & 0xff );
		*inIndx = ( *inIndx + 1 ) & circMask;
	}
	return( i );	/* return number of bytes loaded */
}
