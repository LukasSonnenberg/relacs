/*
  voltageclamp/slowinactivation.h
  Slow inactivation of sodium channels with activation curve and time constant

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

#ifndef _RELACS_VOLTAGECLAMP_SLOWINACTIVATION_H_
#define _RELACS_VOLTAGECLAMP_SLOWINACTIVATION_H_ 1

#include <relacs/multiplot.h>
#include <relacs/repro.h>
#include <relacs/ephys/traces.h>
#include <relacs/str.h>
#include <relacs/voltageclamp/summary.h>
#include <relacs/voltageclamp/pnsubtraction.h>
using namespace relacs;

namespace voltageclamp {


/*!
\class SlowInactivation
\brief [RePro] Slow inactivation of sodium channels with activation curve and time constant
\author Lukas Sonnenberg
\version 1.0 (Jul 25, 2020)
*/


class SlowInactivation : public PNSubtraction
{
  Q_OBJECT

friend class Summary;

public:

  SlowInactivation( void );
  virtual int main( void );

private:

    std::vector<double> getMinimas( void );

protected:

    MultiPlot P;

};


}; /* namespace voltageclamp */

#endif /* ! _RELACS_VOLTAGECLAMP_SLOWINACTIVATION_H_ */
