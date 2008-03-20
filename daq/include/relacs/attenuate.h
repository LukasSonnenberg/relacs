/*
  attenuate.h
  Attenuates a single output channel.

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

#ifndef _ATTENUATE_H_
#define _ATTENUATE_H_


#include <relacs/device.h>
#include <relacs/attenuator.h>
using namespace std;


/*!
\class Attenuate
\author Jan Benda
\version 1.2
\brief Attenuates a single output channel.

This class is an interface for attenuating a single output line.
It allows to convert a requested intensity into an attenuation level
for the attenuator device. The carrier frequency of the output signal
can be used as a parameter for the conversion.
You have to reimplement decibel() and intensity()
with the required transformation of the intensity into an attenuation level.
init() is called before the attenuator is used.
You can reimplement this function to load calibration data
from a file, for example.
save() may be called to save some (calibration) data into a file.

Via a constructor or the open()-function a specific attenuation device
for a single output line is assigned to the Attenuate-class.
The isOpen()-function checks whether the attenuator is valid and opened.
With clear() Attenuate can be disconnected from the attenuator.
close() additionally closes the associated attenuator.
The aoChannel() function returns the number of the channel 
of the analog output device aoDevice() where the attenuator is connected to.

The attenuation is set by requesting an intensity using write().
This intensity is mapped into an attenuation level in decibel
by the function decibel().
The intensity can be any quantity which is most usefull for your application.
What exactly the intensity specifies is determined by the implementation
of the decibel() function.
The decibel() function gets as a second argument a frequency.
This second argument can be used as an parameter for the conversion from
intensites to attenuation levels.
For example, if your output generates a sound wave via a loadspeaker,
the necessary attenuation for getting a specific sound intensity depends on the
carrierfrequency of your signal.

Attenuators usually can be set to discrete attenuation levels only.
If you request a specific intensity via write(), than it is very likely
that the actually set attenuation level results in a slightly different
intensity.
To make this intensity known to the user, write() sets the 
\a intensity variable to the actually set intensity.
To be able to compute the intensity from the attenuation level you have to
implement the intensity()-function as the inverse function of decibel().

To check whether a requested intensity is possible and what intensity would
be set without setting the attenuation level,
you can use the testWrite()-function.

The mute()-function can be used to mute the output line.
testMute() checks whether muting of the output line is possible.

Possible return values of write(), testWrite() and mute() are:
- \c 0: success
- \c NotOpen: The device driver for the attenuator is not open.
- \c InvalidDevice: An invalid device index is requested, i.e.
     the requested output line is not supported by the attenuator device.
- \c WriteError: Failed in setting the attenuation level.
- \c Underflow: The requested attenuation level is too high,
     i.e. the requested signal amplitude is too small.
- \c Overflow: The requested attenuation level is too low,
     i.e. the requested signal amplitude is too large.
- \c IntensityUnderflow: The requested intensity is too small
     to be converted into an attenuation level by the decibel()-function.
- \c IntensityOverflow: The requested intensity is too large
     to be converted into an attenuation level by the decibel()-function.
- \c IntensityError: Another error occured during conversion of
     an intensity into an attenuation level by the decibel()-function.
*/


class Attenuate : public Device
{

    /*! Device type id for Attenuate devices. */
  static const int Type = 4;


public:

    /*! Constructor. */
  Attenuate( void );
    /*! Construct Attenuate with device class \a deviceclass. 
        \sa setDeviceClass() */
  Attenuate( const string &deviceclass );
    /*! Destructor. */
  virtual ~Attenuate( void );

    /*! Assign the output line \a index of the  attenuator \a att
	to the attenuator wrapper.
	Returns zero on success, or InvalidDevice (or any other negative number
	indicating the error).
        \sa isOpen(), init(), setAODevice(), setAOChannel() */
  virtual int open( Device &att, long index=0 );
    /*! Does nothing (needed for compatibility to other devices). */
  virtual int open( const string &device, long index=0 );
     /*! True if the hardware driver is open and the device index is supported.
        \sa open() */
  virtual bool isOpen( void ) const;
    /*! Close the attenuator wrapper and the associated
        attenuator device.
        \sa clear(), open(), isOpen() */
  virtual void close ( void );
    /*! Close the attenuator wrapper without closing the associated
        attenuator device.
        \sa close(), open(), isOpen() */
  void clear ( void );

    /*! Returns a string with some information about the Attenuate device. */
  virtual string info( void ) const;
    /*! Returns a string with the current settings of the Attenuate device. */
  virtual string settings( void ) const;

  /*! This function is called after an attenuator is assigned to this class
      and before the attenuator is used.
      The default implementation does nothing.
      However, you can reimplement this function to load calibration data,
      for example.
      \sa open() */
  virtual void init( void );

  /*! Save some data into a file in directory \a path.
      The default implementation does nothing.
      However, you can reimplement this function to save calibration data,
      for example. */
  virtual void save( const string &path ) const;

    /*! Set intensity to \a intensity.
        The parameter \a frequency may be used for calculating the right
	attenuation level.
	In most cases \a frequency might be
	the carrier frequency of the signal.
	The function decibel() is used to calculate the attenuation level.
	Since attenuators have a certain resolution, 
	the actually set intensity may differ from the requested
	intensity \a intensity.
	The set intensity is returned in \a intensity.
	If you want to mute the output line, call mute().
	\return
	- 0 on success
	- \c NotOpen: The device driver for the attenuator is not open.
	- \c InvalidDevice: An invalid device index is requested, i.e.
	  the requested output line is not supported by the attenuator device.
	- \c WriteError: Failed in setting the attenuation level.
	- \c Underflow: The requested attenuation level is too high,
	  i.e. the requested signal amplitude is too small.
	  In this case, the minimum possible \a intensity is returned.
	- \c Overflow: The requested attenuation level is too low,
	  i.e. the requested signal amplitude is too large.
	  In this case, the maximum possible \a intensity is returned.
	- \c IntensityUnderflow: The requested intensity is too small
 	  to be converted into an attenuation level by the decibel()-function.
	  In this case, the minimum possible \a intensity is returned.
	- \c IntensityOverflow: The requested intensity is too large
	  to be converted into an attenuation level by the decibel()-function.
	  In this case, the maximum possible \a intensity is returned.
	- \c IntensityError: Another error occured during conversion of
	  an intensity into an attenuation level by the decibel()-function.
	\sa testWrite(), mute()
    */
  int write( double &intensity, double frequency );
    /*! Does the same as write() except setting the attenuator.
        Using this function it can be checked whether the intensities are valid
	and to what value the intensity will be actually adjusted.
	\return
	- 0 on success
	- \c NotOpen: The device driver for the attenuator is not open.
	- \c InvalidDevice: An invalid device index is requested, i.e.
	  the requested output line is not supported by the attenuator device.
	- \c Underflow: The requested attenuation level is too high,
	  i.e. the requested signal amplitude is too small.
	  In this case, the minimum possible \a intensity is returned.
	- \c Overflow: The requested attenuation level is too low,
	  i.e. the requested signal amplitude is too large.
	  In this case, the maximum possible \a intensity is returned.
	- \c IntensityUnderflow: The requested intensity is too small
 	  to be converted into an attenuation level by the decibel()-function.
	  In this case, the minimum possible \a intensity is returned.
	- \c IntensityOverflow: The requested intensity is too large
	  to be converted into an attenuation level by the decibel()-function.
	  In this case, the maximum possible \a intensity is returned.
	- \c IntensityError: Another error occured during conversion of
	  an intensity into an attenuation level by the decibel()-function.
	\sa write(), testMute()
    */
  int testWrite( double &intensity, double frequency ); 

    /*! Mute the output channel.
	\return
	- 0 on success
	- \c NotOpen: The device driver for the attenuator is not open.
	- \c InvalidDevice: An invalid device index is requested, i.e.
	  the requested output line is not supported by the attenuator device.
	- \c WriteError: Failed in setting the attenuation level.
        \sa testMute(), write(), testWrite()
     */
  int mute( void );
    /*! Test muting the output channel.
	\return
	- 0 on success
	- \c NotOpen: The device driver for the attenuator is not open.
	- \c InvalidDevice: An invalid device index is requested, i.e.
	  the requested output line is not supported by the attenuator device.
	- \c WriteError: Failed in setting the attenuation level.
        \sa mute(), write(), testWrite()
     */
  int testMute( void );

    /*! Returns the channel number of the analog output device
        which is attenuated by this instance of the Attenuate class.
        \sa setAOChannel(), aoDevice(), open() */
  int aoChannel( void ) const;
    /*! Set the channel number for the analog output device
        which should be attenuated by this
        instance of the Attenuate class to \a channel.
        \sa aoChannel(), setAODevice(), open() */
  void setAOChannel( int channel );

    /*! Returns the identifier string of the analog output device
        which is attenuated by this instance of the Attenuate class.
        \sa setAODevice(), aoChannel(), open() */
  string aoDevice( void ) const;
    /*! Set the identifier string for the analog output device
        which should be attenuated by this
        instance of the Attenuate class to \a channel.
        \sa aoDevice(), setAOChannel(), open() */
  void setAODevice( const string &deviceid );

    /*! Return code indicating that the device driver is not opened. */
  static const int NotOpen = Attenuator::NotOpen;
    /*! Return code indicating an invalid output line of the attenuator. */
  static const int InvalidDevice = Attenuator::InvalidDevice;
    /*! Return code indicating a failure in reading
        the attenuation level from the device. */
  static const int ReadError = Device::ReadError;
    /*! Return code indicating a failure in writing
        the attenuation level to the device. */
  static const int WriteError = Device::WriteError;
    /*! Return code indicating a too high requested attenuation level,
        i.e. the requested signal amplitude is too small. */
  static const int Underflow = Attenuator::Underflow;
    /*! Return code indicating a too low requested attenuation level,
        i.e. the requested signal amplitude is too large. */
  static const int Overflow = Attenuator::Overflow;

    /*! Return code indicating an underflow in calculating the level,
        i.e. the requested intensity is too small. */
  static const int IntensityUnderflow = -7;
    /*! Return code indicating an overflow in calculating the level,
        i.e. the requested intensity is too large. */
  static const int IntensityOverflow = -8;
    /*! Return code indicating an unspecific error in calculating the level. */
  static const int IntensityError = -9;


protected:

    /*! Transforms the requested intensity \a intensity
        for the carrier frequency \a frequency of the signal into 
	an attenuation level \a db for the attenuator.
	This function is used by write() to set the attenuator
	to the requested intensity.
	If the computation of \a db fails, \a db should be set to a
        meaningful value.
	\return
	- 0 on success
	- \c IntensityUnderflow: The requested intensity is too small
 	  to be converted into an attenuation level.
	  In this case, the attenuation \a db corresponding to the
	  minimum possible intensity is returned.
	- \c IntensityOverflow: The requested intensity is too large
	  to be converted into an attenuation level.
	  In this case, the attenuation \a db corresponding to the
	  maximum possible intensity is returned.
	- \c IntensityError: Another error occured during conversion of
	  an intensity into an attenuation level.
	\sa write(), testWrite(), intensity()
    */
  virtual int decibel( double intensity, double frequency, double &db )=0;
    /*! Transform the attenuation level \a decibel
        for the carrier frequency \a frequency of the signal into
        the intesity \a intens.
	This function should be the inverse function of decibel()
	and is used by write() to return the actually set
	intensity from the set attenuation level.
	\sa write(), testWrite(), decibel()
    */
  virtual void intensity( double &intens, double frequency, double decibel )=0;


protected:

    /*! A string describing the current settings of the Attenuate device.
        \sa settings() */
  string Settings;


private:

    /*! The attenuator device. */
  Attenuator *Att;
    /*! The device index for \a Att. */
  int Index;

    /*! The device identifier of the analog output device 
        which is attenuated by this instance of the Attenuate class. */
  string AODevice;
    /*! The channel number of the analog output device
        which is attenuated by this instance of the Attenuate class. */
  int AOChannel;
};

#endif