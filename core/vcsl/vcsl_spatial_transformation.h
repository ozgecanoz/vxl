// This is core/vcsl/vcsl_spatial_transformation.h
#ifndef vcsl_spatial_transformation_h
#define vcsl_spatial_transformation_h
#ifdef VCL_NEEDS_PRAGMA_INTERFACE
#pragma interface
#endif
//:
// \file
// \author Fran�ois BERTEL
// \brief transformation between 2 spatial coordinate systems
//
// \verbatim
//  Modifications
//   2000/06/28 Fran�ois BERTEL Creation. Adapted from IUE
//   2001/04/10 Ian Scott (Manchester) Converted perceps header to doxygen
//   2002/01/22 Peter Vanroose - added lmi() as it is used in vcsl_matrix.cxx
//   2002/01/22 Peter Vanroose - return type of lqi(), lvi(), execute() and inverse() changed to non-ptr
//   2002/01/28 Peter Vanroose - vcl_vector members beat_ and interpolator_ changed to non-ptr
// \endverbatim

#include <vcsl/vcsl_spatial_transformation_sptr.h>

#include <vbl/vbl_ref_count.h>
#include <vcl_vector.h>
#include <vnl/vnl_vector.h>
#include <vnl/vnl_quaternion.h>

typedef vcl_vector<double> list_of_scalars;
typedef vcl_vector<vnl_vector<double> > list_of_vectors;

enum vcsl_interpolator
{
  vcsl_linear,
  vcsl_cubic,
  vcsl_spline
};

//: Transformation between 2 spatial coordinate systems
// A spatial transformation can be static or dynamic
class vcsl_spatial_transformation : public vbl_ref_count
{
 public:

  //***************************************************************************
  // Constructors/Destructor
  //***************************************************************************

  //: Default constructor. Do nothing
  explicit vcsl_spatial_transformation(void) {}

  //: Destructor. Do nothing
  virtual ~vcsl_spatial_transformation() {}

  //***************************************************************************
  // Status report
  //***************************************************************************

  //: Return the list of time clocks
  virtual vcl_vector<double> beat(void) const { return beat_; }

  //: Return the list of interpolators
  virtual vcl_vector<vcsl_interpolator> interpolators(void) const
  { return interpolator_; }

  //: Is `time' between the two time bounds ?
  virtual bool valid_time(double time) const;

  //: Is `this' invertible at time `time'?
  //  REQUIRE: valid_time(time)
  virtual bool is_invertible(double time) const=0;

  //: Is `this' correctly set ?
  virtual bool is_valid(void) const;

  //***************************************************************************
  // Basic operations
  //***************************************************************************

  //: Return the index of the beat inferior or equal to `time'
  //  REQUIRE: valid_time(time)
  virtual int matching_interval(double time) const;


  //: Image of `v' by `this'
  //  REQUIRE: is_valid()
  virtual vnl_vector<double> execute(const vnl_vector<double> &v,
                                     double time) const=0;

  //: Image of `v' by the inverse of `this'
  //  REQUIRE: is_invertible(time)
  //  REQUIRE: is_valid()
  virtual vnl_vector<double> inverse(const vnl_vector<double> &v,
                                     double time) const=0;

  //***************************************************************************
  // Status setting
  //***************************************************************************

  //: Set the list of time clocks
  virtual void set_beat(vcl_vector<double> const& new_beat);

  //: Set the list of interpolators
  virtual void set_interpolators(vcl_vector<vcsl_interpolator> const& new_interpolators);

  //: Empty the time clock and interpolators, thereby making the transf static
  void set_static();

  //***************************************************************************
  // Interpolators
  //***************************************************************************

  //: Linear interpolation on scalar values
  virtual double lsi(double v0,
                     double v1,
                     int index,
                     double time) const;

  //: Linear interpolation on vnl_vectors
  virtual vnl_vector<double> lvi(const vnl_vector<double> &v0,
                                 const vnl_vector<double> &v1,
                                 int index,
                                 double time) const;

  //: Linear interpolation on vnl_matrices
  virtual vnl_matrix<double> lmi(const vnl_matrix<double> &m0,
                                 const vnl_matrix<double> &m1,
                                 int index,
                                 double time) const;

  //: Linear interpolation on quaternions
  virtual vnl_quaternion<double> lqi(const vnl_quaternion<double> &v0,
                                     const vnl_quaternion<double> &v1,
                                     int index,
                                     double time) const;

 protected:
  //: List of time clocks
  vcl_vector<double> beat_;
  vcl_vector<vcsl_interpolator> interpolator_;
};

#endif // vcsl_spatial_transformation_h
