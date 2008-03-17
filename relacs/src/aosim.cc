/*
  aosim.cc
  Implementation of AnalogOutput simulating an analog output device

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

#include "analoginput.h"
#include "aosim.h"


AOSim::AOSim( void ) 
  : AnalogOutput( "Analog Output Simulation", SimAnalogOutputType )
{
}


AOSim::~AOSim( void )
{
}


int AOSim::open( const string &device, long mode )
{
  clearSettings();
  setDeviceName( "AO Simulation" );
  setDeviceVendor( "RELACS" );
  setDeviceFile( device );
  return 0;
}


int AOSim::open( Device &device, long mode )
{
  clearSettings();
  setDeviceName( "AO Simulation" );
  setDeviceVendor( "RELACS" );
  setDeviceFile( device.deviceIdent() );
  return 0;
}


bool AOSim::isOpen( void ) const
{
  return true;
}


void AOSim::close( void )
{
}


int AOSim::reset( void )
{
  clearSettings();
  return 0;
}


int AOSim::channels( void ) const
{
  return 2;
}


int AOSim::bits( void ) const
{
  return 12;
}


double AOSim::maxRate( void ) const
{
  return 500000.0;
}


int AOSim::testWriteDevice( OutList &sigs )
{
  return sigs.failed() ? -1 : 0;
}


int AOSim::prepareWrite( OutList &sigs )
{
  // ao still running:
  if ( running() ) {
    sigs.addError( DaqError::Busy );
    return -1;
  }

  // success:
  setSettings( sigs );
  return 0;
}


int AOSim::convertData( OutList &sigs )
{
  return sigs.failed() ? -1 : 0;
}


int AOSim::startWrite( OutList &sigs )
{
  return 0;
}


int AOSim::writeData( OutList &sigs )
{
  return 0;
}


bool AOSim::running( void ) const
{
  return false;
}


int AOSim::error( void ) const
{
  return 0;
}


int AOSim::getAISyncDevice( const vector< AnalogInput* > &ais ) const
{
  for ( unsigned int k=0; k<ais.size(); k++ ) {
    if ( ais[k]->deviceFile().size() > 0 && deviceFile().size() > 0 &&
	 ais[k]->deviceFile()[ais[k]->deviceFile().size()-1] == deviceFile()[deviceFile().size()-1] ) {
      return k;
    }
  }
  return -1;
}
