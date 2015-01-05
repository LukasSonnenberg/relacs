/*
  defaultrepro.h
  Does nothing

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

#ifndef _RELACS_DEFAULTREPRO_H_
#define _RELACS_DEFAULTREPRO_H_ 1


#include <relacs/repro.h>

namespace relacs {


/*! \class DefaultRePro
    \author Jan Benda
    \version 2.0
    \brief Does nothing
*/


class DefaultRePro : public RePro
{
  Q_OBJECT

public:

    /*! Constructs a DefaultRePro-RePro: intialize widgets and create options. */
  DefaultRePro( void );
    /*! Deconstructs a DefaultRePro-RePro. */
  ~DefaultRePro( void );

    /*! Read options, create stimulus and start the output of the search stimuli. */
  int main( void );


protected:

  double Duration;

};


}; /* namespace relacs */

#endif /* ! _RELACS_DEFAULTREPRO_H_ */

