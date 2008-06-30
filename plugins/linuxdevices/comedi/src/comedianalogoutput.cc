/*
  comedi/comedianalogoutput.cc
  Interface for accessing analog output of a daq-board via comedi.

  RELACS - RealTime ELectrophysiological data Acquisition, Control, and Stimulation
  Copyright (C) 2002-2007 Jan Benda <j.benda@biologie.hu-berlin.de>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 3 of the License, or
  (at your option) any later version.
  
  RELACS is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
  
  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <iostream>
#include <sstream>
#include <cstdio>
#include <cmath>
#include <ctime>
#include <unistd.h>
#include <fcntl.h>
#include <relacs/comedi/comedianaloginput.h>
#include <relacs/comedi/comedianalogoutput.h>
using namespace std;
using namespace relacs;

namespace comedi {


ComediAnalogOutput::ComediAnalogOutput( void ) 
  : AnalogOutput( "Comedi Analog Output", ComediAnalogIOType )
{
  ErrorState = 0;
  IsPrepared = false;
  IsRunning = false;
  DeviceP = NULL;
  SubDevice = 0;
  LongSampleType = false;
  BufferElemSize = 0;
  MaxRate = 1000.0;
  Sigs = NULL;
  UnipolarExtRefRangeIndex = -1;
  BipolarExtRefRangeIndex = -1;
}


ComediAnalogOutput::ComediAnalogOutput(  const string &device, long mode ) 
  : AnalogOutput( "Comedi Analog Output", ComediAnalogIOType )
{
  ErrorState = 0;
  IsPrepared = false;
  IsRunning = false;
  DeviceP = NULL;
  SubDevice = 0;
  LongSampleType = false;
  BufferElemSize = 0;
  MaxRate = 1000.0;
  Sigs = NULL;
  UnipolarExtRefRangeIndex = -1;
  BipolarExtRefRangeIndex = -1;
  open( device, mode );
}


ComediAnalogOutput::~ComediAnalogOutput( void ) 
{
  close();
}


int ComediAnalogOutput::open( const string &device, long mode )
{ 
  if ( isOpen() )
    return -5;

  clearSettings();
  if ( device.empty() )
    return InvalidDevice;

  // open comedi device:
  DeviceP = comedi_open( device.c_str() );
  if ( DeviceP == NULL ) {
    cerr << "! error: ComediAnalogOutput::open() -> "
	 << "Device-file " << device << " could not be opened!\n";
    return NotOpen;
  }

  // get AO subdevice:
  int subdev = comedi_find_subdevice_by_type( DeviceP, COMEDI_SUBD_AO, 0 );
  if ( subdev < 0 ) {
    cerr << "! error: ComediAnalogOutput::open() -> "
	 << "No SubDevice for AO found on device "  << device << '\n';
    comedi_close( DeviceP );
    DeviceP = NULL;
    return InvalidDevice;
  }
  SubDevice = subdev;

  // lock AI subdevice:
  if ( comedi_lock( DeviceP, SubDevice ) != 0 ) {
    cerr << "! error: ComediAnalogOutput::open() -> "
	 << "Locking of AO SubDevice failed on device " << device << '\n';
    comedi_close( DeviceP );
    DeviceP = NULL;
    SubDevice = 0;
    return NotOpen;
  }  

  // check for async. command support:
  if ( ( comedi_get_subdevice_flags( DeviceP, SubDevice ) & SDF_CMD ) == 0 ) {
    cerr << "! error: ComediAnalogOutput::open() -> "
	 << "Device "  << device << " not supported! "
	 << "SubDevice needs to support async. commands!" << endl;
    comedi_unlock( DeviceP,  SubDevice );
    comedi_close( DeviceP );
    DeviceP = NULL;
    SubDevice = 0;
    return InvalidDevice;
  }

  // set basic device infos:
  setDeviceName( comedi_get_board_name( DeviceP ) );
  setDeviceVendor( comedi_get_driver_name( DeviceP ) );
  setDeviceFile( device );

  /* XXX
  // set comedi file-descriptor to non-blocking writing mode:
  retVal = fcntl( comedi_fileno(DeviceP), F_SETFL, O_NONBLOCK );//|O_ASYNC
  if ( retVal < 0 )
    cerr << " !  ComediAnalogOutput::open() -> "
	 << "Switching the device "  << device 
	 << " to non-blocking mode failed: " << strerror( errno ) << endl;
  */

  // set size of comedi-internal buffer to maximum:
  // XXXX maybe use an internal maximum as well 
  // (in case comedi max is way too much)?
  int bufSize = comedi_get_max_buffer_size( DeviceP, SubDevice );
  comedi_set_buffer_size( DeviceP, SubDevice, bufSize );

  // initialize ranges:
  UnipolarRange.clear();
  BipolarRange.clear();
  UnipolarRangeIndex.clear();
  BipolarRangeIndex.clear();
  UnipolarExtRefRangeIndex = -1;
  BipolarExtRefRangeIndex = -1;
  int nRanges = comedi_get_n_ranges( DeviceP, SubDevice, 0 );  
  for ( int i = 0; i < nRanges; i++ ) {
    comedi_range *range = comedi_get_range( DeviceP, SubDevice, 0, i );
    // TODO: if a ranges is not supported but comedi thinks so: set max = -1.0
    // i.e. NI 6070E PCI: range #3&4 (-1..1V, 0..1V) not supported
    if ( range->min < 0.0 ) {
      if ( range->unit & RF_EXTERNAL )
	BipolarExtRefRangeIndex = i;
      else {	
	BipolarRange.push_back( *range );
	BipolarRangeIndex.push_back( i );
      }
    }
    else {
      if ( range->unit & RF_EXTERNAL )
	UnipolarExtRefRangeIndex = i;
      else {	
	UnipolarRange.push_back( *range );
	UnipolarRangeIndex.push_back( i );
      }
    }
  }

  // bubble-sorting Uni/BipolarRange according to Uni/BipolarRange.max:
  for( unsigned int i = 0; i < UnipolarRangeIndex.size(); i++ ) {
    for ( unsigned int j = i+1; j < UnipolarRangeIndex.size(); j++ ) {
      if (  UnipolarRange[i].max < UnipolarRange[j].max ) {
	comedi_range rangeSwap = UnipolarRange[i];
	UnipolarRange[i] = UnipolarRange[j];
	UnipolarRange[j] = rangeSwap;
	unsigned int indexSwap = UnipolarRangeIndex[i];
	UnipolarRangeIndex[i] = UnipolarRangeIndex[j];
	UnipolarRangeIndex[j] = indexSwap;
      }
    }
  }
  for( unsigned int i = 0; i < BipolarRangeIndex.size(); i++ ) {
    for ( unsigned int j = i+1; j < BipolarRangeIndex.size(); j++ ) {
      if (  BipolarRange[i].max < BipolarRange[j].max ) {
	comedi_range rangeSwap = BipolarRange[i];
	BipolarRange[i] = BipolarRange[j];
	BipolarRange[j] = rangeSwap;
	unsigned int indexSwap = BipolarRangeIndex[i];
	BipolarRangeIndex[i] = BipolarRangeIndex[j];
	BipolarRangeIndex[j] = indexSwap;
      }
    }
  }

  // get size of datatype for sample values:
  LongSampleType = ( comedi_get_subdevice_flags( DeviceP, SubDevice ) &
		     SDF_LSAMPL );
  if ( LongSampleType )
    BufferElemSize = sizeof( lsampl_t );
  else
    BufferElemSize = sizeof( sampl_t );

  // try to find out the maximum sampling rate:
  comedi_cmd cmd;
  memset( &cmd,0, sizeof(comedi_cmd) );
  unsigned int chanlist = CR_PACK( 0, 0, AREF_GROUND );
  int retVal = comedi_get_cmd_generic_timed( DeviceP, SubDevice, &cmd,
					     1/*chans*/,
					     (unsigned int)1e8/*Hz*/ );
  if ( retVal < 0 ){
    cmd.subdev = SubDevice;
    cmd.start_src =        TRIG_NOW;
    cmd.start_arg =        0;
    cmd.scan_begin_src =   TRIG_TIMER;
    cmd.convert_src =      TRIG_TIMER;
    cmd.convert_arg =      1;
    cmd.scan_end_src =     TRIG_COUNT;
    cmd.stop_src =         TRIG_COUNT;
  }
  cmd.scan_begin_arg =   1;
  cmd.chanlist           = &chanlist;
  cmd.chanlist_len       = 1;
  cmd.scan_end_arg = 1;
  if ( cmd.stop_src == TRIG_COUNT )
    cmd.stop_arg = 1;
  retVal = comedi_command_test( DeviceP, &cmd );
  if ( retVal == 1 || retVal == 2 )
    retVal = comedi_command_test( DeviceP, &cmd );
  if ( retVal == 1 || retVal == 2 )
    retVal = comedi_command_test( DeviceP, &cmd );
  if ( cmd.scan_begin_arg > 0 )
    MaxRate = 1.0e9 / cmd.scan_begin_arg;
  else //XXX set default for NI E-series (mio-driver doesn't return a valid frequency)
    MaxRate = 1.0e6; 

  // clear flags:
  ErrorState = 0;
  IsPrepared = false;
  IsRunning = false;
  ComediAOs.clear();
 
  return 0;
}


bool ComediAnalogOutput::isOpen( void ) const 
{ 
  return ( DeviceP != NULL );
}


void ComediAnalogOutput::close( void )
{ 
  if ( ! isOpen() )
    return;

  reset();

  // unlock:
  int error = comedi_unlock( DeviceP, SubDevice );
  if ( error < 0 )
    cerr << "! warning: ComediAnalogOutput::close() -> "
	 << "Unlocking of AO subdevice on device " << deviceFile() << "failed\n";
  
  // close:
  error = comedi_close( DeviceP );
  if ( error )
    cerr << "! warning: ComediAnalogOutput::close() -> "
	 << "Closing of AI subdevice on device " << deviceFile() << "failed.\n";

  // clear flags:
  DeviceP = NULL;
  SubDevice = 0;
  ComediAOs.clear();
}


int ComediAnalogOutput::channels( void ) const
{ 
  if ( !isOpen() )
    return -1;
  return comedi_get_n_channels( DeviceP, SubDevice );
}


int ComediAnalogOutput::bits( void ) const
{ 
  if ( !isOpen() )
    return -1;
  int maxData = comedi_get_maxdata( DeviceP, SubDevice, 0 );
  return (int)( log( maxData+2.0 )/ log( 2.0 ) );
}


double ComediAnalogOutput::maxRate( void ) const 
{ 
  return MaxRate;
}


int ComediAnalogOutput::maxRanges( void ) const
{
  return UnipolarRangeIndex.size() > BipolarRangeIndex.size() ?
    UnipolarRangeIndex.size() : BipolarRangeIndex.size();
}


double ComediAnalogOutput::unipolarRange( int index ) const
{
  if ( (index < 0) || (index >= (int)UnipolarRangeIndex.size()) )
    return -1.0;
  return UnipolarRange[index].max;
}


double ComediAnalogOutput::bipolarRange( int index ) const
{
  if ( (index < 0) || (index >= (int)BipolarRangeIndex.size()) )
    return -1.0;
  return BipolarRange[index].max;
}


template < typename T >
int ComediAnalogOutput::convert( OutList &sigs )
{
  // copy and sort signal pointers:
  OutList ol;
  ol.add( sigs );
  ol.sortByChannel();

  // set scaling factors:
  unsigned int iDelay = sigs[0].indices( sigs[0].delay() );
  double scale[ ol.size() ];
  double offs[ ol.size() ];
  for ( int k=0; k<ol.size(); k++ ) {
    scale[k] = ol[k].scale() * ol[k].gain();
    offs[k] = ol[k].offset() * ol[k].gain();
  }

  // allocate buffer:
  int nbuffer = ol.size() * ( sigs[0].size() + iDelay );
  T *buffer = new T [nbuffer];

  // convert data and multiplex into buffer:
  T *bp = buffer;
  for ( int i=-iDelay; i<ol[0].size(); i++ ) {
    for ( int k=0; k<ol.size(); k++ ) {
      int v;
      if ( i < 0 ) // simulate delay
	v = (T) ::rint( offs[k] );
      else
	v = (T) ::rint( ol[k][i] * scale[k] + offs[k] );
      if ( v > ol[k].maxData() )
	v = ol[k].maxData();
      else if ( v < ol[k].minData() ) 
	v = ol[k].minData();
      *bp = v;
      ++bp;
    }
  }

  sigs[0].setDeviceBuffer( (char *)buffer, nbuffer, sizeof( T ) );

  return 0;
}


int ComediAnalogOutput::convertData( OutList &sigs )
{
  if ( LongSampleType )
    return convert<lsampl_t>( sigs );
  else  
    return convert<sampl_t>( sigs );
}


int ComediAnalogOutput::testWriteDevice( OutList &sigs )
{
  if ( !isOpen() )
    return -1;

  // check channel ordering:
  if ( sigs.size() > 1 ) {
    vector< unsigned int > chs( sigs.size() );
    for ( int k=0; k<sigs.size(); k++ )
      chs[k] = sigs[k].channel();
    sort( chs.begin(), chs.end() );
    for ( unsigned int k=0; k<chs.size(); k++ ) {
      if ( chs[k] != k ) {
	sigs.addError( DaqError::InvalidChannelSequence );
	break;
      }
    }
  }
  
  int aref = AREF_GROUND;

  memset( ChanList, 0, sizeof( ChanList ) );
  memset( &Cmd, 0, sizeof( comedi_cmd ) );

  // ranges:
  int maxrange = 1 << bits();
  for ( int k=0; k<sigs.size(); k++ ) {
    // minimum and maximum values:
    double min = sigs[k].requestedMin();
    double max = sigs[k].requestedMax();
    if ( min == OutData::AutoRange || max == OutData::AutoRange ) {
      double smin = 0.0;
      double smax = 0.0;
      minMax( smin, smax, sigs[k] );
      if ( min == OutData::AutoRange )
	min = smin;
      if ( max == OutData::AutoRange )
	max = smax;
    }
    //    cerr << "ComediAnalogOutput::testWriteDevice() -> min: " << min << " max: " << max << "\n";
    // reference and polarity:
    bool unipolar = false;
    if ( min >= 0.0 )
      unipolar = true;
    double extref = false;
    if ( max == OutData::ExtRef )
      extref = true;
    else {
      // maximum value:
      min = ::fabs( min );
      if ( min > max )
	max = min;
    }
    //    cerr << "ComediAnalogOutput::testWriteDevice() -> unipolar: " << unipolar << " max: " << max << "\n";
    // set range:
    double maxboardvolt = -1.0;
    double maxvolt = sigs[k].getVoltage( max );
    int index = unipolar ? UnipolarRangeIndex.size() - 1 
                         :  BipolarRangeIndex.size() - 1;
    for( ; index >= 0; index-- ) {
      if ( unipolar && unipolarRange( index ) > maxvolt ) {
	maxboardvolt = unipolarRange( index );
	break;
      }
      if ( !unipolar && bipolarRange( index ) > maxvolt ){
	maxboardvolt = bipolarRange( index );
	break;
      }
    }
    if ( index < 0 )
      sigs[k].addError( DaqError::InvalidReference );
        
    if ( sigs[k].noIntensity() ) {
      if ( ! extref ) {
	if ( externalReference() < maxboardvolt ) {
	  if ( maxvolt < externalReference() )
	    extref = true;
	}
	else
	  if ( maxboardvolt == -1.0 )
	    extref = true;
      }
      if ( extref ) {
	if ( externalReference() < 0.0 ) {
	  sigs[k].addError( DaqError::InvalidReference );
	  extref = false;
	}
	else {
	  if ( externalReference() == 0.0 )
	    maxboardvolt = 1.0;
	  else
	    maxboardvolt = externalReference();
	  index = unipolar ? UnipolarExtRefRangeIndex 
	                   :  BipolarExtRefRangeIndex;
	}
      }
      if ( unipolar )
	sigs[k].setGain( maxrange/maxboardvolt, 0.0 );
      else
	sigs[k].setGain( maxrange/2/maxboardvolt, maxboardvolt );
    }
    else {
      if ( extref && externalReference() < 0.0 ) {
	sigs[k].addError( DaqError::InvalidReference );
	extref = false;
      }
      if ( unipolar )
	sigs[k].setGain( maxrange, 0.0 );
      else
	sigs[k].setGain( maxrange/2, maxrange/2 );
    }
    //    cerr << "ComediAnalogOutput::testWriteDevice() -> gain: " << sigs[k].gain() << " offset: " << sigs[k].offset() << " index: " << index << "\n";

    int gainIndex = index;
    if ( unipolar )
      gainIndex |= 1<<14;
    if ( extref )
      gainIndex |= 1<<15;
    
    // multiple references?
    if ( ( gainIndex & 1<<15 ) != ( sigs[0].gainIndex() & 1<<15 ) ) {
      sigs[k].addError( DaqError::MultipleReferences );
      gainIndex = sigs[0].gainIndex();
    }

    sigs[k].setGainIndex( gainIndex );
    sigs[k].setMinData( 0 );
    sigs[k].setMaxData( maxrange - 1 );
    //    cerr << "ComediAnalogOutput::testWriteDevice() -> gainIndex: " << sigs[k].gainIndex() << " mindata: " << sigs[k].minData() << " maxData: " << sigs[k].maxData() << "\n";

    // set up channel in chanlist:
    if ( unipolar )
      ChanList[k] = CR_PACK( sigs[k].channel(),
			     UnipolarRangeIndex[index], aref );
    else
      ChanList[k] = CR_PACK( sigs[k].channel(),
			     BipolarRangeIndex[index], aref );
  }


  /* // XXX i'm not sure if testing works on running device...
  if ( running() ) {
    sigs.addError( DaqError::Busy );
    return -1;
  }
  */

  if ( sigs[0].sampleRate() * double( sigs.size() ) > maxRate() )
    sigs.addError( DaqError::InvalidSampleRate );

  if ( sigs.failed() )
    return -1;
  
  // try automatic command generation:
  unsigned int intervalLength = (int)( 1e9 * sigs[0].sampleInterval() );
  int retVal = comedi_get_cmd_generic_timed( DeviceP, SubDevice, &Cmd, sigs.size(), intervalLength );
  if ( Cmd.scan_begin_arg < intervalLength ) {
    cerr << " ComediAnalogOutput::testWriteDevice(): "
	 << "Requested sampling rate bigger than supported! max "
	 << Cmd.scan_begin_arg << "ns sampling interval possible." << endl;/////TEST/////
    sigs.addError( DaqError::InvalidSampleRate );    
  }
  if ( retVal < 0 ) {
    Cmd.scan_begin_src = TRIG_TIMER; // when to perform a scan is triggered by timer
    Cmd.scan_begin_arg = intervalLength; // nsec's between 2 scans (1/sampling-frequency)
    Cmd.convert_src = TRIG_TIMER;
    Cmd.convert_arg = intervalLength/ sigs.size();
    Cmd.scan_end_src = TRIG_COUNT;
  }

  // adapt command to our purpose:
  comedi_cmd testCmd;
  comedi_get_cmd_src_mask( DeviceP, SubDevice, &testCmd );
  if ( testCmd.start_src & TRIG_INT )
    Cmd.start_src = TRIG_INT;
  else
    Cmd.start_src = TRIG_NOW;
  Cmd.start_arg = 0;
  if ( Cmd.convert_src & TRIG_NOW )
    Cmd.convert_arg = 1;
  Cmd.scan_end_arg = sigs.size();
  
  // test if countinous-state is supported:
  if ( sigs[0].continuous() && !(testCmd.stop_src & TRIG_NONE) ) {
    cerr << " ComediAnalogOutput::testWriteDevice(): "
	 << " continuous mode not supported!" << endl;/////TEST/////
    sigs.addError( DaqError::InvalidContinuous );
    sigs.setContinuous( false );
  }
  if ( !sigs[0].continuous() && !(testCmd.stop_src & TRIG_COUNT) ) {
    cerr << " ComediAnalogOutput::testWriteDevice(): "
	 << " only continuous mode supported!" << endl;/////TEST/////
    sigs.addError( DaqError::InvalidContinuous );
    sigs.setContinuous( true );
  }
    
  // set countinous-state
  if ( sigs[0].continuous() ) {
      Cmd.stop_src = TRIG_NONE;
      Cmd.stop_arg = 0;
    }
  if ( !sigs[0].continuous() ) {
      Cmd.stop_src = TRIG_COUNT;
      // set length of acquisition as number of scans:
      Cmd.stop_arg = sigs[0].size() + sigs[0].indices( sigs[0].delay());
  }

  Cmd.chanlist = ChanList;
  Cmd.chanlist_len = sigs.size();
  // XXX ATTENTION:  maybe TRIG_WAKE_EOS causes many interrupts!
  //  Cmd.flags = TRIG_WAKE_EOS | TRIG_RT;
  Cmd.flags = TRIG_ROUND_NEAREST;

  // test command:
  // XXX analyse errors!
  // after first call pf comedi_command_test Cmd should be ok:
  retVal = comedi_command_test( DeviceP, &Cmd );
  retVal = comedi_command_test( DeviceP, &Cmd );
  // if this second call is not successfull, then something is really wrong:
  if ( retVal ) {
    Cmd.flags &= ~TRIG_RT;
    retVal = comedi_command_test( DeviceP, &Cmd );
    retVal = comedi_command_test( DeviceP, &Cmd );
    if ( retVal ) {
      Cmd.flags = 0;
      retVal = comedi_command_test( DeviceP, &Cmd );
      retVal = comedi_command_test( DeviceP, &Cmd );
    }
  }
  if ( retVal != 0 )
    return -retVal;

  return 0;
}


int ComediAnalogOutput::prepareWrite( OutList &sigs )
{
  if ( !isOpen() )
    return -1;

  reset();

  // copy and sort signal pointers:
  OutList ol;
  ol.add( sigs );
  ol.sortByChannel();
  
  Sigs = &sigs;

  int error = testWriteDevice( ol );
  if ( error )
    return error;

  if ( ol.success() )
    setSettings( ol );

  //  cerr << " ComediAnalogOutput::prepareWrite(): success" << endl;//////TEST/////

  IsPrepared = ol.success();

  return sigs.success() ? 0 : -1;
}


int ComediAnalogOutput::executeCommand( void )
{
  ErrorState = 0;
  if ( comedi_command( DeviceP, &Cmd ) < 0 ) {
    cerr << "AO command failed: " << comedi_strerror( comedi_errno() ) << endl;
    /*
    traces.addErrorStr( deviceFile() + " - execution of comedi_cmd failed: "
			+ comedi_strerror( comedi_errno() ) );
    */
    return -1;
  }
  fillWriteBuffer();
  return 0;
}


int ComediAnalogOutput::startWrite( OutList &sigs )
{
  //  cerr << " ComediAnalogOutput::startWrite(): begin" << endl;/////TEST/////

  if ( !prepared() ) {
    cerr << "AO not prepared!\n";
    return -1;
  }

  // setup instruction list:
  comedi_insnlist insnlist;
  insnlist.n_insns = ComediAOs.size();
  insnlist.insns = new comedi_insn[insnlist.n_insns];
  for ( unsigned int k=0; k<insnlist.n_insns; k++ ) {
    insnlist.insns[k].insn = INSN_INTTRIG;
    insnlist.insns[k].subdev = -1;
    InsnData[0] = 0;
    insnlist.insns[k].data = InsnData;
    insnlist.insns[k].n = 1;
  }
  bool success = true;
  int ilinx = 0;
  for ( unsigned int k=0; k<ComediAOs.size() && success; k++ ) {
    if ( ComediAOs[k]->prepared() ) {
      if ( ComediAOs[k]->executeCommand() < 0 )
	success = false;
      else
	insnlist.insns[ilinx++].subdev = ComediAOs[k]->comediSubdevice();
    }
  }
  insnlist.n_insns = ilinx;
  if ( success ) {
    int ninsns = comedi_do_insnlist( DeviceP, &insnlist );

    if ( ninsns == ilinx ) {
      for ( unsigned int k=0; k<ComediAOs.size(); k++ ) {
 	ComediAOs[k]->setRunning();
      }
    }
    else {
      success = false;
    }
  }
  delete [] insnlist.insns;
  
  return success ? 0 : -1;
}


int ComediAnalogOutput::writeData( OutList &sigs )
{
  //device stopped?
  if ( !running() ) {
    sigs.addErrorStr( "ComediAnalogOutput::writeData: " +
		      deviceFile() + " is not running!" );
    cerr << "ComediAnalogOutput::writeData: device is not running! comedi_strerror: " << comedi_strerror( comedi_errno() ) << endl;/////TEST/////
    return 0;/////TEST/////
  }

  return fillWriteBuffer();
}


int ComediAnalogOutput::reset( void ) 
{ 
  //  cerr << " ComediAnalogOutput::reset()" << endl;/////TEST/////

  if ( !isOpen() )
    return NotOpen;
  if ( comedi_cancel( DeviceP, SubDevice ) < 0 )
    return WriteError;
  // XXX clear buffers!

  clearSettings();
  ErrorState = 0;
  IsPrepared = false;
  IsRunning = false;

  return 0;
}


bool ComediAnalogOutput::running( void ) const
{   
  return ( comedi_get_subdevice_flags( DeviceP, SubDevice ) & SDF_RUNNING );
}


int ComediAnalogOutput::error( void ) const
{
  return ErrorState;
  /*
    0: ok
    1: OverflowUnderrun
    2: Unknown (device error)
  */

}


void ComediAnalogOutput::take( const vector< AnalogOutput* > &aos,
			       vector< int > &aoinx )
{
  ComediAOs.clear();
  for ( unsigned int k=0; k<aos.size(); k++ ) {
    if ( aos[k]->analogOutputType() == ComediAnalogIOType &&
	 aos[k]->deviceFile() == deviceFile() ) {
      aoinx.push_back( k );
      ComediAOs.push_back( dynamic_cast< ComediAnalogOutput* >( aos[k] ) );
    }
  }
}


int ComediAnalogOutput::fillWriteBuffer( void )
{
  if ( !isOpen() )
    return -1;

  ErrorState = 0;
  bool failed = false;
  int elemWritten = 0;

  if ( (*Sigs)[0].deviceBufferMaxPop() <= 0 ) {
    //    (*Sigs).addErrorStr( "ComediAnalogOutput::writeData: " +
    //		      deviceFile() + " - buffer-underrun in outlist!" );
    //    (*Sigs).addError( DaqError::OverflowUnderrun );
    cerr << "ComediAnalogOutput::writeData: buffer-underrun in outlist!"  
	 << endl;/////TEST/////
    //    (*Sigs)[0].deviceBufferReset();/////TEST////
    return -1;/////TEST////
  }
  // try to write twice
  for ( int tryit = 0;
	tryit < 2 && !failed && (*Sigs)[0].deviceBufferMaxPop() > 0; 
	tryit++ ){
    
    int bytesWritten = write( comedi_fileno(DeviceP),
			      (*Sigs)[0].deviceBufferPopBuffer(),
			      (*Sigs)[0].deviceBufferMaxPop() * BufferElemSize );
    //    cerr << " ComediAnalogOutput::writeData():  bytes written:" << bytesWritten << endl;/////TEST/////

    if ( bytesWritten < 0 && errno != EAGAIN && errno != EINTR ) {
      (*Sigs).addErrorStr( errno );
      failed = true;
      cerr << " ComediAnalogOutput::writeData(): error" << endl;/////TEST/////
    }
    else if ( bytesWritten > 0 ) {
      (*Sigs)[0].deviceBufferPop( bytesWritten / BufferElemSize );
      elemWritten += bytesWritten / BufferElemSize;
    }

  }


  // no more data:
  if ( (*Sigs)[0].deviceBufferMaxPop() == 0 ) {
    IsRunning = false;
    if ( ! failed )
      return 0;
  }

  if ( failed || errno == EINTR )
    switch( errno ) {

    case EPIPE: 
      ErrorState = 1;
      (*Sigs).addErrorStr( deviceFile() + " - buffer-underrun: "
			+ comedi_strerror( comedi_errno() ) );
      (*Sigs).addError( DaqError::OverflowUnderrun );
      cerr << " ComediAnalogOutput::writeData(): buffer-underrun: "
	   << comedi_strerror( comedi_errno() ) << endl;/////TEST/////
      return -1;

    case EBUSY:
      ErrorState = 2;
      (*Sigs).addErrorStr( deviceFile() + " - device busy: "
			+ comedi_strerror( comedi_errno() ) );
      (*Sigs).addError( DaqError::Busy );
      cerr << " ComediAnalogOutput::writeData(): device busy: "
	   << comedi_strerror( comedi_errno() ) << endl;/////TEST/////
      return -1;

    default:
      ErrorState = 2;
      (*Sigs).addErrorStr( "Error while writing to device-file: " + deviceFile()
			+ "  comedi: " + comedi_strerror( comedi_errno() )
			+ "  system: " + strerror( errno ) );
      cerr << " ComediAnalogOutput::writeData(): buffer-underrun: "
	   << "  comedi: " << comedi_strerror( comedi_errno() ) 
	   << "  system: " << strerror( errno )
	
	   << endl;/////TEST/////
      (*Sigs).addError( DaqError::Unknown );
      return -1;
    }
  
  return elemWritten;
}


comedi_t* ComediAnalogOutput::comediDevice( void ) const
{
  return DeviceP;
}


int ComediAnalogOutput::comediSubdevice( void ) const
{
  if ( !isOpen() )
    return -1;
  return SubDevice;
}


int ComediAnalogOutput::bufferSize( void ) const
{
  if( !isOpen() )
    return -1;
  return comedi_get_buffer_size( DeviceP, SubDevice ) / BufferElemSize;
}


int ComediAnalogOutput::reload( void )
{
  cerr << " ComediAnalogOutput::reload()" << endl;/////TEST/////
  if ( !isOpen() )
    return -1;
  if ( loaded() )
    return 0;

  if ( !prepared() ||  comedi_command( DeviceP, &Cmd ) < 0 ) {
    cerr << "ComediAnalogOutput::reload()-> " << deviceFile()
	 <<" - execution of comedi_cmd failed: " << comedi_strerror( comedi_errno() );
    cerr << " ComediAnalogOutput::reload() -> ERROR!" << endl;/////TEST/////
    return WriteError;
  }
  return 0;
}


bool ComediAnalogOutput::loaded( void ) const 
{ 
  if ( !isOpen() )
    return false;
  return SDF_RUNNING & comedi_get_subdevice_flags( DeviceP, SubDevice );
}


bool ComediAnalogOutput::prepared( void ) const 
{ 
  return IsPrepared;
}


void ComediAnalogOutput::setRunning( void )
{
  IsRunning = true;
  ErrorState = 0;
}


}; /* namespace comedi */
