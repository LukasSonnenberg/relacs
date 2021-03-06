/*
  point.h
  A point in 3D space.

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

#ifndef _RELACS_POINT_H
#define _RELACS_POINT_H 1

#include <deque>
#include <iostream>
using namespace std;

namespace relacs {


class Transform;


/*!
\class Point
\brief A point in 3D space.
\author Jan Benda, Alexander Ott, Fabian Sinz

A point is specified in 4D homogeneous coordinates that hold the x, y,
and z coordinate and the perspective scaling factor w. w is by default
set to 1, and all mathematical operations only apply to the x, y, and
z coordinates, leaving w untouched. w is only changed when applying a
transformation matrix to the point. Use homDiv() to divide all 4
coordinates by w.
 */

class Point {

public:

    /*! A non-defined point. */
  static const Point None;
    /*! The point (0, 0, 0). */
  static const Point Origin;
    /*! The point (1, 1, 1). */
  static const Point Ones;
    /*! The point (1, 0, 0). */
  static const Point UnitX;
    /*! The point (0, 1, 0). */
  static const Point UnitY;
    /*! The point (0, 0, 1). */
  static const Point UnitZ;

    /*! Constructor. */
  Point( void );
    /*! Copy constructor. */
  Point( const Point &p );
    /*! A point with coordinates \a x, \a y, \a z and homogeneity \a h. */
  Point( double x, double y, double z, double h=1.0 );
    /*! A point with coordinates from c. */
  Point( const double c[3] );

    /*! The x-coordinate of the point. */
  double x( void ) const { return Coords[0]; }
    /*! The x-coordinate of the point. */
  double &x( void ) { return Coords[0]; }
    /*! The y-coordinate of the point. */
  double y( void ) const { return Coords[1]; }
    /*! The y-coordinate of the point. */
  double &y( void ) { return Coords[1]; }
    /*! The z-coordinate of the point. */
  double z( void ) const { return Coords[2]; }
    /*! The z-coordinate of the point. */
  double &z( void ) { return Coords[2]; }
    /*! The homogeneity of the point. */
  double w( void ) const { return Coords[3]; }
    /*! The homogeneity of the point. */
  double &w( void ) { return Coords[3]; }

    /*! The coordinate of the point of the i-th dimension (0-3). */
  double operator[]( int i ) const { return Coords[i]; }
    /*! The coordinate of the point of the i-th dimension. */
  double &operator[]( int i ) { return Coords[i]; }

    /*! Assign the coordinates \a x, \a y, \a z, \a h to this point. */
  Point &assign( double x, double y, double z, double h=1.0 );
    /*! Assign coordinates of point \a p to this point. */
  Point &assign( const Point &p );
    /*! Assign coordinates of point \a p to this point. */
  Point &operator=( const Point &p );

    /*! Return the negative of all coordinates. */
  Point operator-( void ) const;

    /*! Add coordinates of point \a p to this point. */
  Point operator+( const Point &p ) const;
    /*! Subtract coordinates of point \a p from this point. */
  Point operator-( const Point &p ) const;
    /*! Multiply coordinates of point \a p elementwise with this point. */
  Point operator*( const Point &p ) const;
    /*! Divide coordinates of this point by the ones of \a p elementwise. */
  Point operator/( const Point &p ) const;

    /*! Add coordinates of point \a p to this point. */
  Point &operator+=( const Point &p );
    /*! Subtract coordinates of point \a p from this point. */
  Point &operator-=( const Point &p );
    /*! Multiply coordinates of point \a p to this point elementwise. */
  Point &operator*=( const Point &p );
    /*! Divide coordinates of this point by the ones of point \a p elementwise. */
  Point &operator/=( const Point &p );

    /*! Add \a a to all coordinates of this point. */
  Point operator+( double a ) const;
    /*! Subtract \a a from all coordinates of this point. */
  Point operator-( double a ) const;
    /*! Multiply all coordinates of this point with \a a. */
  Point operator*( double a ) const;
    /*! Divide all coordinates of this point by \a a. */
  Point operator/( double a ) const;

    /*! Add \a to each element of \a p. */
  friend Point operator+( double a, const Point &p );
    /*! Subtract each element of \a p from \a a. */
  friend Point operator-( double a, const Point &p );
    /*! Multiply each element of \a p with \a a. */
  friend Point operator*( double a, const Point &p );
    /*! Divide \a a by each element of \a p. */
  friend Point operator/( double a, const Point &p );

    /*! Add \a a to all coordinates of this point. */
  Point &operator+=( double a );
    /*! Subtract \a a from all coordinates of this point. */
  Point &operator-=( double a );
    /*! Multiply all coordinates of this point with \a a. */
  Point &operator*=( double a );
    /*! Divide all coordinates of this point by \a a. */
  Point &operator/=( double a );

    /*! Dot product between the coordinates of this point and point \a p. */
  double dot( const Point &p ) const;
    /*! Cross product between the coordinates of this point and point \a p. */
  Point cross( const Point &p ) const;

    /*! The magnitude (length) of the vector. */
  double magnitude( void ) const;
    /*! The angle between this vector and \a p in radians. */
  double angle( const Point &p ) const;

    /*! Normalize the lenght of the vector by its magnitude. */
  Point &normalize( void );
    /*! Return the normalized vector. */
  Point normalized( void ) const;

    /*! Divide all 4 coordinates by the w coordinate. */
  Point &homDivide( void );
    /*! Return the normalized vector. */
  Point homDivided( void ) const;

    /*! True if this point is not defined. */
  bool isNone( void ) const;

    /*! True if this point and \a p are the same, i.e. have the same coordinates. */
  bool operator==( const Point &p ) const;
    /*! True if this point and \a p differ in at least one element. */
  bool operator!=( const Point &p ) const;
    /*! True if all coordinates of \c this are smaller than the ones of \a p. */
  bool operator<( const Point &p ) const;
    /*! True if all coordinates of \c this are smaller or equal than the ones of \a p. */
  bool operator<=( const Point &p ) const;
    /*! True if all coordinates of \c this are greater than the ones of \a p. */
  bool operator>( const Point &p ) const;
    /*! True if all coordinates of \c this are greater or equal than the ones of \a p. */
  bool operator>=( const Point &p ) const;

    /*! Distance between this point and \a p. */
  double distance( const Point &p ) const;
    /*! The center between this point and \a p. */
  Point center( const Point &p ) const;
    /*! The minimum coordinates of this point and \a p. */
  Point min( const Point &p ) const;
    /*! The maximum coordinates of this point and \a p. */
  Point max( const Point &p ) const;
    /*! The minimum coordinates of the points in \a pts. */
  friend Point min( const deque<Point> &pts );
    /*! The maximum coordinates of the points in \a pts. */
  friend Point max( const deque<Point> &pts );

    /*! The point with absolute coordinates. */
  friend Point abs( Point p );

    /*! Apply transformation matrix \a trafo to this point.
        That is, compute m * p. */
  Point &operator*=( const Transform &trafo );

    /*! Write the coordinates of the point to stream \a str. */
  friend ostream &operator<<( ostream &str, const Point &p );


protected:

  static const int Dim = 3;
  double Coords[Dim+1];

};


}; /* namespace relacs */

#endif /* ! _RELACS_POINT_H_ */
