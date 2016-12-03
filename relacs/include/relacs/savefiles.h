/*
  savefiles.h
  Save data to files

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

#ifndef _RELACS_SAVEFILES_H_
#define _RELACS_SAVEFILES_H_ 1

#include <deque>
#include <map>
#include <fstream>
#include <ctime>
#include <QWidget>
#include <QLabel>
#include <QMutex>
#include <relacs/str.h>
#include <relacs/options.h>
#include <relacs/tablekey.h>
#include <relacs/outdata.h>
#include <relacs/outdatainfo.h>
#include <relacs/eventdata.h>
#include <relacs/repro.h>
#include <relacs/spiketrace.h>
#include <relacs/dataindex.h>

#ifdef HAVE_NIX
#include <nix.hpp>
#endif

namespace relacs {


class RELACSWidget;
class FilterDetectors;
/*!
\class SaveFiles
\brief Save data to files
\author Jan Benda

is owned by RELACSWidget and used by RELACSPlugin (path() and stimulusData())
Settings (defaultPath()) and MetaData (path() only)

SaveFile sets the following environment variables:
- \c RELACSDEFAULTPATH : The default base path where RELACS stores data (inbetween sessions).
- \c RELACSDATAPATH: The base path where RELACS is currently storing data.

\todo platform independent mkdir in openFiles()!
\todo saveStimulus: adaptive time for calculating the mean rate
\todo check it carefully!
\todo warning on Disk full (or even before!)
\todo File formats:
- .wav: enough flexibility, compression possible,
  but file size limited to 2GB has to be known in advance,
- .au: raw data with unlimited file size und minimal header information
  allows for float and double
*/

class SaveFiles : public QWidget, public Options
{
  Q_OBJECT

public:

    /*! Flag for the modes of traces or events, indicating that they should be saved. */
  static const int SaveTrace = 0x0010;
    /*! Flag for the modes of events, indicating that their mean quality should be saved. */
  static const int SaveMeanQuality = 0x0400;

    /*! Flag for the Options to mark the last values of the output traces. */
  static const int TraceFlag = 32768;

  SaveFiles( RELACSWidget *rw, int height,
	     QWidget *parent=0 );
  ~SaveFiles( void );

    /*! The current status of saving data to files.
        \return \c true if files are opened and data are currently written
	into the files.
        \sa filesOpen(), save(bool), holdOn(), holdOff() */
  bool saving( void ) const;
    /*! The current status of having files opened for saving data.
        \return \c true if files are open.
        \sa writing() */
  bool filesOpen( void ) const;

    /*! Do not update Saving. */
  void holdOn( void );
    /*! Allow updating Saving. */
  void holdOff( void );

    /*! \return the base path where data are currently stored.
        \sa addPath(), setPath(), defaultPath() */
  string path( void ) const;
    /*! Set the base path for all data to be stored and
        the corresponding environment variable RELACSDATAPATH
        \param[in] path the base path for storing data.
        \a path(), setPathTemplate() */
  void setPath( const string &path );
    /*! Expand file name by the current base path for storing data.
        \param[in] file the name of the file
	\return \a file added to the current path for storing data.
        \sa path() */
  string addPath( const string &file ) const;
    /*! Stores \a file in the list of files of the currently running RePro. */
  void storeFile( const string &file ) const;

    /*! \return the template for the base path where data are to stored.
        \sa setPathTemplate(), path() */
  string pathTemplate( void ) const;
    /*! Set the template for the base path for all data to be stored.
        Also adjusts the width of the widget accordingly.
        \param[in] path the template for the base path.
	\note The template is only set if \a path is not an empty string.
        \a pathTemplate() */
  void setPathTemplate( const string &path );

    /*! \return the default base path where data are stored
        if no session is running.
        \sa addDefaultPath(), setDefaultPath(), path() */
  string defaultPath( void ) const;
    /*! Set the default base path for all data to be stored and
        the corresponding environment variable RELACSDEFAULTPATH.
	If the current path() equals defaultPath() then
	the current base path is alse set via setPath().
        \param[in] defaultpath the base path for storing data.
	\note The pathes are only set if \a defaultpath is not an empty string.
        \a defaultPath() */
  void setDefaultPath( const string &defaultpath );
    /*! Expand file name by the default base path for storing data.
        \param[in] file the name of the file
	\return \a file added to the default base path for storing data.
        \sa defaultPath() */
  string addDefaultPath( const string &file ) const;

    /*! React to settings of the stimulus options.
        This function calls notifyStimulusData() in all RELACSPlugins. */
  virtual void notify( void );
    /*! Lock the stimulus data. */
  void lock( void ) const;
    /*! Unlock the stimulus data mutex. */
  void unlock( void ) const;
    /*! The mutex of the stimulus data. */
  QMutex *mutex( void );

    /*! Copies pointers of each element of \a il and \a el to this. */
  void setTracesEvents( const InList &il, const EventList &el );

    /*! Switch writing data to file on or off.
        Call this only at the very beginning of your RePro::main() code,
	i.e. before writing any stimulus. */
  void save( bool on );
    /*! Save data traces and events to files */
  void saveTraces( void );
    /*! Save output-meta-data to files. */
  void save( const OutData &signal );
    /*! Save output-meta-data to files. */
  void save( const OutList &signal );
    /*! Save RePro meta data to files. */
  void save( const RePro &rp );

  /*! \return \c true if there is still a stimulus pending
        that needs to be written into the index files. */
  bool signalPending( void ) const;
    /*! Clear a pending signal. */
  void clearSignal( void );

    /*! If no file is open: create a new file name, make a directory,
        open and initialize the data-, event-, and stimulus files. */
  void openFiles( void );
    /*! Close files and delete them and/or remove base directory. */
  void deleteFiles( void );
    /*! Close files and keep them. */
  void completeFiles( void );

    /*! \return a pointer to the index of all recording, repro, and
        stimulus data. */
  DataIndex *dataIndex( void );


protected:

    /*! Set the base path for all data to be stored and
        the corresponding environment variable RELACSDATAPATH
        \param[in] path the base path for storing data.
        This is the not-locked variant of setPath() */
  void setThePath( const string &path );
    /*! The current status of saving data to files.
        \return \c true if files are opened and data are currently written
	into the files.
        This is the not-locked variant of saving(). */
  bool isSaving( void ) const;

    /*! Switch saving to files on or off. \sa save( bool ) */
  void writeToggle( void );
    /*! Write data traces to files. \sa saveTraces() */
  void writeTraces( void );
    /*! Write events to files. \sa saveTraces() */
  void writeEvents( void );
    /*! Write pending stimuli to files. \sa save( const OutData& ), save( const OutList& ) */
  void writeStimulus( void );
    /*! Write information about a RePro to files. \sa save( const RePro& ) */
  void writeRePro( void );

    /*! Close all open files */
  void closeFiles( void );
    /*! Remove the directory \a dirname with all its files and all its sub-directories. */
  bool removeDir( const QString &dirname );
    /*! Generate new pathname using PathTemplate, PathTime, and PathNumber. */
  string pathName( void ) const;
    /*! Add the file \a filename to the list of files to be removed and to the metadata. */
  void addFile( const string &filename );
    /*! Open the file path() + filename
        as specified by \a type (ios::out, ios::in, ios::binary, ...).
	Add it to the list of files to be removed and to the metadata
        and print an error message if opening of the file failed. */
  ofstream *openFile( const string &filename, int type );

    /*! The file \a filename will be removed if the session is not
        saved. */
  void addRemoveFile( const string &filename );
    /*! Reset the list of files that have to be deleted if the session
        is not to be saved. */
  void clearRemoveFiles( void );
    /*! Remove all files from the list and clear the list of files
        that have to be deleted if the session is not to be saved. */
  void removeFiles( void );

  /*! Open and initialize the NIX file that
        contains *all* information. */
  void createNIXFile( void );
  
  virtual void customEvent( QEvent *qce );

    /*! Are there any files open to save in? */
  bool FilesOpen;
    /*! Should be saved into the files? */
  bool Saving;
    /*! Hold toggling saving. */
  bool Hold;

    /*! The path (directory or common basename)
        where all data of the current session are stored. */
  string Path;
    /*! The path from the previous session. */
  string PrevPath;
    /*! The template from which \a Path is generated. */
  string PathTemplate;
    /*! The default path (directory or common basename)
        where all data are stored. */
  string DefaultPath;

    /*! Identification number for pathes used to create a base path
        from \a PathFormat. */
  int  PathNumber;
    /*! The time used to generate the previous base path. */
  time_t PathTime;

    /*! Time of start of the session. */
  double SessionTime;

    /*! The local copy of all input traces. */
  InList IL;
    /*! The local copy of all event traces. */
  EventList EL;

    /*! Start of current stimulus. */
  double SignalTime;
    /*! Start of previous stimulus. */
  double PrevSignalTime;

    /*! The options of SaveFiles at the time of writing a stimulus. Contains
        values of all output traces right before writing the new stimulus
        and additional information. */
  Options StimulusData;
    /*! Properties and descriptions of all output traces of the current stimulus. */
  deque< OutDataInfo > Stimuli;
    /*! All stimuli of a session used by a RePro. */
  map< string, map < Options, string > > ReProStimuli;

    /*! Name of the current RePro. */
  string ReProName;
    /*! Number of stimuli written by the current RePro, 
        keys are the names of the RePros. */
  map< string, int > ReProStimulusCount;
    /*! The settings of the current RePro. */
  Options ReProInfo;
    /*! List of file names opened by the current RePro. */
  mutable deque< string > ReProFiles;

  bool ReProData;

  bool ToggleOn;
  bool ToggleData;

    /*! The data browser. */
  DataIndex DI;

    /*! A list of files which have to be deleted if the session is not
        to be saved. */
  deque<string> RemoveFiles;

  class RelacsFiles {

  public:

    RelacsFiles( void );

      /*! Open all necessary files. */
    void open( const InList &IL, const EventList &EL, 
	       const Options &data, const Acquire *acquire, 
	       const string &path, SaveFiles *save, const AllDevices *devices );

      /*! Set index for traces to current size of each trace in \a IL. */
    void resetIndex( const InList &IL );
      /*! Set index for events to current size of each event list in \a EL. */
    void resetIndex( const EventList &EL );
      /*! Write data traces to files. */
    void writeTraces( const InList &IL, bool stimulus );
      /*! Write events with \a offs subtracted to files. \sa saveTraces() */
    void writeEvents( const InList &IL, const EventList &EL );
      /*! Write pending stimuli to files. \sa save( const OutData& ), save( const OutList& ) */
    void writeStimulus( const InList &IL, const EventList &EL, 
			const deque< OutDataInfo > &stimuliinfo, 
			const deque< bool > &newstimuli, const Options &data, 
			const deque< Options > &stimuliref, int *stimulusindex,
			double sessiontime, const string &reproname, const Acquire *acquire );
      /*! Write information about a RePro to files. \sa save( const RePro& ) */
    void writeRePro( const Options &reproinfo, const deque< string > &reprofiles );

    void traceSignalIndices( deque<int> &traceindex );
    void eventsSignalIndices( deque<int> &eventsindex );

      /*! Close all files. */
    void close( const string &path, const deque< string > &reprofiles,
		MetaData &metadata );


  protected:

      /*! Open and initialize the files holding the traces from the
	  analog input channels. */
    void openTraceFiles( const InList &IL, SaveFiles *save );
      /*! Open and initialize the files recording the event times. */
    void openEventFiles( const EventList &EL, SaveFiles *save );
      /*! Open and initialize the stimulus file that
          contains indices to he traces and event files. */
    void openStimulusFiles( const InList &IL, const EventList &EL, 
			    const Options &data, const Acquire *acquire, SaveFiles *save );
      /*! Open and initialize the XML file that
          contains all information. */
    void openXMLFiles( const string &path, SaveFiles *save, const AllDevices *devices );

      /*! File with stimuli and indices to traces and events. */
    ofstream *SF;
      /*! File with stimulus descriptions. */
    ofstream *SDF;

      /*! XML file containing all data. */
    ofstream *XF;
      /*! XML file containing stimulus descriptions. */
    ofstream *XSF;
    bool DatasetOpen;

    struct TraceFile {
        /*! The name of the file for the trace. */
      string FileName;
        /*! The file stream. */
      ofstream *Stream;
        /*! Current index to trace data from where on to save data. */
      long Index;
        /*! Number of so far written trace data. */
      long Written;
        /*! Start of stimulus as an index to the written trace data. */
      long SignalOffset;
    };
      /*! files for all voltage traces. */
    deque< TraceFile > TraceFiles;

    struct EventFile {
        /*! The name of the file for the events. */
      string FileName;
        /*! The file stream. */
      ofstream *Stream;
        /*! Index to event data. */
      long Index;
        /*! Already written lines. */
      long Written;
        /*! Line index to the signal start in the events files. */
      long SignalEvent;
        /*! Save mean quality in the stimulus file. */
      bool SaveMeanQuality;
        /*! The key for the event file. */
      TableKey Key;
    };
    deque< EventFile > EventFiles;

    TableKey StimulusKey;
  };
  RelacsFiles RelacsIO;

  #ifdef HAVE_NIX
  struct NixTrace {
    nix::DataArray data;
    size_t         index;
    size_t         written;
    nix::NDSize    offset;
  };

  struct NixEventData {
    nix::DataArray data;
    nix::MultiTag tag;
    int input_trace;
    size_t index;
    nix::NDSize offset;
    //we belong to the el_index of the EventList
    size_t el_index;
  };

  struct NixFile {
    nix::File    fd;
    nix::Block   root_block;
    nix::Section root_section;
    nix::MultiTag event_tag;
    nix::DataArray event_positions;
    nix::DataArray event_extents;
    nix::DataArray time_feat, delay_feat, amplitude_feat, carrier_feat;
    std::vector<nix::DataArray> data_features;

    string create ( string path );
    void close  ( void );
    void saveMetadata ( const AllDevices *devices );
    void saveMetadata ( const MetaData &mtdt );
    void writeStimulus ( const InList &IL, const deque< OutDataInfo > &stim_info, 
			 string rp_name, double sessiontime, RELACSWidget *RW, 
			 const Options &stim_options );
    void initTraces ( const InList &IL );
    void writeTraces ( const InList &IL );
    void writeChunk ( NixTrace &trace, size_t to_read, const void *data);
    void initEvents ( const EventList &EL, FilterDetectors *FD );
    void writeEvents ( const EventList &EL, double offset );
    void resetIndex ( const InList &IL );
    void resetIndex ( const EventList &EL );
    void appendValue( nix::DataArray &array, double value ); 
    nix::DataArray createFeature( nix::Block &block, nix::MultiTag &mtag,
				  std::string name, std::string type,
				  std::string unit, std::string label,
				  nix::LinkType link_type=nix::LinkType::Indexed);
    string rid; //recording id

    vector<NixTrace> traces;
    vector<NixEventData> events;
  };
  NixFile NixIO;
  #endif
  
  RELACSWidget *RW;

  QLabel *FileLabel;
  QFont NormalFont;
  QFont HighlightFont;
  QPalette NormalPalette;
  QPalette HighlightPalette;
  SpikeTrace *SaveLabel;
  QHBoxLayout *StatusInfoLayout;

  mutable QMutex SaveMutex;
  mutable QMutex StimulusDataLock;

};


}; /* namespace relacs */

#endif /* ! _RELACS_SAVEFILES_H_ */

