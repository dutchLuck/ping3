/*
 * G E N F U N . C
 *
 * genFun.c last edited Thu Oct 26 16:26:46 2023
 * 
 */

#include <stdio.h>		/* printf() */
#include <stdlib.h>		/* atoi() malloc() free() atol() strtod() */
#include <limits.h>		/* LONG_MIN LONG_MAX INT_MIN */
#include "genFun.h"

#ifndef  FALSE
#define  FALSE  ((int)0)
#endif
#ifndef  TRUE
#define  TRUE  (! FALSE)
#endif


void  clearByteArray( unsigned char *  ptr, int  sizeOfArray )  {
	for( ; sizeOfArray > 0; --sizeOfArray )  *ptr++ = ( unsigned char ) 0;
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


long  convertOptionStringToLong( long  defltValue, char *  strng, char *  flgName, int *  flgActive, int  strictFlag )  {
	long  result;

	result = defltValue;	/* Set default value if flag is not active */
	if( *flgActive )  {
		if( strng == ( char * ) NULL )  {
			if( strictFlag )  {
    			*flgActive = FALSE;
    			printf( "Warning: Parameter value for option %s is uninitialised, ignoring option %s\n", flgName, flgName );
			}
		}
		else if( *strng == '\0' )  {	/* Assume "" used as option value so return default */
			if( strictFlag )  {
    			*flgActive = FALSE;
				printf( "Warning: Parameter value for option %s contains no information, ignoring option %s\n", flgName, flgName );
			}
		}
		else  {
#ifdef DEBUG  
			printf( "Debug: String for option %s is \"%s\"\n", flgName, strng );
#endif
		 /* Convert option string specified to signed long, if possible */
    		result = atol( strng );
		 /* Rough check on atol() output - is result zero when option string likely wasn't zero */
    		if(( result == 0L ) && ( *strng != '0' ))  {
    			*flgActive = FALSE;
    			result = defltValue;
    			printf( "Warning: Unable to convert \"%s\" into an integer for option %s, ignoring option %s\n",
    				strng, flgName, flgName );
			}
#ifdef DEBUG  
			printf( "Debug: The conversion of string \"%s\" for option %s resulted in %ld\n", strng, flgName, result );
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

	result = defltValue;	/* Set default value if flag is not active */
	if( *flgActive )  {
		if( strng == ( char * ) NULL )  {
			if( strictFlag )  {
    			*flgActive = FALSE;
    			printf( "Warning: Parameter value for option %s is uninitialised, ignoring option %s\n", flgName, flgName );
			}
		}
		else if( *strng == '\0' )  {	/* Assume "" used as option value so return default */
			if( strictFlag )  {
    			*flgActive = FALSE;
				printf( "Warning: Parameter value for option %s contains no information, ignoring option %s\n", flgName, flgName );
			}
		}
		else  {
#ifdef DEBUG  
			printf( "Debug: String for option %s is \"%s\"\n", flgName, strng );
#endif
		 /* Convert option string specified to double, if possible */
    		result = strtod( strng, &endPtrStore );
		 /* Check on strtod output - No conversion acheived if endptr == nptr for strtod() */
    		if( endPtrStore == strng )  {
    			*flgActive = FALSE;
    			result = defltValue;
    			printf( "Warning: Unable to convert \"%s\" into an double precision floating point number for option %s, ignoring option %s\n",
    				strng, flgName, flgName );
			}
#ifdef DEBUG  
			printf( "Debug: The conversion of string \"%s\" for option %s resulted in %ld\n", strng, flgName, result );
#endif
		}
	}
	return( result );
}
