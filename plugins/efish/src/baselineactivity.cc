/*
  efish/baselineactivity.cc
  ISI statistics and EOD locking of baseline activity.

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
#include <iomanip>
#include <relacs/stats.h>
#include <relacs/kernel.h>
#include <relacs/tablekey.h>
#include <relacs/optwidget.h>
#include <relacs/detector.h>
#include <relacs/filter.h>
#include <relacs/efish/baselineactivity.h>
using namespace relacs;

namespace efish {


BaselineActivity::BaselineActivity( void )
  : RePro( "BaselineActivity", "BaselineActivity", "efish",
	   "Jan Benda", "2.1", "Nov 26, 2009" ),
    P( 1, this )
{
  // parameter:
  Duration = 0.3;
  Repeats = 0;
  ISIMax = 0.02;
  ISIStep = 0.0002;
  RateDeltaT = 0.00005;   // seconds
  RateTMax = 0.002;
  EODDuration = 2.0;
  SaveEODTrace = false;
  SaveEODTimes = false;
  AutoDetect = 1;
  SpikesFastDelay = 1.0;
  SpikesFastDecay = 4.0;
  SpikesSlowDelay = 1.0;
  SpikesSlowDecay = 10.0;
  BeatStep = 0.005;
  ChirpStep = 2.0;
  ChirpMin = 10.0;
  Adjust = true;

  // add some parameter as options:
  addLabel( "Timing" ).setStyle( OptWidget::Bold );
  addNumber( "duration", "Duration", Duration, 0.01, 100.0, 0.05, "sec", "ms" );
  addInteger( "repeats", "Repeats", Repeats, 0, 100000, 2 );
  addLabel( "Analysis" ).setStyle( OptWidget::Bold );
  addNumber( "isimax", "Maximum interspike-interval", 
	      ISIMax, 0.001, 1.0, 0.001, "sec", "ms" );
  addNumber( "isistep", "Interspike-interval resolution",
	      ISIStep, 0.0001, 0.01, 0.0002, "sec", "ms" );
  addNumber( "ratedt", "Resolution of firing rate",
	     RateDeltaT, 0.00001, 0.01, 0.00002, "s", "ms" );
  addNumber( "ratetmax", "Maximum time for firing rate",
	     RateTMax, 0.00001, 0.01, 0.00002, "s", "ms" );
  addLabel( "Files" ).setStyle( OptWidget::Bold );
  addNumber( "eodduration", "EOD duration",
	      EODDuration, 0.1, 10.0, 0.1, "sec", "ms" );
  addBoolean( "saveeodtrace", "Save EOD trace", SaveEODTrace );
  addBoolean( "saveeodtimes", "Save EOD times", SaveEODTimes );
  addLabel( "Control" ).setStyle( OptWidget::Bold );
  addText( "auto", "Automatically set detector parameter?", "never|once|always", 0, OptWidget::SelectText );
  addBoolean( "adjust", "Adjust input gain?", Adjust );

  // header for files:
  Header.addInteger( "index" );
  Header.addInteger( "trace" );
  Header.addNumber( "EOD rate", "Hz", "%.1f" );
  Header.addNumber( "EOD period", "ms", "%.3f" );
  Header.addNumber( "duration", "sec", "%.3f" );
  for ( int k=0; k<MaxSpikeTraces; k++ ) {
    if ( SpikeEvents[k] >= 0 ) {
      Str ns( k+1 );
      Header.addNumber( "firing frequency"+ns, "Hz", "%.1f" );
      Header.addNumber( "CV"+ns, "1", "%.3f" );
      Header.addNumber( "p-value"+ns, "1", "%.3f" );
    }
  }
  Header.addText( "session time" );
  Header.addLabel( "settings:" );

  // variables:
  Spikes.resize( MaxSpikeTraces );
  for ( int k=0; k<MaxSpikeTraces; k++ ) {
    ISIH[k] = 0;
    Trials[k] = 0; 
    Spikes[k].clear();
    SpikeRate[k] = 0;
    EODSpikes[k].clear();
  }
  NerveAmplP.clear();
  NerveAmplT.clear();
  NerveAmplM.clear();
  EODCycle = 0;
  EODPeriod = 0.0;
  EODRate = 0.0;
  FirstSignal = 0.0;
  SearchDuration = 0.0;
  EODTimes.clear();
  Count = 0; 
}


BaselineActivity::~BaselineActivity( void )
{
}


int BaselineActivity::main( void )
{
  Count = 0; 

  // get options:
  Duration = number( "duration" );
  Repeats = integer( "repeats" );
  ISIMax = number( "isimax" );
  ISIStep = number( "isistep" );
  RateDeltaT = number( "ratedt" );
  RateTMax = number( "ratetmax" );
  EODDuration = number( "eodduration" );
  SaveEODTrace = boolean( "saveeodtrace" );
  SaveEODTimes = boolean( "saveeodtimes" );
  AutoDetect = index( "auto" );
  Adjust = boolean( "adjust" );

  // don't print repro message:
  if ( Repeats <= 0 )
    noMessage();

  // don't save files:
  if ( Repeats <= 0 )
    noSaving();

  // plot trace:
  plotToggle( true, true, Duration, 0.0 );

  // plot:
  P.lock();
  P.resize( SpikeTraces*2+NerveTraces, 2, ( SpikeTraces < 2 ) );
  double pdx = 0.5;
  double pdy = 1.0;
  if ( SpikeTraces > 1 ) {
    pdx = 1.0 / SpikeTraces;
    pdy = 1.0 / ( 2 + NerveTraces );
  }
  else {
    pdx = 0.5;
    pdy = 1.0 / ( SpikeTraces + NerveTraces );
  }
  int n=0;
  for ( int k=0; k<MaxSpikeTraces; k++ ) {
    if ( SpikeEvents[k] >= 0 ) {
      Str ns = "";
      if ( SpikeTraces > 1 )
	ns = " " + Str( n+1, 0 );
      if ( SpikeTraces > 1 )
	P[2*n].setOrigin( n*pdx, ( NerveTraces + 1 )*pdy );
      else
	P[2*n].setOrigin( 0.0, NerveTraces*pdy );
      P[2*n].setSize( pdx, pdy );
      P[2*n].setTitle( "ISI Histogram" + ns );
      P[2*n].setTitlePos( 0.0, Plot::Graph, 0.0, Plot::SecondAxis, Plot::Left );
      P[2*n].setXLabel( "ISI [ms]" );
      P[2*n].noXGrid();
      P[2*n].setYRange( 0.0, Plot::AutoScale );
      P[2*n].setYFallBackRange( 0.0, 10.0 );
      P[2*n].setMinYTics( 1.0 );
      P[2*n].setYLabel( "" );
      P[2*n].setLMarg( 5.0 );
      
      if ( SpikeTraces > 1 )
	P[2*n+1].setOrigin( n*pdx, NerveTraces*pdy );
      else
	P[2*n+1].setOrigin( pdx, NerveTraces*pdy );
      P[2*n+1].setSize( pdx, pdy );
      P[2*n+1].setTitle( "EOD Firing Rate" + ns );
      P[2*n+1].setTitlePos( 0.0, Plot::Graph, 0.0, Plot::SecondAxis, Plot::Left );
      P[2*n+1].setXLabel( "Time [ms]" );
      P[2*n+1].setYRange( 0.0, Plot::AutoMinScale );
      P[2*n+1].setYFallBackRange( 0.0, 100.0 );
      P[2*n+1].setMinYTics( 100.0 );
      P[2*n+1].setYLabel( "" );
      P[2*n+1].setLMarg( 5.0 );

      n++;
    }
  }
  if ( NerveTrace[0] >= 0 ) {
    P[2*n].setOrigin( 0.0, 0.0 );
    P[2*n].setSize( 1.0, pdy );
    P[2*n].setTitle( "Nerve Potential" );
    P[2*n].setTitlePos( 0.0, Plot::Graph, 0.0, Plot::SecondAxis, Plot::Left );
    P[2*n].setXLabel( "Time / ms" );
    P[2*n].setAutoScaleY();
    P[2*n].setYLabel( "" );
    P[2*n].setLMarg( 5.0 );
    P[2*n].setRMarg( 1.0 );
    P[2*n].setBMarg( 5.5 );
  }
  P.unlock();

  // data:
  FirstSignal = 0.0;
  SearchDuration = 0.0;

  for ( int k=0; k<MaxSpikeTraces; k++ ) {
    if ( SpikeEvents[k] >= 0 ) {
      ISIH[k] = new SampleDataD( 0.0, 2.0*ISIMax, ISIStep );
      Trials[k] = 0;
      SpikeRate[k] = new SampleDataD( 0.0, RateTMax, RateDeltaT );
      Spikes[k].clear();
      EODSpikes[k].clear();
      if ( Repeats <= 0 ) {
	Spikes[k].reserve( int( Duration * 1000.0 ) );
	EODSpikes[k].reserve( int( Duration * 1000.0 ) );
      }
      else {
	Spikes[k].reserve( int( Duration * Repeats * 1000.0 ) );
	EODSpikes[k].reserve( int( Duration * Repeats * 1000.0 ) );
      }
    }
  }
  if ( Repeats <= 0 )
    EODTimes.reserve( int( Duration * 1500.0 ) );
  else {
    EODTimes.reserve( int( Duration * Repeats * 1500.0 ) );
  }
  EODCycle = new SampleDataD( 0.0, RateTMax, RateDeltaT );
  EODPeriod = 0.0;
  EODRate = 0.0;
  if ( NerveTrace[0] >= 0 ) {
    NerveAmplP.clear();
    NerveAmplT.clear();
    NerveAmplM.clear();
    NerveAmplP.reserve( EODTimes.capacity() );
    NerveAmplT.reserve( EODTimes.capacity() );
    NerveAmplM.reserve( EODTimes.capacity() );
  }

  if ( AutoDetect > 0 && Repeats <= 0 ) {
    // setup Spike Detector:
    for ( int k=0; k<MaxSpikeTraces; k++ ) {
      if ( SpikeEvents[k] >= 0 ) {
	lockDetectorEvents( SpikeEvents[k] );
	detectorEventsOpts( SpikeEvents[k] ).setNumber( "delay", SpikesFastDelay );
	detectorEventsOpts( SpikeEvents[k] ).setNumber( "decay", SpikesFastDecay );
	unlockDetectorEvents( SpikeEvents[k] );
      }
    }
    // setup Beat detector:
    if ( LocalBeatPeakEvents[1] >= 0 && 
	 ( totalRuns() <= 0 || AutoDetect > 1 ) ) {
      double beatthresh = BeatStep * trace( LocalEODTrace[1] ).maxValue();
      lockDetectorEvents( LocalBeatPeakEvents[1] );
      detectorEventsOpts( LocalBeatPeakEvents[1] ).setNumber( "minthresh", beatthresh );
      detectorEventsOpts( LocalBeatPeakEvents[1] ).setNumber( "threshold", beatthresh );
      unlockDetectorEvents( LocalBeatPeakEvents[1] );
    }
    // setup Chirp detector:
    if ( ChirpEvents >= 0 &&
	 ( totalRuns() <= 0 || AutoDetect > 1 ) ) {
      lockDetectorEvents( ChirpEvents );
      detectorEventsOpts( ChirpEvents ).setNumber( "minthresh", ChirpMin );
      detectorEventsOpts( ChirpEvents ).setNumber( "threshold", ChirpMin );
      unlockDetectorEvents( ChirpEvents );
    }
  }

  // trigger:
  // XXX  setupTrigger( traces(), events() );

  // stimulus:
  OutData signal( 10, 0.0001 );
  signal.setDelay( 0.0 );
  signal.setIdent( "zero" );
  signal.setTraceName( "EField-global" );
  signal.setIntensity( 0.0 );
  signal = 0.0;

  for ( int count=0;
	( Repeats <= 0 || count < Repeats ) && softStop() == 0; 
	count++ ) {

    // message:
    if ( Repeats == 0 && count%100 == 0 )
      message( "Search ..." );
    else if ( Repeats > 0 )
      message( "Search Loop <b>" + Str( count ) + "</b>" );

    // output stimulus:
    write( signal );
    sleep( Duration );
    if ( interrupt() )
      break;
    testWrite( signal );
    // signal failed?
    if ( !signal.success() ) {
      if ( signal.busy() ) {
	warning( "Output still busy!<br> Probably missing trigger.<br> Output of this signal software-triggered.", 2.0 );
	signal.setStartSource( 0 );
	signal.setPriority();
	write( signal );
	sleep( Duration );
	// trigger:
	// XXX setupTrigger( traces(), events() );
      }
      else {
	string s = "Output of stimulus failed!<br>Error code is <b>";
	s += Str( signal.error() ) + ": " + signal.errorStr() + "</b>";
	warning( s, 2.0 );
	write( signal );
	sleep( Duration );
      }
      stop();
      return Failed;
    }

    // adjust gain of daq board:
    if ( Adjust ) {
      for ( int k=0; k<MaxSpikeTraces; k++ )
	if ( SpikeTrace[k] >= 0 )
	  adjust( trace( SpikeTrace[k] ), Duration, 0.8 );
      if ( NerveTrace[0] >= 0 )
	adjust( trace( NerveTrace[0] ), Duration, 0.8 );
      if ( EODTrace >= 0 ) {
	double val1 = trace( EODTrace ).maxAbs( trace( EODTrace ).signalTime(), 0.1 );
	if ( val1 > 0.0 )
	  adjustGain( trace( EODTrace ), val1 );
      }
      double val2 = trace( LocalEODTrace[0] ).maxAbs( trace( LocalEODTrace[0] ).signalTime(), 0.1 );
      if ( val2 > 0.0 )
	adjustGain( trace( LocalEODTrace[0] ), 1.25 * val2 );
      activateGains();
    }

    // analyze:
    analyze();
    plot();

  }

  setMessage();
  save();
  stop();
  return Completed;
}


void BaselineActivity::stop( void )
{
  if ( AutoDetect > 0 && Repeats <= 0 ) {
    for ( int k=0; k<MaxSpikeTraces; k++ ) {
      if ( SpikeEvents[k] >= 0 ) {
	lockDetectorEvents( SpikeEvents[k] );
	detectorEventsOpts( SpikeEvents[k] ).setNumber( "delay", SpikesSlowDelay );
	detectorEventsOpts( SpikeEvents[k] ).setNumber( "decay", SpikesSlowDecay );
	unlockDetectorEvents( SpikeEvents[k] );
      }
    }
  }
  for ( int k=0; k<MaxSpikeTraces; k++ ) {
    if ( SpikeEvents[k] >= 0 ) {
      delete ISIH[k];
      delete SpikeRate[k];
    }
    ISIH[k] = 0;
    SpikeRate[k] = 0;
    Spikes[k].clear();
    Spikes[k].reserve( 0 );
    EODSpikes[k].clear();
    EODSpikes[k].reserve( 0 );
  }
  NerveAmplP.free();
  NerveAmplT.free();
  NerveAmplM.free();
  delete EODCycle;
  EODCycle = 0;
  EODTimes.clear();
  EODTimes.free( 0 );
}


void BaselineActivity::saveSpikes( int trace )
{
  // create file:
  ofstream df( addPath( "basespikes" + Str(trace+1) + ".dat" ).c_str(),
	       ofstream::out | ofstream::app );
  if ( ! df.good() )
    return;

  // write header and key:
  Header.save( df, "# " );
  Options::save( df, "#   " );
  df << '\n';
  TableKey key;
  key.addNumber( "time", "ms", "%8.2f" );
  key.saveKey( df, true, false );

  // write data into file:
  if ( Spikes[trace].size() > 0 ) {
    for ( int j=0; j<Spikes[trace].size(); j++ ) {
      key.save( df, 1000.0 * Spikes[trace][j], 0 );
      df << '\n';
    }
  }
  else
    df << "  -0\n";
  df << '\n' << '\n';
}


void BaselineActivity::saveISIH( int trace )
{
  // create file:
  ofstream df( addPath( "baseisih" + Str(trace+1) + ".dat" ).c_str(),
	       ofstream::out | ofstream::app );
  if ( ! df.good() )
    return;

  // write header and key:
  Header.save( df, "# " );
  Options::save( df, "#   " );
  df << '\n';
  TableKey key;
  key.addNumber( "t", "ms", "%5.2f" );
  key.addNumber( "eod", "1", "%6.3f" );
  key.addNumber( "n", "1", "%6.0f" );
  key.addNumber( "p", "1", "%5.3f" );
  key.saveKey( df, true, false );

  // normalization factor:
  double norm = 0.0;
  for ( int j=0; j<ISIH[trace]->size(); j++ )
    norm += (*ISIH[trace])[j];

  // write data into file:
  for ( int j=0; j<ISIH[trace]->size(); j++ ) {
    key.save( df, 1000.0 * ISIH[trace]->pos( j ), 0 );
    key.save( df, ISIH[trace]->pos( j ) * EODRate, 1 );
    key.save( df, (*ISIH[trace])[j] );
    key.save( df, (*ISIH[trace])[j]/norm );
    df << '\n';
  }

  df << '\n' << '\n';
}


void BaselineActivity::saveRate( int trace )
{
  // create file:
  ofstream df( addPath( "baserate" + Str(trace+1) + ".dat" ).c_str(),
	       ofstream::out | ofstream::app );
  if ( ! df.good() )
    return;

  // write header and key:
  Header.save( df, "# " );
  Options::save( df, "#   " );
  df << '\n';
  TableKey key;
  key.addNumber( "time", "ms", "%5.3f" );
  key.addNumber( "rate", "Hz", "%5.1f" );
  key.addNumber( "ampl", EOD2Unit, "%6.3f" );
  key.saveKey( df, true, false );

  // write data into file:
  for ( int j=0; j<SpikeRate[trace]->size(); j++ ) {
    key.save( df, 1000.0 * SpikeRate[trace]->pos( j ), 0 );
    key.save( df, (*SpikeRate[trace])[j] );
    key.save( df, (*EODCycle)[j] );
    df << '\n';
  }

  df << '\n' << '\n';
}


void BaselineActivity::saveNerve( void )
{
  // create file:
  ofstream df( addPath( "basenerveampl.dat" ).c_str(),
	       ofstream::out | ofstream::app );
  if ( ! df.good() )
    return;

  // write header and key:
  Header.save( df, "# " );
  Options::save( df, "#   " );
  df << '\n';
  TableKey key;
  key.addLabel( "peak" );
  key.addNumber( "time", "ms", "%9.2f" );
  key.addNumber( "ampl", "uV", "%6.1f" );
  key.addLabel( "trough" );
  key.addNumber( "time", "ms", "%9.2f" );
  key.addNumber( "ampl", "uV", "%6.1f" );
  key.addLabel( "average" );
  key.addNumber( "time", "ms", "%9.2f" );
  key.addNumber( "ampl", "uV", "%6.1f" );
  key.saveKey( df, true, true );

  // write data into file:
  for ( int k=0; k<NerveAmplP.size() &&
	  k<NerveAmplT.size() &&
	  k<NerveAmplM.size(); k++ ) {
    key.save( df, 1000.0 * NerveAmplP.x(k), 0 );
    key.save( df, NerveAmplP.y(k) );
    key.save( df, 1000.0 * NerveAmplT.x(k), 0 );
    key.save( df, NerveAmplT.y(k) );
    key.save( df, 1000.0 * NerveAmplM.x(k), 0 );
    key.save( df, NerveAmplM.y(k) );
    df << '\n';
  }
  df << '\n' << '\n';
}


void BaselineActivity::saveEODTrace( void )
{
  // create file:
  ofstream df( addPath( "baseeodtrace.dat" ).c_str(),
	       ofstream::out | ofstream::app );
  if ( ! df.good() )
    return;

  // write header and key:
  Header.save( df, "# " );
  Options::save( df, "#   " );
  df << '\n';
  TableKey key;
  key.addNumber( "t", "ms", "%7.2f" );
  key.addNumber( "EOD", EOD2Unit, "%6.3f" );
  key.saveKey( df, true, false );

  // write data into file:
  long inx = trace( LocalEODTrace[0] ).indices( EODDuration );
  for ( int k=0; k<inx; k++ ) {
    key.save( df, 1000.0 * trace( LocalEODTrace[0] ).interval( k ), 0 );
    key.save( df, trace( LocalEODTrace[0] )[trace( LocalEODTrace[0] ).size()-inx-k] );
    df << '\n';
  }

  df << '\n' << '\n';
}


void BaselineActivity::saveEODTimes( void )
{
  // create file:
  ofstream df( addPath( "baseeodtimes.dat" ).c_str(),
	       ofstream::out | ofstream::app );
  if ( ! df.good() )
    return;

  // write header and key:
  Header.save( df, "# " );
  Options::save( df, "#   " );
  df << '\n';
  TableKey key;
  key.addNumber( "time", "ms", "%8.2f" );
  key.saveKey( df, true, false );

  // write data into file:
  for ( int k=0; k<EODTimes.size(); k++ ) {
    key.save( df, 1000.0 * EODTimes[k], 0 );
    df << '\n';
  }

  df << '\n' << '\n';
}


void BaselineActivity::save( void )
{
  if ( Repeats <= 0 )
    return;

  Header.setInteger( "index", totalRuns()-1 );
  Header.setInteger( "trace", -1 );
  Header.setNumber( "EOD rate", EODRate );
  Header.setNumber( "EOD period", 1000.0 * EODPeriod );
  Header.setNumber( "duration", SearchDuration );
  for ( int k=0; k<MaxSpikeTraces; k++ ) {
    if ( SpikeEvents[k] >= 0 ) {
      Str ns( k+1 );
      Header.setNumber( "firing frequency"+ns, FRate[k] );
      Header.setNumber( "CV"+ns, CV[k] );
      Header.setNumber( "p-value"+ns, PValue[k] );
    }
  }
  Header.setText( "session time", sessionTimeStr() );  

  for ( int trace=0; trace<MaxSpikeTraces; trace++ ) {
    if ( SpikeEvents[trace] >= 0 ) {
      Header.setInteger( "trace", trace );
      saveSpikes( trace );
      saveISIH( trace );
      saveRate( trace );
      Filter *d = detectorEvents( SpikeEvents[trace] );
      if ( d != 0 )
	d->save();
    }
  }
  if ( NerveTrace[0] >= 0 ) {
    Header.setInteger( "trace", 0 );
    saveNerve();
  }

  if ( SaveEODTrace )
    saveEODTrace();
  if ( SaveEODTimes )
    saveEODTimes();  
}


void BaselineActivity::plot( void )
{
  P.lock();

  int n=0;
  for ( int k=0; k<MaxSpikeTraces; k++ ) {
    if ( SpikeEvents[k] >= 0 ) {
      P[2*n].clear();
      if ( ! P[2*n].zoomedXRange() )
	P[2*n].setXRange( 0.0, 1000.0 * ISIMax );
      for ( double x=EODPeriod; x < ISIMax; x += EODPeriod )
	P[2*n].plotVLine( 1000.0 * x );
      P[2*n].plot( *ISIH[k], 1000.0, Plot::Transparent, 0, Plot::Solid, Plot::Box, 0, Plot::Red, Plot::Red );

      P[2*n+1].clear();
      double tmax = SpikeRate[k]->rangeBack();
      if ( tmax > EODPeriod )
	tmax = EODPeriod;
      if ( ! P[2*n+1].zoomedXRange() )
	P[2*n+1].setXRange( 0.0, 1000.0 * tmax );

      double ss = 1.0/EODSpikes[k].size();
      if ( ss < 0.01 )
	ss = 0.01;
      int i = 0;
      for ( unsigned int j = 0; j < EODSpikes[k].size(); j++ ) {
	i++;
	P[2*n+1].plot( EODSpikes[k][j], 1000.0, 1.0 - i*ss, Plot::Graph, 2, Plot::StrokeUp,
		       ss, Plot::Graph, Plot::Red, Plot::Red );
      }
      
      P[2*n+1].plot( *SpikeRate[k], 1000.0, Plot::Yellow, 2, Plot::Solid );

      P[2*n+1].plot( *EODCycle, 1000.0, Plot::Green, 2, Plot::Solid );
      P[2*n+1].back().setAxis( Plot::X1Y2 );

      n++;
    }
  }

  if ( NerveTrace[0] >= 0 ) {
    P[2*n].clear();
    double l = NerveAmplM.x().back();
    if ( ! P[2*n].zoomedXRange() )
      P[2*n].setXRange( 1000.0*(l-Duration), 1000.0*l );
    P[2*n].plot( NerveAmplM, 1000.0, Plot::Cyan, 1, Plot::Solid );
  }
  P.unlock();

  P.draw();
}


void BaselineActivity::analyzeSpikes( const EventData &se, int k )
{
  // interspike interval histogram:
  *ISIH[k] = 0.0;
  se.addIntervalHistogram( FirstSignal, FirstSignal+SearchDuration, *ISIH[k] );
  double sd = 0.0;
  double isi = se.interval( FirstSignal, FirstSignal+SearchDuration, &sd );
  CV[k] = isi > 1.0e-6 ? sd / isi : 0.0;

  // EOD spike rate:
  if ( Repeats <= 0 || Count == 0 ) {
    Trials[k] = 0;
    *SpikeRate[k] = 0.0;
  }
  for ( int j=0; j<EODTimes.size(); j++ ) {
    se.addRate( *SpikeRate[k], Trials[k], 0.0, EODTimes[j] + FirstSignal );
  }

  // spikes:
  se.copy( FirstSignal, FirstSignal+SearchDuration, Spikes[k] );

  // eod spikes:
  if ( Repeats <= 0 || Count == 0 )
    EODSpikes[k].clear();
  for ( int j=0; j<EODTimes.size(); j++ ) {
    double lat = se.latency( EODTimes[j] + FirstSignal );
    if ( lat >= 0.0 && lat < EODPeriod )
      EODSpikes[k].push_back( ArrayD( 1, lat ) );
  }

  // Firing Rate:
  Str ns(k+1);
  FRate[k] = se.rate( FirstSignal, FirstSignal+SearchDuration );
  controlOpts( "Session" ).setNumber( "firingrate"+ns, FRate[k] );
  PValue[k] = EODRate > 0.0 ? FRate[k]/EODRate : 0.0;
  controlOpts( "Session" ).setNumber( "pvalue"+ns, PValue[k] );

  if ( Repeats > 0 ) {
    metaData( "Cell" ).setNumber( "Firing Rate"+ns, FRate[k] );
    metaData( "Cell" ).setNumber( "P-Value"+ns, PValue[k] );
  }
}


void BaselineActivity::analyze( void )
{
  if ( LocalEODEvents[0] < 0  )
    return;

  const EventData &eod2 = events( LocalEODEvents[0] );
  const InData &eodt2 = trace( LocalEODTrace[0] );

  if ( Repeats <= 0 || Count == 0 )
    FirstSignal = eod2.signalTime();
  SearchDuration = eodt2.currentTime() - FirstSignal;

  // EOD period & rate:
  EODRate = eod2.frequency( FirstSignal, FirstSignal+SearchDuration );
  EODPeriod = 1.0/EODRate;
  if ( Repeats > 0 )
    metaData( "Cell" ).setNumber( "EOD Frequency", EODRate );

  // EOD times
  eod2.copy( FirstSignal, FirstSignal+SearchDuration, EODTimes );

  // EOD cycle:
  EOD2Unit = eodt2.unit();
  if ( !EODTimes.empty() ) {
    eodt2.copy( EODTimes.previousTime( SearchDuration - 2.0*RateTMax, EODTimes[0] ) + FirstSignal,
		*EODCycle );
    /*
    eodt2.copy( EODTimes[0] + FirstSignal,
		*EODCycle );
		*/
  }
  else
    *EODCycle = 0.0;

  for ( int k=0; k<MaxSpikeTraces; k++ )
    if ( SpikeEvents[k] >= 0 )
      analyzeSpikes( events( SpikeEvents[k] ), k );
  if ( NerveTrace[0] >= 0 ) {
    const InData &nd = trace( NerveTrace[0] );
    // nerve amplitudes:
    // peak and trough amplitudes:
    static double threshold = 0.001;
    InData::const_iterator firstn = nd.begin( FirstSignal );
    InData::const_iterator lastn = nd.end();
    InDataTimeIterator firstntime = nd.timeBegin( FirstSignal );
    EventList peaktroughs( 2, (int)rint(50000.0*Duration), true );
    if ( Repeats <= 0 || Count == 0 ) {
      NerveAmplP.clear();
      NerveAmplT.clear();
      NerveAmplM.clear();
      NerveAmplP.reserve( (int)rint(5000.0*SearchDuration) );
      NerveAmplT.reserve( (int)rint(5000.0*SearchDuration) );
      NerveAmplM.reserve( (int)rint(5000.0*SearchDuration) );
      double min = nd.min( FirstSignal, FirstSignal+4.0/EODRate );
      double max = nd.max( FirstSignal, FirstSignal+4.0/EODRate );
      threshold = 0.5*(max-min);
      if ( threshold < 1.0e-8 )
	threshold = 0.001;
      D.init( firstn, lastn, firstntime );
    }
    D.peakTrough( firstn, lastn, peaktroughs,
		  threshold, threshold, threshold, NerveAcceptEOD );
    // store amplitudes:
    for ( int k=0; k<peaktroughs[0].size(); k++ )
      NerveAmplP.push( peaktroughs[0][k] - FirstSignal, 
		       peaktroughs[0].eventSize( k ) );
    for ( int k=0; k<peaktroughs[1].size(); k++ )
      NerveAmplT.push( peaktroughs[1][k] - FirstSignal, 
		       peaktroughs[1].eventSize( k ) );
    // averaged amplitude:
    double st = (peaktroughs[0].back() - peaktroughs[0].front())/double(peaktroughs[0].size()-1);
    int si = nd.indices( st );
    int inx = nd.indices( FirstSignal + NerveAmplM.size()*st );
    double tt=0.0;
    for ( int k=NerveAmplM.size(); tt+2.0*st<SearchDuration; k++ ) {
      tt = nd.interval( inx ) - FirstSignal;
      NerveAmplM.push( tt, nd.mean( inx, inx+si ) );
      inx += si;
    }
  }

  if ( AutoDetect > 1 && Repeats <= 0 ) {
    // setup Beat detector:
    lockDetectorEvents( LocalBeatPeakEvents[1] );
    if ( events( LocalBeatPeakEvents[1] ).count( trace( LocalEODTrace[0] ).currentTime() - 0.1 ) > 0 ) {
      double beatthresh = detectorEventsOpts( LocalBeatPeakEvents[1] ).number( "minthresh" );
      beatthresh += BeatStep * eodt2.maxValue();
      detectorEventsOpts( LocalBeatPeakEvents[1] ).setNumber( "minthresh", beatthresh );
      beatthresh = detectorEventsOpts( LocalBeatPeakEvents[1] ).number( "threshold" );
      beatthresh += BeatStep * eodt2.maxValue();
      detectorEventsOpts( LocalBeatPeakEvents[1] ).setNumber( "threshold", beatthresh );
    }
    else {
      double beatthresh = detectorEventsOpts( LocalBeatPeakEvents[1] ).number( "minthresh" );
      beatthresh -= BeatStep * eodt2.maxValue();
      if ( beatthresh >= BeatStep * eodt2.maxValue() )
	detectorEventsOpts( LocalBeatPeakEvents[1] ).setNumber( "minthresh", beatthresh );
    }
    unlockDetectorEvents( LocalBeatPeakEvents[1] );
    // setup Chirp detector:
    lockDetectorEvents( ChirpEvents );
    if ( events( ChirpEvents ).count(  trace( EODTrace ).currentTime() - 0.1 ) > 0 ) {
      double chirpmax = detectorEventsOpts( ChirpEvents ).number( "maxthresh" );
      double chirpthresh = detectorEventsOpts( ChirpEvents ).number( "threshold" );
      chirpthresh += ChirpStep;
      if ( chirpthresh < chirpmax )
	detectorEventsOpts( ChirpEvents ).setNumber( "threshold", chirpthresh );
    }
    else {
      double chirpthresh = detectorEventsOpts( ChirpEvents ).number( "threshold" );
      chirpthresh -= ChirpStep;
      if ( chirpthresh >= ChirpMin )
	detectorEventsOpts( ChirpEvents ).setNumber( "threshold", chirpthresh );
    }
    unlockDetectorEvents( ChirpEvents );
  }
}


addRePro( BaselineActivity );

}; /* namespace efish */

#include "moc_baselineactivity.cc"