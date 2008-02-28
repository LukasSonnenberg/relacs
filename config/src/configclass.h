/*
  config.h
  Base class for each class that has some parameters to be configured.

  RELACS - RealTime ELectrophysiological data Acquisition, Control, and Stimulation
  Copyright (C) 2002-2007 Jan Benda <j.benda@biologie.hu-berlin.de>

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

#ifndef _CONFIGCLASS_H_
#define _CONFIGCLASS_H_

#include <string>
#include <fstream>
#include <vector>
#include "strqueue.h"
#include "options.h"
using namespace std;

/*!
\class ConfigClass
\brief Base class for each class that has some parameters (Options) to be configured.
\author Jan Benda
\version 1.0
*/

class Configure;
class ConfigClass;
typedef vector<ConfigClass*> ConfigList;

class ConfigClass : public Options
{

public:

  enum Action {
    Save=1,
    Dialog=2,
    Menu=4,
    Help=8
  };

  ConfigClass( const string &ident, int group=0, int mode=0xffff,
	       int selectmask=0 );
    /*! Copy constructor. */
  ConfigClass( const ConfigClass &C );
    /*! Deconstructs a ConfigClass. */
  virtual ~ConfigClass( void );

  const string &configIdent( void ) const;
  void setConfigIdent( const string &ident );

  int configGroup( void ) const;
  void setConfigGroup( int group );

  int configMode( void ) const;
  void setConfigMode( int mode );

  int configSelectMask( void ) const;
  void setConfigSelectMask( int mask );

  void addConfig( void );
  void readConfig( void );

  virtual void readConfig( StrQueue &sq );
  virtual void saveConfig( ofstream &str );
  virtual int configSize( void ) const;
  virtual void config( void );

  static void setConfigList( ConfigList *cl );
  static void setConfigure( Configure *cfg );


private:

  string ConfigIdent;
  int ConfigGroup;
  int ConfigMode;
  int ConfigSelect;
  static ConfigList *Configs;
  static Configure *CFG;

};


#endif