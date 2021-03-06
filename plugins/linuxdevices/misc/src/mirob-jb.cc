/*
  misc/mirob.cc
  The Mirob module linear robot from MPH

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

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <relacs/misc/mirob.h>
#include <pthread.h>
#include <termios.h>
#include <errno.h>
#include <fcntl.h>
#include <TML_lib.h>

#define WAIT      1
#define STOP      1
#define DONT_WAIT 0
#define DONT_STOP 0
#define	NO_ADDITIVE	0
#define FORWARD true
#define BACKWARD false



using namespace std;
using namespace relacs;

namespace misc {


PosDaemon::PosDaemon(watchdog_data* ptr){
  info = ptr;
  MaxSpeed = 50;
  MaxAcc = 0.3;
}

int PosDaemon::Start(){
  log("Robot Safety Daemon: Starting ");
  int code = pthread_create(&id, NULL, PosDaemon::EntryPoint, (void*) this);

  if (!code){

    int waitcycle = 0;
    struct timespec waitForDog;
    waitForDog.tv_sec = 1;
    waitForDog.tv_nsec = 0;

    while (info->active == false && waitcycle++ < 10){
      log(".");
      nanosleep(&waitForDog,NULL);
    }

    if (waitcycle == 11){
      log("[Failed]");
      return 1;
    }else{
      log("[OK]");
    }
  }else{
      log("[Failed]");
  }
  return 0;
}


int PosDaemon::Stop(){
  log("Robot Safety Daemon: Stopping Daemon.");
  info->active = false;
  pthread_join(id, NULL); // wait for watchdog to return
  return 0;
}

int PosDaemon::Run()
{
   Setup();
   Execute();
   Exit();
   return 0;
}


void PosDaemon::log(const char* text){
  if (info->gcb != NULL){
    info->gcb->logBox->append(QString(text));
  }else{
    cout << text << endl;
  }
}

void PosDaemon::log(relacs::Str text){
  if (info->gcb != NULL){
    info->gcb->logBox->append(QString(((string)text).c_str()));
  }else{
    cout << (string)text << endl;
  }

}


/*static */
void * PosDaemon::EntryPoint(void * pthis){
   PosDaemon * pt = (PosDaemon*)pthis;
   pt->Run();
   return NULL;
}

void PosDaemon::Setup(){

  /************ open watchdog ****************/
  // open device:
  if ( TS_OpenChannel( info->Device, info->ChannelType, info->HostID, info->Baudrate ) < 0 ) {
    cerr << "WATCHDOG Communication error! " << TS_GetLastErrorText() << endl;
  }

  // load setup file:
  int setupindex = TS_LoadSetup( info->SetupFile );
  if ( setupindex < 0 ) {
    cerr << "WATCHDOG Failed to load setup file! " << TS_GetLastErrorText() << endl;
  }

  //   setup axis:
  for ( int k=1; k<=3; k++ ) {
    if ( ! TS_SetupAxis( k, setupindex ) )  {
      cerr << "WATCHDOG Failed to setup axis " << k << "! " << TS_GetLastErrorText() << endl;
    }
    if ( ! TS_SelectAxis( k ) ) {
      cerr << "WATCHDOG Failed to select axis " << k << "! " << TS_GetLastErrorText() << endl;
    }

    if ( ! TS_SetTargetPositionToActual()){
      cerr << "WATCHDOG Failed to set target position to actual for axis " << k << "! " << TS_GetLastErrorText() << endl;
    }

    if ( ! TS_DriveInitialisation() ) {
      cerr << "WATCHDOG Failed to initialize drive for axis " << k << "! " << TS_GetLastErrorText() << endl;
    }
    if ( ! TS_Power( POWER_ON ) ) {
      cerr << "WATCHDOG Failed to power on drive for axis " << k << "! " << TS_GetLastErrorText() << endl;
    }

    WORD axison = 0;
    while ( axison == 0 ) {
      // Check the status of the power stage:
      if ( ! TS_ReadStatus( REG_SRL, axison ) ) {
	cerr << "WATCHDOG Failed to read status for axis " << k << "! " << TS_GetLastErrorText() << endl;
      }
      axison = ((axison & 1<<15) != 0 ? 1 : 0);
    }
  }
  // set active state true to signal Mirob that watchdog was created
  info->active = true;
  
}

int PosDaemon::activateAxis(int axis){
  // select axis:
  if ( ! TS_SelectAxis( axis ) ) {
    cerr << "WATCHDOG Failed to select axis " << axis << "! " << TS_GetLastErrorText() << endl;
    return 1;
  }
  return 0;
      
}

int PosDaemon::acquireInformation(){
  for (int axis = 1; axis <= 3; ++axis){

    activateAxis(axis);

    // check low limit switch
    if(!TS_GetInput(INPUT_24,limitNeg[axis-1])){
      cerr << "WATCHDOG Failed to get INPUT_24 for axis " << axis << "! " << TS_GetLastErrorText() << endl;
      continue;
    }
    // check high limit switch
    if(!TS_GetInput(INPUT_2,limitPos[axis-1])){
      cerr << "WATCHDOG Failed to get INPUT_2 for axis " << axis << "! " << TS_GetLastErrorText() << endl;
      continue;
    }

    // read position:
    if ( ! TS_GetLongVariable( "APOS", readAPOS[axis-1]) ) {
      cerr << "WATCHDOG Failed to read position of axis " << axis << "! " << TS_GetLastErrorText() << endl;
      continue;
    }
    trueAPOS[axis-1] = readAPOS[axis-1]; // hack to account for TML_Lib bug

//       if ( ! TS_ReadStatus( REG_MCR, reg[3] ) ) {
// 	cerr << "Failed to read status for axis " << axis << "! " << TS_GetLastErrorText() << endl;
// 	return NULL;
//       }

  }
  return 0;
  
}

bool PosDaemon::isInsideForbiddenZone(){
  return info->forbiddenZones->insideZone((double)trueAPOS[0],(double)trueAPOS[1],(double)trueAPOS[2]);
}

void PosDaemon::updateGUI(){
  //--------- call position callback if exists --------
  if (info->gcb != NULL){
    info->gcb->XPosLCD->display((double)trueAPOS[0]);
    info->gcb->YPosLCD->display((double)trueAPOS[1]);
    info->gcb->ZPosLCD->display((double)trueAPOS[2]);
    
    info->gcb->XLowLimLCD->display((int)limitNeg[0]);
    info->gcb->XHiLimLCD->display((int)limitPos[0]);
    
    info->gcb->YLowLimLCD->display((int)limitNeg[1]);
    info->gcb->YHiLimLCD->display((int)limitPos[1]);
    
    info->gcb->ZLowLimLCD->display((int)limitNeg[2]);
    info->gcb->ZHiLimLCD->display((int)limitPos[2]);
    
  }

}

int PosDaemon::gotoNegLimitsAndSetHome(){
  log("Robot Safety Daemon: Jogging to negative limits and setting new home!");

  BYTE var;
  var = 0;
  for (int axis = 3; axis >= 1; --axis){
    if (activateAxis(axis) > 0) return 1;

    if(!TS_GetInput(INPUT_24,var)){
      cerr << "Failed to get INPUT_24 for axis " << axis << "! " << TS_GetLastErrorText() << '\n';
      return 1;
    }

    if ((int)var == 1){


      // Execute jogging with the prescribed parameters; start motion immediately
      if(!TS_MoveVelocity(-MaxSpeed, MaxAcc , UPDATE_IMMEDIATE, FROM_REFERENCE)){
	cerr << "Failed to move to limit for axis " << axis \
	     << "! " << TS_GetLastErrorText() << '\n';
	return 1;
      }

      //Wait for positive limit switch to be reached. Stop at limit switch reach.
      if (!TS_SetEventOnLimitSwitch(LSW_NEGATIVE, TRANSITION_HIGH_TO_LOW, WAIT, STOP)){
	cerr << "Failed to wait for negative limit switch event for axis " \
	     << axis << "! " << TS_GetLastErrorText() << '\n';
	return 1;
      }

      
      if( ! TS_SetEventOnMotionComplete( WAIT, DONT_STOP ) ) { 
	cerr << "Failed to wait on axis " << axis << "! " << TS_GetLastErrorText() << '\n';
	return 1;
      }
      cerr << "Reached!" << endl;

      // drive back
      if(!TS_MoveVelocity(MaxSpeed, MaxAcc , UPDATE_IMMEDIATE, FROM_MEASURE)){
	cerr << "Failed to move to limit for axis " << axis \
	     << "! " << TS_GetLastErrorText() << '\n';
	return 1;
      }


      if (!TS_SetEventOnLimitSwitch(LSW_NEGATIVE, TRANSITION_LOW_TO_HIGH, WAIT, STOP)){
	cerr << "Failed to wait for negative limit switch event for axis " \
	     << axis << "! " << TS_GetLastErrorText() << '\n';
	return 1;
      }


    }else{
      cerr << "MIROB already at negative limit switch for axis "\
	   << axis << "!" << endl;
    }
    if (!TS_SetPosition(0)){
	cerr << "Failed to set position to 0 for axis " << axis \
	     << "! " << TS_GetLastErrorText() << '\n';
	return 1;

    }

  }
  info->setNegLimitAsHome = false;
  log("Robot Safety Daemon: New home completed!");
  return 0;
}

int PosDaemon::handleLimitErrors(){
  bool ishit = false;
  double speed = 0;
  bool positive = false;
  //------------ check for errors ---------------------
  for (int axis = 1; axis <=3; ++axis){
    // ---- handle negative  limit hit
    if ( (int)limitNeg[axis-1] == 0){
      log("Robot Safety Daemon:  negative limit hit on axis " + Str(axis) + \
	  "! Trying to move it back !");

      speed = 30;
      ishit = true;
      positive = false;

    }
    // ------ handle positive limit hit
    if ( (int)limitPos[axis-1] == 0){
      cerr << "Robot Safety Daemon:  positive limit hit on axis " << axis << "! Moving axis back into limits!" << endl;
      speed = -30;
      ishit = true;
      positive = true;

    }

    if (ishit){
      activateAxis(axis);

//       if(!TS_Execute("var_i1 = 0x0832; (var_i1),dm=1")) 
// 	cerr << "Robot Safety Daemon:  could not issue execute! " << TS_GetLastErrorText() << endl;

      // Execute jogging with the prescribed parameters; start motion immediately
      if(!TS_MoveVelocity(speed, 0.3 , UPDATE_IMMEDIATE, FROM_MEASURE)){
	cerr << "Robot Safety Daemon:  Failed to move to limit for axis " << axis \
	     << "! " << TS_GetLastErrorText() << endl;
      }

      if (positive){
	//Wait for positive limit switch to be reached. Stop at limit switch reach.
	if (!TS_SetEventOnLimitSwitch(LSW_POSITIVE, TRANSITION_LOW_TO_HIGH, WAIT, STOP)){
	  cerr << "Robot Safety Daemon:  Failed to wait for negative limit switch event for axis " \
	       << axis << "! " << TS_GetLastErrorText() << endl;
	}
      }else{
	//Wait for positive limit switch to be reached. Stop at limit switch reach.
	if (!TS_SetEventOnLimitSwitch(LSW_NEGATIVE, TRANSITION_LOW_TO_HIGH, WAIT, STOP)){
	  cerr << "Robot Safety Daemon:  Failed to wait for negative limit switch event for axis " \
	       << axis << "! " << TS_GetLastErrorText() << endl;
	}
      }

//       if(!TS_Execute("var_i1 = 0x0832; (var_i1),dm=0")) 
// 	cerr << "Robot Safety Daemon:  could not issue execute! " << TS_GetLastErrorText() << endl;

      ishit = false;
      log("Robot Safety Daemon: Successfully moved back!");
    }
     

  }
  return 0;
}

void PosDaemon::Execute(){
  int axis = 1;


  while (info->active){
    acquireInformation(); // read out important info from the robot

    updateGUI();
    //-------- check for whether watchdog is in forbidden zone -----------
    if (isInsideForbiddenZone()){
      if (!info->stopped){
	log("Robot Safety Daemon: Mirob entered forbidden zone! Stopping it!");
	for (axis = 1; axis <=3; ++axis){
	  if ( !TS_SelectAxis( axis ) ){
	    cerr << "Robot Safety Daemon: Problem with selecting axis! " << TS_GetLastErrorText() << endl;
	  }
	  if ( !TS_Stop( ) ){
	    cerr << "Robot Safety Daemon: Could not stop Mirob! " << TS_GetLastErrorText() << endl;
	  }
	}
	info->stopped = true;
	log("Robot Safety Daemon: Carefully move Mirob ouside zone!");
      }
    }else if(info->stopped){
      log("Robot Safety Daemon: Mirob ouside zone again! ");
      info->stopped = false;
    }
    

    if (info->setNegLimitAsHome){
      gotoNegLimitsAndSetHome();
    }
    handleLimitErrors();
  
    
    nanosleep(&info->sleeptime,NULL);
  }

}

void PosDaemon::Exit(){
  for ( int k=1; k<=3; k++ ) {
    if ( ! TS_SelectAxis( k ) )
      cerr << "Failed to select axis " << k << "! " << TS_GetLastErrorText() << endl;
    if ( ! TS_Stop() )
      cerr << "Failed to stop motion for axis " << k << "! " << TS_GetLastErrorText() << endl;
    if ( ! TS_Power( POWER_OFF ) )
      cerr << "Failed to power off drive for axis " << k << "! " << TS_GetLastErrorText() << endl;
  }
  TS_CloseChannel( -1 );
  cerr << "Robot Safety Daemon closed" << endl;

}


/********** MIROB **************************************************/

const string Mirob::SetupFile = "mirob2.t.zip";


/********************* initialization part **************************/

Mirob::Mirob( const string &device )
  : Manipulator( "Mirob" )
{
  Opened = false;
  for ( int k=0; k<3; k++ ) {
    Speed[k] = MaxSpeed;
    Acceleration[k] = MaxAcc;
  }

  open( device );
}

Mirob::Mirob( void )
  : Manipulator( "Mirob" )
{
  for ( int k=0; k<3; k++ ) {
    Speed[k] = MaxSpeed;
    Acceleration[k] = MaxAcc;
  }
  Opened = false;
}

Mirob::~Mirob( void )
{
  close();
}

int Mirob::open( const string &device )
{
  clearError();
  if ( Opened )
    return 0;

  Info.clear();
  Settings.clear();


  // open device:
  if ( TS_OpenChannel( device.c_str(), ChannelType, HostID, Baudrate ) < 0 ) {
    setErrorStr( "communication error: " + TS_GetLastErrorText() );
    return InvalidDevice;
  }

  // load setup file:
  int setupindex = TS_LoadSetup( SetupFile.c_str() );
  if ( setupindex < 0 ) {
    setErrorStr( "failed to load setup file: " + TS_GetLastErrorText() );
    return 1;
  }

  // setup axis:
  for ( int k=1; k<=3; k++ ) {
    if ( ! TS_SetupAxis( k, setupindex ) )  {
      setErrorStr( "failed to setup axis " + Str( k ) + ": " + TS_GetLastErrorText() );
      return 1;
    }
    if ( ! TS_SelectAxis( k ) ) {
      setErrorStr( "failed to select axis " + Str( k ) + ": " + TS_GetLastErrorText() );
      return 1;
    }
    
    if ( ! TS_SetTargetPositionToActual()){
      setErrorStr( "failed to set target position to actual for axis " + Str( k ) + ": " + TS_GetLastErrorText() );
      return 1;
    }
    

    if ( ! TS_DriveInitialisation() ) {
      setErrorStr( "failed to initialize drive for axis " + Str( k ) + ": " + TS_GetLastErrorText() );
      return 1;
    }
    
    if ( ! TS_Power( POWER_ON ) ) {
      setErrorStr( "failed to power on drive for axis " + Str( k ) + ": " + TS_GetLastErrorText() );
      return 1;
    }
    WORD axison = 0;
    while ( axison == 0 ) {
      // Check the status of the power stage:
      if ( ! TS_ReadStatus( REG_SRL, axison ) ) {
	setErrorStr( "failed to read status for axis " + Str( k ) + ": " + TS_GetLastErrorText() );
	return 1;
      }
      axison = ((axison & 1<<15) != 0 ? 1 : 0);
    }
  }
  /**** deploy watchdog thread *********************************/
  watchdog_info.ChannelType = ChannelType;
  watchdog_info.HostID = HostID;
  watchdog_info.Baudrate = Baudrate;
  watchdog_info.SetupFile = SetupFile.c_str();
  watchdog_info.Device = device.c_str();
  watchdog_info.gcb = NULL;
  watchdog_info.active = false;
  watchdog_info.active = false;
  watchdog_info.forbiddenZones = &forbiddenZones;
  watchdog_info.sleeptime.tv_sec = watchdog_sleep_sec;
  watchdog_info.sleeptime.tv_nsec = watchdog_sleep_nsec;
  watchdog_info.setNegLimitAsHome = false;

  watchdog =  new PosDaemon(&watchdog_info);
  watchdog->Start();
  
  /*************************************************************/


  setDeviceName( "Mirob" );
  setDeviceVendor( "MPH" );
  setDeviceFile( device );
  Device::addInfo();

  

  Opened = true;

  return 0;
}


// int Mirob::startWatchdog(void){
//   watchdog_info.active = false;
//   watchdog_info.active = false;
//   watchdog_info.forbiddenZones = &forbiddenZones;
//   watchdog_info.sleeptime.tv_sec = watchdog_sleep_sec;
//   watchdog_info.sleeptime.tv_nsec = watchdog_sleep_nsec;


//   cerr << "Creating watchdog ";
//   if ( pthread_create( &watchdog_thread, NULL, watchdog, (void*) &watchdog_info) > 0 ) return 1;

//   int waitcycle = 0;
//   struct timespec waitForDog;
//   waitForDog.tv_sec = 1;
//   waitForDog.tv_nsec = 0;

//   while (watchdog_info.active == false && waitcycle++ < 10){
//     cerr << ".";
//     nanosleep(&waitForDog,NULL);
//   }
//   if (waitcycle == 11){
//     cerr << "[FAILED]" <<  endl;
//     return 1;
//   }
  
//   cerr << "[OK]" << endl;
//   return 0;
// }

// int Mirob::stopWatchdog(void){
//   // closing watchdog
//   watchdog_info.active = false;
//   pthread_join(watchdog_thread, NULL); // wait for watchdog to return
//   return 0;
// }


int Mirob::restartWatchdog(void){
  watchdog->Stop();
  watchdog->Start();

//   stopWatchdog();
//   startWatchdog();
  return 0;
}

void Mirob::setGUICallback(GUICallback* gcb){
  watchdog_info.gcb = gcb;
}


void Mirob::close( void )
{
  clearError();
  if ( Opened ) {
    for ( int k=1; k<=3; k++ ) {
      if ( ! TS_SelectAxis( k ) )
	setErrorStr( "failed to select axis " + Str( k ) + ": " + TS_GetLastErrorText() );
      if ( ! TS_Stop() )
	setErrorStr( "failed to stop motion for axis " + Str( k ) + ": " + TS_GetLastErrorText() );
      if ( ! TS_Power( POWER_OFF ) )
	setErrorStr( "failed to power off drive for axis " + Str( k ) + ": " + TS_GetLastErrorText() );
    }
    TS_CloseChannel( -1 );
  }
  Opened = false;
  Info.clear();
  Settings.clear();

  watchdog->Stop();
}


/********************* activation and reactivation part *************/

int Mirob::reset( void )
{
  for ( int k=1; k<=3; k++ ) {
    if ( ! TS_SelectAxis( k ) ) {
      addErrorStr( "failed to select axis " + Str( k ) + "! " + TS_GetLastErrorText() );
      return 1;
    }
    if ( ! TS_ResetFault() ) {
      addErrorStr( "failed to reset fault axis " + Str( k ) + "! " + TS_GetLastErrorText() );
      return 1;
    }
    if ( ! TS_Power( POWER_ON ) ) {
      addErrorStr( "failed to power on drive for axis " + Str( k ) + "! " + TS_GetLastErrorText() );
      return 1;
    }
    WORD axison = 0;
    while ( axison == 0 ) {
      // Check the status of the power stage:
      if ( ! TS_ReadStatus( REG_SRL, axison ) ) {
	addErrorStr( "failed toread status for axis " + Str( k ) + "! " + TS_GetLastErrorText() );
	return 1;
      }
      axison = ((axison & 1<<15) != 0 ? 1 : 0);
    }
    /*
    if ( ! TS_Reset() ) {
      addErrorStr( "failed to reset axis " + Str( k ) + "! " + TS_GetLastErrorText() );
      return 1;
    }
   // Requires much more setup afterwards!
    */
  }

  return 0;
}

int Mirob::syncTposApos(void){
  cerr << "MIROB Setting target position to actual position!" << endl;
  for (int k = 1; k <= 3; ++k){
    if ( ! TS_SetTargetPositionToActual()){
      cerr << "Failed to set target position to actual for axis " << k << "! " << TS_GetLastErrorText() << '\n';
      return 1;
    }
  }
  return 0;

}

int Mirob::activateAxis(int ax){
    if ( ! TS_SelectAxis(ax ) ) {
      cerr << "Failed to select axis " << ax << "! " << TS_GetLastErrorText() << '\n';
      return 1;
    }
    return 0;
}

/****************** velocity part *******************************/
int Mirob::getV(double &x, double &y, double &z){

  if (activateAxis(1) > 0 || !TS_GetFixedVariable("ASPD",x)){
    cerr << "Failed to get APOS for axis 1! " << TS_GetLastErrorText() << '\n';
    return 1;
  }

  if (activateAxis(2) > 0 || !TS_GetFixedVariable("ASPD",y)){
    cerr << "Failed to get APOS for axis 2! " << TS_GetLastErrorText() << '\n';
    return 1;
  }

  if (activateAxis(3) > 0 || !TS_GetFixedVariable("ASPD",z)){
    cerr << "Failed to get APOS for axis 3! " << TS_GetLastErrorText() << '\n';
    return 1;
  }
  
  return 0;
}

int Mirob::setV(double v, int ax){
  if (activateAxis(ax) > 0 || !TS_MoveVelocity(v,MaxAcc,UPDATE_IMMEDIATE,FROM_MEASURE)){
    cerr << "Failed to set velocity "<< v << " for axis " << ax << "! " << TS_GetLastErrorText() << '\n';
    return 1;
  }
  return 0;
}

int Mirob::setVX(double v){
  return setV(v,1);
}

int Mirob::setVY(double v){
  return setV(v,2);
}

int Mirob::setVZ(double v){
  return setV(v,3);
}

int Mirob::setV(double vx, double  vy, double vz){
  double speed = sqrt(vx*vx + vy*vy + vz*vz );
  if (speed > MaxSpeed){
    vx *= MaxSpeed/speed;
    vy *= MaxSpeed/speed;
    vz *= MaxSpeed/speed;

  }
  return (setVX(vx) + setVY(vy) + setVZ(vz) > 0) ? 1 : 0; 

}


/***************** positioning part *****************************/

int Mirob::stop(){
  cerr << "MIROB Stop!" << endl;
  for (int axis = 3; axis >= 1; --axis){
    if (activateAxis(axis) > 0) return 1;

    if (!TS_Stop()){
	cerr << "Could not stop Mirob! " << TS_GetLastErrorText() << endl;
	return 1;
    }
//     if (!TS_Power(POWER_OFF)){
// 	cerr << "Could not power off Mirob! " << TS_GetLastErrorText() << endl;
// 	return 1;
//     }
//     if (!TS_Power(POWER_ON)){
// 	cerr << "Could not power on Mirob! " << TS_GetLastErrorText() << endl;
// 	return 1;
//     }


  }  
  return 0;
}
                             
int Mirob::gotoNegLimitsAndSetHome(void){
  watchdog_info.setNegLimitAsHome = true;
  return 0;
}


int Mirob::step( double x, int axis )
{
  cerr << "AXIS " << axis << " step by " << x << '\n';
  // select axis:
  if ( ! TS_SelectAxis( axis ) ) {
    cerr << "Failed to select axis " << axis << "! " << TS_GetLastErrorText() << '\n';
    return 1;
  }
  // move:
  long step = long( rint( x ) );
  if ( ! TS_MoveRelative( step, Speed[axis-1], Acceleration[axis-1],
			  NO_ADDITIVE, UPDATE_IMMEDIATE, FROM_REFERENCE ) ) {
    cerr << "Failed to move on axis " << axis << "! " << TS_GetLastErrorText() << '\n';
    return 1;
  }
  // wait:
  if( ! TS_SetEventOnMotionComplete( WAIT, DONT_STOP ) ) { 
    cerr << "Failed to wait on axis " << axis << "! " << TS_GetLastErrorText() << '\n';
    return 1;
  }

  return 0;
}

int Mirob::absPos( double x, double y, double z, double speed )
{
  speed = (speed > MaxSpeed) ? MaxSpeed : speed;
  double d[3] = {x,y,z};
  
  double dx,dy,dz;
  dx = abs(posX() - x);
  dy = abs(posY() - y);
  dz = abs(posZ() - z);

  double v_len = sqrt(dx*dx + dy*dy + dz*dz );

  double vx = speed * dx/v_len;
  double vy = speed * dy/v_len;
  double vz = speed * dz/v_len;

  double v[3] = {vx,vy,vz};



  for (int axis = 1; axis <= 3; ++axis){
    if ( ! TS_SelectAxis( axis ) ) {
      cerr << "Failed to select axis " << axis << "! " << TS_GetLastErrorText() << '\n';
      return 1;
    }
    // move:
    long step = long( rint( d[axis - 1] ) );
    if ( ! TS_MoveAbsolute(step, v[axis-1], MaxAcc, UPDATE_IMMEDIATE, FROM_MEASURE)){
      cerr << "Failed to move absolute on axis " << axis << "! " << TS_GetLastErrorText() << '\n';
      return 1;

    }

//     // wait:
//     if( ! TS_SetEventOnMotionComplete( WAIT, DONT_STOP ) ) { 
//       cerr << "Failed to wait on axis " << axis << "! " << TS_GetLastErrorText() << '\n';
//       return 1;
//     }
    
  }

  return 0;
}


int Mirob::suspendUntilPositionReached(double x, \
      double y, double z, double tol){
  
  double apos[3] = {-1,-1,-1};
  double tpos[3] = {x,y,z};
  
  while (true){
    for (int axis = 1; axis <= 3; ++axis){
      if (activateAxis(axis) > 0) return 1;
      apos[axis-1] = pos(axis);
    }
    
    cerr << "MIROB Pos " << apos[0] << ", " \
	   << apos[1] << ", " \
	   << apos[2] << endl;
    
      
    if ( (abs(apos[0] - tpos[0]) > tol) | 
	 (abs(apos[1] - tpos[1]) > tol) | 
	 (abs(apos[2] - tpos[2]) > tol)){
      nanosleep(&watchdog_info.sleeptime,NULL);
    }else{
      cerr << "MIROB position reached! " << endl;
      return 0;
    }
    
  }
  return 0;
  
}

int Mirob::suspendUntilStop(){
  double vx, vy, vz;

  while (true){
    for (int axis = 1; axis <= 3; ++axis){
      if (activateAxis(axis) > 0) return 1;
      getV(vx,vy,vz);

      cerr << "\rMIROB Speed " << vx << ", " \
	   << vy << ", " << vz ;
      
      if (vx < 1 && vy < 1 && vz < 1){
	return 0;
      }else{
	nanosleep(&watchdog_info.sleeptime,NULL);
      }
    }
  }
  return 0;
}

int Mirob::stepX( double x )
{
  return step( x, 1 );
}

int Mirob::stepY( double y )
{
  return step( y, 2 );
}

int Mirob::stepZ( double z )
{
  return step( z, 3 );
}

double Mirob::pos( int axis ) const
{
  // select axis:
  if ( ! TS_SelectAxis( axis ) ) {
    cerr << "Failed to select axis " << axis << "! " << TS_GetLastErrorText() << '\n';
    return 1;
  }

  // read position:
  long apos = 0;
  if ( ! TS_GetLongVariable( "APOS", apos) ) {
    cerr << "Failed to read position of axis " << axis << "! " << TS_GetLastErrorText() << '\n';
    return 0.0;
  }

  int apos2 = apos; // this is a hack to account for the bug in TML_lib
  return (double) apos2;
}

double Mirob::posX( void ) const
{
  return pos( 1 );
}

double Mirob::posY( void ) const
{
  return pos( 2 );
}

double Mirob::posZ( void ) const
{
  return pos( 3 );
}

int Mirob::step(double dx, double dy, double dz, double speed, bool wait){
  
  /*	Motion parameters for axis 1 */	
  double v_len = sqrt(dx*dx + dy*dy + dz*dz );


  if (speed > MaxSpeed)
    speed = MaxSpeed;

  double vx = speed * abs(dx)/v_len;
  double vy = speed * abs(dy)/v_len;
  double vz = speed * abs(dz)/v_len;
  double v[3] = {vx,vy,vz};
  long d[3] = {long(rint(dx)),long(rint(dy)),long(rint(dz))};


  for (int k = 1; k <= 3; ++k){
    if (activateAxis(k) > 0|| \
	!TS_MoveRelative(d[k-1], v[k-1], MaxAcc, NO_ADDITIVE, \
			 UPDATE_IMMEDIATE, FROM_REFERENCE)){
      cerr << "Failed to step at axis " << k << "! " << TS_GetLastErrorText() << '\n';
      return 1;
    }
  }

  if (wait){
    suspendUntilStop();
  }
  
  return 0;
}

int Mirob::clear(int axis)
{
  // select axis:
  if ( ! TS_SelectAxis( axis ) ) {
    cerr << "Failed to select axis " << axis << "! " << TS_GetLastErrorText() << '\n';
    return 1;
  }
  if (!TS_SetPosition(0)){
    cerr << "Failed to set home position for axis " << axis << "! " << TS_GetLastErrorText() << '\n';
    return 1;
  }
  return 0;
}

int Mirob::clearX( void )
{
  return clear(1);
}

int Mirob::clearY( void )
{
  return clear(2);
}

int Mirob::clearZ( void )
{
  return clear(3);
}


int Mirob::clear( void )
{
  clearX();
  clearY();
  clearZ();
  return 0;
}


int Mirob::homeX( void )
{
  /*
  double dist = -Pos[0];
  double steps = 0.0;
  if ( dist > 0.0 )
    steps = dist / PosAmplitude[0] / PosGain[0];
  else
    steps = dist / NegAmplitude[0] / NegGain[0];
  return stepX( steps );
  */
  return stepX(-posX());
}


int Mirob::homeY( void )
{
  /*
  double dist = -Pos[1];
  double steps = 0.0;
  if ( dist > 0.0 )
    steps = dist / PosAmplitude[1] / PosGain[1];
  else
    steps = dist / NegAmplitude[1] / NegGain[1];
  return stepY( steps );
  */
  return stepY(-posY());
}


int Mirob::homeZ( void )
{
  /*
  double dist = -Pos[2];
  double steps = 0.0;
  if ( dist > 0.0 )
    steps = dist / PosAmplitude[2] / PosGain[2];
  else
    steps = dist / NegAmplitude[2] / NegGain[2];
  return stepZ( steps );
  */
  return stepZ(-posZ());
}


int Mirob::home( void )
{
  return homeX() + ( homeY() << 1 ) + ( homeZ() << 2 );
}


int Mirob::setAmplX( double posampl, double negampl )
{
  /*
  int pa = int( rint( posampl ) );
  int na = negampl < 0.0 ? pa : int( rint( negampl ) );

  if ( pa >= 1 && pa <= 80 && na >= 1 && na <= 80 ) {
    amplitudepos( 0, pa );
    amplitudeneg( 0, na );
    return 0;
  }
  else
    return 1;
  */
  return 0;
}


int Mirob::setAmplY( double posampl, double negampl )
{
  /*
  int pa = int( rint( posampl ) );
  int na = negampl < 0.0 ? pa : int( rint( negampl ) );

  if ( pa >= 1 && pa <= 80 && na >= 1 && na <= 80 ) {
    amplitudepos( 1, pa );
    amplitudeneg( 1, na );
    return 0;
  }
  else
    return 2;
  */
  return 0;
}


int Mirob::setAmplZ( double posampl, double negampl )
{
  /*
  int pa = int( rint( posampl ) );
  int na = negampl < 0.0 ? pa : int( rint( negampl ) );

  if ( pa >= 1 && pa <= 80 && na >= 1 && na <= 80 ) {
    amplitudepos( 2, pa );
    amplitudeneg( 2, na );
    return 0;
  }
  else
    return 4;
  */
  return 0;
}


double Mirob::minAmplX( void ) const
{
  return 1.0;
}


double Mirob::maxAmplX( void ) const
{
  return 80.0;
}


/**************** tool control part ***************************/
int Mirob::clampTool(void){
  if (activateAxis(3) > 0 || !TS_SetOutput(OUTPUT_30, IO_HIGH) ||!TS_SetOutput(OUTPUT_31, IO_LOW) ){
    cerr << "Failed to clamp tool!" << TS_GetLastErrorText() << '\n';
    return 1;
  }
  return 0;
}

int Mirob::releaseTool(void){

  if (activateAxis(3) > 0 || !TS_SetOutput(OUTPUT_31, IO_HIGH) ||!TS_SetOutput(OUTPUT_30, IO_LOW) ){
    cerr << "Failed to release tool!" << TS_GetLastErrorText() << '\n';
    return 1;
  }
  return 0;
}



/**************** trajectory part  ***************************/
int Mirob::startRecording(void){
  syncTposApos();
  cerr << "MIROB position recording started! " << endl;
  record0 = *new positionUpdate(posX(), posY(), posZ());
  recordedSteps.clear();
  return 0;
}

int Mirob::recordStep(void){
  syncTposApos();
  //positionUpdate tmp;
  int l = recordedSteps.size();

  cerr << "posX " << posX() << endl;
  cerr << "posY " << posY() << endl;
  cerr << "posZ " << posZ() << endl;

  // record step
  positionUpdate tmp(posX() - record0.x, \
		     posY() - record0.y, \
		     posZ() - record0.z);

  recordedSteps.push_back(tmp);
  
  // update position
  record0.x = posX();
  record0.y = posY();
  record0.z = posZ();
  miroblog("Recorded step " + Str(recordedSteps[l].x) 
	   + ", " + Str(recordedSteps[l].y) + ", "+  Str(recordedSteps[l].z));
  miroblog("New position is " + Str(record0.x) + ", " 
	   +  Str(record0.y) + ", "+  Str(record0.z));
  miroblog("Recorded " + Str(l+1) + " updates!");
  return 0;
}

int Mirob::stopRecording(){
  miroblog("MIROB position recording stopped! ");
  return 0;
}

int Mirob::executeRecordedTrajectory(double speed, bool forward, bool wait){
  syncTposApos();
  int l = recordedSteps.size(), start,stop;
  double sgn, update;
  if (forward){
    sgn = 1; 
    update = 1;
    start = 0;
    stop = l;
  }else{
    sgn = -1;
    update = -1;
    start = l-1;
    stop  = -1;
  }
  
  for (; start != stop; start += update){
    cerr << "Executing step " << sgn*recordedSteps[start].x\
	 << ", " << sgn*recordedSteps[start].y \
	 << ", " << sgn*recordedSteps[start].z << endl;
    step(sgn*recordedSteps[start].x, \
	 sgn*recordedSteps[start].y, \
	 sgn*recordedSteps[start].z, speed, wait);
  }

  return 0;
  

}

/******** forbidden zone stuff ************************************/

int Mirob::recordPosition(void){
  positions.push_back( Point3D(posX(), posY(), posZ()) );
  int l = positions.size();
  
  miroblog("MIROB: Recorded position (" + Str(positions[l-1].x)
	   + ", " + Str(positions[l-1].y) + ", " + Str(positions[l-1].z) + 
	   ")! " + Str(l) + " positions in total");
  return 0;
}
  
int Mirob::clearPositions(void){
  cerr << "MIROB clearing positions!" << endl;
  positions.clear();
  return 0;
}

int Mirob::makePositionsForbiddenZone(void){
  forbiddenZones.addZone(positions);
  clearPositions();
  return 0;
}


void Mirob::miroblog(const char* text){
  if (watchdog_info.gcb != NULL){
    watchdog_info.gcb->logBox->append(QString(text));
  }
}

void Mirob::miroblog(Str text){
  if (watchdog_info.gcb != NULL){
    watchdog_info.gcb->logBox->append(QString(((string)text).c_str()));
  }
}


}; /* namespace misc */
