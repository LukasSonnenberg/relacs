/*
  voltageclamp/pnsubtraction.cc
  P-N Subtraction for removing stimulus artifacts in voltageclamp recordings

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

#include <relacs/fitalgorithm.h>
#include <cmath>
#include <relacs/voltageclamp/pnsubtraction.h>
#include <algorithm> // for copy() and assign()
#include <iterator> // for back_inserter
#include <relacs/randomstring.h>
#include <relacs/ephys/amplifiercontrol.h>

using namespace relacs;

namespace voltageclamp {


PNSubtraction::PNSubtraction( const string &name,
	 const string &pluginset,
	 const string &author,
	 const string &version,
	 const string &date)
  : RePro( name, pluginset, author, version, date )
{
  // add some options:
  addNumber( "pn", "p/N", -4, -100, 100, 1 );
  addBoolean( "qualitycontrol", "Quality control", true );
  addNumber( "pulseduration", "Pulse duration", 0.1, 0.0, 1000.0, 0.001, "sec", "ms").setActivation( "qualitycontrol", "true" );
  addNumber( "f0", "minimum pulse frequency", 10.0, 0.0, 1000.0, 1.0, "Hz", "Hz" ).setActivation( "qualitycontrol", "true" );
  addNumber( "f1", "maximum pulse frequency", 500.0, 0.0, 5000.0, 1.0, "Hz", "Hz" ).setActivation( "qualitycontrol", "true" );
}

int PNSubtraction::main( void )
{
  return Completed;
}

SampleDataD PNSubtraction::PN_sub( OutData signal, Options &opts, double &holdingpotential, double &pause, double &mintime, double &maxtime, double &t0) {
  int pn = number( "pn" );
  double samplerate = signal.sampleRate();
  bool qualitycontrol = boolean( "qualitycontrol" );
  double pulseduration = number( "pulseduration" );
  double f0 = number( "f0" );
  double f1 = number( "f1" );

  // assign random id for later connection between qualitycontrol, pn and traces
  std::string randomId = randomString(40);

  // add p/n option to signal
  Parameter &pn1 = opts.addNumber( "pn", pn );
  Parameter &qc1 = opts.addBoolean( "qualitycontrol", qualitycontrol );
  Parameter &rid = opts.addText( "TraceId", randomId );

  signal.setMutable( pn1 );
  signal.setMutable( qc1 );
  signal.setMutable( rid );
  signal.setDescription( opts );

  // don't print repro message:
  noMessage();

  SampleDataD pn_trace(mintime, signal.rangeBack(), 1 / samplerate);
  if ( pn != 0 ) {
    // give pn stimulus
    OutData pn_signal = signal;
    pn_signal.setTrace(PotentialOutput[0]);
    pn_signal = holdingpotential + (signal - holdingpotential) / pn;
    pn_signal.description().setType("stimulus/PNSubtraction");

    for (int i = 0; i < ::abs(pn); i++) {
      write(pn_signal);
      if (signal.error())
        return false;
      sleep(pause);

      if (interrupt()) {
        break;
      };

      SampleDataD currenttrace(mintime, maxtime, trace(CurrentTrace[0]).stepsize(), 0.0);
      trace(CurrentTrace[0]).copy(signalTime(), currenttrace);

      pn_trace += currenttrace;
    };
//  pn_trace -= pn_trace.mean(signalTime() + t0 - 0.001, signalTime() + t0);

    if (interrupt()) {
      return pn_trace;
    };
  };
  
  // make short quality assuring test-pulse
  ArrayD I;
  ArrayD Vp;

  double stepduration = 0.010;
  if ( qualitycontrol ) {
    OutData qc_signal1;
    qc_signal1.setTrace( PotentialOutput[0] );
    qc_signal1.constWave( stepduration, -1.0, -100 );

    OutData qc_signal2;
    qc_signal2.setTrace( PotentialOutput[0] );
    qc_signal2.constWave( stepduration, -1.0, -170 );
    
    OutData qc_signal3;
    qc_signal3.setTrace( PotentialOutput[0] );
    qc_signal3.constWave( stepduration, -1.0, -135 );

    OutData qc_signal4;
    qc_signal4.setTrace( PotentialOutput[0] );
    qc_signal4.sweepWave( pulseduration, -1.0, f0, f1, 20.0, 0.0 );
    qc_signal4 = qc_signal4 + holdingpotential - 20;

    OutData qc_signal5;
    qc_signal5.setTrace( PotentialOutput[0] );
    qc_signal5.constWave( 0.010, -1.0, holdingpotential );

    qc_signal1.append( qc_signal2 );
    qc_signal1.append( qc_signal3 );
    qc_signal1.append( qc_signal4 );
    qc_signal1.append( qc_signal5 );

    qc_signal1.description().setType( "stimulus/QualityControl" );
    Options opts_qc = qc_signal1.description();
    Parameter &qc_rid = opts_qc.addText( "TraceId", randomId );
    Parameter &qc_f0 = opts_qc.addNumber( "f1", f0 );
    Parameter &qc_f1 = opts_qc.addNumber( "f0", f1 );
    qc_signal1.setMutable( qc_rid );
    qc_signal1.setMutable( qc_f0 );
    qc_signal1.setMutable( qc_f1 );
    qc_signal1.setDescription( opts_qc );

    write(qc_signal1);
    sleep(pause);

    SampleDataD PCS_currenttrace(0.0, 3 * stepduration + pulseduration, trace(CurrentTrace[0]).stepsize(), 0.0 );
    trace(CurrentTrace[0]).copy(signalTime(), PCS_currenttrace );
    SampleDataD PCS_potentialtrace(0.0, 3 * stepduration + pulseduration, trace(SpikeTrace[0]).stepsize(), 0.0 );
    trace(SpikeTrace[0]).copy(signalTime(), PCS_potentialtrace );

  };

  signal.description().setType( "stimulus/Trace" );

  write(signal);
  sleep(pause);

  SampleDataD currenttrace( mintime, maxtime, trace(CurrentTrace[0]).stepsize(), 0.0);
  trace(CurrentTrace[0]).copy(signalTime(), currenttrace );

  if ( qualitycontrol ) {
//    ArrayD Vp( PCS_potentialtrace.size());
//    ArrayD I( PCS_potentialtrace.size());
//    for ( int i=0; i<PCS_potentialtrace.size(); i++ ) {
//      Vp[i] = PCS_potentialtrace[i];
//      I[i] = PCS_currenttrace[i];
//    }
    ArrayD param_leak = pcsFitLeak( PCS_potentialtrace, PCS_currenttrace, stepduration );
//    double x = prt();
    cerr << param_leak << "\n";
  }
  else if ( pn != 0 )
  {
    currenttrace -= pn / ::abs(pn) * pn_trace;// - currenttrace.mean(signalTime() + t0 - 0.001, signalTime() + t0);
    currenttrace -= currenttrace.mean(-samplerate / 500, 0);
  };
  return currenttrace;
};


ArrayD PNSubtraction::dxdt( const ArrayD &x, const double &dt, const double &kernelsize ) {
  ArrayD x2( x.size() );
  ArrayD dx( x.size() );

  int khalf = floor(kernelsize / 2.0);

  // convolve x with ArrayD (kernelsize, 1/kernelsize)
  if ( kernelsize > 1 ) {
    for ( int i = khalf; i < (x.size() - khalf); i++ ) {
      double s = 0.0;
      for ( int j = (i - khalf); j < (i+khalf); j++ ) {
        if ( j < 0.0 ){
          s += x[0] ;
        }
        else if ( j >= x.size() ) {
          s += x[x.size() - 1];
        }
        else {
          s += x[j];
        }
      }
      x2[i] = s/kernelsize;
    }
  }

  // derive smoothed trace
  for ( int i = 0; i < x.size(); i++ ) {
    if ( i == 0 ) {
      double x0 = 2 * x2[0] - x2[1];
      dx[i] = (x2[i+1] - x0) / (2 * dt);
    }
    else if ( i == (x.size()-1) ) {
      double x1 = 2 * x2[i] - x2[i-1];
      dx[i] = (x1 - x2[i-1]) / (2 * dt);
    }
    else {
      dx[i] = (x2[i + 1] - x2[i - 1]) / (2 * dt);
    }
  }
  return dx;
}


double passiveMembraneFuncDerivs( double t, const ArrayD &p, ArrayD &dfdp ) {
      double a = p[0];
      double b = p[1];
      double c = p[2];
      double d = p[3];
      double e = p[4];

      ArrayD I;
      ArrayD dI;
      ArrayD Vp;
      ArrayD dVp;
      ArrayD d2Vp;
      double dt;
      int idx = t/dt;

      double y = I[idx] + a * dI[idx] - b * Vp[idx] - c * dVp[idx] - d * d2Vp[idx] + e;
      dfdp[0] = dI[idx];
      dfdp[1] = -Vp[idx];
      dfdp[2] = -dVp[idx];
      dfdp[3] = -d2Vp[idx];
      dfdp[4] = 1.0;
      return y;
};


ArrayD PNSubtraction::pcsFitLeak(const SampleDataD &potentialtrace, const SampleDataD &currenttrace, double stepduration ) {
  double dt = currenttrace.stepsize();

  //first step
  int idx00 = 1 / 2 * stepduration / dt;
  int idx01 = 2 * stepduration / dt - 2;
  //second step
  int idx10 = 3 / 2 * stepduration / dt;
  int idx11 = 3 * stepduration / dt - 2;
  //third
  int idx20 = 5 / 2 * stepduration / dt;
  int idx21 = 4 * stepduration / dt - 2;

  //empty potential and current arrays

//  ArrayD Vp(potentialtrace.size());
//  ArrayD I(potentialtrace.size());
//  for ( int i=0; i<potentialtrace.size(); i++ ) {
//    Vp[i] = potentialtrace[i];
//    I[i] = currenttrace[i];
//  }
  const ArrayD &Vp = potentialtrace.array();
  ArrayD dVp = dxdt( Vp, dt, 3.0 );
  ArrayD d2Vp = dxdt( dVp, dt, 3.0 );
  const ArrayD &I = currenttrace.array();
  ArrayD dI = dxdt( I, dt, 3.0 );
  ArrayD time( idx01-idx00 + idx11-idx10 + idx21-idx20 );
  ArrayD y( idx01-idx00 + idx11-idx10 + idx21-idx20 );
  cerr << dt << ", " << stepduration << ", " << stepduration/dt << "\n";

  //fill arrays
  int i = -1;
  for ( int j = idx00; j < idx01; j++) {
    i += 1;
    time[i] = j/dt;
    y[i] = 0.0;
  }
  for ( int j = idx10; j < idx11; j++) {
    i += 1;
    time[i] = j/dt;
    y[i] = 0.0;
  }
  for ( int j = idx20; j < idx21; j++) {
    i += 1;
    time[i] = j/dt;
    y[i] = 0.0;
  }

  double Rs;
  double EL;
  double gL;

//  // Fit leak current
  ArrayD param(5, 0.0);
  param[1] = gL / (1 + gL * Rs);
  param[4] = EL * gL / (1 + gL * Rs);
  ArrayI pf(5, 0);
  pf[1] = 1;
  pf[4] = 1;
  ArrayD err( time.size(), 1.0);
  ArrayD uncert(5, 0.0);
  double chisq = 0.0;

  marquardtFit( time, y, err, passiveMembraneFuncDerivs, param, pf, uncert, chisq);
  return param;
}


//double PNSubtraction::linearFuncDerivs( double x, const ArrayD &p, ArrayD &dfdp ) {
//  double m = p[0];
//  double b = p[1];
//  double y = m * x + b;
//  dfdp[0] = x;
//  dfdp[1] = 1.0;
//  return y;
//};


//double currentPulseFuncDerivs(  double t, const ArrayD &p, ArrayD &dfdp ) {
//  double dT = p[4];
//  double tau = p[0];
//  double V0 = p[1];
//  double V1 = p[2];
//  double V2 = p[3];
//  double y = 0.0;
//
//  double V11 = (V0 - V1) * ::exp( -dT / tau ) + V1;
//  double V21 = (V11- V2) * ::exp( -dT / tau ) + V2;
//
//  double ex1 = ::exp( - (t - 1*dT) / tau);
//  double ex2 = ::exp( - (t - 2*dT) / tau);
//  double ex3 = ::exp( - (t - 3*dT) / tau);
//
//  if (t < dT ) {
//    y = V0;
//    dfdp[0] = 0.0;
//    dfdp[1] = 1.0;
//    dfdp[2] = 0.0;
//    dfdp[3] = 0.0;
//  }
//  else if ( t < 2*dT ) {
//    y = ( V0 - V1 ) * ex1 + V1;
//    dfdp[0] = (t-dT) / (tau*tau) * (V0-V1) * ex1;
//    dfdp[1] = ex1;
//    dfdp[2] = - ex1 + 1.0;
//    dfdp[3] = 0.0;
//  }
//  else if ( t < 3*dT ) {
//    y = ( V11 - V2 ) * ex2 + V2;
//    dfdp[0] = (t-1*dT) / (tau*tau) * (V0-V1) * ex1 +
//              (t-2*dT) / (tau*tau) * (V1-V2) * ex2;
//    dfdp[1] = ex1;
//    dfdp[2] = - ex1 + ex2;
//    dfdp[3] = - ex2 + 1.0;
//  }
//  else if ( t < 4*dT ) {
//    y = ( V21 - V0 ) * ex3 + V0;
//    dfdp[0] = (t-1*dT) / (tau*tau) * (V0-V1) * ex1 +
//              (t-2*dT) / (tau*tau) * (V1-V2) * ex2 +
//              (t-3*dT) / (tau*tau) * (V2-V0) * ex3;
//    dfdp[1] = ex1 - ex3 + 1.0;
//    dfdp[2] = - ex1 + ex2;
//    dfdp[3] = - ex2 + ex3;
//  };
//  dfdp[4] = 0.0;
//  return y;
//};


//void PNSubtraction::analyzeCurrentPulse( SampleDataD voltagetrace, double I0 ) {
//      double pulseamplitude = number( "pulseamplitude" );
//      double pulseduration = number( "pulseduration" );
//      double samplerate = trace( SpikeTrace[0] ).sampleRate();
//      const InData &intrace = trace( SpikeTrace[0] );
//      int dT = intrace.indices(pulseduration);
//
//      // Fit exponentials to CurrentPulse
//      ArrayD param( 5, 1.0 );
//      param[0] = .05;
//      param[1] = mean(voltagetrace.begin(), voltagetrace.begin()+10) + 1;
//      param[2] = mean(voltagetrace.begin() + 2*dT - 10, voltagetrace.begin() + 2*dT ) + 1;
//      param[3] = mean(voltagetrace.begin() + 3*dT - 10, voltagetrace.begin() + 3*dT ) + 3;
//      param[4] = pulseduration;
//      ArrayD error( voltagetrace.size(), 1.0 );
//      ArrayD uncertainty( 5, 0.0 );
//      ArrayI paramfit( 5, 1 );
//      paramfit[4] = 0;
//      double chisq = 0.0;
//
//      marquardtFit( voltagetrace.range(), voltagetrace, error, currentPulseFuncDerivs,
//                    param, paramfit, uncertainty, chisq );
//
//      // Fit leak current
//      ArrayD I_leak( 3, 1.0 ); I_leak[0] = I0; I_leak[1] = I0 + 2*pulseamplitude; I_leak[2] = I0 + pulseamplitude;
//      ArrayD V_leak( 3, 1.0 ); V_leak[0] = param[1]; V_leak[1] = param[2]; V_leak[2] = param[3];
//      ArrayD p_leak( 2, 1.0 ); p_leak[0] = 0.1; p_leak[1] = 0.0;
//      ArrayD err_leak( 2, 1.0 );
//      ArrayD uncert_leak( 2, 0.0 );
//      ArrayI pf_leak( 2, 1 );
//      marquardtFit( V_leak, I_leak, err_leak, linearFuncDerivs, p_leak, pf_leak, uncert_leak, chisq );
//
//      // Compute error and expected error
//      ArrayD errV( voltagetrace.size(), 1.0);
//      for ( int i=0; i<voltagetrace.size(); i++ ) {
//        errV[i] = voltagetrace[i] - currentPulseFuncDerivs( i/samplerate, param, uncertainty );
//      };
//
//      gL = p_leak[0];
//      EL = p_leak[1];
//      tau = param[0];
//      Cm = tau * gL;
//
//      cerr << "tau=" << param[0]*1000.0 << "ms, Cm=" << Cm*1000.0 << "pF\n";
//      cerr << "with std=" << errV.stdev() << " and expected min std of " << voltagetrace.stdev( 0.0, pulseduration - 1/samplerate ) << "\n";
//
//    };

}; /* namespace voltageclamp */

#include "moc_pnsubtraction.cc"
