/*****************************************************************************
 *
 * bin2wav.c
 * 
 *
 * RELACS
 * Relaxed ELectrophysiological data Acquisition, Control, and Stimulation
 * Copyright (C) 2002-2011 Jan Benda <benda@bio.lmu.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 * 
 * RELACS is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 *****************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <values.h>
#include <getopt.h>


struct WAVHeader {
  char           chunkID[4];       /* 12 byte */
  unsigned int   chunkSize;
  char           chunkFormat[4];

  char           subChunk1ID[4];   /* 24 byte */
  unsigned int   subChunk1Size;
  unsigned short wFormatTag;
  unsigned short wChannels;
  unsigned int   dwSamplesPerSec;
  unsigned int   dwAvgBytesPerSec;
  unsigned short wBlockAlign;
  unsigned short wBitsPerSample;

  char           subChunk2ID[4];   /* 8 byte */
  unsigned int   subChunk2Size;

};


char binfile[200] = "";
char datfile[200] = "signal.dat";
int datasize = 2;
int datatype = 'i';  /* i: integer, f: float, d: double */
int datasign = 0;
int datachannels = 1;
long offset = 0;
long ndata = LONG_MAX;
double samplerate = 10000.0;
double gain = 1.0;


void extractData( void )
{
  FILE *BF;
  FILE *DF;
  struct WAVHeader header;
  char buffer[2048];
  long m, n, k, t;
  signed short *swp, d;
  float *fp;
  float v;
  int c, sn;
  int outdatasize;
  int outndata;

  BF = fopen( binfile, "r" );
  if ( BF == NULL ) {
    fprintf( stderr, "can't open %s!\n", binfile );
    return;
  }
  fseek( BF, offset, SEEK_SET );
  outdatasize = datasize;
  outndata = ndata;
  if ( datatype == 'f' ) {
    outdatasize = 2;
    outndata = ndata / datasize;
    outndata *= outdatasize;
  }
  DF = fopen( datfile, "wb" );
  header.chunkID[0] = 'R';
  header.chunkID[1] = 'I';
  header.chunkID[2] = 'F';
  header.chunkID[3] = 'F';
  header.chunkSize = 36 + outndata;
  header.chunkFormat[0] = 'W';
  header.chunkFormat[1] = 'A';
  header.chunkFormat[2] = 'V';
  header.chunkFormat[3] = 'E';
  header.subChunk1ID[0] = 'f';
  header.subChunk1ID[1] = 'm';
  header.subChunk1ID[2] = 't';
  header.subChunk1ID[3] = ' ';
  header.subChunk1Size = 16;
  header.wFormatTag = 1;
  header.wChannels = datachannels;
  header.dwSamplesPerSec = (unsigned long)rint( samplerate );
  header.wBitsPerSample = outdatasize*8;
  header.wBlockAlign = header.wChannels * header.wBitsPerSample/8;
  header.dwAvgBytesPerSec = header.dwSamplesPerSec * header.wBlockAlign;
  header.subChunk2ID[0] = 'd';
  header.subChunk2ID[1] = 'a';
  header.subChunk2ID[2] = 't';
  header.subChunk2ID[3] = 'a';
  header.subChunk2Size = outndata;
  fwrite( &header, sizeof( struct WAVHeader ), 1, DF );
  n = 0;
  t = 0;
  c = 0;
  sn = 0;
  do {
    m = fread( buffer, 1, 2048, BF );
    if ( n+m > ndata )
      m = ndata - n;
    if ( datatype == 'f' ) {
      fp = (float *)buffer;
      for ( k=0; k<m; k+=datasize ) {
	v = *fp*gain;
	if ( fabs( v ) >= 1.0 ) {
	  fprintf( stderr, "warning: data value %g too high.\n", v );
	  v = 1.0;
	}
	d = (signed short)rint( v*0x7fff );
	fwrite( &d, 2, 1, DF );
	sn++;
	c++;
	if ( c >= datachannels ) {
	  c = 0;
	  t++;
	}
	fp++;
      }
    }
    else if ( datatype == 'i' && datasize == 2 && datasign == 1 ) {
      swp = (signed short *)buffer;
      for ( k=0; k<m; k+=datasize ) {
	fwrite( swp, datasize, 1, DF );
	c++;
	if ( c >= datachannels ) {
	  c = 0;
	  t++;
	}
	swp++;
      }
    }
    else {
      fprintf( stderr, "sorry! Data format not supported.\n" );
      fprintf( stderr, "data type: %c\n", datatype );
      fprintf( stderr, "data sign: %s\n", datasign > 0 ? "signed" : "unsigned" );
      fprintf( stderr, "data size: %d\n", datasize );
      break;
    }
    n += m;
  } while ( m == 2048 && n<ndata);
  if ( n < ndata )
    fprintf( stderr, "warning: read only %ld from %ld requested bytes.\n", n, ndata );
  fclose( DF );
  fclose( BF  );
}

 
void WriteUsage()
{
  printf( "\nusage:\n" );
  printf( "\n" );
  printf( "bin2wav [-o|O ## -u|U ## -n|N ## -T ## -s ## -d ## -f -F -c ## -v] -t ## | -r ## <binfile> <wavfile>\n" );
  printf( "\n" );
  printf( "Save binary data from file <binfile> as a wave file <wavfile>.\n" );
  printf( "-o : save data starting from byte offset ##.\n" );
  printf( "-O : save data starting from byte offset ## times size of data type.\n" );
  printf( "-u : save data upto byte offset ##.\n" );
  printf( "-U : save data upto byte offset ## times size of data type.\n" );
  printf( "-n : save at maximum ## bytes.\n" );
  printf( "-N : save at maximum ## lines (i.e. ## times size of data type times\n" );
  printf( "     number of channels bytes).\n" );
  printf( "-T : save at maximum ## seconds of data.\n" );
  printf( "-t : the sampling interval ## in seconds.\n" );
  printf( "-r : the sampling rate ## in Hertz.\n" );
  printf( "\n" );
  printf( "Usually the type of the data contained in the binary file\n" );
  printf( "is determined from its extension. The following options can be\n" );
  printf( "used to specify the data type directly:\n" );
  printf( "-s : specify sign of the binary data (0=unsigned, 1=signed, default=signed).\n" );
  printf( "-d : specify size of the binary data type in bytes (1, 2, 4, 8, default=2).\n" );
  printf( "-f : the binary data type is float (4 byte).\n" );
  printf( "-F : the binary data type is double (8 byte).\n" );
  printf( "-c : specify number of channels multiplexed in the binary data file (default=1).\n" );
  printf( "-v : print settings to stderr.\n" );
  printf( "\n" );
  exit( 0 );
}


void ReadArgs( int argc, char *argv[] )
{
  int c;
  long upto = 0;
  int offsd = 0;
  int uptod = 0;
  int ndatad = 0;
  double time = 0.0;
  int setsign = 1;
  int setsize = 1;
  int setcol = 1;
  int showvals = 0;
  char *sp;

  if ( argc <= 1 )
    WriteUsage();
  static struct option longoptions[] = {
    { "version", 0, 0, 0 },
    { "help", 0, 0, 0 },
    { 0, 0, 0, 0 }
  };
  optind = 0;
  opterr = 0;
  int longindex = 0;
  while ( (c = getopt_long( argc, argv, "o:O:u:U:n:N:T:t:r:s:d:fFc:g:v",
			    longoptions, &longindex )) >= 0 ) {
    switch ( c ) {
    case 0: switch ( longindex ) {
      case 0:
	printf( "bin2wav 1.0\n" );
	exit( 0 );
	break;
      case 1:
	WriteUsage();
	break;
      }
      break;

    case 'o': if ( optarg == NULL || sscanf( optarg, "%ld", &offset ) == 0 )
	offset = 0;
      break;
    case 'O': if ( optarg == NULL || sscanf( optarg, "%ld", &offset ) == 0 )
	offset = 0;
      offsd = 1;
      break;
    case 'u': if ( optarg == NULL || sscanf( optarg, "%ld", &upto ) == 0 )
	upto = 0;
      break;
    case 'U': if ( optarg == NULL || sscanf( optarg, "%ld", &upto ) == 0 )
	upto = 0;
      uptod = 1;
      break;
    case 'n': if ( optarg == NULL || sscanf( optarg, "%ld", &ndata ) == 0 )
	ndata = 0;
      break;
    case 'N': if ( optarg == NULL || sscanf( optarg, "%ld", &ndata ) == 0 )
	ndata = 0;
      ndatad = 1;
      break;
    case 'T': if ( optarg == NULL || sscanf( optarg, "%lf", &time ) == 0 )
	time = 0.0;
      break;
    case 't': if ( optarg == NULL || sscanf( optarg, "%lf", &samplerate ) == 0 )
	samplerate = 10000.0;
      else
        samplerate = 1.0/samplerate;
      break;
    case 'r': if ( optarg == NULL || sscanf( optarg, "%lf", &samplerate ) == 0 )
	samplerate = 10000.0;
      break;
    case 's': if ( optarg == NULL || sscanf( optarg, "%d", &datasign ) == 0 )
	datasign = 1;
      setsign = 0;
      break;
    case 'd': if ( optarg == NULL || sscanf( optarg, "%d", &datasize ) == 0 )
	datasize = 2;
      setsize = 0;
      break;
    case 'f': datasize = sizeof( float );
      datatype = 'f';
      setsize = 0;
      break;
    case 'F': datasize = sizeof( double );
      datatype = 'd';
      setsize = 0;
      break;
    case 'c': if ( optarg == NULL || sscanf( optarg, "%d", &datachannels ) == 0 )
	datachannels = 1;
      setcol = 0;
      break;
    case 'g': if ( optarg == NULL || sscanf( optarg, "%lf", &gain ) == 0 )
	gain = 1.0;
      break;
    case 'v': showvals = 1;
      break;
    default : WriteUsage();
    }
  }
  if ( optind >= argc-1 || argv[optind][0] == '?' )
    WriteUsage();

  strcpy( binfile, argv[optind] );
  strcpy( datfile, argv[optind+1] );

  sp = strrchr( binfile, '.' );
  if ( sp != NULL ) {
    sp++;
    if ( *sp == 'r' ) {
      if ( setsize ) {
	datasign = 1;
	datasize = sizeof( float );
	datatype = 'f';
      }
      if ( setcol )
	datachannels = 1;
    }
    else {
      if ( setsign )
	datasign = ( *sp == 's' );
      sp++;
      if ( setsize ) {
	if ( *sp == 'b' )
	  datasize = 1;
	else if ( *sp == 'w' )
	  datasize = 2;
	else if ( *sp == 'd' )
	  datasize = 4;
	else if ( *sp == 'q' )
	  datasize = 8;
      }
      sp++;
      if ( setcol ) {
	datachannels = atoi( sp );
	if ( datachannels <= 0 )
	  datachannels = 1;
      }
    }
  }

  if ( offsd )
    offset *= datasize;
  if ( time > 0.0 ) {
    ndata = (int)floor( time * samplerate );
    ndata *= datasize * datachannels;
  }
  else if ( ndatad )
    ndata *= datasize * datachannels;
  if ( uptod )
    upto *= datasize;
  if ( upto > 0 )
    ndata = upto - offset;

  if ( showvals ) {
    fprintf( stderr, "binary file: %s\n", binfile );
    fprintf( stderr, "data file: %s\n", datfile );
    fprintf( stderr, "offset: %ld bytes\n", offset );
    fprintf( stderr, "ndata: %ld bytes\n", ndata );
    fprintf( stderr, "data sign: %s\n", datasign > 0 ? "signed" : "unsigned" );
    fprintf( stderr, "data size: %d bytes\n", datasize );
    fprintf( stderr, "data type: %c\n", datatype );
    fprintf( stderr, "data channels: %d\n", datachannels );
    fprintf( stderr, "sampling rate: %g Hz\n", samplerate );
  }
}


int main( int argc, char *argv[] )
{
  ReadArgs( argc, argv );  
  extractData();
  return 0;
}
