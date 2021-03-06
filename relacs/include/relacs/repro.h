/*
  repro.h
  Parent class of all research programs.

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

#ifndef _RELACS_REPRO_H_
#define _RELACS_REPRO_H_ 1

//#include <QGlobal>
#include <QMutex>
#include <QWaitCondition>
#include <QThread>
#include <QDateTime>
#include <relacs/outlist.h>
#include <relacs/optdialog.h>
#include <relacs/relacsplugin.h>

namespace relacs {


class ReProThread;


/*! 
\class RePro
\author Jan Benda
\author Christian Machens
\brief Parent class of all research programs.
\todo ~RePro: auto remove from RePros?
\todo addPath: something with a format string giving the RePro's name, totalRuns, etc.
*/

class RePro : public RELACSPlugin
{
  Q_OBJECT

public:
  
  static const int MacroFlag = Parameter::ChangedFlag >> 1;
  static const int OverwriteFlag = Parameter::ChangedFlag >> 2;
  static const int CurrentFlag = Parameter::ChangedFlag >> 3;

    /*! Different return values for the main() function. */
  enum DoneState {
      /*! The RePro completed its job and terminated normally. */
    Completed,
      /*! The RePro terminated before completing its job
	  (usually due to a request by the user). */
    Aborted,
      /*! The RePro was not able to do its job properly. */
    Failed,
      /*! Keep executing the RePro.
	  Don't use this as a return value for main(). */
    Continue
  };

    /*! Constructs a RePro with name \a name (has to be identical
        to the class name) that belongs to the set of plugins 
	named \a pluginset.
	The implementation of a class derived from RePro
	has the version \a version and was written
	by \a author on \a date.
        The date defaults to the date of compilation. */
  RePro( const string &name,
	 const string &pluginset="",
	 const string &author="unknown",
	 const string &version="unknown",
	 const string &date=__DATE__ );
    /*! Destructs a RePro. */
  virtual ~RePro( void );

    /*! Set the name of the RePro to \a name. */
  virtual void setName( const string &name );

    /*! Read the RePro's default settings from the config file. */
  virtual void readConfig( StrQueue &sq );
    /*! Saves the RePro's default settings to the config file. */
  virtual void saveConfig( ofstream &str );

    /*! This function is called when the RePro is started.
        You need to implement this function with some useful code, 
	e.g. read options, create and play stimulus, 
	analyze the read voltage traces and events, etc.
        This function is executed as a thread.
	Within this thread the RePro is locked (via lock()).
	Access to those data is unlocked during sleep(), sleepOn(), and sleepWait().
	Beware that main() is executed in a thread different from the GUI thread.
	Use postCustomEvent() to call some functions that eventually
	paint something on the screen.
	The function should return one of the \a DoneStates except Continue.
	After main() terminated
        a message is written ( "Repro succesfully completed",
	"RePro stopped" or "RePro interupted", depending on the return value)
        and RELACS is informed. 
	You need to return from main() if the sleep(), sleepOn(),
	or interrupt() functions return \c true.
	Also, if you lock() and unlock() the RePro manually,
	you should check interrupt() directly after each unlock().
	If interrupt() returns \c true, you should be careful with
	posting custom events via postCustomEvent, since they might be executed
	after the RePro terminated. This includes calling message() and
	Plot::draw() or MultiPlot::draw().
	The RePro's widget is enabled during execution of main().
        \sa interrupt(), sleep(), timeStamp(), sleepOn(), sleepWait(), enable() */
  virtual int main( void ) = 0;

    /*! Returns \c true if the RePro thread should be stopped immediately.
        Use this from within main()
        \code
         int main( void )
	 {
	 ...
	 if ( interrupt() )
	   return Aborted;
	 ...
	 }
        \endcode
        */
  bool interrupt( void ) const;

    /*! Start execution of the %RePro thread. 
        Reimplement main() to make the thread do something. */
  void start( QThread::Priority priority = QThread::InheritPriority );
    /*! Kindly ask the RePro to stop. 
        Sets interrupt() to \c true and terminates all sleeps.
        You still need to wait on the RePro thread to finish. */
  virtual void requestStop( void );
    /*! Returns \c true if the %RePro therad is running. */
  bool isRunning( void ) const;
    /*! Wait on the %RePro's thread to terminate or at most \a time seconds,
        if \a time is greater than zero. */
  bool wait( double time=-1.0 );

    /*! Sleep for some time.
        Right before returning, the data and event buffers are updated.
	\param[in] t the time to sleep in seconds.
	\param[in] tracetime the size the input data should have after the sleep.
	For internal use only!
        \return \a true if the main() thread needs to be stopped.
        \sa sleepOn(), timeStamp(), sleepWait(), interrupt() */
  bool sleep( double t, double tracetime=-1.0 );
    /*! Memorize the current time. 
        This time is used by sleepOn() to calculate the remaining
	time to sleep. 
        \sa sleepOn(), sleep(), sleepWait() */
  void timeStamp( void );
    /*! Sleep for \a t seconds relative to the last call of timeStamp(). 
        Right before returning, the data and event buffers are updated.
	\param[in] t the time to sleep since the last call to timeStamp() in seconds.
        \return \a true if the main() thread needs to be stopped.
        \sa sleep(), sleepWait(), timeStamp(), interrupt() */
  bool sleepOn( double t );
    /*! Wait on the RePro's waitcondition for sleeping
        or sleep for the specified time.
        The data and event buffers are NOT updated after sleeping.
        \param[in] time the maximum time to be waiting for,
	i.e. the time to sleep in seconds.
        If \a time is smaller than zero, sleepWait() waits forever.
        \return \c false if sleepWait() slept for the specified time,
	\c true if sleeping was interrupted by wake().
        \sa wake() */
  bool sleepWait( double time=-1.0 );
    /*! Wake all threads that wait on the RePro's waitcondition for sleeping.
        In particular, interrupt a sleep(), sleepOn(), or sleepWait().
        \sa sleepWait() */
  void wake( void );

    /*! Enable the RePro's widget, i.e. allow inputs to its GUI elements.
        \sa disable() */
  void enable( void );
    /*! Disable the RePro's widget, i.e. do not allow inputs to its GUI elements.
        \sa enable() */
  void disable( void );

    /*! Reimplement this function to reset or initialize your RePro
        when a new session is started.
        The current implementation resets the counters completeRuns(),
	totalRuns(), allRuns(), failedRuns().
        So you should call from your implementation RePro::sessionStarted(). */
  virtual void sessionStarted( void );
    /*! Reimplement this function to reset your RePro 
        after a session is stopped.
        If the session was saved \a saved is set to \c true.
        The current implementation resets the counters completeRuns(),
	totalRuns(), allRuns(), failedRuns().
        So you should call from your implementation RePro::sessionStopped( saved ). */
  virtual void sessionStopped( bool saved );

    /*! In main(): true if previous call of this RePro was
        completed, i.e. main() returned Completed.
        \sa aborted(), failed(), completeRuns() */
  bool completed( void ) const;
    /*! In main(): true if previous call of this RePro was
        aborted by the user, i.e. main() returned Aborted.
        \sa completed(), failed(), totalRuns() */
  bool aborted( void ) const;
    /*! In main(): true if previous call of this RePro failed,
	i.e. main() returned Failed.
        \sa completed(), aborted(), failedRuns() */
  bool failed( void ) const;

    /*! Number of completed runs of this RePro (main() returned Completed)
        since last call of sessionStarted() or sessionStopped().
        \sa completed(), totalRuns(), allRuns(), failedRuns() */
  int completeRuns( void ) const;
    /*! Number of runs of this RePro which did not fail
        since last call of sessionStarted() or sessionStopped().
	i.e. where main() returned Aborted or Completed.
        \sa completeRuns(), allRuns(), failedRuns() */
  int totalRuns( void ) const;
    /*! Number of runs of this RePro
        since last call of sessionStarted() or sessionStopped().
        \sa completeRuns(), totalRuns(), failedRuns() */
  int allRuns( void ) const;
    /*! Number of runs of this RePro which failed
        (main() returned Failed)
        since last call of sessionStarted() or sessionStopped().
        \sa completeRuns(), totalRuns(), allRuns() */
  int failedRuns( void ) const;

    /*! Time in seconds this RePro is currently running.
        \sa reproTimeStr() */
  double reproTime( void ) const;
    /*! A string displaying the elapsed time of the current RePro.
        \sa reproTime() */
  string reproTimeStr( void ) const;
    /*! The time of the session when the RePro was started in seconds. */
  double reproStartTime( void ) const;

    /*! Test of a single output signal \a signal for validity.
        Returns 0 on success or a negative number if \a signal
        is not valid.
        The error state of \a signal is set appropriately. */
  int testWrite( OutData &signal );
    /*! Test of a multiple output signals \a signal for validity.
        Returns 0 on success or a negative number if \a signal
        is not valid.
        The error state of \a signal is set appropriately. */
  int testWrite( OutList &signal );

    /*! Blocking output of a signal \a signal.
        The function blocks until the output of the signal finished.
        See OutData about how to specify output channel, sampling rate, 
	intensity, delay, etc. 
	\param[in] signal The signal to be written out on an analog output.
	\param[in] setsignal if set \c true (default) then the start time of this
	signal is recorded.
	\return 0 on success.
        If the output of the signal failed, a negative number is returned and
        the reason is specified in the error state of \a signal. */
  int write( OutData &signal, bool setsignaltime=true );
    /*! Blocking output of multiple signals \a signal.
        The function blocks until the output of the signal finished.
        See OutList about how to specify output channel, sampling rate, 
	intensity, delay, etc. 
	\param[in] signal The signals to be written out on analog outputs.
	\param[in] setsignal if set \c true (default) then the start time of this
	signal is recorded.
	\return 0 on success.
        If the output of the signal failed, a negative number is returned and
        the reason is specified in the error state of \a signal. */
  int write( OutList &signal, bool setsignaltime=true );

    /*! Non-blocking output of a signal \a signal.
        The function returns immediately after signal output has been initiated.
        See OutData about how to specify output channel, sampling rate, 
	intensity, delay, etc. 
	\param[in] signal The signal to be written out on an analog output.
	\param[in] setsignal if set \c true (default) then the start time of this
	signal is recorded.
	\return 0 on success.
        If the output of the signal failed, a negative number is returned and
        the reason is specified in the error state of \a signal. 
	\note During the output of the stimulus, \a signal must exist 
	and must not be modified! */
  int startWrite( OutData &signal, bool setsignaltime=true );
    /*! Non-blocking output of multiple signals \a signal.
        The function returns immediately after signal output has been initiated.
        See OutList about how to specify output channel, sampling rate, 
	intensity, delay, etc. 
	\param[in] signal The signals to be written out on analog outputs.
	\param[in] setsignal if set \c true (default) then the start time of the
	signals is recorded.
	\return 0 on success.
        If the output of the signal failed, a negative number is returned and
        the reason is specified in the error state of \a signal. 
	\note During the output of the stimulus, \a signal must exist 
	and must not be modified! */
  int startWrite( OutList &signal, bool setsignaltime=true );

    /*! Direct output of a single data value as specified by \a signal
        to the DAQ board.
	Only the output trace ( OutData::setTrace() ) or the the name of the
	output trace ( OutData::setTraceName() ), as well as the
	single data value need to be specified.
	If \a setsignal is set \c true (default) then the start time of this
	signal is recorded.
	\return 0 on success, a negative number if the output of the signal
	failed. The reason for the failure is specified in the error state
	of \a signal. */
  int directWrite( OutData &signal, bool setsignaltime=true );
    /*! Direct output of single data values as specified by \a signal
        to different channels of the DAQ board.
	Only the output traces ( OutData::setTrace() ) or the the name of the
	output traces ( OutData::setTraceName() ), as well as the
	single data values need to be specified.
	If \a setsignal is set \c true (default) then the start time of this
	signal is recorded.
	\return 0 on success, a negative number if the output of the signals
	failed. The reason for the failure is specified in the error state
	of \a signal. */
  int directWrite( OutList &signal, bool setsignaltime=true );

    /*! Set the output of channel \a channel on device \a device to zero.
        Returns 0 on success or a negative number on error. 
        If \a device is negative, the default device is used. */
  int writeZero( int channel, int device );
    /*! Set the output of the trace with index \a index to zero.
        Returns 0 on success or a negative number on error. */
  int writeZero( int index );
    /*! Set the output of the trace with name \a trace to zero.
        Returns 0 on success or a negative number on error. */
  int writeZero( const string &trace );

    /*! Stop analog output of all analog output devices. */
  int stopWrite( void );

    /*! Returns the minimum possible attenuation level for the output trace
        at index \a trace.
        If \a trace is invalid or \a trace is not connected to an attenuator
        then OutData::NoLevel is returned.
        \sa maxLevel() */
  double minLevel( int trace ) const;
    /*! Returns the minimum possible attenuation level for output trace
        with name \a trace. */
  double minLevel( const string &trace ) const;
    /*! Returns the maximum possible attenuation level for the output trace
        at index \a trace.
        If \a trace is invalid or \a trace is not connected to an attenuator
        then OutData::NoLevel is returned.
        \sa minLevel() */
  double maxLevel( int trace ) const;
    /*! Returns the maximum possible attenuation level for output trace
        with name \a trace. */
  double maxLevel( const string &trace ) const;
    /*! Returns in \a l all possible attenuation levels
        sorted by increasing attenuation levels (highest last) 
	for the output trace at index \a trace.
        If \a trace is invalid or \a trace is not connected to an attenuator
        then \a l is returned empty.
        \sa minLevel(), maxLevel() */
  void levels( int trace, vector<double> &l ) const;
    /*! Returns in \a l all possible attenuation levels
        sorted by increasing attenuation levels (highest last) 
	for the output trace with name \a trace. */
  void levels( const string &trace, vector<double> &l ) const;

    /*! Returns the minimum possible intensity at the given stimulus
        \a frequency for the output trace at index \a trace.
        If \a trace is invalid or \a trace is not connected to an attenuator
        then OutData::NoIntensity is returned.
        \sa maxIntensity(), minLevel(), maxLevel() */
  double minIntensity( int trace, double frequency=0.0 ) const;
    /*! Returns the minimum possible intensity for output trace
        with name \a trace. */
  double minIntensity( const string &trace, double frequency=0.0 ) const;
    /*! Returns the maximum possible intensity at the given stimulus
        \a frequency for the output trace at index \a trace.
        If \a trace is invalid or \a trace is not connected to an attenuator
        then OutData::NoIntensity is returned.
        \sa minIntensity(), minLevel(), maxLevel() */
  double maxIntensity( int trace, double frequency=0.0 ) const;
    /*! Returns the maximum possible intensity for output trace
        with name \a trace. */
  double maxIntensity( const string &trace, double frequency=0.0 ) const;
    /*! Returns in \a ints all possible intensities at the given stimulus
        \a frequency sorted by increasing intensities 
	for the output trace at index \a trace.
        If \a trace is invalid or \a trace is not connected to an attenuator
        then \a intens is returned empty.
        \sa minIntensity(), maxIntensity() */
  void intensities( int trace, vector<double> &ints, double frequency=0.0 ) const;
    /*! Returns in \a ints all possible intensities at the given stimulus
        \a frequency sorted by increasing intensities 
	for the output trace with name \a trace. */
  void intensities( const string &trace, vector<double> &ints, double frequency=0.0 ) const;

    /*! Print the default messages if \a message is \c true.
        This is the default.
        \sa noMessage(), message() */
  void setMessage( bool message=true );
    /*! Do not print any of the default messages.
        \sa setMessage(), message()*/
  void noMessage( void );
    /*! Write a message \a msg in the right half of the status line.
        \sa setMessage(), noMessage() */
  void message( const string &msg );

    /*! Stores \a file in the list of files for this run of the RePro.
        \return \a file added to the base path for the current session. */
  string addPath( const string &file ) const;

    /*! Call this function in the beginning of the main() function,
        if your implementation includes some interactive widgets
	and you do not want RELACS to automatically remove the keyboard focus
	from these widgets after some timeout. */
  void keepFocus( void );

    /*! Add \a key to the list of keys that are forced to be passed
        to keyPressEvent().
        You need to handle this key event in a reimplementation 
        of keyPressEvent(). */
  void grabKey( int key );
    /*! Remove \a key from the list of keys that are forced to be passed
        to keyPressEvent(). */
  void releaseKey( int key );

    /*! How many requests to stop the repro are there
        since the RePro was started?
        \sa setSoftStop(), clearSoftStop(), setSoftStopKey() */
  int softStop( void );
    /*! Set the number of requested stops for the repro to \a s.
        \sa softStop(), clearSoftStop(), setSoftStopKey() */
  void setSoftStop( int s=1 );
    /*! Set the number of requested stops for the repro to zero.
        \sa softStop(), setSoftStop(), setSoftStopKey() */
  void clearSoftStop( void );
    /*! Set the keycode initiating a soft stop to \a keycode.
        \sa softStop(), setSoftStop(), clearSoftStop() */
  void setSoftStopKey( int keycode );

    /*! Switch plotting of raw traces on (\a on = \c true, default)
        or off (\a off = \c false). 
	\sa tracePlotOff(), tracePlotSignal(), tracPlotContinuous() */
  void tracePlotOn( bool on=true );
    /*! Switch plotting of raw traces off.
        \sa tracePlotOn(), tracePlotSignal(), tracPlotContinuous() */
  void tracePlotOff( void );
    /*! Raw traces are plotted relative to the beginning of the last
        output signal in a window of \a length seconds width.
        The start time of the signal is \a offs seconds 
        from the left margin of that window.
        \sa tracePlotOn(), tracPlotContinuous() */
  void tracePlotSignal( double length, double offs=0.0 );
    /*! Raw traces are plotted relative to the beginning of the last
        output signal. The width of the plot window is not changed.
        \sa tracePlotOn(), tracPlotContinuous() */
  void tracePlotSignal( void );
    /*! Raw traces are plotted continuously in a window of
        \a length seconds width.
        \sa tracePlotOn(), tracPlotSignal() */
  void tracePlotContinuous( double length );
    /*! Raw traces are plotted continuously. The width of the
        plot window is not changed.
        \sa tracePlotOn(), tracPlotSignal() */
  void tracePlotContinuous( void );

    /*! Return the name of the currently running macro.
        If the %RePro was called from the %RePro menu
        then "RePro" is returned. */
  string macroName( void );
    /*! Return the parameters of the currently running macro. */
  string macroParam( void );

    /*! Return a path where the RePro can store some global stuff. 
        This is the \c repropath from the RELACS settings plus
        the name of the %RePro.
        If \a version is \c true, a subdirectory is created for
        each version of the %RePro.
        \sa addReProPath() */
  string reproPath( bool version=true );
    /*! Returns \a file added to the reproPath().
        \sa reProPath() */
  string addReProPath( const string &file, bool version=true );

    /*! This function is called to set the options of the RePro. 
        A standard dialog (OptDialog) is launched to set the repro's options. 
        Reimplement it, if you want another interface. */
  virtual void dialog( void );
    /*! Options of the RePro that overwrite macro options. */
  Options &overwriteOptions( void );

    /*! Check for valid options.
        \param[in] opttxt string with options that are tesed for validity
	\return error message */
  string checkOptions( const string &opttxt );

    /*! Determine whether data (voltage traces and event lists)
        of this run of the RePro should be saved to disk.
        Only during a running session, data can be saved to disk.
        By default, data are saved to disk.
        \note Call this function at the very beginning of your 
	main() code, i.e. before writing any stimulus,
	if you want to save nothing during the run of the RePro.
        \sa SaveFiles::writing(), noSaving() */
  virtual void setSaving( bool saving );
    /*! Indicate that the data of this RePro don't have to be saved to disk.
        \note Call this function at the very beginning of your 
	main() code, i.e. before writing any stimulus,
	if you want to save nothing during the run of the RePro.
        Shortcut for setSaving( false ). */
  void noSaving( void );

    /*! The eventfilter that is used to grab keys. \sa grabKey() */
  virtual bool eventFilter( QObject *watched, QEvent *e );


protected:

    /*! Reimplement this function to handle key-press events.
        This implementation handles softStop()*/
  virtual void keyPressEvent( QKeyEvent *event );
    /*! Reimplement this function to handle key-release events. */
  virtual void keyReleaseEvent( QKeyEvent *event );

    /*! Reimplement this function to modify the Options dialog.
        This function is called after the RePro Options have been added to the dialog. */
  virtual void modifyDialog( OptDialog *od );


protected slots:

    /*! If you reimplement this event handler,
        don't forget to call this implementation
        via RePro::customEvent().
	\note Make sure that you do not block this forever! This will freeze the GUI!
	Better use tryLock() and if this fails, post the event again.
        \sa RELACSPlugin::postCustomEvent() */
  virtual void customEvent( QEvent *qce );


private:

    /*! Return \c true if the analog output was interrupted and set interrupt() to \c true. */
  bool interruptWrite( void );

    /*! Install the event filter for grabbing keys. */
  void grabKeys( void );
    /*! Uinstall the event filter for grabbing keys and reset list of grabbed keys. */
  void releaseKeys( void );

  friend class ReProThread;

    /*! The thread that runs the RePro.
        It calls main(). */
  void run( void );

  ReProThread *Thread;
  int Interrupt;   // 0: no interruption, 1: stop write, 2: interrupt repro
  mutable QMutex InterruptLock;
  QWaitCondition SleepWait;
  QTime SleepTime;
  double TraceTime;

  int LastState;
  int CompleteRuns;
  int TotalRuns;
  int AllRuns;
  int FailedRuns;
  double ReProStartTime;

  Options OverwriteOpt;

  vector< int > GrabKeys;
  int GrabKeysBaseSize;
  bool GrabKeysInstalled;
  mutable QMutex GrabKeyLock;

  int SoftStop;
  mutable QMutex SoftStopLock;
  int SoftStopKey;

  bool PrintMessage;

  mutable QTime ReProTime;


private slots:

  virtual void dClosed( int r );


};


class ReProThread : public QThread
{

public:
  
  ReProThread( RePro *r );
  virtual void run( void );
  void usleep( unsigned long usecs );


private:

  RePro *R;

};


}; /* namespace relacs */

#endif /* ! _RELACS_REPRO_H_ */

