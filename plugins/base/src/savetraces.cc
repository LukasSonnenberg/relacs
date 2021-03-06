/*
  base/savetraces.cc
  Saves data from selected input traces or events for each run into files

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

#include <deque>
#include <QVBoxLayout>
#include <relacs/tablekey.h>
#include <relacs/digitalio.h>
#include <relacs/savefiles.h>
#include <relacs/base/savetraces.h>
using namespace relacs;

namespace base {


SaveTraces::SaveTraces( void )
  : RePro( "SaveTraces", "base", "Jan Benda", "1.2", "Dec 9, 2014" )
{
  // add some options:
  newSection( "General" );
  addNumber( "duration", "Duration", 0.0, 0.0, 1000000.0, 1.0, "sec" ).setStyle( OptWidget::SpecialInfinite );
  addBoolean( "savedata", "Save raw data", false );
  addBoolean( "split", "Save each run into a separate file", false );
  addBoolean( "dioout", "Indicate recording on a DIO line", false );
  addText( "diodevice", "Name of the digitial I/O device", "dio-1" ).setActivation( "dioout", "true" );
  addInteger( "dioline", "Output line on the digitial I/O device", 0, 0, 128 ).setActivation( "dioout", "true" );

  // widget:
  QVBoxLayout *vb = new QVBoxLayout;

  QLabel *label = new QLabel;
  vb->addWidget( label );

  RecordingLabel = new QLabel( "Recording:" );
  RecordingLabel->setAlignment( Qt::AlignHCenter );
  vb->addWidget( RecordingLabel );

  ElapsedTimeLabel = new QLabel;
  ElapsedTimeLabel->setAlignment( Qt::AlignHCenter );
  vb->addWidget( ElapsedTimeLabel );

  label = new QLabel;
  vb->addWidget( label );

  CommentLabel = new QLabel( "Press space to stop recording." );
  CommentLabel->setAlignment( Qt::AlignHCenter );
  vb->addWidget( CommentLabel );

  setLayout( vb );
}


void SaveTraces::preConfig( void )
{
  erase( "Analog input traces" );
  erase( "Events" );
  if ( traces().size() > 0 ) {
    newSection( "Analog input traces" );
    for ( int k=0; k<traces().size(); k++ )
      addBoolean( "trace-" + traces()[k].ident(), traces()[k].ident(), true );
  }
  int n = 0;
  for ( int k=0; k<events().size(); k++ ) {
    if ( ( events()[k].mode() &
	   ( StimulusEventMode + RestartEventMode + RecordingEventMode ) ) == 0 )
      n++;
  }
  if ( n > 0 ) {
    newSection( "Events" );
    for ( int k=0; k<events().size(); k++ ) {
      if ( ( events()[k].mode() &
	     ( StimulusEventMode + RestartEventMode + RecordingEventMode ) ) == 0 )
	addBoolean( "events-" + events()[k].ident(), events()[k].ident(), true );
    }
  }
}


class SaveTracesEvent : public QEvent
{

public:

  SaveTracesEvent( int num )
    : QEvent( Type( User+11 ) ),
      Num( num ),
      Value( 0.0 )
  {
  }
  SaveTracesEvent( double val )
    : QEvent( Type( User+12 ) ),
      Num( -1 ),
      Value( val )
  {
  }

  int Num;
  double Value;
};


int SaveTraces::main( void )
{
  // get options:
  double duration = number( "duration" );
  bool savedata = boolean( "savedata" );
  bool split = boolean( "split" );
  bool dioout = boolean( "dioout" );
  string diodevice = text( "diodevice" );
  int dioline = integer( "dioline" );

  // don't print repro message:
  noMessage();

  // don't save files:
  if ( ! savedata )
    noSaving();

  // plot trace:
  tracePlotContinuous();

  // init fonts:
  QCoreApplication::postEvent( this,
			       new SaveTracesEvent( split ? completeRuns()+1 : -1 ) );

  double starttime = currentTime();

  // header for files:
  Options header;
  Parameter &pp = header.addText( "trace", "" );
  lockStimulusData();
  header.newSection( stimulusData() );
  unlockStimulusData();
  header.newSection( settings() );

  // open files:
  deque<int> tracenum;
  deque<ofstream*> tracefile;
  deque<int> traceindex;
  deque<double> tracetime;
  deque<TableKey> tracekey;
  for ( int k=0; k<traces().size(); k++ ) {
    if ( boolean( "trace-" + traces()[k].ident() ) ) {
      tracenum.push_back( k );
      if ( split ) {
	string fn = addPath( "savetrace-" + traces()[k].ident() +
			     '-' + Str( completeRuns()+1 ) + ".dat" );
	tracefile.push_back( new ofstream( fn.c_str() ) );
      }
      else {
	string fn = addPath( "savetrace-" + traces()[k].ident() + ".dat" );
	tracefile.push_back( new ofstream( fn.c_str(),
					   ofstream::out | ofstream::app ) );
      }
      traceindex.push_back( traces()[k].size() );
      tracetime.push_back( traces()[k].currentTime() );
      pp.setText( traces()[k].ident() );
      header.save( *tracefile.back(), "# " );
      *tracefile.back() << '\n';
      tracekey.push_back( TableKey() );
      tracekey.back().addNumber( "t", "sec", "%11.6f" );
      tracekey.back().addNumber( traces()[k].ident(), traces()[k].unit(), "%11.5g" );
      tracekey.back().saveKey( *tracefile.back() );
    }
  }
  pp.setName( "events" );
  deque<int> eventsnum;
  deque<ofstream*> eventsfile;
  deque<int> eventsindex;
  deque<TableKey> eventskey;
  for ( int k=0; k<events().size(); k++ ) {
    if ( boolean( "events-" + events()[k].ident() ) ) {
      eventsnum.push_back( k );
      if ( split ) {
	string fn = addPath( "saveevents-" + events()[k].ident() +
			     '-' + Str( completeRuns()+1 ) + ".dat" );
	eventsfile.push_back( new ofstream( fn.c_str() ) );
      }
      else {
	string fn = addPath( "saveevents-" + events()[k].ident() + ".dat" );
	eventsfile.push_back( new ofstream( fn.c_str(),
					    ofstream::out | ofstream::app ) );
      }
      eventsindex.push_back( events()[k].size() );
      pp.setText( events()[k].ident() );
      header.save( *eventsfile.back(), "# " );
      *eventsfile.back() << '\n';
      eventskey.push_back( TableKey() );
      eventskey.back().addNumber( "t", "sec", "%11.6f" );
      if ( events()[k].sizeBuffer() )
	eventskey.back().addNumber( events()[k].sizeName(), events()[k].sizeUnit(),
				    events()[k].sizeFormat() );
      if ( events()[k].widthBuffer() )
	eventskey.back().addNumber( events()[k].widthName(), events()[k].widthUnit(),
				    events()[k].widthFormat() );
      eventskey.back().saveKey( *eventsfile.back() );
    }
  }

  // find dio device:
  DigitalIO *dio = 0;
  if ( dioout ) {
    dio = digitalIO( diodevice );
    if ( dio != 0 ) {
      dio->allocateLine( dioline );
      dio->configureLine( dioline, true );
      dio->write( dioline, true );
    }
  }

  // run:
  do {

    sleep( 0.5 );

    // save data:
    for ( unsigned int k=0; k<tracefile.size(); k++ ) {
      while ( traceindex[k] < traces()[tracenum[k]].size() ) {
	tracekey[k].save( *tracefile[k],
			  traces()[tracenum[k]].pos( traceindex[k] )
			  - tracetime[k], 0 );
	tracekey[k].save( *tracefile[k],
			  traces()[tracenum[k]][traceindex[k]] );
	*tracefile[k] << '\n';
	traceindex[k]++;
      }
    }
    for ( unsigned int k=0; k<eventsfile.size(); k++ ) {
      while ( eventsindex[k] < events()[eventsnum[k]].size() ) {
	eventskey[k].save( *eventsfile[k],
			   events()[eventsnum[k]][eventsindex[k]] - starttime,
			   0 );
	if ( events()[eventsnum[k]].sizeBuffer() )
	  eventskey[k].save( *eventsfile[k],
			     events()[eventsnum[k]].eventSize( eventsindex[k] )
			     * events()[eventsnum[k]].sizeScale() );
	if ( events()[eventsnum[k]].widthBuffer() )
	  eventskey[k].save( *eventsfile[k],
			     events()[eventsnum[k]].eventWidth( eventsindex[k] )
			     * events()[eventsnum[k]].widthScale() );
	*eventsfile[k] << '\n';
	eventsindex[k]++;
      }
    }

    // update time:
    QCoreApplication::postEvent( this, new SaveTracesEvent( currentTime() -
							    starttime ) );


  }  while ( softStop() == 0 && ! interrupt() &&
	     ( duration <= 0 || currentTime() - starttime < duration ) );

  if ( dio != 0 )
    dio->write( dioline, false );

  // close files:
  for ( unsigned int k=0; k<tracefile.size(); k++ ) {
    *tracefile[k] << "\n\n";
    delete tracefile[k];
  }
  for ( unsigned int k=0; k<eventsfile.size(); k++ ) {
    *eventsfile[k] << "\n\n";
    delete eventsfile[k];
  }

  return Completed;
}


void SaveTraces::customEvent( QEvent *qce )
{
  switch ( qce->type() - QEvent::User ) {
  case 11: {
    SaveTracesEvent *se = dynamic_cast<SaveTracesEvent*>( qce );
    if ( se->Num < 0 )
      RecordingLabel->setText( "Recording:" );
    else
      RecordingLabel->setText( string( "Recording " + Str( se->Num ) + ":" ).c_str() );
    QFont nf( widget()->font() );
    nf.setPointSize( 3 * widget()->fontInfo().pointSize() / 2 );
    RecordingLabel->setFont( nf );
    CommentLabel->setFont( nf );

    nf.setPointSize( 2 * widget()->fontInfo().pointSize() );
    nf.setBold( true );
    ElapsedTimeLabel->setFont( nf );
  }
  case 12: {
    SaveTracesEvent *se = dynamic_cast<SaveTracesEvent*>( qce );
    int secs = (int)::round( se->Value );
    int mins = secs / 60;
    secs -= mins * 60;
    int hours = mins / 60;
    mins -= hours * 60;
    string ts = "";
    if ( hours > 0 )
      ts += Str( hours ) + ":";
    ts += Str( mins, 2, '0' ) + ":";
    ts += Str( secs, 2, '0' );
    ElapsedTimeLabel->setText( ts.c_str() );
    break;
  }
  default:
    RePro::customEvent( qce );
  }
}


addRePro( SaveTraces, base );

}; /* namespace base */

#include "moc_savetraces.cc"
