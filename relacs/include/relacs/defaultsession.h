/*
  defaultsession.h
  Minimum default Session

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

#ifndef _DEFAULTSESSION_H_
#define _DEFAULTSESSION_H_

#include <qpushbutton.h>
#include "control.h"


/*! 
\class DefaultSession
\author Jan Benda
\version 1.0
\brief Minimum default Session
*/


class DefaultSession : public Control
{
  Q_OBJECT

public:

  DefaultSession( void );
  ~DefaultSession( void );

  void sessionStarted( void );
  void sessionStopped( bool saved );


private:

  QPushButton *SessionButton;

};

#endif