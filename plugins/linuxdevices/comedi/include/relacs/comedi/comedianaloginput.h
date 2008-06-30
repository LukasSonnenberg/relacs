/*
  comedi/comedianaloginput.h
  Interface for accessing analog input of a daq-board via comedi.

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

#ifndef _COMEDI_COMEDIANALOGINPUT_H_
#define _COMEDI_COMEDIANALOGINPUT_H_

#include <comedilib.h>
#include <vector>
#include <relacs/analoginput.h>
using namespace std;
using namespace relacs;

namespace comedi {


class ComediAnalogOutput;

/*! 
\class AnalogInput
\author Marco Hackenberg
\version 0.1
\brief Interface for accessing analog input of a daq-board via comedi.
\bug fix errno usage
\todo support delays in testReadDevice() and convertData()!
\todo Fix usage of ErrorState variable (also in readData() )
\todo Fix usage of IsRunning, IsLoaded, and IsPrepared

insmod /usr/src/kernels/rtai/modules/rtai_hal.ko
insmod /usr/src/kernels/rtai/modules/rtai_ksched.ko
modprobe kcomedilib
modprobe ni_pcimio
sleep 1
/usr/src/comedilib/comedi_config/comedi_config /dev/comedi0 ni_pcimio
sleep 1
/usr/src/comedilib/comedi_calibrate/comedi_calibrate /dev/comedi0

*/


class ComediAnalogInput : public AnalogInput
{

  friend class ComediAnalogOutput;
  friend class DynClampAnalogInput;

public:

    /*! Create a new ComediAnalogInput without opening a device. */
  ComediAnalogInput( void );
    /*! Open the analog input driver specified by its device file \a device. */
  ComediAnalogInput( const string &device, long mode=0 );
    /*! Stop analog input and close the daq driver. */
  virtual ~ComediAnalogInput( void );

    /*! Open the analog input device on device file \a device.
        \todo  if a ranges is not supported but comedi thinks so: set max = -1.0
        i.e. NI 6070E PCI: range #8 (0..20V) not supported
        \todo do we need to set the file descriptor to O_NONBLOCK? 
        \todo maybe use an internal maximum buffer size (in case comedi max is way too much)? */
  virtual int open( const string &device, long mode=0 );
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
    /*! Start analog input of the input traces \a traces on the device
        after they were prepared by prepareRead().
	If an error ocurred in any channel, the corresponding errorflags in the
	InData structure are filled and a negative value is returned.
        The channels in \a traces are not sorted.
	Also start possible pending acquisition on other devices
	that are known from take(). */
  virtual int startRead( InList &sigs );
    /*! Read data from a running data acquisition.
        Returns the number of new data values that were added to the \a traces
	(sum over all \a traces).
	If an error ocurred in any channel, the corresponding errorflags in the
	InList structure are filled and a negative value is returned. */
  virtual int readData( InList &sigs );

    /*! Stop any running ananlog input activity on the device.
        Returns zero on success, otherwise one of the flags 
        NotOpen, InvalidDevice, ReadError.
        \sa close(), open(), isOpen() */
  virtual int stop ( void );
    /*! Stop any running ananlog input activity and reset the device.
        Returns zero on success, otherwise one of the flags 
        NotOpen, InvalidDevice, ReadError.
        \sa close(), open(), isOpen()
        \todo clear buffers! */
  virtual int reset( void );
  
    /*! True if analog input is running. */
  virtual bool running( void ) const;

    /*! Get error status of the device. 
        0: no error
	-1: input buffer overflow
        other: unknown */
  virtual int error( void ) const;

    /*! Check for every analog input and analog output device in \a ais
        and \a aos, respectively,
        whether it can be simultaneously started by startRead()
        from this device.
        Add the indices of those devices to \a aiinx and \a aoinx,
        respectively. 
	\todo needs to be implemented. */
  virtual void take( const vector< AnalogInput* > &ais,
		     const vector< AnalogOutput* > &aos,
		     vector< int > &aiinx, vector< int > &aoinx );


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
        This function is called by testRead().
        \todo does testing work on running devices? 
	\todo check if setting of sampling rate is correct for acquisition
	from two or more channels
	\todo maybe put the second half into prepareRead,
	since it is called there anyways.
	\todo analyse errors from test command */
  int testReadDevice( InList &traces );

    /*! Execute the command that was prepared by prepareRead(). */
  int executeCommand( void );

    /*! Returns the pointer to the comedi device file.
        \sa subdevice() */
  comedi_t* comediDevice( void ) const;
    /*! Comedi internal index of analog input subdevice. */
  int comediSubdevice( void ) const;

    /*! returns buffer-size of device in samples. */
  int bufferSize( void ) const;
  
    /* Reloads the prepared configuration commands of the following acquisition 
       into the registers of the hardware after stop() was performed.
       Deletes all data values that a still available via readData().
       For internal usage.
       \sa stop(), readData(), prepareRead() */
  int reload( void );
    /* True, if configuration command for acquisition is successfully loaded
       into the registers of the hardware.
       For internal usage.
       \sa running(), reload() */
  bool loaded( void ) const;
    /*! True if analog input was prepared using testReadDevice() and prepareRead() */
  bool prepared( void ) const;

    /* Sets the running status and unsets the prepared status. For internal 
       usage. */
  void setRunning( void );


private:

    /*! Unique analog I/O device type id for all 
        Comedi DAQ devices. */
  static const int ComediAnalogIOType = 1;

  int ErrorState;
  bool IsPrepared;
  mutable bool IsRunning;

    /*! Pointer to the comedi device. */
  comedi_t *DeviceP;
    /*! The comedi subdevice number. */
  unsigned int SubDevice;
    /*! True if the sample type is lsampl_t. */
  bool LongSampleType;
    /*! The size of a single sample in bytes. */
  unsigned int BufferElemSize;  
    /*! The maximum sampling rate supported by the DAQ board. */
  double MaxRate;

  comedi_cmd Cmd;
  unsigned int ChanList[512];
  lsampl_t InsnData[1];

    /*! Holds the list of supported unipolar comedi ranges. */
  vector< comedi_range > UnipolarRange;
    /*! Holds the list of supported bipolar comedi ranges. */
  vector< comedi_range > BipolarRange;
    /*! Maps unipolar range indices to comei range indices. */
  vector< unsigned int > UnipolarRangeIndex;
    /*! Maps bipolar range indices to comei range indices. */
  vector< unsigned int > BipolarRangeIndex;

    /*! List of analog input subdevices that can be 
        started via an instruction list together with this subdevice. */
  vector< ComediAnalogInput* > ComediAIs;
    /*! List of analog output subdevices that can be 
        started via an instruction list together with this subdevice. */
  vector< ComediAnalogOutput* > ComediAOs;

};


}; /* namespace comedi */

#endif /* ! _COMEDI_COMEDIANALOGINPUT_H_ */
