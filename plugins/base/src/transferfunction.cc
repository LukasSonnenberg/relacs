/*
  base/transferfunction.cc
  Measures the transfer function with white-noise stimuli.

  RELACS - Relaxed ELectrophysiological data Acquisition, Control, and Stimulation
  Copyright (C) 2002-2010 Jan Benda <benda@bio.lmu.de>

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

#include <relacs/tablekey.h>
#include <relacs/base/transferfunction.h>
using namespace relacs;

namespace base {


TransferFunction::TransferFunction( void )
  : RePro( "TransferFunction", "base", "Jan Benda", "1.2", "Nov 28, 2010" )
{
  // options:
  addLabel( "Stimulus" ).setStyle( OptWidget::Bold );
  addSelection( "outtrace", "Output trace", "V-1" );
  addNumber( "amplitude", "Amplitude", 1.0, 0.0, 100000.0, 1.0, "", "" );
  addNumber( "fmax", "Maximum frequency", 1000.0, 0.0, 10000000.0, 100.0, "Hz", "Hz" );
  addNumber( "duration", "Duration of noise stimulus", 1.0, 0.0, 100.0, 0.1, "s", "ms" );
  addNumber( "pause", "Length of pause inbetween successive stimuli", 1.0, 0.0, 100.0, 0.1, "s", "ms" );
  addInteger( "repeats", "Repeats", 100, 0, 10000, 1 );
  addLabel( "Analysis" ).setStyle( OptWidget::Bold );
  addSelection( "intrace", "Input trace", "V-1" );
  addSelection( "size", "Number of data points for FFT", "1024|64|128|256|512|1024|2048|4096|8192|16384|32768|65536|131072|262144|524288|1048576" );
  addBoolean( "overlap", "Overlap FFT windows", true );
  addSelection( "window", "FFT window function", "Hanning|Bartlett|Blackman|Blackman-Harris|Hamming|Hanning|Parzen|Square|Welch" );

  // plot:
  P.lock();
  P.resize( 2, 1, true );
  P.setCommonXRange( 0, 1 );
  P[0].setLMarg( 8 );
  P[0].setRMarg( 6 );
  P[0].noXTics();
  P[0].setXRange( 0.0, 1000.0 );
  P[0].setYLabel( "Gain" );
  P[0].setYLabelPos( 2.0, Plot::FirstMargin,
		     0.5, Plot::Graph, Plot::Center, -90.0 );
  P[0].setYRange( 0.0, Plot::AutoScale );
  P[0].setY2Label( "Coherence" );
  P[0].setY2Tics();
  P[0].setY2Range( 0.0, 1.0 );
  P[1].setLMarg( 8 );
  P[1].setRMarg( 6 );
  P[1].setXLabel( "Frequency [Hz]" );
  P[1].setXRange( 0.0, 1000.0 );
  P[1].setYLabel( "Phase" );
  P[1].setYLabelPos( 2.0, Plot::FirstMargin,
		     0.5, Plot::Graph, Plot::Center, -90.0 );
  P[1].setYRange( -3.15, 3.15 );
  P.unlock();
  setWidget( &P );
}


void TransferFunction::config( void )
{
  setText( "intrace", traceNames() );
  setToDefault( "intrace" );
  setText( "outtrace", outTraceNames() );
  setToDefault( "outtrace" );
}


void TransferFunction::notify( void )
{
  int outtrace = index( "outtrace" );
  if ( outtrace >= 0 && outtrace < outTracesSize() ) {
    OutName = outTrace( outtrace ).traceName();
    OutUnit = outTrace( outtrace ).unit();
    setUnit( "amplitude", OutUnit );
  }

  int intrace = index( "intrace" );
  if ( intrace >= 0 && intrace < traces().size() )
    InName = trace( intrace ).ident();
    InUnit = trace( intrace ).unit();
}


int TransferFunction::main( void )
{
  int outtrace = index( "outtrace" );
  double amplitude = number( "amplitude" );
  double fmax = number( "fmax" );
  double duration = number( "duration" );
  double pause = number( "pause" );
  int repeats = integer( "repeats" );
  int intrace = traceIndex( text( "intrace", 0 ) );
  SpecSize = integer( "size" );
  Overlap = boolean( "overlap" );
  int win = index( "window" );
  switch ( win ) {
  case 0: Window = bartlett; break;
  case 1: Window = blackman; break;
  case 2: Window = blackmanHarris; break;
  case 3: Window = hamming; break;
  case 4: Window = hanning; break;
  case 5: Window = parzen; break;
  case 6: Window = square; break;
  case 7: Window = welch; break;
  default: Window = hanning;
  }

  // check parameter:
  if ( amplitude <= 0.0 ) {
    warning( "Amplitude of noise stimulus must be greater than zero!" );
    return Failed;
  }
  if ( fmax > 0.5*trace( intrace ).sampleRate()+1.0e-8 ) {
    warning( "Maximum frequency " + Str( fmax, 0, 10, 'g' ) +
	     "Hz must be less than or equal to half the sampling rate " +
	     Str( trace( intrace ).sampleRate(), 0, 10, 'g' ) + "Hz!" );
    return Failed;
  }
  if ( trace( intrace ).interval( SpecSize ) > 0.25*duration ) {
    warning( "Number of data points for FFT too large! Must be less than a quarter of the stimulus duration, i.e. less than " +
	     Str( trace( intrace ).indices( duration )/4 ) +
	     "! Alternatively, you can increase the stimulus duration to at least " +
	     Str( 40.*trace( intrace ).interval( SpecSize ) ) + "s." );
    return Failed;
  }

  MeanGain.clear();
  SquareGain.clear();
  StdevGain.clear();
  MeanPhase.clear();
  SquarePhase.clear();
  StdevPhase.clear();
  MeanCoherence.clear();
  SquareCoherence.clear();
  StdevCoherence.clear();

  // don't print repro message:
  noMessage();

  // plot trace:
  tracePlotSignal( duration );

  // plot:
  P.lock();
  P[0].resetRanges();
  P[0].setYLabel( "Gain [" + InUnit + "/" + OutUnit + "]" );
  P[1].resetRanges();
  P.unlock();

  // files:
  ofstream tf;
  TableKey tracekey;
  Options header;
  header.addInteger( "index", completeRuns() );
  header.addInteger( "ReProIndex", reproCount() );
  header.addNumber( "ReProTime", reproStartTime(), "s", "%0.3f" );
  openTraceFile( tf, tracekey, header );

  // signal:
  OutData signal;
  signal.setIdent( "WhiteNoise, fmax=" + Str( fmax ) + "Hz" );
  signal.back() = 0.0;
  signal.setTrace( outtrace );

  // write stimulus:
  sleep( pause );
  timeStamp();
  for ( int count=0;
	( repeats <= 0 || count < repeats ) && softStop() == 0;
	count++ ) {

    Str s = "Amplitude <b>" + Str( amplitude ) + " " + outTrace( outtrace ).unit() +"</b>";
    s += ",  Frequency <b>" + Str( fmax, "%.0f" ) + " Hz</b>";
    s += ",  Loop <b>" + Str( count+1 ) + "</b>";
    if ( repeats > 0 )
      s += " of <b>" + Str( repeats ) + "</b>";
    message( s );

    signal.noiseWave( fmax, duration, amplitude );
    // debug:
    if ( signal.length() < duration )
      printlog( "WARNING: noiseWave() too short! duration=" + Str( duration ) +
		" length=" + Str( signal.length() ) );
    write( signal );
    if ( signal.failed() ) {
      warning( signal.errorText() );
      return Failed;
    }

    sleep( duration + 0.1*pause );
    if ( interrupt() ) {
      if ( count > 0 )
	break;
      else
	return Aborted;
    }

    SampleDataF input, output;
    analyze( signal, trace( intrace ), duration, count, input, output );

    // plot gain:
    P.lock();
    P[0].clear();
    if ( ! P[0].zoomedXRange() && ! P[1].zoomedXRange() )
      P[0].setXRange( 0.0, fmax );
    P[0].plot( MeanCoherence+StdevCoherence, 1.0, Plot::Yellow, 1, Plot::Solid );
    P[0].back().setAxis( Plot::X1Y2 );
    P[0].plot( MeanCoherence-StdevCoherence, 1.0, Plot::Yellow, 1, Plot::Solid );
    P[0].back().setAxis( Plot::X1Y2 );
    P[0].plot( MeanCoherence, 1.0, Plot::Yellow, 3, Plot::Solid );
    P[0].back().setAxis( Plot::X1Y2 );
    P[0].plot( MeanGain+StdevGain, 1.0, Plot::Red, 1, Plot::Solid );
    P[0].plot( MeanGain-StdevGain, 1.0, Plot::Red, 1, Plot::Solid );
    P[0].plot( MeanGain, 1.0, Plot::Red, 3, Plot::Solid );
    P[1].clear();
    if ( ! P[0].zoomedXRange() && ! P[1].zoomedXRange() )
      P[1].setXRange( 0.0, fmax );
    P[1].plotHLine( 0.0, Plot::White, 2 );
    P[1].plot( MeanPhase+StdevPhase, 1.0, Plot::Blue, 1, Plot::Solid );
    P[1].plot( MeanPhase-StdevPhase, 1.0, Plot::Blue, 1, Plot::Solid );
    P[1].plot( MeanPhase, 1.0, Plot::Blue, 3, Plot::Solid );
    P.unlock();
    P.draw();

    // save:
    saveTrace( tf, tracekey, count, input, output );

    sleepOn( duration+pause );
    if ( interrupt() ) {
      if ( count > 0 )
	break;
      else
	return Aborted;
    }

  }

  saveData( header );

  return Completed;
}



void TransferFunction::analyze( const OutData &signal, const InData &data,
				double duration, int count,
				SampleDataF &input, SampleDataF &output )
{
  // de-mean:
  input.resize( 0.0, duration, data.stepsize() );
  input.interpolate( signal );
  SampleDataD x( input );
  x -= mean( x );

  output.resize( 0.0, duration, data.stepsize() );
  data.copy( signalTime(), output );
  SampleDataD y( output );
  y -= mean( y );

  // transfer fucntion:
  SampleDataD trans( SpecSize );
  SampleDataD cohere( SpecSize/2 );
  transfer( x, y, trans, cohere, Overlap, Window );

  // gain and phase:
  SampleDataD gain( trans.size()/2 );
  SampleDataD phase( trans.size()/2 );
  hcMagnitude( trans, gain );
  hcPhase( trans, phase );

  // average:
  if ( count <= 0 ) {
    MeanGain = gain;
    SquareGain = gain*gain;
    StdevGain = gain;  // setting the right range
    StdevGain = 0.0;
    MeanPhase = phase;
    SquarePhase = phase*phase;
    StdevPhase = phase;  // setting the right range
    StdevPhase = 0.0;
    MeanCoherence = cohere;
    SquareCoherence = cohere*cohere;
    StdevCoherence = cohere;  // setting the right range
    StdevCoherence = 0.0;
  }
  else {
    for ( int k=0; k<gain.size(); k++ ) {
      double g = gain[k];
      MeanGain[k] += (g - MeanGain[k])/(count+1);
      SquareGain[k] += (g*g - SquareGain[k])/(count+1);
      StdevGain[k] = sqrt( fabs( SquareGain[k] - MeanGain[k]*MeanGain[k] ) );
      double p = phase[k];
      MeanPhase[k] += (p - MeanPhase[k])/(count+1);
      SquarePhase[k] += (p*p - SquarePhase[k])/(count+1);
      StdevPhase[k] = sqrt( fabs( SquarePhase[k] - MeanPhase[k]*MeanPhase[k] ) );

      double c = cohere[k];
      MeanCoherence[k] += (c - MeanCoherence[k])/(count+1);
      SquareCoherence[k] += (c*c - SquareCoherence[k])/(count+1);
      StdevCoherence[k] = sqrt( fabs( SquareCoherence[k] - MeanCoherence[k]*MeanCoherence[k] ) );
    }
  }
}


void TransferFunction::openTraceFile( ofstream &tf, TableKey &tracekey,
				      const Options &header )
{
  tracekey.addNumber( "t", "ms", "%7.2f" );
  tracekey.addNumber( OutName, OutUnit, "%8.3f" );
  tracekey.addNumber( InName, InUnit, "%8.3f" );
  tf.open( addPath( "transferfunction-traces.dat" ).c_str(),
	   ofstream::out | ofstream::app );
  header.save( tf, "# " );
  tf << "# status:\n";
  stimulusData().save( tf, "#   " );
  tf << "# settings:\n";
  settings().save( tf, "#   " );
  tf << '\n';
  tracekey.saveKey( tf, true, false );
  tf << '\n';
}


void TransferFunction::saveTrace( ofstream &tf, TableKey &tracekey,
				  int index,
				  const SampleDataF &input,
				  const SampleDataF &output )
{
  tf << "# index: " << index << '\n';
  for ( int k=0; k<input.size(); k++ ) {
    tracekey.save( tf, 1000.0*input.pos( k ), 0 );
    tracekey.save( tf, input[k] );
    tracekey.save( tf, output[k] );
    tf << '\n';
  }
  tf << '\n';
}


void TransferFunction::saveData( const Options &header )
{
  ofstream df( addPath( "transferfunction-data.dat" ).c_str() );

  header.save( df, "# " );
  df << "# status:\n";
  stimulusData().save( df, "#   " );
  df << "# settings:\n";
  settings().save( df, "#   " );
  df << '\n';

  TableKey datakey;
  datakey.addNumber( "f", "Hz", "%7.2f" );
  datakey.addNumber( "gain", InUnit + "/" + OutUnit, "%9.4f" );
  datakey.addNumber( "s.d.", InUnit + "/" + OutUnit, "%9.4f" );
  datakey.addNumber( "phase", "1", "%6.3f" );
  datakey.addNumber( "s.d.", "1", "%6.3f" );
  datakey.addNumber( "coherence", "1", "%6.4f" );
  datakey.addNumber( "s.d.", "1", "%6.4f" );
  datakey.saveKey( df );

  for ( int k=0; k<MeanGain.size(); k++ ) {
    datakey.save( df, MeanGain.pos( k ), 0 );
    datakey.save( df, MeanGain[k] );
    datakey.save( df, StdevGain[k] );
    datakey.save( df, MeanPhase[k] );
    datakey.save( df, StdevPhase[k] );
    datakey.save( df, MeanCoherence[k] );
    datakey.save( df, StdevCoherence[k] );
    df << '\n';
  }
  
  df << "\n\n";
}


addRePro( TransferFunction );

}; /* namespace base */

#include "moc_transferfunction.cc"
