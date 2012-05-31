/*
  daqflex/daqflexanaloginput.h
  Interface for accessing analog input of a DAQFlex board from Measurement Computing.

  RELACS - Relaxed ELectrophysiological data Acquisition, Control, and Stimulation
  Copyright (C) 2002-2012 Jan Benda <benda@bio.lmu.de>

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

#ifndef _DAQFLEX_DAQFLEXANALOGINPUT_H_
#define _DAQFLEX_DAQFLEXANALOGINPUT_H_

#include <relacs/daqflex/daqflexcore.h>
#include <relacs/analoginput.h>
using namespace std;
using namespace relacs;

namespace daqflex {


/*! 
\class DAQFlexAnalogInput
\author Jan Benda
\brief [AnalogInput] Interface for accessing analog input of a DAQFlex board from Measurement Computing.
*/


class DAQFlexAnalogInput : public AnalogInput
{

public:

    /*! Create a new DAQFlexAnalogInput without opening a device. */
  DAQFlexAnalogInput( void );
    /*! Open the analog input driver specified by its device file \a device. */
  DAQFlexAnalogInput( DAQFlexCore &device, const Options &opts );
    /*! Stop analog input and close the daq driver. */
  virtual ~DAQFlexAnalogInput( void );

    /*! Open analog input on DAQFlexCore device \a device. */
  virtual int open( DAQFlexCore &daqflexdevice, const Options &opts );
    /*! Open analog input on DAQFlexCore device \a device. */
  virtual int open( Device &device, const Options &opts );
    /*! Returns true if driver was succesfully opened. */
  virtual bool isOpen( void ) const;
    /*! Stop all activity and close the device. */
  virtual void close( void );

    /*! Number of analog input channels. */
  virtual int channels( void ) const;
    /*! Resolution in bits of analog input. */
  virtual int bits( void ) const;
    /*! Maximum sampling rate in Hz of analog input. */
  virtual double maxRate( void ) const;

    /*! Maximum number of analog input ranges. */
  virtual int maxRanges( void ) const;
    /*! Voltage range \a index in Volt for unipolar mode.
        If -1 is returned this range is not supported. */
  virtual double unipolarRange( int index ) const;
    /*! Voltage range \a index in Volt for bipolar mode.
        If -1 is returned this range is not supported. */
  virtual double bipolarRange( int index ) const;

    /*! Prepare analog input of the input traces \a traces on the device.
	If an error ocurred in any trace, the corresponding errorflags in
	InData are set and a negative value is returned.
	This function assumes that \a traces successfully passed testRead().
        The channels in \a traces are not sorted. */
  virtual int prepareRead( InList &traces );
    /*! Start analog input of the input traces on the device
        after they were prepared by prepareRead().
	If an error ocurred in any channel, the corresponding errorflags in the
	InData structure are filled and a negative value is returned.
	Also start possible pending acquisition on other devices
	that are known from take(). */
  virtual int startRead( void );
    /*! Read data from a running data acquisition.
        Returns the total number of read data values.
	If an error ocurred in any channel, the corresponding errorflags in the
	InList structure are filled and a negative value is returned. */
  virtual int readData( void );
    /*! Convert data from and push them to the traces.
        Returns the number of new data values that were added to the traces
	(sum over all traces).
	If an error ocurred in any channel, the corresponding errorflags in the
	InList structure are filled and a negative value is returned. */
  virtual int convertData( void );

    /*! Stop any running ananlog input activity on the device.
        Returns zero on success, otherwise one of the flags 
        NotOpen, InvalidDevice, ReadError.
        \sa close(), open(), isOpen() */
  virtual int stop ( void );
    /*! Stop any running ananlog input activity and reset the device.
        Returns zero on success, otherwise one of the flags 
        NotOpen, InvalidDevice, ReadError.
        \sa close(), open(), isOpen() */
  virtual int reset( void );
  
    /*! True if the analog input driver is running. */
  virtual bool running( void ) const;

    /*! Get error status of the device. 
        0: no error
	-1: input buffer overflow
        other: unknown */
  virtual int error( void ) const;


protected:

    /*! Device driver specific tests on the settings in \a sigs
        for each input channel.
	Before this function is called, the validity of the settings in 
	\a sigs was already tested by testReadData().
	This function should test whether the settings are really supported
	by the hardware.
	If an error ocurred in any trace, the corresponding errorflags in the
	InData are set and a negative value is returned.
        The channels in \a sigs are not sorted.
        This function is called by testRead(). */
  int testReadDevice( InList &traces );


private:

    /*! Unique analog I/O device type id for all 
        DAQFlex devices. */
  static const int DAQFlexAnalogIOType = 2;

    /*! The DAQFlex device. */
  DAQFlexCore *DAQFlexDevice;

    /*! Holds the list of supported bipolar ranges. */
  vector< double > BipolarRange;\

  struct Calibration {
    double Offset;
    double Slope;
  };

    /*! True if the command is prepared. */
  bool IsPrepared;

    /*! True if a command is supposed to be running.
        \note this differs from running(), which indicated that the driver is still running. */
  bool IsRunning;

  int ErrorState;

    /*! The input traces that were prepared by prepareRead(). */
  InList *Traces;
    /*! Size of the driver buffer used for getting the data from the daq board. */
  int ReadBufferSize;
    /*! Size of the internal buffer used for getting the data from the driver. */
  int BufferSize;
    /*! The number of samples written so far to the internal buffer. */
  int BufferN;
    /*! The internal buffer used for getting the data from the driver. */
  char *Buffer;
    /*! Index to the trace in the internal buffer. */
  int TraceIndex;

    /*! The total number of samples to be acquired, 0 for continuous acquisition. */
  int TotalSamples;
    /*! The number of samples so far read in by readData(). */
  int CurrentSamples;

};


}; /* namespace comedi */

#endif /* ! _DAQFLEX_DAQFLEXANALOGINPUT_H_ */