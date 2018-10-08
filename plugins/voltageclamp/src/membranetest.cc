/*
  voltageclamp/membranetest.cc
  get resistance of patch clamp

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

#include <relacs/voltageclamp/membranetest.h>
#include <relacs/fitalgorithm.h>
using namespace relacs;

namespace voltageclamp {


membranetest::membranetest( void )
  : RePro( "membranetest", "voltageclamp", "Lukas Sonnenberg", "1.0", "Sep 03, 2018" ) {
  // add some options:
//  newSection("Stimulus");
  addNumber("holdingpotential", "Holding Potential", -100.0, -1000.0, 1000.0, 1.0, "mV");
  addNumber("amplitude", "Amplitude of output signal", 20.0, -1000.0, 1000.0, 0.1, "mV");
  addNumber("duration", "Duration of output", 0.002, 0.001, 1000.0, 0.001, "sec", "ms");
  addNumber("pause", "Duration of pause bewteen outputs", 0.198, 0.001, 1000.0, 0.001, "sec", "ms");
  addInteger("repeats", "Repetitions of stimulus", 3, 0, 10000, 1).setStyle(OptWidget::SpecialInfinite);
  addBoolean("infinite", "Infinite repetitions of membranetest", true);
  //  newSection("Analysis");
//  addNumber( "sswidth", "Window length for steady-state analysis", 0.05, 0.001, 1.0, 0.001, "sec", "ms" );
  addBoolean("plotstd", "Plot standard deviation of current", false);
//  addSelection("setdata", "Set results to the session variables", "rest only|always|never");
//  addText( "checkoutput", "Outputs that need to be at their default value", "Current-1" ).setActivation( "setdata", "rest only" );

  // plot:
  P.lock();
  P.setXLabel("Time [ms]");
  P.unlock();
  setWidget(&P);
}

int membranetest::main( void )
{
//  OutParams.clear();
  // get options:
  double holdingpotential = number( "holdingpotential" );
  double amplitude = number( "amplitude" );
  double duration = number( "duration" );
  bool infinite = boolean( "infinite" );
  double pause = number( "pause" );
//  int repeats = integer( "repeats" );

//  double sswidth = number( "sswidth" );
//  texts( "checkoutput", OutParams );

  if ( pause < 2.0*duration ) {
    warning("Pause must be at least two times the stimulus duration!");
    return Failed;
  }

  // don't print repro message:
  noMessage();

  // holding potential:
  OutData holdingsignal;
  holdingsignal.setTrace( PotentialOutput[0] );
  holdingsignal.constWave( holdingpotential );
  holdingsignal.setIdent( "VC=" + Str( holdingpotential ) + "mV" );

  // plot:
  P.lock();
  P.clear();
//  P.setXRange( -500.0*Duration, 2000.0*Duration );
//  P.setYLabel( trace( SpikeTrace[0] ).ident() + " [" + VUnit + "]" );
  P.draw();
  P.unlock();

  // signal:
  OutData signal;
  signal.setTrace( PotentialOutput[0] );
  signal.pulseWave( duration, -1.0, holdingpotential + amplitude, holdingpotential );

  // sleep:
  sleepWait( pause );
  if ( interrupt() )
    return Aborted;

  write(holdingsignal);
  sleep(pause);

  if (infinite) {
    while (not interrupt()) {
      stimulus(signal);
    };
    return Aborted;
  }
  else {
    stimulus(signal);
    if (interrupt()) {
      return Aborted;
    };
  };
  return Completed;
}


void membranetest::stimulus(OutData &signal) {
  double duration = number( "duration" );
  double pause = number( "pause" );
  int repeats = integer( "repeats" );
  bool plotstd = boolean( "plotstd" );

  double samplerate = trace( SpikeTrace[0] ).sampleRate();
  SampleDataF MeanPot = SampleDataF( -duration, 2*duration, 1/samplerate, 0.0 );
  SampleDataF MeanSqPot = SampleDataF( -duration, 2*duration, 1/samplerate, 0.0 );
  SampleDataF StdPot = SampleDataF( -duration, 2*duration, 1/samplerate, 0.0 );


  for ( int Count=0; ( repeats <= 0 || Count < repeats ) && softStop() == 0; Count++ ) {
    write(signal);
    sleep(pause);

    SampleDataF currenttrace(-duration, 2 * duration, trace(CurrentTrace[0]).stepsize(), 0.0);
    trace(CurrentTrace[0]).copy(signalTime(), currenttrace);

    for (int i = 0; i < currenttrace.size(); i++) {
      MeanPot[i] += (currenttrace[i] - MeanPot[i]) / (Count + 1);
      MeanSqPot[i] += (currenttrace[i] * currenttrace[i] - MeanSqPot[i]) / (Count + 1);

      double diff = MeanSqPot[i] - MeanPot[i] * MeanPot[i];
      if ( diff >= 0.0) {
        StdPot[i] = sqrt(MeanSqPot[i] - MeanPot[i] * MeanPot[i]);
      }
      else { StdPot[i] = 0.0; };
    };

    if (interrupt()) {
      break;
    };
  };

  if (interrupt()) {
    return;
  };

  // plot
  P.lock();
  P.clear();
  P.plot(MeanPot, 1000.0, Plot::Yellow, 1, Plot::Solid);
  if (plotstd) {
    P.plot(MeanPot + StdPot, 1000.0, Plot::Red, 2, Plot::Solid);
    P.plot(MeanPot - StdPot, 1000.0, Plot::Red, 2, Plot::Solid);
  };
  P.draw();
  P.unlock();

  resistance( MeanPot, StdPot );

}

void membranetest::resistance( SampleDataF &MeanPot, SampleDataF &StdPot ) {
  double duration  = number( "duration" );
  double samplerate = trace( SpikeTrace[0] ).sampleRate();
  double amplitude = number( "amplitude" );
  double maximum = max( MeanPot );
  int index = maxIndex( MeanPot );
//  int idx0 = index;
  int idx05 = 1.5*duration*samplerate;
  int idx1 = MeanPot.index(duration) - 1;
//  int idx_t0 = MeanPot.index( 0.0 );

  double I0 = 0;
  for (int i=0; i<(duration*samplerate-1); i++ ) {
    I0 += MeanPot[i];
  };
  I0 = I0/(duration*samplerate-1);

  // R_a
  double R_a = amplitude/(maximum - I0);

  // fit exponential to estimate resistance
  ArrayD param( 3, 1.0 );
  param[0] = maximum;
  param[1] = -1e-5;
  param[2] = MeanPot[idx1];
  ArrayD uncertainty ( 3, 0.0 );
  ArrayI paramfit( 3, 1 );
//  paramfit[1] = 1;
//  paramfit[2] = 1;
  double chisq = 1.0;

//  int fitresult = 0;

  int fitresult = marquardtFit(
//          MeanPot.range().begin()+idx_t0, MeanPot.range().begin()+idx_t0+(idx1-index),
          MeanPot.range().begin()+index, MeanPot.range().begin()+idx1,
          MeanPot.begin()+index, MeanPot.begin()+idx1,
          StdPot.begin()+index, StdPot.begin()+idx1,
          expFunc2Derivs, param, paramfit, uncertainty, chisq
          );

  double I1 = param[2];

  SampleDataD y(MeanPot.pos(index), duration, 1.0/samplerate);
  for ( int i = 0; i<y.size(); i++) {
    y[i] = expFunc2(y.pos(i), param);
  };

  if ( fitresult == 0 ) {
    cerr << "fit worked\n";
    P.lock();
    P.setTitle("R_a = " + Str( R_a, "%.3f" ) + ", R = " + Str(amplitude / (I1 - I0), "%.1f"));
    P.plot( y, 1000.0, Plot::Green, 2, Plot::Solid);

    P.draw();
    P.unlock();
  }
  else if ( MeanPot[idx05] < (MeanPot[idx1]+3*StdPot[idx1]) ) {
    double I_steady = mean( MeanPot.begin()+idx05, MeanPot.begin()+idx1 );
    cerr << "fit failed, take mean of last half of duration\n"
    << "fitresult = " << fitresult << "\n";

    P.lock();
    P.setTitle("R_a = " + Str( R_a, "%.3f" ) + ", R_m = " + Str(amplitude / (I_steady - I0), "%.1f"));
    P.plotLine( duration/2*1000, I_steady, duration*1000, I_steady, Plot::Green, 2, Plot::Solid);
    P.draw();
    P.unlock();
  }
  else {
    cerr << "fit failed\n";
  };

}


addRePro( membranetest, voltageclamp );

}; /* namespace voltageclamp */

#include "moc_membranetest.cc"
