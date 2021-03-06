/*
  parameter.cc
  A Parameter has a name, value and unit.

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

#include <cmath>
#include <cstdlib>
#include <cstdio>
#include <iomanip>
#include <sstream>
#include <iterator>
#include <algorithm>
#include <relacs/options.h>
#include <relacs/parameter.h>

namespace relacs {


Parameter::Parameter( void )
{
  clear();
}


Parameter::Parameter( const Parameter &p )
{
  assign( p );
}


Parameter::Parameter( const string &name, const string &request,
		      const string &strg, int flags, int style,
		      Options *parentsection )
  : ParentSection( parentsection )
{
  clear( name, request, Text );

  string e;
  setText( strg );
  e += Warning;
  setDefaultText( strg );
  e += Warning;
  InternUnit = "";
  OutUnit = "";
  setFlags( flags );
  setStyle( style );
  Warning = e;
}


Parameter::Parameter( const string &name, const string &request,
		      const char *strg, int flags, int style,
		      Options *parentsection )
  : ParentSection( parentsection )
{
  clear( name, request, Text );

  string e;
  string s( strg );
  setText( s );
  e += Warning;
  setDefaultText( s );
  e += Warning;
  InternUnit = "";
  OutUnit = "";
  setFlags( flags );
  setStyle( style );
  Warning = e;
}


Parameter::Parameter( const string &name, const string &request,
		      double number, double error,
		      double minimum, double maximum, double step,
		      const string &internunit, const string &outputunit,
		      const string &format, int flags, int style,
		      Options *parentsection )
  : ParentSection( parentsection )
{
  clear( name, request, Number );

  string e;
  setUnit( internunit, outputunit );
  e += Warning;
  setMinMax( minimum, maximum, step );
  e += Warning;
  setNumber( number, error );
  e += Warning;
  setDefaultNumber( number );
  e += Warning;
  setFormat( format );
  e += Warning;
  setFlags( flags );
  setStyle( style );
  Warning = e;
}


Parameter::Parameter( const string &name, const string &request,
		      double number, const string &unit,
		      const string &format, int flags, int style,
		      Options *parentsection )
  : ParentSection( parentsection )
{
  clear( name, request, Number );
  string e;
  setUnit( unit, unit );
  e += Warning;
  setNumber( number );
  e += Warning;
  setDefaultNumber( number );
  e += Warning;
  setFormat( format );
  e += Warning;
  setFlags( flags );
  setStyle( style );
  Warning = e;
}


Parameter::Parameter( const string &name, const string &request,
		      const vector<double> &numbers,
		      const vector<double> &errors,
		      double minimum, double maximum, double step,
		      const string &internunit, const string &outputunit,
		      const string &format, int flags, int style,
		      Options *parentsection )
  : ParentSection( parentsection )
{
  clear( name, request, Number );

  string e;
  setUnit( internunit, outputunit );
  e += Warning;
  setMinMax( minimum, maximum, step );
  e += Warning;
  setNumbers( numbers, errors );
  e += Warning;
  if ( numbers.size() > 0 ) {
    setDefaultNumber( numbers[0] );
    e += Warning;
  }
  setFormat( format );
  e += Warning;
  setFlags( flags );
  setStyle( style );
  Warning = e;
}


#ifdef HAVE_LIBRELACSSHAPES
Parameter::Parameter( const string &name, const string &request,  
		      const Point &p, double minimum,
		      double maximum, double step,
		      const string &internunit, const string &outputunit, 
		      const string &format, int flags, int style,
		      Options *parentsection )
{
  clear( name, request, Number );

  string e;
  setUnit( internunit, outputunit );
  e += Warning;
  setMinMax( minimum, maximum, step );
  e += Warning;
  setPoint( p );
  e += Warning;
  setDefaultPoint( p );
  e += Warning;
  setFormat( format );
  e += Warning;
  setFlags( flags );
  setStyle( style );
  Warning = e;
}
#endif


Parameter::Parameter( const string &name, const string &request,
		      long number, long error,
		      long minimum, long maximum, long step,
		      const string &internunit, const string &outputunit,
		      int width, int flags, int style,
		      Options *parentsection )
  : ParentSection( parentsection )
{
  clear( name, request, Integer );

  string e;
  setUnit( internunit, outputunit );
  e += Warning;
  setMinMax( minimum, maximum, step );
  e += Warning;
  setInteger( number, error );
  e += Warning;
  setDefaultInteger( number );
  e += Warning;
  setFormat( width, 0, 'f' );
  e += Warning;
  setFlags( flags );
  setStyle( style );
  Warning = e;
}


Parameter::Parameter( const string &name, const string &request,
		      bool dflt, int flags, int style,
		      Options *parentsection )
  : ParentSection( parentsection )
{
  clear( name, request, Boolean );

  string e;
  setBoolean( dflt );
  e += Warning;
  setDefaultBoolean( dflt );
  e += Warning;
  setFlags( flags );
  setStyle( style );
  Warning = e;
}


Parameter::Parameter( const string &name, const string &request,
		      ValueType type,
		      int yearhour, int monthminutes, int dayseconds,
		      int flags, int style,
		      Options *parentsection )
  : ParentSection( parentsection )
{
  string e;
  if ( type != Date && type != Time ) {
    e = "type is neither Date nor Time";
    type = Date;
  }

  clear( name, request, type );

  if ( isDate() )
    setDate( yearhour, monthminutes, dayseconds );
  else
    setTime( yearhour, monthminutes, dayseconds, 0 );
  e += Warning;
  if ( isDate() )
    setDefaultDate( yearhour, monthminutes, dayseconds );
  else
    setDefaultTime( yearhour, monthminutes, dayseconds, 0 );
  e += Warning;
  setFlags( flags );
  setStyle( style );
  Warning = e;
}


Parameter::Parameter( const string &name, const string &request,
		      int hour, int minutes, int seconds, int milliseconds,
		      int flags, int style,
		      Options *parentsection )
  : ParentSection( parentsection )
{
  string e;

  clear( name, request, Time );

  setFormat( "%02H:%02M:%02S.%03Z" );
  setTime( hour, minutes, seconds, milliseconds );
  e += Warning;
  setDefaultTime( hour, minutes, seconds, milliseconds );
  e += Warning;
  setFlags( flags );
  setStyle( style );
  Warning = e;
}


Parameter::Parameter( const string &name, const string &value )
{
  Str names = name;
  Str request = "";
  if ( names.size() > 2 && names[names.size()-1] == ')' ) {
    // request string:
    int n = names.rfind( '(' );
    if ( n >= 0 ) {
      request = names.mid( n+1, names.size()-2 );
      names.erase( n-1 );
      names.strip( Str::WhiteSpace + '"' );
      request.strip( Str::WhiteSpace + '"' );
    }
  }
  // set parameter with value:
  clear( names, request, NoType );
  assign( value );
  setToDefault();

  if ( names.empty() )
    Warning = "\"" + name + "\": missing name! ";

  Flags |= ChangedFlag;
}


Parameter::~Parameter( void )
{
}


Parameter &Parameter::clear( const string &name, const string &request,
			     ValueType type )
{
  Name = name;
  Request = request.empty() ? name : request;
  VType = type;
  Flags = 0;
  if ( ( ! String.empty() && ! String[0].empty() ) ||
       ( ! Value.empty() && Value[ 0 ] != 0.0 ) )
    Flags |= ChangedFlag;
  Style = 0;
  setFormat();
  String.resize( 1, "" );
  DefaultString.resize( 1, "" );
  Year.resize( 1 ); Year[0] = 0;
  Month.resize( 1 ); Month[0] = 0;
  Day.resize( 1 ); Day[0] = 0;
  DefaultYear.resize( 1 ); DefaultYear[0] = 0;
  DefaultMonth.resize( 1 ); DefaultMonth[0] = 0;
  DefaultDay.resize( 1 ); DefaultDay[0] = 0;
  Hour.resize( 1 ); Hour[0] = 0;
  Minutes.resize( 1 ); Minutes[0] = 0;
  Seconds.resize( 1 ); Seconds[0] = 0;
  MilliSeconds.resize( 1 ); MilliSeconds[0] = 0;
  DefaultHour.resize( 1 ); DefaultHour[0] = 0;
  DefaultMinutes.resize( 1 ); DefaultMinutes[0] = 0;
  DefaultSeconds.resize( 1 ); DefaultSeconds[0] = 0;
  DefaultMilliSeconds.resize( 1 ); DefaultMilliSeconds[0] = 0;
  Value.resize( 1 );
  Value[ 0 ] = 0.0;
  DefaultValue.resize( 1 );
  DefaultValue[ 0 ] = 0.0;
  Error.resize( 1 );
  Error[ 0 ] = -1.0;
  Minimum = -MAXDOUBLE;
  Maximum = MAXDOUBLE;
  Step = 1.0;
  InternUnit = "";
  OutUnit = "";
  ActivationName.clear();
  ActivationValues.clear();
  ActivationNumber.clear();
  ActivationUnit.clear();
  ActivationComparison.clear();
  ActivationType.clear();
  ActivationStatus.clear();
  Warning = "";
  clearSelectOptions();
  return *this;
}


Parameter &Parameter::operator=( const Parameter &p )
{
  return assign( p );
}


Parameter &Parameter::assign( const Parameter &p )
{
  if ( this == &p )
    return *this;

  ParentSection = p.ParentSection;
  Name = p.Name;
  Request = p.Request;
  VType = p.VType;
  Flags = p.Flags;
  if ( ( ! String.empty() && ! p.String.empty() && String[0] != p.String[0] ) ||
       ( ! Value.empty() && ! p.Value.empty() && Value[0] != p.Value[0] ) )
    Flags |= ChangedFlag;
  Style = p.Style;
  Format = p.Format;
  String = p.String;
  DefaultString = p.DefaultString;
  Year = p.Year;
  Month = p.Month;
  Day = p.Day;
  DefaultYear = p.DefaultYear;
  DefaultMonth = p.DefaultMonth;
  DefaultDay = p.DefaultDay;
  Hour = p.Hour;
  Minutes = p.Minutes;
  Seconds = p.Seconds;
  MilliSeconds = p.MilliSeconds;
  DefaultHour = p.DefaultHour;
  DefaultMinutes = p.DefaultMinutes;
  DefaultSeconds = p.DefaultSeconds;
  DefaultMilliSeconds = p.DefaultMilliSeconds;
  Value = p.Value;
  DefaultValue = p.DefaultValue;
  Error = p.Error;
  Minimum = p.Minimum;
  Maximum = p.Maximum;
  Step = p.Step;
  InternUnit = p.InternUnit;
  OutUnit = p.OutUnit;
  ActivationName = p.ActivationName;
  ActivationValues = p.ActivationValues;
  ActivationNumber = p.ActivationNumber;
  ActivationUnit = p.ActivationUnit;
  ActivationComparison = p.ActivationComparison;
  ActivationType = p.ActivationType;
  ActivationStatus = p.ActivationStatus;
  Warning = "";

  SelectableValues = p.SelectableValues;

  return *this;
}


Parameter &Parameter::operator=( const string &value )
{
  return assign( value );
}


Parameter &Parameter::assign( const string &value )
{
  Warning = "";

  // Multiple options style and combo box mode
  if (isText() && ((style() & (Parameter::Select)) == Parameter::Select))
  {
    // trim helper function because there isn't any in the stl
    static const auto trim = [](const std::string& val)
    {
      std::string s(val);
      s.erase(s.begin(), std::find_if(s.begin(), s.end(), std::not1(std::ptr_fun<int, int>(std::isspace))));
      s.erase(std::find_if(s.rbegin(), s.rend(), std::not1(std::ptr_fun<int, int>(std::isspace))).base(), s.end());
      return s;
    };

    auto type = std::count(value.begin(), value.end(), '[');
    // new style "[ [ selected1, ...,  selectedn], [ value1, ..., valuem ] ]"
    if (type == 3)
    {
      auto find = value.find_first_of("[", 1);
      std::string values = trim(value.substr(find, value.find_first_of("]", find) - find + 1));

      find = value.find_first_of("[", find + 1) + 1;
      std::string definitions = trim(value.substr(find, value.find_first_of("]", find) - find));

      setText(values);

      StrQueue sq;
      sq.assign(definitions, ",");
      clearSelectOptions();
      for (int i = 0; i< sq.size(); ++i)
      {
        // transform special characters
        std::string str = sq[i].strip();
        if (str == "~")
          str = "";
        else if (str.empty())
          continue;
        else if (str[0] == '"' && str[str.size() - 1] == '"')
        {
          str.erase(0, 1);
          str.erase(str.size() - 1, 1);
        }

        addSelectOption(str);
      }
    }
    // fallback style "[ selected, [ value1, ..., valuen ] ]"
    // fallback style "[ selected, value1, ..., valuen ] ]"
    else if (type == 2 || type == 1)
    {
      std::string copy(value);

      // remove additional backets
      if (type == 2)
      {
        auto find = copy.find_first_of("[", 1);
        copy.erase(find, 1);
        find = copy.find_last_of("]");
        copy.erase(find, 1);
      }

      // read everything and discard repeated values
      setText(trim(copy));
      for (int i = 1; i < String.size(); ++i)
        addSelectOption(String[i]);

      String.resize(1);
    }
  }
  else if ( isText() && ( size() > 1 || ( style() & Parameter::SelectText ) > 0 ) ) {
    selectText( value );
  }
  else {
    setText( value );
    if ( isNotype() ) {
      // check for time:
      bool time = true;
      for ( int k=0; time && k<String.size(); k++ ) {
	int hour, minutes, seconds, milliseconds;
	if ( Str( String[k] ).time( hour, minutes, seconds, milliseconds ) != 0 )
	  time = false;
      }
      if ( time ) {
	setValueType( Time );
	Hour.clear();
	Minutes.clear();
	Seconds.clear();
	MilliSeconds.clear();
	for ( int k=0; k<String.size(); k++ ) {
	  int hour, minutes, seconds, milliseconds;
	  Str( String[k] ).time( hour, minutes, seconds, milliseconds );
	  addTime( hour, minutes, seconds, milliseconds );
	}
	setUnit( "", "" );
      }
      else {
	// check for date:
	bool date = true;
	for ( int k=0; date && k<String.size(); k++ ) {
	  int year, month, day;
	  if ( Str( String[k] ).date( year, month, day ) != 0 )
	    date = false;
	}
	if ( date ) {
	  setValueType( Date );
	  Year.clear();
	  Month.clear();
	  Day.clear();
	  for ( int k=0; k<String.size(); k++ ) {
	    int year, month, day;
	    Str( String[k] ).date( year, month, day );
	    addDate( year, month, day );
	  }
	  setUnit( "", "" );
	}
	else {
	  // check for numbers:
	  bool num = true;
	  for ( int k=0; num && k<String.size(); k++ ) {
	    double error=-1.0;
	    string unit = "";
	    int next = 0;
	    if ( String[k].number( error, unit, MAXDOUBLE, 0, &next,
				   Str::DoubleWhiteSpace ) == MAXDOUBLE ||
		 next < String[k].size() )
	      num = false;
	  }
	  if ( num ) {
	    setValueType( Number );
	    Value.clear();
	    Error.clear();
	    for ( int k=0; k<String.size(); k++ )
	      addNumber( String[ k ], "" );
	    bool integer = true;
	    if ( ! InternUnit.empty() && InternUnit != "L" )
	      integer = false;
	    for ( unsigned int k=0; integer && k<Value.size(); k++ )
	      if ( ::fabs( Value[k] - rint( Value[k] ) ) > 1.0e-8 )
		integer = false;
	    setValueType( integer ? Integer : Number );
	    if ( integer ) {
	      setFormat( "%.0f" );
	      setStep( 1.0 );
	    }
	    else {
	      setFormat( "%g" );
	      double max = 0.0;
	      double step = 1.0;
	      for ( int k=0; k<String.size() && k<(int)Value.size(); k++ ) {
		double val = fabs( ::fabs( Value[k] ) );
		if ( val > max )
		  max = val;
		int prec = 0;
		int pp = String[k].find( '.' );
		if ( pp >= 0 ) {
		  int np = String[k].findFirstNot( Str::Digit, pp+1 );
		  if ( np < 0 )
		    np = String[k].size();
		  prec = np - pp - 1;
		}
		double pstep = 1.0;
		if ( prec > 0 )
		  pstep = pow( 10.0, -prec );
		if ( pstep < step )
		  step = pstep;
	      }
	      if ( max/step < 10.0 )
		max = 10.0*step;
	      max = 1000.0 * ceil10( max );
	      setMinMax( -max, max, step );
	    }
	  }
	  else {
	    // check for boolean:
	    bool b = true;
	    for ( int k=0; k<String.size(); k++ )
	      if ( String[k] != "false" && String[k] != "true" ) {
		b = false;
		break;
	      }
	    if ( b ) {
	      setValueType( Boolean );
	      setUnit( "", "" );
	      for ( int k=0; k<String.size(); k++ )
		Value.push_back( String[k] == "true" ? 1.0 : 0.0 );
	    }
	    else {
	      setValueType( Text );
	    }
	  }
	}
      }
      setFormat();
    }
  }

  return *this;
}


Parameter &Parameter::setValue( const Parameter &p )
{
  Warning = "";

  if ( valueType() != p.valueType() ) {
    Warning = "Paramter differ in their type - cannot set value.";
    return *this;
  }

  if ( ( ! String.empty() && ! p.String.empty() && String[0] != p.String[0] ) ||
       ( ! Value.empty() && ! p.Value.empty() && Value[0] != p.Value[0] ) )
    Flags |= ChangedFlag;

  String = p.String;
  Year = p.Year;
  Month = p.Month;
  Day = p.Day;
  Hour = p.Hour;
  Minutes = p.Minutes;
  Seconds = p.Seconds;
  MilliSeconds = p.MilliSeconds;
  Value = p.Value;
  Error = p.Error;
  InternUnit = p.InternUnit;

  return *this;
}


bool operator==( const Parameter &p1, const Parameter &p2 )
{
  return ( p1.name() == p2.name() );
}


bool operator==( const Parameter &p, const string &name )
{
  // XXX implement comparison with special characters ^*xxx* ...
  return ( p.name() == name );
}


Options *Parameter::parentSection( void )
{
  return ParentSection;
}


const Options *Parameter::parentSection( void ) const
{
  return ParentSection;
}


void Parameter::setParentSection( Options *parentsection )
{
  ParentSection = parentsection;
}


bool Parameter::nonDefault( void ) const
{
  if ( isAnyNumber() )
    return ( DefaultValue != Value );
  else if ( isDate() )
    return ( DefaultYear != Year || DefaultMonth != Month || DefaultDay != Day );
  else if ( isTime() )
    return ( DefaultHour != Hour || DefaultMinutes != Minutes ||
	     DefaultSeconds != Seconds || DefaultMilliSeconds != MilliSeconds );
  else if ( isText() )
    return ( DefaultString != String );
  else
    return false;
}


Str Parameter::warning( void ) const
{
  return Warning;
}


Str Parameter::name( void ) const
{
  return Name;
}


Parameter &Parameter::setName( const string &name )
{
  Name = name;
  return *this;
}


Str Parameter::request( void ) const
{
  return Request;
}


Parameter &Parameter::setRequest( const string &request )
{
  Request = request;
  return *this;
}


Parameter::ValueType Parameter::valueType( void ) const
{
  return VType;
}


bool Parameter::valueType( int mask ) const
{
  return ( mask == 0 ||
	   ( mask > 0 && ( mask & valueType() ) > 0 ) ||
	   ( mask < 0 && ( -mask & valueType() ) == 0 ) );
}


Parameter &Parameter::setValueType( ValueType type )
{
  VType = type;
  return *this;
}


int Parameter::flags( void ) const
{
  return Flags;
}


bool Parameter::flags( int selectflag ) const
{
  return ( selectflag == 0 ||
	   ( ( selectflag == NonDefault ||
	       ( flags() & abs( selectflag ) ) ) &&
	     ( selectflag > 0 || nonDefault() ) ) );
}


Parameter &Parameter::setFlags( int flags )
{
  Flags = flags;
  return *this;
}


Parameter &Parameter::addFlags( int flags )
{
  Flags |= flags;
  return *this;
}


Parameter &Parameter::delFlags( int flags )
{
  Flags &= ~flags;
  return *this;
}


Parameter &Parameter::clearFlags( void )
{
  Flags = 0;
  return *this;
}


int Parameter::changedFlag( void )
{
  return ChangedFlag;
}


bool Parameter::changed( void ) const
{
  return ( Flags & ChangedFlag );
}


int Parameter::style( void ) const
{
  return Style;
}


Parameter &Parameter::setStyle( int style )
{
  Style = style;
  return *this;
}


Parameter &Parameter::addStyle( int style )
{
  Style |= style;
  return *this;
}


Parameter &Parameter::delStyle( int style )
{
  Style &= ~style;
  return *this;
}


Str Parameter::format( void ) const
{ 
  Warning = "";
  return Format; 
}


Parameter &Parameter::setFormat( const string &format )
{
  Warning = "";
  if ( format.empty() ) {
    if ( isNumber() )
      Format = "%g";
    else if ( isBoolean() || isInteger() )
      Format = "%.0f";
    else if ( isDate() )
      Format = "%04Y-%02m-%02d";
    else if ( isTime() )
      Format = "%02H:%02M:%02S";
    else
      Format = "%s";
  }
  else {
    // format checking: XXX still missing XXX ...
    Format = format;
  }
  return *this;
}


Parameter &Parameter::setFormat( int width, int prec, char fmt )
{
  Warning = "";

  if ( isDate() || isTime() )
    return *this;

  char format[100];

  // check format character:
  string f( "risfgeFGEub" );
  if ( f.find( fmt ) == string::npos ) {
    if ( isNumber() )
      fmt = 'g';
    else if ( isBoolean() || isInteger() )
      fmt = 'f';
    else
      fmt = 's';
    if ( fmt != '-' )
      Warning = "invalid format specifier";
  }
  
  // set zero precision for integer:
  if ( isInteger() )
    prec = 0;
  
  // create format string:
  if ( width == 0 ) {
    if ( prec < 0 )
      sprintf( format, "%%%c", fmt );
    else
      sprintf( format, "%%.%d%c", prec, fmt );
  }
  else {
    if ( prec < 0 )
      sprintf( format, "%%%d%c", width, fmt );
    else
      sprintf( format, "%%%d.%d%c", width, prec, fmt );
  }
  
  Format = format;

  return *this;
}


int Parameter::formatWidth( void ) const
{
  return Format.totalWidth();
}


int Parameter::size( void ) const
{
  if ( isAnyNumber() )
    return Value.size();

  else if ( isText() )
    return String.size();

  else if ( isDate() )
    return Year.size();

  else if ( isTime() )
    return Hour.size();

  else
    return 0;
}


bool Parameter::isText( void ) const
{
  return ( VType == Text );
}


Str Parameter::text( int index, const string &format, const string &unit ) const
{ 
  Warning = "";

  if ( index < 0 ) {
    Warning = "Parameter::text -> negative index for parameter '" +
      Name + "' requested!";
    return "";
  }

  if ( isText() && index >= String.size() ) {
    Warning = "Parameter::text -> requested parameter '" +
      Name + "' has only " + Str( String.size() ) + " text values !";
    return "";
  }

  if ( isAnyNumber() && index >= (int)Value.size() ) {
    Warning = "Parameter::text -> requested parameter '" +
      Name + "' has only " + Str( Value.size() ) + " number values !";
    return "";
  }

  if ( isDate() && index >= (int)Year.size() ) {
    Warning = "Parameter::text -> requested parameter '" +
      Name + "' has only " + Str( Year.size() ) + " date values !";
    return "";
  }

  if ( isTime() && index >= (int)Hour.size() ) {
    Warning = "Parameter::text -> requested parameter '" +
      Name + "' has only " + Str( Hour.size() ) + " time values !";
    return "";
  }

  Str f( format );
  if ( f.empty() )
    f = Format;

  f.format( Name, 'i' );
  f.format( Request, 'r' );

  string typestr = "notype";
  if ( isText() )
    typestr = "string";
  else if ( isNumber() )
    typestr = "number";
  else if ( isInteger() )
    typestr = "integer";
  else if ( isBoolean() )
    typestr = "boolean";
  else if ( isDate() )
    typestr = "date";
  else if ( isTime() )
    typestr = "time";
  f.format( typestr, 'T' );

  Str s( "" );
  if ( index < String.size() )
    s = String[index];

  Str u( unit );
  if ( u.empty() )
    u = OutUnit;

  if ( isAnyNumber() ) {

    double uv = changeUnit( 1.0, InternUnit, u );

    double v = index < (int)Value.size() ? Value[index] : 0.0;
    double e = index < (int)Error.size() ? Error[index] : 0.0;
    v *= uv;
    e *= uv;

    if ( s.empty() && f == "%s" ) {
      if ( isBoolean() )
	f = "%b";
      else if ( isInteger() )
	if ( e >= 0.0 )
	  f = "(%.0f+-%.0F)%u";
	else
	  f = "%.0f%u";
      else
	if ( e >= 0.0 )
	  f = "(%g+-%G)%u";
	else
	  f = "%g%u";
    }

    if ( e < 0 )
      e = 0.0;
    
    f.format( v, "fge" );
    f.format( e, "FGE" );
    
    string b( v != 0 ? "true" : "false" );
    f.format( b, 'b' );
  }
  else if ( isDate() ) {
    if ( s.empty() && f == "%s" )
      f = "%04Y-%02m-%02d";
    f.format( Year[index], Month[index], Day[index] );
  }
  else if ( isTime() ) {
    if ( s.empty() && f == "%s" ) {
      if ( MilliSeconds[index] > 0 )
	f = "%02H:%02M:%02S.%03Z";
      else
	f = "%02H:%02M:%02S";
    }
    f.format( 0, 0, 0, Hour[index], Minutes[index], Seconds[index], MilliSeconds[index] );
  }

  if ( u == "1" )
    u == "";
  u.replace( "%", "%%" );
  f.format( u, 'u' );

  if ( isText() ) {
    f.format( s.dir(), 'p' );
    f.format( s.notdir(), 'd' );
    f.format( s.name(), 'n' );
    f.format( s.extension(), 'x' );
  }
  f.format( s, 's' );

  f.replace( "%%", "%" );

  return f;
}


Str Parameter::text( const string &format, const string &unit ) const
{
  return text( 0, format, unit );
}


void Parameter::texts( vector<string> &s, const string &format, const string &unit ) const
{
  s.reserve( size() );
  s.clear();
  for ( int k=0; k<size(); k++ )
    s.push_back( text( k, format, unit ) );
}


Str Parameter::allText( const string &format, const string &unit, const string &separator ) const
{
  string s = "";
  if ( size() > 0 )
    s = text( 0, format, unit );
  for ( int k=1; k<size(); k++ )
    s += separator + text( k, format, unit );
  return s;
}


Parameter &Parameter::setText( const string &strg )
{
  return addText( strg, true );
}


Parameter &Parameter::setText( const Parameter &p )
{
  Warning = "";
  if ( ! isText() || ! p.isText() ) {
    Warning = "Parameter::setText() -> '" + Name + "' or '" + p.name()
      + "' is not of type text!";
    return *this;
  }

  if ( String.size() != p.String.size() ||
       ( String.size() > 0 && p.String.size() > 0 && String[0] != p.String[0] ) )
    Flags |= ChangedFlag;

  String = p.String;

  return *this;
}


Parameter &Parameter::addText( const string &strg, bool clear )
{
  Warning = "";

  // split strg:
  StrQueue sq;
  if ( strg.find( '|' ) != string::npos )
    sq.assign( strg, "|", "\"" );  // compatibility with old styles
  else {
    if ( ! strg.empty() && strg[0] == '[' ) {
      Str s = strg;
      s.preventFirst( '[' );
      s.preventLast( ']' );
      sq.assign( s, ",", "\"" );
    }
    else
      sq.assign( strg, "" );
  }
  if ( sq.empty() )
    sq.add( "" );
  // remove leading and trailing spaces, and make '~' an empty string:
  bool stringtype = false;
  for ( int k=0; k<sq.size(); k++ ) {
    sq[k].strip( ' ' );
    if ( isText() && ! unit().empty() && sq[k].size() > unit().size() &&
	 string( sq[k], sq[k].size()-unit().size(), unit().size() ).compare( unit() ) == 0 )
      sq[k].resize( sq[k].size()-unit().size() );
    bool quotes = ( sq[k][0] == '"' && sq[k][sq[k].size()-1] == '"' );
    if ( quotes )
      stringtype = true;
    if ( ! sq[k].empty() && quotes ) {
      sq[k].erase( 0, 1 );
      sq[k].erase( sq[k].size() - 1 );
    }
    if ( sq[k] == "~" )
      sq[k].clear();
  }
  if ( isNotype() && stringtype )
    setValueType( Text );

  // clear:
  if ( clear ) {
    // changed:
    if ( String.empty() || String[0] != sq[0] )
      Flags |= ChangedFlag;
    String.clear();
    Value.clear();
    Error.clear();
    Year.clear();
    Month.clear();
    Day.clear();
    Hour.clear();
    Minutes.clear();
    Seconds.clear();
    MilliSeconds.clear();
  }

  // add sq:
  String.add( sq );

  if ( isDate() ) {
    for ( int k=0; k<sq.size(); k++ ) {
      int year, month, day;
      int d = String[ String.size() - sq.size() + k ].date( year, month, day );
      if ( d != 0 ) {
	if ( ! String[ String.size() - sq.size() + k ].empty() )
	  Warning += "string '" + String[ String.size() - sq.size() + k ]
	    + "' is not a valid date!";
      }
      else
	addDate( year, month, day );
    }
  }
  else if ( isTime() ) {
    for ( int k=0; k<sq.size(); k++ ) {
      int hour, minutes, seconds, milliseconds;
      int d = String[ String.size() - sq.size() + k ].time( hour, minutes,
							    seconds, milliseconds );
      if ( d != 0 ) {
	if ( ! String[ String.size() - sq.size() + k ].empty() )
	  Warning += "string '" + String[ String.size() - sq.size() + k ]
	    + "' is not a valid time!";
      }
      else
	addTime( hour, minutes, seconds, milliseconds );
    }
  }
  else {
    // get numbers:
    for ( int k=0; k<sq.size(); k++ )
      addNumber( String[ String.size() - sq.size() + k ], "" );
  }
  return *this;
}


Str Parameter::defaultText( int index, const string &format,
			    const string &unit ) const
{
  Warning = "";

  if ( index < 0 ) {
    Warning = "Parameter::defaultText -> negative index for parameter '" +
      Name + "' requested!";
    return "";
  }

  if ( isText() && index >= DefaultString.size() ) {
    Warning = "Parameter::defaultText -> requested parameter '" +
      Name + "' has only " + Str( DefaultString.size() ) + " values !";
    return "";
  }

  if ( isAnyNumber() && index >= (int)Value.size() ) {
    Warning = "Parameter::defaultText -> requested parameter '" +
      Name + "' has only " + Str( Value.size() ) + " values !";
    return "";
  }

  if ( isDate() && index >= (int)DefaultYear.size() ) {
    Warning = "Parameter::defaultText -> requested parameter '" +
      Name + "' has only " + Str( Year.size() ) + " date values !";
    return "";
  }

  if ( isTime() && index >= (int)DefaultHour.size() ) {
    Warning = "Parameter::defaultText -> requested parameter '" +
      Name + "' has only " + Str( Hour.size() ) + " time values !";
    return "";
  }

  Str f( format );
  if ( f.empty() )
    f = Format;

  f.format( Name, 'i' );
  f.format( Request, 'r' );

  string typestr = "notype";
  if ( isText() )
    typestr = "string";
  else if ( isNumber() )
    typestr = "number";
  else if ( isInteger() )
    typestr = "integer";
  else if ( isBoolean() )
    typestr = "boolean";
  else if ( isDate() )
    typestr = "date";
  else if ( isTime() )
    typestr = "time";
  f.format( typestr, 'T' );

  Str s( "" );
  if ( index < DefaultString.size() )
    s = DefaultString[index];

  string u( unit );
  if ( u.empty() )
    u = OutUnit;

  if ( isAnyNumber() ) {
    double uv = changeUnit( 1.0, InternUnit, u );

    double v = DefaultValue[index];
    double e = 0.0;
    v *= uv;
    e *= uv;

    f.format( v, "fge" );
    f.format( e, "FGE" );

    string b( v != 0 ? "true" : "false" );
    f.format( b, 'b' );
  }
  else if ( isDate() ) {
    if ( s.empty() && f == "%s" )
      f = "%04Y-%02m-%02d";
    f.format( DefaultYear[index], DefaultMonth[index], DefaultDay[index] );
  }
  else if ( isTime() ) {
    if ( s.empty() && f == "%s" ) {
      if ( DefaultMilliSeconds[index] > 0 )
	f = "%02H:%02M:%02S.%03Z";
      else
	f = "%02H:%02M:%02S";
    }
    f.format( 0, 0, 0, DefaultHour[index], DefaultMinutes[index],
	      DefaultSeconds[index], DefaultMilliSeconds[index] );
  }

  if ( u == "1" )
    u == "";
  int up = f.format( u, 'u' );
  if ( up > 0 && u.find( '%' ) != string::npos ) {
    // unit string was replaced and contains a '%': no more formatting!
    return f;
  }

  if ( isText() ) {
    f.format( s.dir(), 'p' );
    f.format( s.notdir(), 'd' );
    f.format( s.name(), 'n' );
    f.format( s.extension(), 'x' );
  }
  f.format( s, 's' );

  return f;
}


Str Parameter::defaultText( const string &format, const string &unit ) const
{
  return defaultText( 0, format, unit );
}


Parameter &Parameter::setDefaultText( const string &strg )
{
  Warning = "";

  DefaultString.clear();
  DefaultValue.clear();
  DefaultYear.clear();
  DefaultMonth.clear();
  DefaultDay.clear();
  DefaultHour.clear();
  DefaultMinutes.clear();
  DefaultSeconds.clear();
  DefaultMilliSeconds.clear();

  return addDefaultText( strg );
}


Parameter &Parameter::addDefaultText( const string &strg )
{
  Warning = "";

  // split strg:
  StrQueue sq;
  if ( strg.find( '|' ) != string::npos )
    sq.assign( strg, "|", "\"" );  // compatibility with old styles
  else {
    if ( ! strg.empty() && strg[0] == '[' ) {
      Str s = strg;
      s.preventFirst( '[' );
      s.preventLast( ']' );
      sq.assign( s, ",", "\"" );
    }
    else
      sq.assign( strg, "" );
  }
  if ( sq.empty() )
    sq.add( "" );
  // remove leading and trailing spaces, and make '~' an empty string:
  for ( int k=0; k<sq.size(); k++ ) {
    sq[k].strip( ' ' );
    bool quotes = ( sq[k][0] == '"' && sq[k][sq[k].size()-1] == '"' );
    if ( ! sq[k].empty() && quotes ) {
      sq[k].erase( 0, 1 );
      sq[k].erase( sq[k].size() - 1 );
    }
    if ( sq[k] == "~" )
      sq[k].clear();
  }

  // add sq:
  DefaultString.add( sq );

  if ( isDate() ) {
    for ( int k=0; k<sq.size(); k++ ) {
      int year, month, day;
      int d = DefaultString[ DefaultString.size() - sq.size() + k ].date( year, month, day );
      if ( d != 0 ) {
	if ( DefaultString[ DefaultString.size() - sq.size() + k ].empty() )
	  Warning += "string '" + DefaultString[ DefaultString.size() - sq.size() + k ]
	    + "' is not a valid date!";
      }
      else 
	addDefaultDate( year, month, day );
    }
  }
  else if ( isTime() ) {
    for ( int k=0; k<sq.size(); k++ ) {
      int hour, minutes, seconds;
      int d = DefaultString[ DefaultString.size() - sq.size() + k ].time( hour, minutes, seconds );
      if ( d != 0 ) {
	if ( DefaultString[ DefaultString.size() - sq.size() + k ].empty() )
	  Warning += "string '" + DefaultString[ DefaultString.size() - sq.size() + k ]
	    + "' is not a valid time!";
      }
      else 
	addDefaultTime( hour, minutes, seconds );
    }
  }
  else {
    // get numbers:
    for ( int k=0; k<sq.size(); k++ )
      addDefaultNumber( DefaultString[ DefaultString.size() - sq.size() + k ], "" );
  }

  return *this;
}


Parameter &Parameter::selectText( const string &strg, int add )
{
  if ( ! isText() ) {
    Warning = "Parameter::selectText -> parameter '" + 
      Name + "' is not of type text!";
    return *this;
  }

  // split strg:
  StrQueue sq;
  if ( strg.find( '|' ) != string::npos )
    sq.assign( strg, "|", "\"" );  // compatibility with old styles
  else {
    if ( ! strg.empty() && strg[0] == '[' ) {
      Str s = strg;
      s.preventFirst( '[' );
      s.preventLast( ']' );
      sq.assign( s, ",", "\"" );
    }
    else
      sq.assign( strg, "" );
  }
  if ( sq.empty() )
    sq.add( "" );
  // remove leading and trailing spaces, and make '~' an empty string:
  for ( int k=0; k<sq.size(); k++ ) {
    sq[k].strip( ' ' );
    bool quotes = ( sq[k][0] == '"' && sq[k][sq[k].size()-1] == '"' );
    if ( ! sq[k].empty() && quotes ) {
      sq[k].erase( 0, 1 );
      sq[k].erase( sq[k].size() - 1 );
    }
    if ( sq[k] == "~" )
      sq[k].clear();
  }

  int inx = String.find( sq[0] );
  if ( inx < 0 ) {
    if ( add > 0 ||
	 ( add == 0 && (Style & SelectText) == 0 ) ) {
      // strg not found, add it:
      String.add( sq[0] );
      if ( String.size() > 1 )
	String.insert( sq[0] );
      Flags |= ChangedFlag;
    }
  }
  else {
    // strg found, make sure it is doubled at the front:
    if ( String.find( String[0], 1 ) > 0 ) {
      if ( inx > 0 ) {
	String[0] = String[inx];
	Flags |= ChangedFlag;
      }
    }
    else if ( inx > 0 && String.size() > 1 ) {
      String.insert( String[inx] );
      Flags |= ChangedFlag;
    }
  }

  // update numbers:
  if ( isDate() ) {
    Year.clear();
    Month.clear();
    Day.clear();
    for ( int k=0; k<String.size(); k++ ) {
      int year, month, day;
      int d = String[k].date( year, month, day );
      if ( d != 0 )
	Warning += "string '" + String[k] + "' is not a valid date!";
      else 
	addDate( year, month, day );
    }
  }
  else if ( isTime() ) {
    Hour.clear();
    Minutes.clear();
    Seconds.clear();
    MilliSeconds.clear();
    for ( int k=0; k<String.size(); k++ ) {
      int hour, minutes, seconds, milliseconds;
      int d = String[k].time( hour, minutes, seconds, milliseconds );
      if ( d != 0 )
	Warning += "string '" + String[k] + "' is not a valid time!";
      else 
	addTime( hour, minutes, seconds, milliseconds );
    }
  }
  else {
    Value.clear();
    Error.clear();
    for ( int k=0; k<String.size(); k++ )
      addNumber( String[k], "" );
  }
  
  return *this;
}


Parameter &Parameter::selectText( int index )
{
  if ( ! isText() ) {
    Warning = "Parameter::selectText -> parameter '" + 
      Name + "' is not of type text!";
    return *this;
  }

  if ( index < 0 )
    return *this;

  if ( String.find( String[0], 1 ) > 0 ) {
    if ( index+1 < String.size() ) {
      String[0] = String[index+1];
      Flags |= ChangedFlag;
    }
  }
  else {
    if ( index < String.size() ) {
      String.insert( String[index] );
      Flags |= ChangedFlag;
    }
  }
  
  // update numbers:
  if ( isDate() ) {
    Year.clear();
    Month.clear();
    Day.clear();
    for ( int k=0; k<String.size(); k++ ) {
      int year, month, day;
      int d = String[k].date( year, month, day );
      if ( d != 0 )
	Warning += "string '" + String[k] + "' is not a valid date!";
      else 
	addDate( year, month, day );
    }
  }
  else if ( isTime() ) {
    Hour.clear();
    Minutes.clear();
    Seconds.clear();
    MilliSeconds.clear();
    for ( int k=0; k<String.size(); k++ ) {
      int hour, minutes, seconds, milliseconds;
      int d = String[k].time( hour, minutes, seconds, milliseconds );
      if ( d != 0 )
	Warning += "string '" + String[k] + "' is not a valid time!";
      else 
	addTime( hour, minutes, seconds, milliseconds );
    }
  }
  else {
    Value.clear();
    Error.clear();
    for ( int k=0; k<String.size(); k++ )
      addNumber( String[k], "" );
  }
  
  return *this;
}


int Parameter::index( void ) const
{
  if ( ! isText() ) {
    Warning = "Parameter::selectText -> parameter '" + 
      Name + "' is not of type text!";
    return 0;
  }
  int inx = String.find( String[0], 1 );
  return inx < 0 ? 0 : inx-1;
}


int Parameter::index( const string &strg ) const
{
  if ( ! isText() ) {
    Warning = "Parameter::selectText -> parameter '" + 
      Name + "' is not of type text!";
    return 0;
  }

  int inx = String.find( strg );

  // not found:
  if ( inx < 0 )
    return -1;

  if ( inx == 0 ) {
    inx = String.find( String[0], 1 );
    return inx < 0 ? 0 : inx-1;
  }

  return inx;
}


const int NUnits = 50;
string UnitPref[NUnits] = { "Deka", "deka", "Hekto", "hekto", "kilo", "Kilo", 
			    "Mega", "mega", "Giga", "giga", "Tera", "tera", 
			    "Peta", "peta", "Exa", "exa", 
			    "Dezi", "dezi", "Zenti", "centi", "Micro", "micro", 
			    "Milli", "milli", "Nano", "nano", "Piko", "piko", 
			    "Femto", "femto", "Atto", "atto", 
			    "da", "h", "K", "k", "M", "G", "T", "P", "E", 
			    "d", "c", "mu", "u", "m", "n", "p", "f", "a" };
double UnitFac[NUnits] = {  1.0,  1.0,  2.0,  2.0,  3.0,  3.0,  
			    6.0,  6.0,  9.0,  9.0,  12.0, 12.0, 
			   15.0, 15.0, 18.0, 18.0,
			   -1.0, -1.0, -2.0, -2.0, -6.0, -6.0, 
			   -3.0, -3.0, -9.0, -9.0, -12.0, -12.0, 
			  -15.0, -15.0, -18.0, -18.0, 
			    1.0,  2.0,  3.0,  3.0,  6.0,  9.0, 12.0, 15.0, 18.0,
			   -1.0, -2.0, -6.0, -6.0, -3.0, -9.0, -12.0, -15.0, -18.0 };

double Parameter::changeUnit( double val, const Str &oldunit,
			      const Str &newunit )
{
  // disect oldUnit into value ov and unit ou:
  double ov = oldunit.number( 1.0 );
  string ou = oldunit.unit();

  // disect newUnit into value nv and unit nu:
  double nv = newunit.number( 1.0 );
  string nu = newunit.unit();

  // missing unit?
  if ( nu.empty() || ou.empty() ) {
    if ( newunit == "1" && oldunit == "%" )
      nv = 100.0;
    else if ( newunit == "%" && oldunit == "1" ) 
      nv = 0.01;
    return val*ov/nv;
  }

  // find out order of old unit:
  int k = 0;
  for ( k=0; k<NUnits; k++ )
    if ( ou.find( UnitPref[k] ) == 0 )
      break;
  double e1 = 0.0;
  if ( k < NUnits && UnitPref[k].length() < ou.length() )
    e1 = UnitFac[k];

  // find out order of new unit:
  for ( k=0; k<NUnits; k++ )
    if ( nu.find( UnitPref[k] ) == 0 )
      break;
  double e2 = 0.0;
  if ( k < NUnits && UnitPref[k].length() < nu.length() )
    e2 = UnitFac[k];

  return val * (ov/nv) * pow( 10.0, e1-e2 );
}


bool Parameter::isAnyNumber( void ) const
{
  return ( VType == Number || VType == Integer || VType == Boolean );
}


bool Parameter::isNumber( void ) const
{
  return ( VType == Number );
}


double Parameter::number( const string &unit, int index, double dflt ) const
{ 
  Warning = "";
  if ( ! isAnyNumber() && ! isText() ) {
    Warning = "Parameter::number -> parameter '" + 
      Name + "' is not of type number!";
    return dflt;
  }
  if ( index < 0 || index >= (int)Value.size() ) {
    Warning = "Parameter::number -> invalid index " +
      Str( index ) + " requested for parameter '" + Name + "' !";
    return dflt;
  }

  return changeUnit( Value[index], InternUnit, unit );
}


void Parameter::numbers( vector<double> &n, const string &unit ) const
{
  n.reserve( size() );
  n.clear();

  Warning = "";
  if ( ! isAnyNumber() && ! isText() ) {
    Warning = "Parameter::numbers -> parameter '" + 
      Name + "' is not of type number!";
    return;
  }

  for ( int k=0; k<size(); k++ )
    n.push_back( changeUnit( Value[k], InternUnit, unit ) );
}


double Parameter::error( const string &unit, int index ) const
{ 
  Warning = "";
  if ( ! isAnyNumber() && ! isText() ) {
    Warning = "Parameter::error -> parameter '" + 
      Name + "' is not of type number!";
    return 0.0;
  }
  if ( index < 0 || index >= (int)Error.size() ) {
    Warning = "Parameter::error -> invalid index " +
      Str( index ) + " requested for parameter '" + Name + "' !";
    return 0.0;
  }

  if ( Error[index] >= 0.0 ) {
    return changeUnit( Error[index], InternUnit, unit );
  }
  return Error[index];
}

void Parameter::errors( vector<double> &n, const string &unit ) const
{
  n.reserve( size() );
  n.clear();

  Warning = "";
  if ( ! isAnyNumber() && ! isText() ) {
    Warning = "Parameter::errors -> parameter '" + 
      Name + "' is not of type number!";
    return;
  }

  for ( int k=0; k<size(); k++ ) {
    if ( Error[k] >= 0.0 )
      n.push_back( changeUnit( Error[k], InternUnit, unit ) );
    else
      n.push_back( Error[k] );
  }
}


Parameter &Parameter::setNumber( double number, double error, 
				 const string &unit )
{
  Warning = "";
  if ( ! isAnyNumber() && ! isText() ) {
    Warning = "Parameter::setNumber -> parameter '" + 
      Name + "' is not of type number!";
    return *this;
  }
  return addNumber( number, error, unit, true );
}


Parameter &Parameter::setNumbers( const vector<double> &numbers,
				  const vector<double> &errors,
				  const string &unit )
{
  Warning = "";
  if ( ! isAnyNumber() && ! isText() ) {
    Warning = "Parameter::setNumbers -> parameter '" + 
      Name + "' is not of type number!";
    return *this;
  }
  if ( numbers.size() == 0 ) {
    Value.resize( 1 );
    Value[ 0 ] = 0.0;
    Error.resize( 1 );
    Error[ 0 ] = -1.0;
    String.resize( 1, "" );
    Warning = "no numbers";
  }
  else {
    string e;
    setNumber( numbers[0], errors[0], unit );
    e += Warning;
    for ( int k=1; k<(int)numbers.size(); k++ ) {
      addNumber( numbers[k], errors[k], unit );
      e += Warning;
    }
    Warning = e;
  }
  return *this;
}


Parameter &Parameter::setNumber( const Parameter &p )
{
  Warning = "";
  if ( ! isNumber() || ! p.isNumber() ) {
    Warning = "Parameter::setNumber() -> '" + Name + "' or '" + p.name()
      + "' is not of type number!";
    return *this;
  }

  if ( Value.size() != p.Value.size() ||
       ( Value.size() > 0 && p.Value.size() > 0 && Value[0] != p.Value[0] ) )
    Flags |= ChangedFlag;

  Value = p.Value;
  Error = p.Error;
  InternUnit = p.InternUnit;
  String = p.String;
  
  return *this;
}


Parameter &Parameter::addNumber( double number, double error,
				 const string &unit, bool clear )
{
  Warning = "";
  if ( ! isAnyNumber() && ! isText() ) {
    Warning = "Parameter::addNumber -> parameter '" + 
      Name + "' is not of type number!";
    return *this;
  }

  // unit:
  if ( InternUnit.empty() && OutUnit.empty() ) {
    if ( number != MAXDOUBLE && ! isBoolean() && ! isText() )
      setUnit( unit );
  }
  else if ( number != MAXDOUBLE ) {
    double u = changeUnit( 1.0, unit, InternUnit );
    number *= u;
    if ( error >= 0.0 )
      error *= u;
  }

  // check range:
  if ( number < Minimum - 1.0e-8 ) {
    Warning += "number=" + Str( number ) + " < Minimum=" + Str( Minimum ) + ", ";
    number = Minimum;
  }
  if ( number != MAXDOUBLE && number > Maximum + 1.0e-8 ) {
    Warning += "number=" + Str( number ) + " > Maximum=" + Str( Maximum ) +", ";
    number = Maximum;
  }
  if ( error > Maximum + 1.0e-8 ) {
    Warning += "error=" + Str( error ) + " > Maximum=" + Str( Maximum ) +", ";
    error = Maximum;
  }

  // clear:
  if ( clear ) {
    // change:
    if ( Value[0] != number )
      Flags |= ChangedFlag;
    Value.clear();
    Error.clear();
    String.clear();
  }

  // add number and error:
  Value.push_back( number );
  Error.push_back( error );

  return *this;
}


Parameter &Parameter::addNumber( const Str &s, const string &unit )
{
  if ( ! isAnyNumber() && ! isText() ) {
    Warning = "Parameter::addNumber -> parameter '" + 
      Name + "' is not of type number!";
    return *this;
  }
  double e = -1.0;
  string u = unit;
  double v = s.number( e, u, MAXDOUBLE );
  if ( u.empty() )
    u = OutUnit;
  if ( v == MAXDOUBLE ) {
    if ( s == "true" )
      v = 1.0;
    else if ( s == "false" )
      v = 0.0;
    else {
      // this is not a number:
      Value.push_back( MAXDOUBLE );
      Error.push_back( MAXDOUBLE );
      return *this;
    }
  }
  return addNumber( v, e, u );
}


#ifdef HAVE_LIBRELACSSHAPES
Point Parameter::point( const string &unit ) const
{
  Warning = "";
  if ( ! isAnyNumber() && ! isText() ) {
    Warning = "Parameter::point -> parameter '" + 
      Name + "' is not of type number!";
    return Point::None;
  }
  if ( Value.size() != 3 ) {
    Warning = "Parameter::point -> 3 coordinates not available in parameter '" + Name + "' !";
    return Point::None;
  }
  Point p;
  for ( int k=0; k<3; k++ )
    p[k] = changeUnit( Value[k], InternUnit, unit );
  return p;
}


Point Parameter::defaultPoint( const string &unit ) const
{
  Warning = "";
  if ( ! isAnyNumber() && ! isText() ) {
    Warning = "Parameter::defaultPoint -> parameter '" + 
      Name + "' is not of type number!";
    return Point::None;
  }
  if ( DefaultValue.size() != 3 ) {
    Warning = "Parameter::defaultPoint -> 3 default coordinates not available in parameter '" + Name + "' !";
    return Point::None;
  }
  Point p;
  for ( int k=0; k<3; k++ )
    p[k] = changeUnit( DefaultValue[k], InternUnit, unit );
  return p;
}


Parameter &Parameter::setPoint( const Point &p,
				const string &unit )
{
  Warning = "";
  if ( ! isAnyNumber() && ! isText() ) {
    Warning = "Parameter::setPoint -> parameter '" + 
      Name + "' is not of type number!";
    return *this;
  }
  string e;
  setNumber( p[0], -1.0, unit );
  e += Warning;
  for ( int k=1; k<3; k++ ) {
    addNumber( p[k], -1.0, unit );
    e += Warning;
  }
  Warning = e;
  return *this;
}


Parameter &Parameter::setDefaultPoint( const Point &p, const string &unit )
{
  Warning = "";
  if ( ! isAnyNumber() && ! isText() ) {
    Warning = "Parameter::setDefaultNumber -> parameter '" + 
      Name + "' is not of type number!";
    return *this;
  }
  DefaultValue.clear();
  DefaultString.clear();
  for ( int k=0; k<3; k++ )
    addDefaultNumber( p[k], unit );
  return *this;
}
#endif


bool Parameter::isInteger( void ) const
{
  return ( VType == Integer );
}


long Parameter::integer( const string &unit, int index, long dflt ) const
{ 
  if ( ! isAnyNumber() && ! isText() ) {
    Warning = "Parameter::integer -> parameter '" + 
      Name + "' is not of type number!";
    return dflt;
  }
  return static_cast<long>( rint( number( unit, index, double( dflt ) ) ) );
}


void Parameter::integers( vector<long> &n, const string &unit ) const
{
  n.reserve( size() );
  n.clear();

  Warning = "";
  if ( ! isAnyNumber() && ! isText() ) {
    Warning = "Parameter::integers -> parameter '" + 
      Name + "' is not of type number!";
    return;
  }

  for ( int k=0; k<size(); k++ )
    n.push_back( static_cast<long>( rint( changeUnit( Value[k], InternUnit, unit ) ) ) );
}


void Parameter::integers( vector<int> &n, const string &unit ) const
{
  n.reserve( size() );
  n.clear();

  Warning = "";
  if ( ! isAnyNumber() && ! isText() ) {
    Warning = "Parameter::integers -> parameter '" + 
      Name + "' is not of type number!";
    return;
  }

  for ( int k=0; k<size(); k++ )
    n.push_back( static_cast<int>( rint( changeUnit( Value[k], InternUnit, unit ) ) ) );
}


Parameter &Parameter::setInteger( long number, long error, 
				  const string &unit )
{
  if ( ! isAnyNumber() && ! isText() ) {
    Warning = "Parameter::setInteger -> parameter '" + 
      Name + "' is not of type number!";
    return *this;
  }
  return setNumber( static_cast<double>( number ),
		    static_cast<double>( error ), unit );
}


Parameter &Parameter::setInteger( const Parameter &p )
{
  Warning = "";
  if ( ! isInteger() || ! p.isInteger() ) {
    Warning = "Parameter::setInteger() -> '" + Name + "' or '" + p.name()
      + "' is not of type integer!";
    return *this;
  }

  if ( Value.size() != p.Value.size() ||
       ( Value.size() > 0 && p.Value.size() > 0 && Value[0] != p.Value[0] ) )
    Flags |= ChangedFlag;

  Value = p.Value;
  Error = p.Error;
  InternUnit = p.InternUnit;
  String = p.String;
  
  return *this;
}


Parameter &Parameter::addInteger( long number, long error, const string &unit )
{
  if ( ! isAnyNumber() && ! isText() ) {
    Warning = "Parameter::addInteger -> parameter '" + 
      Name + "' is not of type number!";
    return *this;
  }
  return addNumber( static_cast<double>( number ),
		    static_cast<double>( error ), unit );
}


double Parameter::defaultNumber( const string &unit, int index ) const
{ 
  Warning = "";
  if ( ! isAnyNumber() && ! isText() ) {
    Warning = "Parameter::defaultNumber -> parameter '" + 
      Name + "' is not of type number!";
    return 0.0;
  }
  if ( index < 0 || index >= (int)DefaultValue.size() ) {
    Warning = "Parameter::defaultNumber -> invalid index " +
      Str( index ) + " requested for parameter '" + Name + "' !";
    return 0.0;
  }

  return changeUnit( DefaultValue[index], InternUnit, unit );
}


Parameter &Parameter::setDefaultNumber( double dflt, const string &unit )
{
  Warning = "";
  if ( ! isAnyNumber() && ! isText() ) {
    Warning = "Parameter::setDefaultNumber -> parameter '" + 
      Name + "' is not of type number!";
    return *this;
  }
  DefaultValue.clear();
  DefaultString.clear();
  return addDefaultNumber( dflt, unit );
}


Parameter &Parameter::addDefaultNumber( double number, const string &unit )
{
  Warning = "";
  if ( ! isAnyNumber() && ! isText() ) {
    Warning = "Parameter::addDefaultNumber -> parameter '" + 
      Name + "' is not of type number!";
    return *this;
  }

  // unit:
  if ( InternUnit.empty() && OutUnit.empty() ) {
    if ( number != MAXDOUBLE && ! isBoolean() )
      setUnit( unit );
  }
  else if ( number != MAXDOUBLE ) {
    double u = changeUnit( 1.0, unit, InternUnit );
    number *= u;
  }
  
  // check range:
  if ( number < Minimum - 1.0e-8 ) {
    Warning += "number=" + Str( number ) + " < Minimum=" + Str( Minimum ) + ", ";
    number = Minimum;
  }
  if ( number != MAXDOUBLE && number > Maximum + 1.0e-8 ) {
    Warning += "number=" + Str( number ) + " > Maximum=" + Str( Maximum ) +", ";
    number = Maximum;
  }
      
  // add number and error:
  DefaultValue.push_back( number );

  return *this;
}


Parameter &Parameter::addDefaultNumber( const Str &s, const string &unit )
{
  if ( ! isAnyNumber() && ! isText() ) {
    Warning = "Parameter::addDefaultNumber -> parameter '" + 
      Name + "' is not of type number!";
    return *this;
  }
  double e = -1.0;
  string u=unit;
  double v = s.number( e, u, MAXDOUBLE, 0, 0, Str::DoubleWhiteSpace );
  if ( u.empty() )
    u = OutUnit;
  if ( v == MAXDOUBLE ) {
    if ( s == "true" )
      v = 1.0;
    else if ( s == "false" )
      v = 0.0;
  }
  return addDefaultNumber( v, u );
}


long Parameter::defaultInteger( const string &unit, int index ) const
{ 
  if ( ! isAnyNumber() && ! isText() ) {
    Warning = "Parameter::defaultInteger -> parameter '" + 
      Name + "' is not of type number!";
    return 0.0;
  }
  return static_cast<long>( rint( defaultNumber( unit, index ) ) );
}

  
Parameter &Parameter::setDefaultInteger( long dflt, const string &unit )
{
  if ( ! isAnyNumber() && ! isText() ) {
    Warning = "Parameter::setDefaultInteger -> parameter '" + 
      Name + "' is not of type number!";
    return *this;
  }
  return setDefaultNumber( static_cast<double>( dflt ), unit );
}


Parameter &Parameter::addDefaultInteger( double number, const string &unit )
{
  if ( ! isAnyNumber() && ! isText() ) {
    Warning = "Parameter::addDefaultInteger -> parameter '" + 
      Name + "' is not of type number!";
    return *this;
  }
  return addDefaultNumber( static_cast<double>( number ), unit );
}


double Parameter::minimum( const string &unit ) const
{ 
  if ( ! isAnyNumber() ) {
    Warning = "Parameter::minimum -> parameter '" + 
      Name + "' is not of type number!";
    return 0.0;
  }
  Warning = "";
  return changeUnit( Minimum, InternUnit, unit );
}


double Parameter::maximum( const string &unit ) const
{ 
  if ( ! isAnyNumber() ) {
    Warning = "Parameter::maximum -> parameter '" + 
      Name + "' is not of type number!";
    return 0.0;
  }
  Warning = "";
  return changeUnit( Maximum, InternUnit, unit );
}


double Parameter::step( const string &unit ) const
{ 
  if ( ! isAnyNumber() ) {
    Warning = "Parameter::step -> parameter '" + 
      Name + "' is not of type number!";
    return 0.0;
  }
  Warning = "";
  return changeUnit( Step, InternUnit, unit );
}


Parameter &Parameter::setStep( double step, const string &unit )
{
  if ( ! isAnyNumber() ) {
    Warning = "Parameter::setStep -> parameter '" + 
      Name + "' is not of type number!";
    return *this;
  }

  double u = changeUnit( 1.0, unit, InternUnit );
  if ( step != 0.0 )
    step *= u;

  Warning = "";
  if ( step < 0.0 ) {
    if ( Minimum > -MAXDOUBLE &&
	 Maximum < MAXDOUBLE )
      Step = - ( Maximum - Minimum ) / step;
    else {
      Warning += "step=" + Str( step ) + " < 0, ";
      Step = 1.0;
    }
  }
  else if ( step == 0.0 ) {
    // auto set stepsize:
    if ( Minimum > -MAXDOUBLE &&
	 Maximum < MAXDOUBLE )
      Step = ( Maximum - Minimum ) / 50.0;
    else {
      Warning += "step=" + Str( step ) + " == 0, ";
      Step = 1.0;
    }
  }
  else
    Step = step;

  return *this;
}


Parameter &Parameter::setStep( long step )
{
  if ( ! isAnyNumber() ) {
    Warning = "Parameter::setStep -> parameter '" + 
      Name + "' is not of type number!";
    return *this;
  }
  return setStep( static_cast<double>( step ) );
}

  
Parameter &Parameter::setMinMax( double minimum, double maximum,
				 double step, const string &unit )
{
  if ( ! isAnyNumber() ) {
    Warning = "Parameter::setMinMax -> parameter '" + 
      Name + "' is not of type number!";
    return *this;
  }
  Warning = "";

  double u = changeUnit( 1.0, unit, InternUnit );
  if ( minimum > -MAXDOUBLE )
    minimum *= u;
  if ( maximum < MAXDOUBLE )
    maximum *= u;
  if ( step != 0.0 )
    step *= u;

  // check range:
  if ( minimum > maximum ) {
    Warning += "minimum=" + Str( minimum ) + " > maximum=" + Str( maximum ) + ", ";
    double swap;
    swap = maximum;
    maximum = minimum;
    minimum = swap;
  }
      
  // set minimum and maximum:
  Minimum = minimum;
  Maximum = maximum;

  // set step:
  return setStep( step );
}

  
Parameter &Parameter::setMinMax( long minimum, long maximum, long step,
				 const string &unit )
{
  if ( ! isAnyNumber() ) {
    Warning = "Parameter::setMinMax -> parameter '" + 
      Name + "' is not of type number!";
    return *this;
  }
  double min = minimum == LONG_MIN ? -MAXDOUBLE : static_cast<double>( minimum );
  double max = maximum == LONG_MAX ? MAXDOUBLE : static_cast<double>( maximum );
  double st = static_cast<double>( step );
  return setMinMax( min, max, st, unit );
}


double Parameter::floorLog10( double v )
{
  return pow( 10.0, floor( log10( v ) ) );
}


double Parameter::floor10( double v, double scale )
{
  double f = floorLog10( scale * v );
  //  v -= 0.1 * scale * v; ????
  return floor( v/f ) * f;
}


double Parameter::ceil10( double v, double scale )
{
  double f = floorLog10( scale * v );
  //  v -= 0.1 * scale * v; ????
  return ceil( v/f ) * f;
}


Str Parameter::unit( void ) const
{ 
  Warning = "";
  return InternUnit; 
}


Str Parameter::outUnit( void ) const
{ 
  Warning = "";
  return OutUnit; 
}


Parameter &Parameter::setUnit( const string &internunit, 
			       const string &outputunit )
{
  Warning = "";
  // check units ...
  if ( outputunit == "%" && internunit == "" )
    InternUnit = "1";
  else
    InternUnit = internunit;
  if ( outputunit.empty() )
    OutUnit = InternUnit;
  else
    OutUnit = outputunit;
  return *this;
}


Parameter &Parameter::setOutUnit( const string &outputunit )
{
  Warning = "";
  if ( outputunit.empty() )
    OutUnit = InternUnit;
  else
    OutUnit = outputunit;
  return *this;
}


Parameter &Parameter::changeUnit( string internunit )
{
  Warning = "";
  // check units ...
  if ( OutUnit == "%" && internunit == "" )
    internunit = "1";
  // convert values:
  double fac = changeUnit( 1.0, InternUnit, internunit );
  for ( unsigned int k=0; k<Value.size(); k++ )
    Value[0] *= fac;
  for ( unsigned int k=0; k<DefaultValue.size(); k++ )
    DefaultValue[0] *= fac;
  // set new unit:
  InternUnit = internunit;
  return *this;
}


bool Parameter::isBoolean( void ) const
{
  return ( VType == Boolean );
}


bool Parameter::boolean( int index ) const
{ 
  Warning = "";
  if ( ! isAnyNumber() && ! isText() ) {
    Warning = "Parameter::boolean -> parameter '" + 
      Name + "' is not of type number!";
    return false;
  }
  if ( index < 0 || index >= (int)Value.size() ) {
    Warning = "Parameter::boolean -> invalid index " +
      Str( index ) + " requested for parameter '" + Name + "' !";
    return false;
  }

  return ( Value[index] != 0.0 ); 
}


void Parameter::booleans( vector<bool> &n ) const
{
  n.reserve( size() );
  n.clear();

  Warning = "";
  if ( ! isAnyNumber() && ! isText() ) {
    Warning = "Parameter::booleans -> parameter '" + 
      Name + "' is not of type number!";
    return;
  }

  for ( int k=0; k<size(); k++ )
    n.push_back( Value[k] != 0.0 );
}


Parameter &Parameter::setBoolean( bool b )
{
  if ( ! isAnyNumber() && ! isText() ) {
    Warning = "Parameter::setBoolean -> parameter '" + 
      Name + "' is not of type number!";
    return *this;
  }
  return setNumber( b ? 1.0 : 0.0 );
}


Parameter &Parameter::setBoolean( const Parameter &p )
{
  Warning = "";
  if ( ! isBoolean() || ! p.isBoolean() ) {
    Warning = "Parameter::setBoolean() -> '" + Name + "' or '" + p.name()
      + "' is not of type boolean!";
    return *this;
  }

  if ( Value.size() != p.Value.size() ||
       ( Value.size() > 0 && p.Value.size() > 0 && Value[0] != p.Value[0] ) )
    Flags |= ChangedFlag;

  Value = p.Value;
  String = p.String;
  
  return *this;
}


bool Parameter::defaultBoolean( int index ) const
{ 
  Warning = "";
  if ( ! isAnyNumber() && ! isText() ) {
    Warning = "Parameter::defaultBoolean -> parameter '" + 
      Name + "' is not of type number!";
    return false;
  }

  if ( index < 0 || index >= (int)Value.size() ) {
    Warning = "Parameter::defaultBoolean -> invalid index " +
      Str( index ) + " requested for parameter '" + Name + "' !";
    return false;
  }

  return ( DefaultValue[index] != 0.0 ); 
}

  
Parameter &Parameter::setDefaultBoolean( bool dflt )
{
  if ( ! isAnyNumber() && ! isText() ) {
    Warning = "Parameter::setDefaultBoolean -> parameter '" + 
      Name + "' is not of type number!";
    return *this;
  }
  return setDefaultNumber( dflt ? 1.0 : 0.0 );
}


bool Parameter::isDate( void ) const
{
  return ( VType == Date );
}


int Parameter::year( int index ) const
{
  if ( ! isDate() ) {
    Warning = "Parameter::year -> parameter '" + 
      Name + "' is not of type date!";
    return -1;
  }
  if ( index < 0 || index >= (int)Year.size() ) {
    Warning = "Parameter::year -> invalid index " +
      Str( index ) + " requested for parameter '" + Name + "' !";
    return -1;
  }
  return Year[index];
}


int Parameter::month( int index ) const
{
  if ( ! isDate() ) {
    Warning = "Parameter::month -> parameter '" + 
      Name + "' is not of type date!";
    return 0;
  }
  if ( index < 0 || index >= (int)Month.size() ) {
    Warning = "Parameter::month -> invalid index " +
      Str( index ) + " requested for parameter '" + Name + "' !";
    return -1;
  }
  return Month[index];
}


int Parameter::day( int index ) const
{
  if ( ! isDate() ) {
    Warning = "Parameter::day -> parameter '" + 
      Name + "' is not of type date!";
    return 0;
  }
  if ( index < 0 || index >= (int)Day.size() ) {
    Warning = "Parameter::day -> invalid index " +
      Str( index ) + " requested for parameter '" + Name + "' !";
    return -1;
  }
  return Day[index];
}


void Parameter::date( int &year, int &month, int &day, int index ) const
{
  year = 0;
  month = 0;
  day = 0;
  if ( ! isDate() ) {
    Warning = "Parameter::date -> parameter '" + 
      Name + "' is not of type date!";
    return;
  }
  if ( index < 0 || index >= (int)Year.size() ) {
    Warning = "Parameter::date -> invalid index " +
      Str( index ) + " requested for parameter '" + Name + "' !";
    return;
  }
  year = Year[index];
  month = Month[index];
  day = Day[index];
}


Parameter &Parameter::setDate( int year, int month, int day )
{
  if ( ! isDate() ) {
    Warning = "Parameter::setDate -> parameter '" + 
      Name + "' is not of type date!";
    return *this;
  }

  // changed:
  if ( Year.size() > 0 && 
       ( Year[0] != year || Month[0] != month || Day[0] != day ) )
    Flags |= ChangedFlag;

  // clear:
  Year.clear();
  Month.clear();
  Day.clear();
  String.clear();

  // set:
  Year.push_back( year );
  Month.push_back( month );
  Day.push_back( day );

  return *this;
}


Parameter &Parameter::addDate( int year, int month, int day )
{
  if ( ! isDate() ) {
    Warning = "Parameter::addDate -> parameter '" + 
      Name + "' is not of type date!";
    return *this;
  }

  // changed:
  if ( Year.empty() )
    Flags |= ChangedFlag;

  // set:
  Year.push_back( year );
  Month.push_back( month );
  Day.push_back( day );

  return *this;
}


Parameter &Parameter::setDate( const string &date )
{
  // read date:
  int year, month, day;
  int isdate = Str( date ).date( year, month, day );
  if ( isdate != 0 ) {
    Warning = "Parameter::setDate -> argument '" + 
      date + "' is not a date!";
    return *this;
  }

  setDate( year, month, day );

  return *this;
}


Parameter &Parameter::setDate( const struct tm &date )
{
  // read date:
  int year = date.tm_year + 1900;
  int month = date.tm_mon + 1;
  int day = date.tm_mday;

  setDate( year, month, day );

  return *this;
}


Parameter &Parameter::setDate( const time_t &time )
{
  setDate( *::localtime( &time ) );

  return *this;
}


Parameter &Parameter::setCurrentDate( void )
{
  setDate( ::time( 0 ) );

  return *this;
}


Parameter &Parameter::setDate( const Parameter &p )
{
  Warning = "";
  if ( ! isDate() || ! p.isDate() ) {
    Warning = "Parameter::setDate() -> '" + Name + "' or '" + p.name()
      + "' is not of type date!";
    return *this;
  }

  if ( Year.size() != p.Year.size() ||
       ( Year.size() > 0 && p.Year.size() > 0 && 
	 ( Year[0] != p.Year[0] || Month[0] != p.Month[0] || Day[0] != p.Day[0] ) ) )
    Flags |= ChangedFlag;

  Year = p.Year;
  Month = p.Month;
  Day = p.Day;
  String = p.String;
  
  return *this;
}


int Parameter::defaultYear( int index ) const
{
  if ( ! isDate() ) {
    Warning = "Parameter::defaultYear -> parameter '" + 
      Name + "' is not of type date!";
    return 0;
  }
  if ( index < 0 || index >= (int)DefaultYear.size() ) {
    Warning = "Parameter::defaultYear -> invalid index " +
      Str( index ) + " requested for parameter '" + Name + "' !";
    return -1;
  }
  return DefaultYear[index];
}


int Parameter::defaultMonth( int index ) const
{
  if ( ! isDate() ) {
    Warning = "Parameter::defaultMonth -> parameter '" + 
      Name + "' is not of type date!";
    return 0;
  }
  if ( index < 0 || index >= (int)DefaultMonth.size() ) {
    Warning = "Parameter::defaultMonth -> invalid index " +
      Str( index ) + " requested for parameter '" + Name + "' !";
    return -1;
  }
  return DefaultMonth[index];
}


int Parameter::defaultDay( int index ) const
{
  if ( ! isDate() ) {
    Warning = "Parameter::defaultDay -> parameter '" + 
      Name + "' is not of type date!";
    return 0;
  }
  if ( index < 0 || index >= (int)DefaultDay.size() ) {
    Warning = "Parameter::defaultDay -> invalid index " +
      Str( index ) + " requested for parameter '" + Name + "' !";
    return -1;
  }
  return DefaultDay[index];
}


void Parameter::defaultDate( int &year, int &month, int &day, int index ) const
{
  year = 0;
  month = 0;
  day = 0;
  if ( ! isDate() ) {
    Warning = "Parameter::defaultDate -> parameter '" + 
      Name + "' is not of type date!";
    return;
  }
  if ( index < 0 || index >= (int)DefaultYear.size() ) {
    Warning = "Parameter::defaultDate -> invalid index " +
      Str( index ) + " requested for parameter '" + Name + "' !";
    return;
  }
  year = DefaultYear[index];
  month = DefaultMonth[index];
  day = DefaultDay[index];
}


Parameter &Parameter::setDefaultDate( int year, int month, int day )
{
  if ( ! isDate() ) {
    Warning = "Parameter::setDefaultDate -> parameter '" + 
      Name + "' is not of type date!";
    return *this;
  }

  // clear:
  DefaultYear.clear();
  DefaultMonth.clear();
  DefaultDay.clear();
  DefaultString.clear();

  // set:
  DefaultYear.push_back( year );
  DefaultMonth.push_back( month );
  DefaultDay.push_back( day );

  return *this;
}


Parameter &Parameter::addDefaultDate( int year, int month, int day )
{
  if ( ! isDate() ) {
    Warning = "Parameter::addDefaultDate -> parameter '" + 
      Name + "' is not of type date!";
    return *this;
  }

  // add:
  DefaultYear.push_back( year );
  DefaultMonth.push_back( month );
  DefaultDay.push_back( day );

  return *this;
}


Parameter &Parameter::setDefaultDate( const string &date )
{
  // read date:
  int year, month, day;
  int isdate = Str( date ).date( year, month, day );
  if ( isdate != 0 ) {
    Warning = "Parameter::setDefaultDate -> argument '" + 
      date + "' is not a date!";
    return *this;
  }

  setDefaultDate( year, month, day );

  return *this;
}


bool Parameter::isTime( void ) const
{
  return ( VType == Time );
}


int Parameter::hour( int index ) const
{
  if ( ! isTime() ) {
    Warning = "Parameter::hour -> parameter '" + 
      Name + "' is not of type time!";
    return 0;
  }
  if ( index < 0 || index >= (int)Hour.size() ) {
    Warning = "Parameter::hour -> invalid index " +
      Str( index ) + " requested for parameter '" + Name + "' !";
    return -1;
  }
  return Hour[index];
}


int Parameter::minutes( int index ) const
{
  if ( ! isTime() ) {
    Warning = "Parameter::minutes -> parameter '" + 
      Name + "' is not of type time!";
    return 0;
  }
  if ( index < 0 || index >= (int)Minutes.size() ) {
    Warning = "Parameter::minutes -> invalid index " +
      Str( index ) + " requested for parameter '" + Name + "' !";
    return -1;
  }
  return Minutes[index];
}


int Parameter::seconds( int index ) const
{
  if ( ! isTime() ) {
    Warning = "Parameter::seconds -> parameter '" + 
      Name + "' is not of type time!";
    return 0;
  }
  if ( index < 0 || index >= (int)Seconds.size() ) {
    Warning = "Parameter::seconds -> invalid index " +
      Str( index ) + " requested for parameter '" + Name + "' !";
    return -1;
  }
  return Seconds[index];
}


int Parameter::milliSeconds( int index ) const
{
  if ( ! isTime() ) {
    Warning = "Parameter::milliSeconds -> parameter '" + 
      Name + "' is not of type time!";
    return 0;
  }
  if ( index < 0 || index >= (int)MilliSeconds.size() ) {
    Warning = "Parameter::milliSeconds -> invalid index " +
      Str( index ) + " requested for parameter '" + Name + "' !";
    return -1;
  }
  return MilliSeconds[index];
}


void Parameter::time( int &hour, int &minutes, int &seconds, int &milliseconds, int index ) const
{
  hour = 0;
  minutes = 0;
  seconds = 0;
  milliseconds = 0;
  if ( ! isTime() ) {
    Warning = "Parameter::time -> parameter '" + 
      Name + "' is not of type time!";
    return;
  }
  if ( index < 0 || index >= (int)Hour.size() ) {
    Warning = "Parameter::time -> invalid index " +
      Str( index ) + " requested for parameter '" + Name + "' !";
    return;
  }
  hour = Hour[index];
  minutes = Minutes[index];
  seconds = Seconds[index];
  milliseconds = MilliSeconds[index];
}


Parameter &Parameter::setTime( int hour, int minutes, int seconds, int milliseconds )
{
  if ( ! isTime() ) {
    Warning = "Parameter::setTime -> parameter '" + 
      Name + "' is not of type time!";
    return *this;
  }

  // changed:
  if ( Hour.size() > 0 &&
       ( Hour[0] != hour || Minutes[0] != minutes || Seconds[0] != seconds || MilliSeconds[0] != milliseconds ) )
    Flags |= ChangedFlag;

  // clear:
  Hour.clear();
  Minutes.clear();
  Seconds.clear();
  MilliSeconds.clear();
  String.clear();

  // add:
  Hour.push_back( hour );
  Minutes.push_back( minutes );
  Seconds.push_back( seconds );
  MilliSeconds.push_back( milliseconds );

  return *this;
}


Parameter &Parameter::addTime( int hour, int minutes, int seconds, int milliseconds )
{
  if ( ! isTime() ) {
    Warning = "Parameter::addTime -> parameter '" + 
      Name + "' is not of type time!";
    return *this;
  }

  // changed:
  if ( Hour.empty() )
    Flags |= ChangedFlag;

  // add:
  Hour.push_back( hour );
  Minutes.push_back( minutes );
  Seconds.push_back( seconds );
  MilliSeconds.push_back( milliseconds );

  return *this;
}


Parameter &Parameter::setTime( const string &time )
{
  // read time:
  int hour, minutes, seconds, milliseconds;
  int istime = Str( time ).time( hour, minutes, seconds, milliseconds );
  if ( istime != 0 ) {
    Warning = "Parameter::setTime -> argument '" + 
      time + "' is not a time!";
    return *this;
  }

  setTime( hour, minutes, seconds, milliseconds );

  return *this;
}


Parameter &Parameter::setTime( const struct tm &time )
{
  // read time:
  int hour = time.tm_hour;
  int minutes = time.tm_min;
  int seconds = time.tm_sec;

  setTime( hour, minutes, seconds, 0 );

  return *this;
}


Parameter &Parameter::setTime( const time_t &time )
{
  setTime( *::localtime( &time ) );

  return *this;
}


Parameter &Parameter::setCurrentTime( void )
{
  setTime( ::time( 0 ) );

  return *this;
}


Parameter &Parameter::setTime( const Parameter &p )
{
  Warning = "";
  if ( ! isTime() || ! p.isTime() ) {
    Warning = "Parameter::setTime() -> '" + Name + "' or '" + p.name()
      + "' is not of type time!";
    return *this;
  }

  if ( Hour.size() != p.Hour.size() ||
       ( Hour.size() > 0 && p.Hour.size() > 0 && 
	 ( Hour[0] != p.Hour[0] || Minutes[0] != p.Minutes[0] ||
	   Seconds[0] != p.Seconds[0] || MilliSeconds[0] != p.MilliSeconds[0] ) ) )
    Flags |= ChangedFlag;

  Hour = p.Hour;
  Minutes = p.Minutes;
  Seconds = p.Seconds;
  MilliSeconds = p.MilliSeconds;
  String = p.String;
  
  return *this;
}


int Parameter::defaultHour( int index ) const
{
  if ( ! isTime() ) {
    Warning = "Parameter::defaultHour -> parameter '" + 
      Name + "' is not of type time!";
    return 0;
  }
  if ( index < 0 || index >= (int)DefaultHour.size() ) {
    Warning = "Parameter::defaultHour -> invalid index " +
      Str( index ) + " requested for parameter '" + Name + "' !";
    return -1;
  }
  return DefaultHour[index];
}


int Parameter::defaultMinutes( int index ) const
{
  if ( ! isTime() ) {
    Warning = "Parameter::defaultMinutes -> parameter '" + 
      Name + "' is not of type time!";
    return 0;
  }
  if ( index < 0 || index >= (int)DefaultMinutes.size() ) {
    Warning = "Parameter::defaultMinutes -> invalid index " +
      Str( index ) + " requested for parameter '" + Name + "' !";
    return -1;
  }
  return DefaultMinutes[index];
}


int Parameter::defaultSeconds( int index ) const
{
  if ( ! isTime() ) {
    Warning = "Parameter::defaultSeconds -> parameter '" + 
      Name + "' is not of type time!";
    return 0;
  }
  if ( index < 0 || index >= (int)DefaultSeconds.size() ) {
    Warning = "Parameter::defaultSeconds -> invalid index " +
      Str( index ) + " requested for parameter '" + Name + "' !";
    return -1;
  }
  return DefaultSeconds[index];
}


int Parameter::defaultMilliSeconds( int index ) const
{
  if ( ! isTime() ) {
    Warning = "Parameter::defaultMilliSeconds -> parameter '" + 
      Name + "' is not of type time!";
    return 0;
  }
  if ( index < 0 || index >= (int)DefaultMilliSeconds.size() ) {
    Warning = "Parameter::defaultMilliSeconds -> invalid index " +
      Str( index ) + " requested for parameter '" + Name + "' !";
    return -1;
  }
  return DefaultMilliSeconds[index];
}


void Parameter::defaultTime( int &hour, int &minutes, int &seconds, int &milliseconds, int index ) const
{
  hour = 0;
  minutes = 0;
  seconds = 0;
  milliseconds = 0;
  if ( ! isDate() ) {
    Warning = "Parameter::defaultTime -> parameter '" + 
      Name + "' is not of type date!";
    return;
  }
  if ( index < 0 || index >= (int)DefaultHour.size() ) {
    Warning = "Parameter::defaultTime -> invalid index " +
      Str( index ) + " requested for parameter '" + Name + "' !";
    return;
  }
  hour = DefaultHour[index];
  minutes = DefaultMinutes[index];
  seconds = DefaultSeconds[index];
  milliseconds = DefaultMilliSeconds[index];
}


Parameter &Parameter::setDefaultTime( int hour, int minutes, int seconds, int milliseconds )
{
  if ( ! isTime() ) {
    Warning = "Parameter::setDefaultTime -> parameter '" + 
      Name + "' is not of type time!";
    return *this;
  }

  // clear:
  DefaultHour.clear();
  DefaultMinutes.clear();
  DefaultSeconds.clear();
  DefaultMilliSeconds.clear();
  DefaultString.clear();

  // set:
  DefaultHour.push_back( hour );
  DefaultMinutes.push_back( minutes );
  DefaultSeconds.push_back( seconds );
  DefaultMilliSeconds.push_back( milliseconds );

  return *this;
}


Parameter &Parameter::addDefaultTime( int hour, int minutes, int seconds, int milliseconds )
{
  if ( ! isTime() ) {
    Warning = "Parameter::addDefaultTime -> parameter '" + 
      Name + "' is not of type time!";
    return *this;
  }

  // add:
  DefaultHour.push_back( hour );
  DefaultMinutes.push_back( minutes );
  DefaultSeconds.push_back( seconds );
  DefaultMilliSeconds.push_back( milliseconds );

  return *this;
}


Parameter &Parameter::setDefaultTime( const string &time )
{
  // read time:
  int hour, minutes, seconds, milliseconds;
  int istime = Str( time ).time( hour, minutes, seconds, milliseconds );
  if ( istime != 0 ) {
    Warning = "Parameter::setDefaultTime -> argument '" + 
      time + "' is not a time!";
    return *this;
  }

  setDefaultTime( hour, minutes, seconds, milliseconds );

  return *this;
}


bool Parameter::isNotype( void ) const
{
  return ( VType == NoType );
}


bool Parameter::empty( void ) const
{
  return ( VType == NoType || Name.empty() );
}


Parameter &Parameter::setDefault( void ) 
{
  if ( isDate() ) {
    if ( Year.size() > 0 && 
	 ( Year[0] != DefaultYear[0] || Month[0] != DefaultMonth[0] || Day[0] != DefaultDay[0] ) )
      Flags |= ChangedFlag;
    Year = DefaultYear;
    Month = DefaultMonth;
    Day = DefaultDay;
  }
  else if ( isTime() ) {
    if ( Hour.size() > 0 &&
	 ( Hour[0] != DefaultHour[0] || Minutes[0] != DefaultMinutes[0] || Seconds[0] != DefaultSeconds[0] || MilliSeconds[0] != DefaultMilliSeconds[0] ) )
    Flags |= ChangedFlag;
    Hour = DefaultHour;
    Minutes = DefaultMinutes;
    Seconds = DefaultSeconds;
    MilliSeconds = DefaultMilliSeconds;
  }
  else {
    if ( Value.size() != DefaultValue.size() ||
	 ( Value.size() > 0 && DefaultValue.size() > 0 && Value[0] != DefaultValue[0] ) )
      Flags |= ChangedFlag;
    Value = DefaultValue;
    Error.clear();
    Error.resize( Value.size(), -1 );
  }
  if ( String.size() != DefaultString.size() ||
       ( String.size() > 0 && DefaultString.size() > 0 && String[0] != DefaultString[0] ) )
    Flags |= ChangedFlag;
  String = DefaultString;
  return *this;
}


Parameter &Parameter::setToDefault( void ) 
{
  if ( isDate() ) {
    DefaultYear = Year;
    DefaultMonth = Month;
    DefaultDay = Day;
  }
  else if ( isTime() ) {
    DefaultHour = Hour;
    DefaultMinutes = Minutes;
    DefaultMilliSeconds = MilliSeconds;
  }
  else
    DefaultValue = Value;
  DefaultString = String;
  return *this;
}


Parameter &Parameter::setActivation( const string &name, const string &value,
				     bool activate )
{
  clearActivation();
  return addActivation( name, value, activate );
}


Parameter &Parameter::addActivation( const string &name, const string &value,
				     bool activate )
{
  ActivationName.push_back( name );
  ActivationValues.push_back( StrQueue() );
  ActivationValues.back().assign( value, "|" );
  ActivationNumber.push_back( 0.0 );
  ActivationUnit.push_back( "" );
  ActivationComparison.push_back( 0 );
  ActivationType.push_back( activate );
  ActivationStatus.push_back( true );
  if ( ActivationValues.back().size() > 0 && ActivationValues.back().front().size() > 0 &&
       ( ActivationValues.back().front()[0] == '=' ||
	 ActivationValues.back().front()[0] == '>' ||
	 ActivationValues.back().front()[0] == '<' ) ) {
    int inx=1;
    if ( ActivationValues.back().front()[0] == '=' )
      ActivationComparison.back() |= 1;
    else if ( ActivationValues.back().front()[0] == '>' )
      ActivationComparison.back() |= 2;
    else
      ActivationComparison.back() |= 4;
    if ( ActivationValues.back().front().size() > 1 &&
	 ( ActivationValues.back().front()[1] == '=' ||
	   ActivationValues.back().front()[1] == '>' ) ) {
      if ( ActivationValues.back().front()[1] == '=' )
	ActivationComparison.back() |= 1;
      else
	ActivationComparison.back() |= 2;
      inx=2;
    }
    Str vs( value );
    ActivationNumber.back() = vs.number( 0.0, inx );
    ActivationUnit.back() = vs.unit( "", inx );
  }
  return *this;
}


Parameter &Parameter::clearActivation( void )
{
  ActivationName.clear();
  ActivationValues.clear();
  ActivationComparison.clear();
  ActivationType.clear();
  ActivationNumber.clear();
  ActivationUnit.clear();
  ActivationStatus.clear();
  return *this;
}


int Parameter::activations( void ) const
{
  return ActivationName.size();
}


string Parameter::activationName( int index ) const
{
  if ( index < 0 || index >= (int)ActivationName.size() )
    return "";
  else
    return ActivationName[index];
}


string Parameter::activationValue( int index ) const
{
  if ( index < 0 || index >= (int)ActivationValues.size() )
    return "";
  else
    return ActivationValues[index].empty() ? "" : ActivationValues[index][0];
}


string Parameter::activationValues( int index ) const
{
  if ( index < 0 || index >= (int)ActivationValues.size() )
    return "";
  else {
    string s = "";
    for ( int k=0; k<ActivationValues[index].size(); k++ ) {
      if ( k > 0 )
	s += '|';
      s += ActivationValues[index][k];
    }
    return s;
  }
}


double Parameter::activationNumber( int index ) const
{
  if ( index < 0 || index >= (int)ActivationNumber.size() )
    return 0.0;
  else
    return ActivationNumber[index];
}


string Parameter::activationUnit( int index ) const
{
  if ( index < 0 || index >= (int)ActivationUnit.size() )
    return "";
  else
    return ActivationUnit[index];
}


int Parameter::activationComparison( int index ) const
{
  if ( index < 0 || index >= (int)ActivationComparison.size() )
    return 0;
  else
    return ActivationComparison[index];
}


bool Parameter::activation( int index ) const
{
  if ( index < 0 || index >= (int)ActivationType.size() )
    return true;
  else
    return ActivationType[index];
}


bool Parameter::testActivation( int index, const string &value )
{
  if ( index >= 0 && index < (int)ActivationValues.size() ) {
    ActivationStatus[index] = false;
    for ( int k=0; k<ActivationValues[index].size(); k++ ) {
      if ( ActivationValues[index][k] == value ) {
	ActivationStatus[index] = true;
	break;
      }
    }
    if ( ! ActivationType[index] )
      ActivationStatus[index] = ! ActivationStatus[index];
  }

  for ( unsigned int k=0; k<ActivationStatus.size(); k++ ) {
    if ( ! ActivationStatus[k] )
      return false;
  }
  return true;
}


bool Parameter::testActivation( int index, double value, double tol )
{
  if ( index >= 0 && index < (int)ActivationComparison.size() ) {
    bool b = true;
    double testvalue = changeUnit( ActivationNumber[index], ActivationUnit[index], OutUnit );

    switch ( ActivationComparison[index] ) {

    case 1:
      b = ( ::fabs( testvalue - value ) < tol );
      if ( ! ActivationType[index] )
	b = !b;
      break;

    case 2:
      b = ( value > testvalue );
      if ( ! ActivationType[index] )
	b = !b;
      break;

    case 3:
      b = ( value >= testvalue-tol );
      if ( ! ActivationType[index] )
	b = !b;
      break;

    case 4:
      b = ( value < testvalue );
      if ( ! ActivationType[index] )
	b = !b;
      break;

    case 5:
      b = ( value <= testvalue+tol );
      if ( ! ActivationType[index] )
	b = !b;
      break;

    case 6:
      b = ( ::fabs( testvalue - value ) >= tol );
      if ( ! ActivationType[index] )
	b = !b;
      break;

    default:
      // string comparison:
      Str f( format() );
      f.format( value, "fge" );
      b = testActivation( index, f );
    }

    ActivationStatus[index] = b;
  }

  for ( unsigned int k=0; k<ActivationStatus.size(); k++ ) {
    if ( ! ActivationStatus[k] )
      return false;
  }
  return true;
}


string Parameter::quoteString( string s, bool always, bool escape )
{
  if ( s.empty() )
    return "~";

  bool quote = false;
  if ( s.find_first_of( ",{}[]:=" ) != string::npos )
    quote = true;
  if ( Str::WhiteSpace.string::find( s[0] ) != string::npos )
    quote = true;
  size_t i = s.find_first_not_of( Str::WhiteSpace );
  if ( i != string::npos && Str::FirstNumber.string::find( s[i] ) != string::npos )
    quote = true;
  if ( always )
    quote = true;
  if ( quote ) {
    if ( escape )
      return "\\\"" + s + "\\\"";
    else
      return '"' + s + '"';
  }
  else
    return s;
}


string Parameter::save( int flags ) const
{
  std::ostringstream os;
  save(os, 0, flags);
  return os.str();
}


ostream &Parameter::save( ostream &str, int width, int flags ) const
{
  bool escape = ( flags & Options::EscapeQuotes );
  // name:
  string is = "";
  if ( name().find_first_of( ",{}[]:=" ) != string::npos ) {
    if ( escape )
      is = "\\\"" + name() + "\\\"";
    else
      is = '"' + name() + '"';
  }
  else
    is = name();
  if ( (flags & Options::PrintRequest) && name() != request() ) {
    if ( request().find_first_of( ",{}[]:=" ) != string::npos ) {
      if ( escape )
	is += " (\\\"" + request() + "\\\")";
      else
	is += " (\"" + request() + "\")";
    }
    else
      is += " (" + request() + ")";
  }
  str << Str( is, -width ) << ": ";

  bool fulllist = ( size() > 1 &&
                   ( (Style & ListAlways) ||
                     (flags & Options::FirstOnly) == 0 ) );
  bool always = ( flags & Options::AlwaysQuote );

  // value:
  if (isText() && (style() & Parameter::Select) == Parameter::Select)
  {
    if (flags & Options::FirstOnly)
    {
      str << "[ ";
      str << quoteString(text(0), always, escape);
      for (int i = 1 ; i < size(); ++i)
        str << ", " << quoteString(text(i), always, escape);
      str << " ]";
    }
    else
    {
      str << "[ ";
      // values
      {
        str << "[ ";
        str << quoteString(text(0), always, escape);
        for (int i = 1 ; i < size(); ++i)
          str << ", " << quoteString(text(i), always, escape);
        str << " ]";
      }
      str << ", ";
      {
        str << "[ ";
        if (!SelectableValues.empty())
        {
          str << quoteString(selectOption(0), always, escape);
          for (unsigned int i = 1 ; i < SelectableValues.size(); ++i)
            str << ", " << quoteString(selectOption(i), always, escape);
        }
        str << " ]";
      }
      str << " ]";
    }
  }
  else if ( isNumber() || isInteger() ) {
    if ( fulllist )
      str << "[ ";
    if ( error( 0 ) >= 0.0 )
      str << text( 0, "(" + format() + "+-" + format().up() + ")" );
    else
      str << text( 0 );
    if ( outUnit() != "1" )
      str << outUnit();
    if ( fulllist ) {
      for ( int k=1; k<(int)Value.size(); k++ ) {
	str << ", ";
	if ( error( k ) >= 0.0 )
	  str << text( k, "(" + format() + "+-" + format().up() + ")" );
	else
	  str << text( k );
	if ( outUnit() != "1" )
	  str << outUnit();
      }
      str << " ]";
    }
  }
  else if ( isBoolean() ) {
    if ( fulllist )
      str << "[ ";
    str << ( boolean( 0 ) ? "true" : "false" );
    if ( fulllist ) {
      for ( int k=1; k<(int)Value.size(); k++ ) {
	str << ", " << ( boolean( k ) ? "true" : "false" );
      }
      str << " ]";
    }
  }
  else if ( isDate() || isTime() || isText() ) {
    if ( fulllist )
      str << "[ ";
    str << quoteString( text( 0 ), always, escape );
    if ( fulllist ) {
      for ( int k=1; k<(int)String.size(); k++ )
	str << ", " << quoteString( text( k ), always, escape );
      str << " ]";
    }
    if ( isText() && ! unit().empty() )
      str << unit();
  }
  else if ( isNotype() )
    str << "! no type !";
  else
    str << "! unknown type !";

  return str;
}


ostream &Parameter::save( ostream &str, const string &textformat,
			  const string &numberformat, const string &boolformat,
			  const string &dateformat, const string &timeformat ) const
{
  if ( isText() )
    str << text( textformat );
  else if ( isNumber() || isInteger() )
    str << text( numberformat );
  else if ( isBoolean() )
    str << text( boolformat );
  else if ( isDate() )
    str << text( dateformat );
  else if ( isTime() )
    str << text( timeformat );
  else if ( isNotype() )
    str << "! no type !";
  else
    str << "! unknown type !";

  return str;
}


ostream &operator<<( ostream &str, const Parameter &p )
{
  p.save( str );
  return str;
}


ostream &Parameter::saveXML( ostream &str, int level, int indent,
			     int flags ) const
{
  string indstr1( level*indent, ' ' );
  string indstr2( indstr1 );
  indstr2 += string( indent, ' ' );
  int maxinx = size();
  if ( (Style & ListAlways) == 0 &&
       (flags & Options::FirstOnly) &&
       maxinx >= 1 )
    maxinx = 1;

  str << indstr1 << "<property>\n";
  str << indstr2 << "<name>" << name() << "</name>\n";
  if ( name() != request() && (flags & Options::PrintRequest) )
    str << indstr2 << "<request>" << request() << "</request>\n";
  for ( int k=0; k<maxinx; k++ ) {
    if ( isNumber() || isInteger() ) {
      string vtype = "float";
      if ( isInteger() )
	vtype = "integer";
      str << indstr2 << "<value>" << Str( number( k ), format() ).strip();
      if ( flags & Options::PrintType )
	str << "<type>" << vtype << "</type>";
      if ( error( 0 ) >= 0.0 )
	str << "<error>" << Str( error( k ), format() ).strip() << "</error>";
      if ( ! outUnit().empty() && outUnit() != "1" )
	str << "<unit>" << unit() << "</unit>";
    }
    else if ( isBoolean() ) {
      str << indstr2 << "<value>" << ( boolean( k ) ? "true" : "false" );
      if ( flags & Options::PrintType )
	str << "<type>boolean</type>";
    }
    else if ( isDate() ) {
      str << indstr2 << "<value>" << text( k, "%04Y-%02m-%02d" );
      if ( flags & Options::PrintType )
	str << "<type>date</type>";
    }
    else if ( isTime() ) {
      str << indstr2 << "<value>" << text( k, "%02H:%02M:%02S" );
      if ( flags & Options::PrintType )
	str << "<type>time</type>";
    }
    else if ( isText() ) {
      str << indstr2 << "<value>" << text( k ).strip();
      if ( flags & Options::PrintType )
	str << "<type>string</type>";
    }
    str << "</value>\n";
  }
  str << indstr1 << "</property>\n";

  return str;
}


bool Parameter::read( const Str &s, const string &assignment )
{
  Warning = "";
  if ( *this == s.ident( 0, assignment ) ) {
    assign( s.value( 0, assignment ) );
    return true;
  }
  else
    return false;
}


bool Parameter::read( const Parameter &p )
{
  Warning = "";
  if ( name() == p.name() ) {
    if ( isDate() && p.isDate() ) {
      if ( ! Year.empty() && ! p.Year.empty() &&
	   ( Year[0] != p.Year[0] || Month[0] != p.Month[0] || Day[0] != p.Day[0] ) )
	Flags |= ChangedFlag;
      Year = p.Year;
      Month = p.Month;
      Day = p.Day;
    }
    else if ( isTime() && p.isTime() ) {
      if ( ! Hour.empty() && ! p.Hour.empty() &&
	   ( Hour[0] != p.Hour[0] || Minutes[0] != p.Minutes[0] ||
	     Seconds[0] != p.Seconds[0] || MilliSeconds[0] != p.MilliSeconds[0] ) )
	Flags |= ChangedFlag;
      Hour = p.Hour;
      Minutes = p.Minutes;
      Seconds = p.Seconds;
      MilliSeconds = p.MilliSeconds;
    }
    else if  ( isText() && p.isText() ) {
      if ( ! String.empty() && ! p.String.empty() && String[0] != p.String[0] )
	Flags |= ChangedFlag;
      if ( p.size() == 1 && ( size() > 1 || ( style() & Parameter::SelectText ) > 0 ) )
	selectText( p.String[0] );
      else
	String = p.String;
    }
    else if ( isAnyNumber() && p.isAnyNumber() ) {
      if ( ! Value.empty() && ! p.Value.empty() && Value[0] != p.Value[0] )
	Flags |= ChangedFlag;
      Value.clear();
      for ( unsigned int k=0; k<p.Value.size(); k++ )
	Value.push_back( changeUnit( p.Value[k], p.InternUnit, InternUnit ) );
      Error.clear();
      for ( unsigned int k=0; k<p.Error.size(); k++ )
	Error.push_back( changeUnit( p.Error[k], p.InternUnit, InternUnit ) );
    }
    else {
      String.clear();
      Value.clear();
      Error.clear();
      Year.clear();
      Month.clear();
      Day.clear();
      Hour.clear();
      Minutes.clear();
      Seconds.clear();
      MilliSeconds.clear();
      for ( int k=0; k<p.size(); k++ )
	addText( p.text( k ) );
    }
    return true;
  }
  else
    return false;
}


Parameter& Parameter::addSelectOption(const std::string& option)
{
  SelectableValues.insert(option);
  return *this;
}

Parameter& Parameter::removeSelectOption(const std::string& option)
{
  SelectableValues.erase(option);
  return *this;
}

Parameter& Parameter::clearSelectOptions()
{
  SelectableValues.clear();
  return *this;
}

const std::set<std::string>& Parameter::selectOptions() const
{
  return SelectableValues;
}

std::string Parameter::selectOption(int index) const
{
  auto itr = SelectableValues.begin();
  std::advance(itr, index);
  return *itr;
}

int Parameter::selectOptionsSize() const
{
  return SelectableValues.size();
}


}; /* namespace relacs */

