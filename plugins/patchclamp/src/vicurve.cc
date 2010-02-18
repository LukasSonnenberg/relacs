/*
  patchclamp/vicurve.cc
  V-I curve measured in current-clamp

  RELACS - Relaxed ELectrophysiological data Acquisition, Control, and Stimulation
  Copyright (C) 2002-2009 Jan Benda <j.benda@biologie.hu-berlin.de>

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

#include <fstream>
#include <relacs/tablekey.h>
#include <relacs/patchclamp/vicurve.h>
using namespace relacs;

namespace patchclamp {


VICurve::VICurve( void )
  : RePro( "VICurve", "VICurve", "patchclamp",
	   "Jan Benda", "1.0", "Feb 12, 2010" ),
    P( 2, 2, true, this ),
    VUnit( "mV" ),
    IUnit( "nA" ),
    VFac( 1.0 ),
    IFac( 1.0 ),
    IInFac( 1.0 )
{
  // add some options:
  addLabel( "Traces" );
  addSelection( "involtage", "Input voltage trace", "V-1" );
  addSelection( "incurrent", "Input current trace", "Current-1" );
  addSelection( "outcurrent", "Output trace", "Current-1" );
  addLabel( "Stimulus" );
  addNumber( "imin", "Minimum injected current", -1.0, -1000.0, 1000.0, 0.001 );
  addNumber( "imax", "Maximum injected current", 1.0, -1000.0, 1000.0, 0.001 );
  addNumber( "istep", "Minimum step-size of current", 0.001, 0.001, 1000.0, 0.001 );
  addBoolean( "userm", "Use membrane resistance for estimating istep from vstep", false );
  addNumber( "vstep", "Minimum step-size of voltage", 1.0, 0.001, 10000.0, 0.1 ).setActivation( "userm", "true" );
  addSelection( "shuffle", "Sequence of currents", RangeLoop::sequenceStrings() );
  addSelection( "ishuffle", "Initial sequence of currents for first repetition", RangeLoop::sequenceStrings() );
  addInteger( "iincrement", "Initial increment for currents", 0, 0, 1000, 1 );
  addInteger( "singlerepeat", "Number of immediate repetitions of a single stimulus", 1, 1, 10000, 1 );
  addInteger( "blockrepeat", "Number of repetitions of a fixed intensity increment", 10, 1, 10000, 1 );
  addInteger( "repeat", "Number of repetitions of the whole V-I curve measurement", 1, 1, 10000, 1 );
  addNumber( "duration", "Duration of current output", 0.1, 0.001, 1000.0, 0.001, "sec", "ms" );
  addNumber( "delay", "Delay before current pulses", 0.1, 0.001, 1.0, 0.001, "sec", "ms" );
  addNumber( "pause", "Duration of pause between current pulses", 0.4, 0.001, 1.0, 0.001, "sec", "ms" );
  addNumber( "vmin", "Minimum value for membrane voltage", -100.0, -1000.0, 1000.0, 1.0 );
  addLabel( "Analysis" );
  addNumber( "sswidth", "Window length for steady-state analysis", 0.05, 0.001, 1.0, 0.001, "sec", "ms" );
  addNumber( "ton", "Timepoint of onset-voltage measurement", 0.01, 0.0, 100.0, 0.001, "sec", "ms" );
  addBoolean( "plotstdev", "Plot standard deviation of membrane potential", true );
  addTypeStyle( OptWidget::Bold, Parameter::Label );
}


void VICurve::config( void )
{
  setText( "involtage", spikeTraceNames() );
  setToDefault( "involtage" );
  setText( "incurrent", currentTraceNames() );
  setToDefault( "incurrent" );
  setText( "outcurrent", currentOutputNames() );
  setToDefault( "outcurrent" );
}


void VICurve::notify( void )
{
  int involtage = index( "involtage" );
  if ( involtage >= 0 && SpikeTrace[involtage] >= 0 ) {
    VUnit = trace( SpikeTrace[involtage] ).unit();
    VFac = Parameter::changeUnit( 1.0, VUnit, "mV" );
    setUnit( "vstep", VUnit );
    setUnit( "vmin", VUnit );
  }

  int outcurrent = index( "outcurrent" );
  if ( outcurrent >= 0 && CurrentOutput[outcurrent] >= 0 ) {
    IUnit = outTrace( CurrentOutput[outcurrent] ).unit();
    setUnit( "imin", IUnit );
    setUnit( "imax", IUnit );
    setUnit( "istep", IUnit );
    IFac = Parameter::changeUnit( 1.0, IUnit, "nA" );
  }

  int incurrent = index( "incurrent" );
  if ( incurrent >= 0 && CurrentTrace[incurrent] >= 0 ) {
    string iinunit = trace( CurrentTrace[incurrent] ).unit();
    IInFac = Parameter::changeUnit( 1.0, iinunit, IUnit );
  }
}


int VICurve::main( void )
{
  // get options:
  int involtage = index( "involtage" );
  int incurrent = traceIndex( text( "incurrent", 0 ) );
  int outcurrent = outTraceIndex( text( "outcurrent", 0 ) );
  double imin = number( "imin" );
  double imax = number( "imax" );
  double istep = number( "istep" );
  bool userm = boolean( "userm" );
  double vstep = number( "vstep" );
  RangeLoop::Sequence shuffle = RangeLoop::Sequence( index( "shuffle" ) );
  RangeLoop::Sequence ishuffle = RangeLoop::Sequence( index( "ishuffle" ) );
  int iincrement = integer( "iincrement" );
  int singlerepeat = integer( "singlerepeat" );
  int blockrepeat = integer( "blockrepeat" );
  int repeat = integer( "repeat" );
  double duration = number( "duration" );
  double delay = number( "delay" );
  double pause = number( "pause" );
  double vmin = number( "vmin" );
  double ton = number( "ton" );
  double sswidth = number( "sswidth" );
  if ( imax <= imin ) {
    warning( "imin must be smaller than imax!" );
    return Failed;
  }
  if ( pause < duration ) {
    warning( "Pause must be at least as long as the stimulus duration!" );
    return Failed;
  }
  if ( pause < delay ) {
    warning( "Pause must be at least as long as the delay!" );
    return Failed;
  }
  if ( sswidth >= duration ) {
    warning( "sswidth must be smaller than stimulus duration!" );
    return Failed;
  }
  if ( involtage < 0 || SpikeTrace[ involtage ] < 0 || SpikeEvents[ involtage ] < 0 ) {
    warning( "Invalid input voltage trace or missing input spikes!" );
    return Failed;
  }
  if ( outcurrent < 0 ) {
    warning( "Invalid output current trace!" );
    return Failed;
  }
  if ( userm ) {
    double rm = metaData( "Cell" ).number( "rmss", "MOhm" );
    if ( rm <= 0 )
      rm = metaData( "Cell" ).number( "rm", "MOhm" );
    if ( rm <= 0 )
      warning( "Membrane resistance was not measured yet!" );
    else {
      Header.addNumber( "rm", rm, "MOhm" );
      vstep = Parameter::changeUnit( vstep, VUnit, "mV" ); 
      double ifac = Parameter::changeUnit( 1.0, "nA", IUnit ); 
      istep = ifac*vstep/rm;
    }
  }
  Header.addNumber( "istep", istep, IUnit );

  // don't print repro message:
  noMessage();

  // plot trace:
  plotToggle( true, true, 2.0*duration+delay, delay );

  // init:
  DoneState state = Completed;
  double samplerate = trace( SpikeTrace[involtage] ).sampleRate();
  Range.set( imin, imax, istep, repeat, blockrepeat, singlerepeat );
  if ( iincrement <= 0 )
    Range.setLargeIncrement();
  else
    Range.setIncrement( iincrement );
  Range.setSequence( ishuffle );
  int prevrepeat = 0;
  Results.clear();
  Results.resize( Range.size(), Data( delay, duration, 1.0/samplerate,
				      incurrent >= 0 ) );

  // plot:
  P.lock();
  P[0].setXLabel( "Time [ms]" );
  P[0].setXRange( -1000.0*delay, 2000.0*duration );
  P[0].setYLabel( "Membrane potential [" + VUnit + "]" );
  P[1].setXLabel( "Current [" + IUnit + "]" );
  P[1].setXRange( imin, imax );
  P[1].setYLabel( "Membrane potential [" + VUnit + "]" );
  P.unlock();

  // signal:
  OutData signal( duration, 1.0/samplerate );
  signal.setTrace( outcurrent );
  signal.setDelay( delay );
  double dccurrent = metaData( "Cell" ).number( "dc", IUnit );

  // write stimulus:
  sleep( pause );
  for ( Range.reset(); ! Range && softStop() == 0; ++Range ) {

    if ( prevrepeat < Range.currentRepetition() ) {
      if ( Range.currentRepetition() == 1 ) {
	Range.setSequence( shuffle );
	Range.update();
      }
      prevrepeat = Range.currentRepetition();
    }

    double amplitude = *Range;
    if ( fabs( amplitude ) < 1.0e-8 )
      amplitude = 0.0;

    Str s = "Current <b>" + Str( amplitude ) + " " + IUnit +"</b>";
    s += ",  Loop <b>" + Str( Range.count()+1 ) + "</b>";
    message( s );

    timeStamp();
    signal.setIdent( "I=" + Str( amplitude ) + IUnit );
    signal = amplitude;
    signal.back() = dccurrent;
    write( signal );
    if ( signal.failed() ) {
      if ( signal.overflow() ) {
	printlog( "Requested amplitude I=" + Str( amplitude ) + IUnit + "too high!" );
	for ( int k = Range.size()-1; k >= 0; k-- ) {
	  if ( Range[k] > signal.maxValue() )
	    Range.setSkip( k );
	  else
	    break;
	}
	Range.noCount();
	continue;
      }
      else if ( signal.underflow() ) {
	printlog( "Requested amplitude I=" + Str( amplitude ) + IUnit + "too small!" );
	for ( int k = 0; k < Range.size(); k++ ) {
	  if ( Range[k] < signal.minValue() )
	    Range.setSkip( k );
	  else
	    break;
	}
	Range.noCount();
	continue;
      }
      else {
	warning( signal.errorText() );
	return Failed;
      }
    }

    sleep( delay + 2.0*duration + 0.01 );
    if ( interrupt() ) {
      if ( Range.count() < 1 )
	state = Aborted;
      break;
    }

    Results[Range.pos()].I = amplitude;
    Results[Range.pos()].DC = dccurrent;
    Results[Range.pos()].analyze( Range.count(), trace( involtage ),
				  events( SpikeEvents[involtage] ), 
				  incurrent >= 0 ? &trace( incurrent ) : 0,
				  IInFac, delay, duration, ton, sswidth );

    if ( Results[Range.pos()].VSS < vmin ) {
      Range.setSkipBelow( Range.pos() );
      Range.noCount();
    }
    if ( Results[Range.pos()].SpikeCount > 1 ) {
      Range.setSkipAbove( Range.pos() );
      Range.noCount();
    }

    plot( duration );
    sleepOn( duration + pause );
    if ( interrupt() ) {
      if ( Range.count() < 1 )
	state = Aborted;
      break;
    }
  }

  if ( state == Completed )
    save();

  return state;
}


void VICurve::plot( double duration )
{
  P.lock();

  // membrane voltage:
  const Data &data = Results[Range.pos()];
  P[0].clear();
  P[0].setTitle( "I=" + Str( data.I, 0, 2, 'f' ) + IUnit );
  P[0].plotVLine( 0, Plot::White, 2 );
  P[0].plotVLine( 1000.0*duration, Plot::White, 2 );
  if ( boolean( "plotstdev" ) ) {
    P[0].plot( data.MeanTrace+data.StdevTrace, 1000.0, Plot::Orange, 1, Plot::Solid );
    P[0].plot( data.MeanTrace-data.StdevTrace, 1000.0, Plot::Orange, 1, Plot::Solid );
  }
  P[0].plot( data.MeanTrace, 1000.0, Plot::Red, 3, Plot::Solid );

  // V-I curves:
  P[1].clear();
  MapD om, pm, sm, rm;
  double imin = Results[Range.next( 0 )].I;
  double imax = imin;
  for ( unsigned int k=Range.next( 0 );
	k<Results.size();
	k=Range.next( ++k ) ) {
    imax = Results[k].I;
    rm.push( Results[k].I, Results[k].VRest );
    om.push( Results[k].I, Results[k].VOn );
    sm.push( Results[k].I, Results[k].VSS );
    pm.push( Results[k].I, Results[k].VPeak );
  }
  P[1].setXRange( imin, imax );
  P[1].plot( rm, 1.0, Plot::Cyan, 3, Plot::Solid, Plot::Circle, 6, Plot::Cyan, Plot::Cyan );
  P[1].plot( om, 1.0, Plot::Green, 3, Plot::Solid, Plot::Circle, 6, Plot::Green, Plot::Green );
  P[1].plot( sm, 1.0, Plot::Red, 3, Plot::Solid, Plot::Circle, 6, Plot::Red, Plot::Red );
  P[1].plot( pm, 1.0, Plot::Orange, 3, Plot::Solid, Plot::Circle, 6, Plot::Orange, Plot::Orange );
  int c = Range.pos();
  MapD am;
  am.push( Results[c].I, Results[c].VRest );
  am.push( Results[c].I, Results[c].VOn );
  am.push( Results[c].I, Results[c].VSS );
  am.push( Results[c].I, Results[c].VPeak );
  P[1].plot( am, 1.0, Plot::Transparent, 3, Plot::Solid, Plot::Circle, 8, Plot::Yellow, Plot::Transparent );

  P.unlock();
  P.draw();
}


void VICurve::save( void )
{
  saveData();
  saveTrace();
}


void VICurve::saveData( void )
{
  ofstream df( addPath( "vicurve-data.dat" ).c_str(),
	       ofstream::out | ofstream::app );

  Header.save( df, "# " );
  df << "# status:\n";
  stimulusData().save( df, "#   " );
  df << "# settings:\n";
  settings().save( df, "#   " );
  df << '\n';

  TableKey datakey;
  datakey.addLabel( "Stimulus" );
  datakey.addNumber( "I", IUnit, "%6.3f" );
  datakey.addNumber( "IDC", IUnit, "%6.3f" );
  datakey.addLabel( "Rest" );
  datakey.addNumber( "Vrest", VUnit, "%6.1f" );
  datakey.addNumber( "s.d.", VUnit, "%6.1f" );
  datakey.addLabel( "Steady-state" );
  datakey.addNumber( "Vss", VUnit, "%6.1f" );
  datakey.addNumber( "s.d.", VUnit, "%6.1f" );
  datakey.addLabel( "Peak" );
  datakey.addNumber( "Vpeak", VUnit, "%6.1f" );
  datakey.addNumber( "s.d.", VUnit, "%6.1f" );
  datakey.addNumber( "tpeak", "ms", "%6.1f" );
  datakey.addLabel( "Onset" );
  datakey.addNumber( "Vpeak", VUnit, "%6.1f" );
  datakey.addNumber( "s.d.", VUnit, "%6.1f" );
  datakey.saveKey( df );

  for ( unsigned int j=Range.next( 0 );
	j<Results.size();
	j=Range.next( ++j ) ) {
    datakey.save( df, Results[j].I, 0 );
    datakey.save( df, Results[j].DC );
    datakey.save( df, Results[j].VRest );
    datakey.save( df, Results[j].VRestsd );
    datakey.save( df, Results[j].VSS );
    datakey.save( df, Results[j].VSSsd );
    datakey.save( df, Results[j].VPeak );
    datakey.save( df, Results[j].VPeaksd );
    datakey.save( df, 1000.0*Results[j].VPeakTime );
    datakey.save( df, Results[j].VOn );
    datakey.save( df, Results[j].VOnsd );
    df << '\n';
  }
  df << "\n\n";
}


void VICurve::saveTrace( void )
{
  ofstream df( addPath( "vicurve-trace.dat" ).c_str(),
	       ofstream::out | ofstream::app );

  Header.save( df, "# " );
  df << "# status:\n";
  stimulusData().save( df, "#   " );
  df << "# settings:\n";
  settings().save( df, "#   " );
  df << '\n';

  TableKey datakey;
  datakey.addNumber( "t", "ms", "%6.2f" );
  datakey.addNumber( "V", VUnit, "%6.2f" );
  datakey.addNumber( "s.d.", VUnit, "%6.2f" );
  if ( ! Results[0].MeanCurrent.empty() )
    datakey.addNumber( "I", IUnit, "%6.3f" );

  for ( unsigned int j=Range.next( 0 );
	j<Results.size();
	j=Range.next( ++j ) ) {
    df << "#     I: " << Str( Results[j].I ) << IUnit << '\n';
    df << "#    DC: " << Str( Results[j].DC ) << IUnit << '\n';
    df << "# VRest: " << Str( Results[j].VRest ) << VUnit << '\n';
    df << "#   VOn: " << Str( Results[j].VOn ) << VUnit << '\n';
    df << "# VPeak: " << Str( Results[j].VPeak ) << VUnit << '\n';
    df << "#   VSS: " << Str( Results[j].VSS ) << VUnit << '\n';
    df << '\n';
    datakey.saveKey( df );
    for ( int k=0; k<Results[j].MeanTrace.size(); k++ ) {
      datakey.save( df, 1000.0*Results[j].MeanTrace.pos( k ), 0 );
      datakey.save( df, Results[j].MeanTrace[k] );
      datakey.save( df, Results[j].StdevTrace[k] );
      if ( ! Results[j].MeanCurrent.empty() )
	datakey.save( df, Results[j].MeanCurrent[k] );
      df << '\n';
    }
    df << '\n';
  }
  df << '\n';
}


VICurve::Data::Data( double delay, double duration, double stepsize,
		     bool current )
  : DC( 0.0 ),
    I( 0.0 ),
    VRest( 0.0 ),
    VRestsd( 0.0 ),
    VOn( 0.0 ),
    VOnsd( 0.0 ),
    VPeak( 0.0 ),
    VPeaksd( 0.0 ),
    VPeakTime( 0.0 ),
    VSS( 0.0 ),
    VSSsd( 0.0 ),
    SpikeCount( 0.0 ),
    MeanTrace( -delay, 2.0*duration, stepsize, 0.0 ),
    SquareTrace( -delay, 2.0*duration, stepsize, 0.0 ),
    StdevTrace( -delay, 2.0*duration, stepsize, 0.0 )
{
  if ( current )
    MeanCurrent = MeanTrace;
  else
    MeanCurrent.clear();
}


void VICurve::Data::analyze( int count, const InData &intrace,
			     const EventData &spikes,
			     const InData *incurrent, double iinfac,
			     double delay, double duration,
			     double ton, double sswidth )
{
  // update averages:
  int inx = intrace.signalIndex() - MeanTrace.index( 0.0 );
  for ( int k=0; k<MeanTrace.size() && inx+k<intrace.size(); k++ ) {
    double v = intrace[inx+k];
    MeanTrace[k] += (v - MeanTrace[k])/(count+1);
    SquareTrace[k] += (v*v - SquareTrace[k])/(count+1);
    StdevTrace[k] = sqrt( SquareTrace[k] - MeanTrace[k]*MeanTrace[k] );
    if ( incurrent != 0 ) {
      double c = iinfac*(*incurrent)[inx+k];
      MeanCurrent[k] += (c - MeanCurrent[k])/(count+1);
    }
  }

  // stimulus amplitude:
  if ( ! MeanCurrent.empty() ) {
    DC = MeanCurrent.mean( -delay, 0.0 );
    I = MeanCurrent.mean( 0.0, duration );
  }

  // resting potential:
  VRest = MeanTrace.mean( -delay, 0.0 );
  VRestsd = MeanTrace.stdev( -delay, 0.0 );

  // steady-state potential:
  VSS = MeanTrace.mean( duration-sswidth, duration );
  VSSsd = MeanTrace.stdev( duration-sswidth, duration );

  // onset potential:
  VOn = MeanTrace[ ton ];
  VOnsd = StdevTrace[ ton ];

  // peak potential:
  VPeak = VRest;
  int vpeakinx = 0;
  if ( VSS > VRest )
    vpeakinx = MeanTrace.maxIndex( VPeak, 0.0, duration-sswidth );
  else
    vpeakinx = MeanTrace.minIndex( VPeak, 0.0, duration-sswidth );
  VPeaksd = StdevTrace[vpeakinx];
  if ( fabs( VPeak - VSS ) <= VSSsd ) {
    VPeak = VSS;
    VPeaksd = VSSsd;
    VPeakTime = 0.0;
  }
  else
    VPeakTime = MeanTrace.pos( vpeakinx );

  // spikes:
  double sigtime = spikes.signalTime();
  SpikeCount = spikes.count( sigtime, sigtime+duration );
}


addRePro( VICurve );

}; /* namespace patchclamp */

#include "moc_vicurve.cc"