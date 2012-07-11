/*
  efish/punitmodel.cc
  A model for P-units of weakly-electric fish.

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

#include <cmath>
#include <relacs/attenuator.h>
#include <relacs/optwidget.h>
#include <relacs/random.h>
#include <relacs/odealgorithm.h>
#include <relacs/efish/punitmodel.h>
using namespace relacs;

namespace efish {


PUnitModel::PUnitModel( void )
  : NeuronModels( "PUnitModel", "efish", "Jan Benda", "1.0", "Nov 27, 2009" )
{
  // EOD:
  EODFreq = 800.0;
  EODFreqSD = 10.0;
  EODAmpl1 = 1.0;
  EODAmpl2 = 1.0;
  EODFreqTau = 1000.0;
  SignalFac = 1.0;
  // Spikes:
  VoltageScale = 1.0;

  // options:
  addLabel( "General" ).setStyle( OptWidget::TabLabel );
  addLabel( "EOD" );
  addNumber( "eodfreq", "Frequency", EODFreq, 0.0, 2000.0, 10.0, "Hz" );
  addNumber( "eodfreqsd", "SD of frequency", EODFreqSD, 0.0, 1000.0, 2.0, "Hz" );
  addNumber( "eodfreqtau", "Timescale of frequency", EODFreqTau, 0.5, 100000.0, 0.5, "s" );
  addNumber( "eodampl1", "Amplitude 1", EODAmpl1, 0.0, 100.0, 0.1, "mV/cm" );
  addNumber( "eodampl2", "Amplitude 2", EODAmpl2, 0.0, 100.0, 0.1, "mV/cm" );
  addNumber( "sigfac", "Factor for signal", SignalFac, 0.0, 100000.0, 0.1 );
  addLabel( "Spikes" );
  addNumber( "voltagescale", "Scale factor for membrane potential", VoltageScale, 0.0, 100.0, 0.1 );

  addOptions();

  addModels();

  addTypeStyle( OptWidget::Bold, Parameter::Label );
}


PUnitModel::~PUnitModel( void )
{
}


void PUnitModel::main( void )
{
  // eod:
  EODFreq = 0.001*2.0*M_PI*number( "eodfreq" );
  EODFreqSD = 0.001*2.0*M_PI*number( "eodfreqsd" );
  EODFreqTau = 1000.0*number( "eodfreqtau" );
  EODAmpl1 = number( "eodampl1" );
  EODAmpl2 = number( "eodampl2" ) / EODAmpl1;
  SignalFac = number( "sigfac" );
  VoltageScale = number( "voltagescale" );

  int sigdimension = 2;

  // spiking neuron and general integration options:
  readOptions();

  // deltat( 0 ) must be integer multiple of delta t for integration:
  int maxs = int( ::floor( 1000.0*deltat( 0 )/timeStep() ) );
  if ( maxs <= 0 )
    maxs = 1;
  setTimeStep( 1000.0 * deltat( 0 ) / maxs );
  int cs = 0;
  setNoiseFac();

  // OU normalisation factor for noise term:
  EODFreqFac = ::sqrt( 2.0*EODFreqTau/timeStep());

  // state variables:
  int simn = sigdimension + neuron()->dimension();
  if ( GMC > 1e-8  ) {
    MMCInx = simn;
    simn++;
  }
  else
    MMCInx = -1;
  if ( GMHC > 1e-8  ) {
    MMHCInx = simn;
    simn++;
    HMHCInx = simn;
    simn++;
  }
  else {
    MMHCInx = -1;
    HMHCInx = -1;
  }
  double simx[simn];
  double dxdt[simn];
  for ( int k=0; k<simn; k++ )
    simx[k] = dxdt[k] = 0.0;
  neuron()->init( simx+sigdimension );

  // equilibrium:
  for ( int c=0; c<100; c++ ) {
    double t = c * timeStep();
    (*neuron())( t, 0.0, simx+sigdimension, dxdt+sigdimension, neuron()->dimension() );
    for ( int k=sigdimension; k<simn; k++ )
      simx[k] += timeStep()*dxdt[k];
  }

  // integrate:
  double t = 1000.0*time( 0 );  // time must be syncrhonous to recorded trace!
  while ( ! interrupt() ) {

    Integrate( t, simx, dxdt, simn, timeStep(), *this );

    cs++;
    if ( cs == maxs ) {
      push( 0, VoltageScale*simx[sigdimension] );
      push( 1, EOD1 );
      push( 2, EOD2 );
      push( 3, Signal );
      cs = 0;
    }

    t += timeStep();
  }
}


void PUnitModel::process( const OutData &source, OutData &dest )
{
  dest = source;
  double intensfac = 0.0;
  if ( source.level() != OutData::NoLevel ) {
    intensfac = ( ::pow( 10.0, -source.level()/20.0 ) );
    if ( source.trace() == GlobalAMEField )
      intensfac /= 0.3;
    else
      intensfac /= 0.4;
  }
  dest *= intensfac;
}


void PUnitModel::operator()( double t, double *x, double *dxdt, int n )
{
  static Random rand;

  // O-U noise for EOD frequency:
  dxdt[0] = ( -x[0] + EODFreqFac*rand.gaussian() ) / EODFreqTau;
  // phase of EOD frequency:
  dxdt[1] = EODFreq + EODFreqSD * x[0];
  EOD1 = EODAmpl1 * ::sin( x[1] );
  EOD2 = EODAmpl2 * EOD1;
  double sglobal = signal( 0.001 * t, GlobalEField );
  double sglobalam = signal( 0.001 * t, GlobalAMEField ) * EOD2;
  Signal = sglobal + sglobalam;
  EOD2 += Signal;
  Signal *= SignalFac;
  double s = EOD2 * neuron()->gain() + neuron()->offset();
  s += noiseFac() * rand.gaussian();
  if ( MMCInx >= 0 )
    s -= GMC*x[MMCInx]*(x[2]-EMC);
  if ( MMHCInx >= 0 )
    s -= GMHC * ::pow( x[MMHCInx], PMMHC ) * ::pow( x[HMHCInx], PHMHC ) * (x[2]-EMHC);
  (*neuron())( t, s, x+2, dxdt+2, n-2 );
  if ( MMCInx >= 0 ) {
    double m0mc = 1.0/(exp(-(x[2]-MVMC)/MWMC)+1.0);
    dxdt[MMCInx] = ( m0mc - x[MMCInx] )/TAUMC;
  }
  if ( MMHCInx >= 0 ) {
    double m0mhc = 1.0/(exp(-(x[2]-MVMHC)/MWMHC)+1.0);
    dxdt[MMHCInx] = ( m0mhc - x[MMHCInx] )/TAUMMHC;
    double h0mhc = 1.0/(exp(-(x[2]-HVMHC)/HWMHC)+1.0);
    dxdt[HMHCInx] = ( h0mhc - x[HMHCInx] )/TAUHMHC;
  }
}


addModel( PUnitModel, efish );

}; /* namespace efish */
