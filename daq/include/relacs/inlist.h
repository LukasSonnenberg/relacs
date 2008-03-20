/*
  inlist.h
  A container for InData

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

#ifndef _INLIST_H_
#define _INLIST_H_

#include <vector> 
#include <relacs/indata.h>
using namespace std;


/*!
\class InList
\author Jan Benda
\version 1.0
\brief A container for InData
*/

class InList
{

public:

    /*! Constructs an empty InList. */
  InList( void );
    /*! Constructs an InList containing the single InData \a data.
        \sa push( InData& ) */
  InList( InData &data );
    /*! Constructs an InList containing the single pointer \a data.
        Setting \a own to \c true transfers the ownership to the InList. 
        \sa add( InData* ) */
  InList( InData *data, bool own=false );
    /*! Copy constructor. */
  InList( const InList &il );
    /*! Destructor. */
  ~InList( void );

    /*! The number of InData in the InList. */
  int size( void ) const { return IL.size(); };
    /*! True if there are no InData contained in the InList. */
  bool empty( void ) const { return IL.empty(); };
    /*! Resize the InList such that it contains \a n InData.
        If \a n equals zero, clear() is called.
	If a larger size than the current size() is requested 
	than empty InData are appended, each of capacity \a m
	and sampling interval \a step seconds.
	\sa clear(), size(), empty(), reserve(), capacity() */
  void resize( int n, int m=0, double step=1.0 );
    /*! Clear the InList, i.e. remove all InData the InList owns. */
  void clear( void );

    /*! Maximum number of InData the InList can hold. */
  int capacity( void ) const { return IL.capacity(); }
    /*! Increase the capacity() of the InList to \a n.  */
  void reserve( int n );

    /*! Assignment. */
  InList &operator=( const InList &il );

    /*! Returns a const reference of the \a i -th InData of the list. */
  const InData &operator[]( int i ) const { return *(IL[i].ID); };
    /*! Returns a reference of the \a i -th InData of the list. */
  InData &operator[]( int i ) { return *(IL[i].ID); };

    /*! Returns a const reference to the first InData in the list. */
  const InData &front( void ) const;
    /*! Returns a reference to the first InData in the list. */
  InData &front( void );
    /*! Returns a const reference to the last InData in the list. */
  const InData &back( void ) const;
    /*! Returns a reference to the last InData in the list. */
  InData &back( void );

    /*! Returns a const reference of the InData element with
        identifier \a ident.
        \warning No "range checking" is performed.
        If there is no InData element with identifier \a ident
        a reference to the first element is returned. */
  const InData &operator[]( const string &ident ) const;
    /*! Returns a reference of the InData element with
        identifier \a ident.
        \warning No "range checking" is performed.
        If there is no InData element with identifier \a ident
        a reference to the first element is returned. */
  InData &operator[]( const string &ident );

    /*! Return the index of the input data trace with identifier \a ident.
        If there is no trace with this identifier -1 is returned. */
  int index( const string &ident ) const;

    /*! Copy \a data as a new element to the end of the list. */
  void push( InData &data );
    /*! Copy each trace from \a traces to the end of the list. */
  void push( const InList &traces );

    /*! Add the pointer \a data as a new element to the end of the list.
        If \a own is set to \c true then the ownership of \a data
        is transfered to the InList, i.e. the InList might delete it. */
  void add( InData *data, bool own=false );
    /*! Add the pointer \a data as a new element to the end of the list.
        If \a own is set to \c true then the ownership of \a data
        is transfered to the InList, i.e. the InList might delete it. */
  void add( const InData *data, bool own=false );
    /*! Add pointers to each trace in \a traces to the end of the list.
        If \a own is set to \c true then the ownership of the traces
        is transfered to the InList, i.e. the InList might delete it. */
  void add( const InList &traces, bool own=false );

    /*! Erase the InData at index \a index. */
  void erase( int index );

    /*! Clear buffer and reset indices of all InData traces. */
  void clearBuffer( void );

    /*! Sort the input traces by increasing channel number. */
  void sortByChannel( void );
    /*! Sort the input traces by increasing device and 
        by increasing channel number. */
  void sortByDeviceChannel( void );

    /*! Set the device id of all traces to \a device. */
  void setDevice( int device );
    /*! Set the reference of all input lines to \a ref.
        Possible values are Differential, Referenced, Nonreferenced.
        Defaults to Differential. */
  void setReference( InData::RefType ref );
    /*! Enable dither for all input lines if \a dither equals \c true,
        i.e. adding white noise to minimze quantization errors
        through averaging.
        By default dither is disabled. */
  void setDither( bool dither=true );
    /*! Set the polarity of all input traces to \a unipolar. 
        If \a unipolar is true only positive values are acquired,
	if it is false positive and negative values are acquired.
        By default acquisition is bipolar. */
  void setUnipolar( bool unipolar );
    /*! Set the source for the start trigger of the data aquisition
        for all traces to \a startsource. */
  void setStartSource( int startsource );
    /*! Set delay for all traces to \a delay (in seconds). */
  void setDelay( double delay );
    /*! Set the priority of all input traces to \a priority.
        If \a priority is \a true then the input trace is processed even
	if there still is a data acquisition running.
	Otherwise the input trace is not processed and returns with an error. */
  void setPriority( bool priority=true );
    /*! Set the sampling rate of all input traces to \a rate Hertz */
  void setSampleRate( double rate );
    /*! Set the sampling interval of all input traces to \a step seconds. */
  void setSampleInterval( double step );
    /*! Set continuous mode of data aquisition for all traces to \a continuous. */
  void setContinuous( bool continuous=true );
    /*! Set the scale factor for all input traces to \a scale.
	The scale factor \a scale is used to scale the voltage data to
	a secondary unit.
        \sa setOffset(), setUnit() */
  void setScale( double scale );
    /*! Set the offset that is added to the scaled voltage data 
        for all input traces to \a offset.
        \sa  setScale(), setUnit() */
  void setOffset( double offset );
    /*! Set the secondary unit for all input traces  to \a unit.
        \sa setScale(), setOffset() */
  void setUnit( const string &unit );
    /*! Set the specifications of a secondary unit for all input traces.
	First, the voltage data are scaled by \a scale.
	Then \a offset is added to get the data in the secondary unit \a unit.
        \sa setScale(), setOffset() */
  void setUnit( double scale, double offset, const string &unit );
    /*! Set the maximum time the hardware driver should buffer the data
        before they are written into the InData buffers to \a time seconds. */
  void setUpdateTime( double time );

    /*! Clear all mode flags for all input traces. 
        \sa setMode(), addMode(), delMode() */
  void clearMode( void );
    /*! Set mode flags for all input traces to \a flags. 
        \sa clearMode(), addMode(), delMode() */
  void setMode( int flags );
    /*! Add the bits specified by \a flags to the mode flags
        of all input traces. 
        \sa clearMode(), setMode(), delMode() */
  void addMode( int flags );
    /*! Clear the bits specified by \a flags of the mode flags
        of all input traces. 
        \sa clearMode(), setMode(), addMode() */
  void delMode( int flags );

    /*! Set index of start of last signal to restart() + \a index
        in all input traces. \sa setSignalTime() */
  void setSignalIndex( int index );
    /*! Set time of start of last signal to \a time
        in all input traces. \sa setSignalIndex() */
  void setSignalTime( double time );
    /*! Set restart-index of all input traces to current size(). 
        \sa restartIndex(), restartTime() */
  void setRestart( void );

    /*! Return string with an error message. */
  string errorText( void ) const;

    /*! Clear all error flags of all input traces. */
  void clearError( void );
    /*! Set error flags of all input traces to \a flags. */
  void setError( long long flags );
    /*! Add the bits specified by \a flags to the error flags
        of all input traces. */
  void addError( long long flags );
    /*! Clear the bits specified by \a flags of the error flags
        of all input traces. */
  void delError( long long flags );
    /*! Add error code \a de originating from daq board to the error flags
        of all input traces. */
  void addDaqError( int de );
    /*! Set additional error string of all input traces to \a strg. */
  void setErrorStr( const string &strg );
    /*! Add \a msg to the additional error string of all input traces. */
  void addErrorStr( const string &strg );
    /*! Set additional error string of all input traces
        to the string describing the 
        standard C error code \a errnum (from \c errno). */
  void setErrorStr( int errnum );
    /*! Add the string describing the standard C error code \a errnum 
        (from \c errno) to the additional error string of all input traces. */
  void addErrorStr( int errnum );
    /*! Returns \c true if all input traces are ok. */
  bool success( void ) const;
    /*! Returns \c true if one or more input traces failed. */
  bool failed( void ) const;

    /*! Write content of all InData variables to stream \a str
        (for debugging only). */
  friend ostream &operator<< ( ostream &str, const InList &il );

    /*! Free the internal buffers that hold the data in a device dependend
        multiplexed format of all input traces. */
  void freeDeviceBuffer( void );
    /*! Clear the internal buffers that hold the data in a device dependend
        multiplexed format of all input traces. */
  void clearDeviceBuffer( void );


 private:

  struct ILE {
    ILE( void ) : ID( NULL ), Own( false ) {};
    ILE( InData *id, bool own ) : ID( id ), Own( own ) {};
    const ILE &operator=( const ILE &ole ) 
    { if ( &ole == this ) return *this; ID = ole.ID; Own = ole.Own; return *this; };
    InData *ID;
    bool Own;
  };

  vector< ILE > IL;

  friend bool lessChannelILE( const ILE &a, const ILE &b );
  friend bool lessDeviceChannelILE( const ILE &a, const ILE &b );

};


#endif