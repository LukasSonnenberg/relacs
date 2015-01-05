/*
  base/highpass.h
  A simple first order high pass filter

  RELACS - Relaxed ELectrophysiological data Acquisition, Control, and Stimulation
  Copyright (C) 2002-2015 Jan Benda <jan.benda@uni-tuebingen.de>

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

#ifndef _RELACS_BASE_HIGHPASS_H_
#define _RELACS_BASE_HIGHPASS_H_ 1

#include <relacs/optwidget.h>
#include <relacs/indata.h>
#include <relacs/filter.h>
using namespace relacs;

namespace base {


/*! 
\class HighPass
\brief [Filter] A simple first-order high-pass filter
\author Jan Benda

The input \a x(t) is filtered with the ordinary differential equation
\f[ \tau \frac{dy}{dt} = x - y \f]
to result in the low-pass filtered output \a y(t).
The output of the high-pass filter is then
the original signal minus the low-pass filtered signal: \a x(t)-y(t) .
The cut-off frequency of the filter is at
\f[ f_c = \frac{1}{2 \pi \tau} \f]

Add the high-pass filter with the following lines to a \c relacs.cfg %file:
\verbatim
*FilterDetectors
  Filter1
        name: HV-1
      filter: HighPass
  inputtrace: V-1
        save: false
        plot: true
  buffersize: 500000
\endverbatim

\par Options
- \c tau=1ms: Time constant (\c number)

\version 0.2 (May 12 2012)
*/


class HighPass : public Filter
{
  Q_OBJECT

public:

    /*! The constructor. */
  HighPass( const string &ident="", int mode=0 );
    /*! The destructor. */
  ~HighPass( void );

  virtual int init( const InData &indata, InData &outdata );
  virtual int adjust( const InData &indata, InData &outdata );
  virtual void notify( void );
  virtual int filter( const InData &indata, InData &outdata );


protected:

  OptWidget LFW;

  double Tau;

  double DeltaT;
  double TFac;
  InDataIterator Index;
  float X;

};


}; /* namespace base */

#endif /* ! _RELACS_BASE_HIGHPASS_H_ */
