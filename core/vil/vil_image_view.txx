#ifndef vil2_image_view_txx_
#define vil2_image_view_txx_
//:
//  \file
//  \brief Represent images of one or more planes of Ts.
//  \author Ian Scott
//
// Note: To keep down size of vil2_image_view
// Please think carefully before adding any new methods.
// In particular any methods that provide new views (e.g. vil2_plane)
// will be more usefully provided as external functions. - IMS.


#include "vil2_image_view.h"
#include <vcl_string.h>
#include <vcl_cassert.h>
#include <vcl_ostream.h>
#include <vil2/vil2_smart_ptr.h>
#include <vil2/vil2_pixel_format.h>
#include <vil2/vil2_image_view_functions.h>

//=======================================================================

template<class T>
vil2_image_view<T>::vil2_image_view()
: top_left_(0),istep_(0),jstep_(0),planestep_(0)
{}

template<class T>
vil2_image_view<T>::vil2_image_view(unsigned ni, unsigned nj, unsigned nplanes)
: top_left_(0),istep_(1),jstep_(0)
{
  resize(ni,nj,nplanes);
}

//: Set this view to look at someone else's memory data.
template<class T>
vil2_image_view<T>::vil2_image_view(const T* top_left, unsigned ni, unsigned nj, unsigned nplanes,
                                    int istep, int jstep, int planestep)
{
  set_to_memory(top_left,ni,nj,nplanes,istep,jstep,planestep);
}

//: Set this view to look at another view's data
//  Need to pass the memory chunk to set up the internal smart ptr appropriately
template<class T>
vil2_image_view<T>::vil2_image_view(const vil2_smart_ptr<vil2_memory_chunk>& mem_chunk,
                                    const T* top_left, unsigned ni, unsigned nj, unsigned nplanes,
                                    int istep, int jstep, int planestep)
 : vil2_image_view_base(ni, nj, nplanes)
 , top_left_(const_cast<T*>( top_left))
 , istep_(istep), jstep_(jstep)
 , planestep_(planestep)
 , ptr_(mem_chunk)
{
  // check view and chunk are in rough agreement
  assert(mem_chunk->size() >= nplanes*ni*nj*sizeof(T));
  assert(top_left >= (const T*)mem_chunk->data() &&
         top_left  < (const T*)mem_chunk->data() + mem_chunk->size());
}

//: Copy constructor
// If this view cannot set itself to view the other data (e.g. because the
// types are incompatible) it will set itself to empty.
template<class T>
vil2_image_view<T>::vil2_image_view(const vil2_image_view_base& that):
top_left_(0), istep_(0), jstep_(0), planestep_(0), ptr_(0)
{
  operator=(that);
}

//: Sort of copy constructor
// If this view cannot set itself to view the other data (e.g. because the
// types are incompatible) it will set itself to empty.
template <class T>
vil2_image_view<T>::vil2_image_view(const vil2_image_view_base_sptr& that):
top_left_(0), istep_(0), jstep_(0), planestep_(0), ptr_(0)
{
  operator=(that);
}
//: Perform deep copy of the src image, placing in this image
template<class T>
void vil2_image_view<T>::deep_copy(const vil2_image_view<T>& src)
{
  resize(src.ni(),src.nj(),src.nplanes());

  int s_planestep = src.planestep();
  int s_istep = src.istep();
  int s_jstep = src.jstep();

  // Do a deep copy
  // This is potentially inefficient
  const T* src_data = src.top_left_ptr();
  T* data = top_left_;
  for (unsigned int p=0;p<nplanes_;++p)
  {
    T* row = data;
    const T* src_row = src_data;
    for (unsigned j=0;j<nj_;++j)
    {
      T* p = row;
      const T* sp = src_row;
      for (int i=0;i<ni_;++i)
      {
        *p = *sp;
        p+=istep_;
        sp+=s_istep;
      }
      row += jstep_;
      src_row += s_jstep;
    }
    src_data += s_planestep;
    data += planestep_;
  }
}

// Notes on convert_components_from_planes() and convert_planes_from_components()
// These are used by the operator= to provide the appropriate smart conversion
// behaviour for the various types.
// I don't think that C++ templates support full pattern matching,
// so we have to provide one template instantiation to cover the general
// compound pixel case (the range of which is possibly infinite)
// We then specialise for all the scalar pixel cases (there are only so
// many scalar types).
// I guess someone could merge all the scalar specialisations using
// macros and substantially reduce the length of this code.


//: Convert planes to components from planes, or do nothing if types are wrong.
template <class T>
inline bool convert_components_from_planes(vil2_image_view<T> &lhs,
                                           const vil2_image_view_base &rhs_base)
{
  typedef typename T::value_type comp_type;

  const unsigned ncomp =
    vil2_pixel_format_num_components(vil2_pixel_format_of(T()));

  if (// both sides have equal component types and rhs has scalar pixels and
      rhs_base.pixel_format() == vil2_pixel_format_component_format(vil2_pixel_format_of(T()) ) &&
      // lhs has number of components equal to rhs's number of planes.
      ncomp == rhs_base.nplanes() )
  {
    const vil2_image_view<comp_type> &rhs = static_cast<const vil2_image_view<comp_type>&>(rhs_base);
    // Check that the steps are suitable for viewing as components
    if (rhs.planestep() != 1 || rhs.istep()%ncomp !=0 || rhs.jstep()%ncomp !=0 ) return false;
    lhs = vil2_image_view<T >(rhs.memory_chunk(),
                              (T const*) rhs.top_left_ptr(),
                              rhs.ni(),rhs.nj(),1,
                              rhs.istep()/ncomp,rhs.jstep()/ncomp,1);
    return true;
  }
  else
    return false;
} 


VCL_DEFINE_SPECIALIZATION
inline bool convert_components_from_planes(vil2_image_view<float> &lhs,
                                         const vil2_image_view_base &rhs_base)
{return false;}  // when lhs has scalar pixels, don't attempt conversion

VCL_DEFINE_SPECIALIZATION
inline bool convert_components_from_planes(vil2_image_view<double> &lhs,
                                         const vil2_image_view_base &rhs_base)
{return false;} 

VCL_DEFINE_SPECIALIZATION
inline bool convert_components_from_planes(vil2_image_view<bool> &lhs,
                                         const vil2_image_view_base &rhs_base)
{return false;} 

VCL_DEFINE_SPECIALIZATION
inline bool convert_components_from_planes(vil2_image_view<vxl_int_8> &lhs,
                                         const vil2_image_view_base &rhs_base)
{return false;} 

VCL_DEFINE_SPECIALIZATION
inline bool convert_components_from_planes(vil2_image_view<vil2_byte> &lhs,
                                         const vil2_image_view_base &rhs_base)
{return false;} 

VCL_DEFINE_SPECIALIZATION
inline bool convert_components_from_planes(vil2_image_view<vxl_int_16> &lhs,
                                         const vil2_image_view_base &rhs_base)
{return false;} 

VCL_DEFINE_SPECIALIZATION
inline bool convert_components_from_planes(vil2_image_view<vxl_uint_16> &lhs,
                                         const vil2_image_view_base &rhs_base)
{return false;} 

VCL_DEFINE_SPECIALIZATION
inline bool convert_components_from_planes(vil2_image_view<vxl_int_32> &lhs,
                                         const vil2_image_view_base &rhs_base)
{return false;} 

VCL_DEFINE_SPECIALIZATION
inline bool convert_components_from_planes(vil2_image_view<vxl_uint_32> &lhs,
                                         const vil2_image_view_base &rhs_base)
{return false;} 


//: Convert components to planes from planes, or do nothing if types are wrong.
template <class T>
inline bool convert_planes_from_components(vil2_image_view<T> &lhs,
                                           const vil2_image_view_base &rhs_base)
{ return false;} // when lhs has non-scalar pixels, don't attempt conversion

VCL_DEFINE_SPECIALIZATION
inline bool convert_planes_from_components(vil2_image_view<vil2_byte> &lhs,
                                           const vil2_image_view_base &rhs_base)
{
  const unsigned ncomp =
    vil2_pixel_format_num_components(rhs_base.pixel_format());

  if (// rhs has just 1 plane
      rhs_base.nplanes() == 1 &&
      // both sides have equal component types
      vil2_pixel_format_component_format(rhs_base.pixel_format()) == VIL2_PIXEL_FORMAT_BYTE)
  {
    // cheat by casting to component type, not pixel type (because we don't know full pixel type at compile time.)
    const vil2_image_view<vil2_byte> &rhs = static_cast<const vil2_image_view<vil2_byte>&>(rhs_base);

    lhs = vil2_image_view<vil2_byte>(rhs.memory_chunk(), rhs.top_left_ptr(),
                                      rhs.ni(),rhs.nj(),ncomp,
                                      rhs.istep()*ncomp,rhs.jstep()*ncomp,1);
    return true;
  }
  else
    return false;
} 

VCL_DEFINE_SPECIALIZATION
inline bool convert_planes_from_components(vil2_image_view<vxl_int_8> &lhs,
                                           const vil2_image_view_base &rhs_base)
{
  const unsigned ncomp =
    vil2_pixel_format_num_components(rhs_base.pixel_format());

  if (// rhs has just 1 plane
      rhs_base.nplanes() == 1 &&
      // both sides have equal component types
      vil2_pixel_format_component_format(rhs_base.pixel_format()) == VIL2_PIXEL_FORMAT_INT_8)
  {
    // cheat by casting to component type, not pixel type (because we don't know full pixel type at compile time.)
    const vil2_image_view<vxl_int_8> &rhs = static_cast<const vil2_image_view<vxl_int_8>&>(rhs_base);

    lhs = vil2_image_view<vxl_int_8>(rhs.memory_chunk(), rhs.top_left_ptr(),
                                      rhs.ni(),rhs.nj(),ncomp,
                                      rhs.istep()*ncomp,rhs.jstep()*ncomp,1);
    return true;
  }
  else
    return false;
} 

VCL_DEFINE_SPECIALIZATION
inline bool convert_planes_from_components(vil2_image_view<vxl_uint_16> &lhs,
                                           const vil2_image_view_base &rhs_base)
{
  const unsigned ncomp =
    vil2_pixel_format_num_components(rhs_base.pixel_format());

  if (// rhs has just 1 plane
      rhs_base.nplanes() == 1 &&
      // both sides have equal component types
      vil2_pixel_format_component_format(rhs_base.pixel_format()) == VIL2_PIXEL_FORMAT_UINT_16)
  {
    // cheat by casting to component type, not pixel type (because we don't know full pixel type at compile time.)
    const vil2_image_view<vxl_uint_16> &rhs = static_cast<const vil2_image_view<vxl_uint_16>&>(rhs_base);

    lhs = vil2_image_view<vxl_uint_16>(rhs.memory_chunk(), rhs.top_left_ptr(),
                                      rhs.ni(),rhs.nj(),ncomp,
                                      rhs.istep()*ncomp,rhs.jstep()*ncomp,1);
    return true;
  }
  else
    return false;
} 

VCL_DEFINE_SPECIALIZATION
inline bool convert_planes_from_components(vil2_image_view<vxl_int_16> &lhs,
                                           const vil2_image_view_base &rhs_base)
{
  const unsigned ncomp =
    vil2_pixel_format_num_components(rhs_base.pixel_format());

  if (// rhs has just 1 plane
      rhs_base.nplanes() == 1 &&
      // both sides have equal component types
      vil2_pixel_format_component_format(rhs_base.pixel_format()) == VIL2_PIXEL_FORMAT_INT_16)
  {
    // cheat by casting to component type, not pixel type (because we don't know full pixel type at compile time.)
    const vil2_image_view<vxl_int_16> &rhs = static_cast<const vil2_image_view<vxl_int_16>&>(rhs_base);

    lhs = vil2_image_view<vxl_int_16>(rhs.memory_chunk(), rhs.top_left_ptr(),
                                      rhs.ni(),rhs.nj(),ncomp,
                                      rhs.istep()*ncomp,rhs.jstep()*ncomp,1);
    return true;
  }
  else
    return false;
} 

VCL_DEFINE_SPECIALIZATION
inline bool convert_planes_from_components(vil2_image_view<vxl_uint_32> &lhs,
                                           const vil2_image_view_base &rhs_base)
{
  const unsigned ncomp =
    vil2_pixel_format_num_components(rhs_base.pixel_format());

  if (// rhs has just 1 plane
      rhs_base.nplanes() == 1 &&
      // both sides have equal component types
       vil2_pixel_format_component_format(rhs_base.pixel_format()) == VIL2_PIXEL_FORMAT_UINT_32)
  {
    // cheat by casting to component type, not pixel type (because we don't know full pixel type at compile time.)
    const vil2_image_view<vxl_uint_32> &rhs = static_cast<const vil2_image_view<vxl_uint_32>&>(rhs_base);

    lhs = vil2_image_view<vxl_uint_32>(rhs.memory_chunk(), rhs.top_left_ptr(),
                                      rhs.ni(),rhs.nj(),ncomp,
                                      rhs.istep()*ncomp,rhs.jstep()*ncomp,1);
    return true;
  }
  else
    return false;
} 

VCL_DEFINE_SPECIALIZATION
inline bool convert_planes_from_components(vil2_image_view<vxl_int_32> &lhs,
                                           const vil2_image_view_base &rhs_base)
{
  const unsigned ncomp =
    vil2_pixel_format_num_components(rhs_base.pixel_format());

  if (// rhs has just 1 plane
      rhs_base.nplanes() == 1 &&
      // both sides have equal component types
      vil2_pixel_format_component_format(rhs_base.pixel_format()) == VIL2_PIXEL_FORMAT_INT_32)
  {
    // cheat by casting to component type, not pixel type (because we don't know full pixel type at compile time.)
    const vil2_image_view<vxl_int_32> &rhs = static_cast<const vil2_image_view<vxl_int_32>&>(rhs_base);

    lhs = vil2_image_view<vxl_int_32>(rhs.memory_chunk(), rhs.top_left_ptr(),
                                      rhs.ni(),rhs.nj(),ncomp,
                                      rhs.istep()*ncomp,rhs.jstep()*ncomp,1);
    return true;
  }
  else
    return false;
} 

VCL_DEFINE_SPECIALIZATION
inline bool convert_planes_from_components(vil2_image_view<float> &lhs,
                                           const vil2_image_view_base &rhs_base)
{
  const unsigned ncomp =
    vil2_pixel_format_num_components(rhs_base.pixel_format());

  if (// rhs has just 1 plane
      rhs_base.nplanes() == 1 &&
      // both sides have equal component types
      vil2_pixel_format_component_format(rhs_base.pixel_format()) == VIL2_PIXEL_FORMAT_FLOAT)
  {
    // cheat by casting to component type, not pixel type (because we don't know full pixel type at compile time.)
    const vil2_image_view<float> &rhs = static_cast<const vil2_image_view<float>&>(rhs_base);

    lhs = vil2_image_view<float>(rhs.memory_chunk(), rhs.top_left_ptr(),
                                      rhs.ni(),rhs.nj(),ncomp,
                                      rhs.istep()*ncomp,rhs.jstep()*ncomp,1);
    return true;
  }
  else
    return false;
} 

VCL_DEFINE_SPECIALIZATION
inline bool convert_planes_from_components(vil2_image_view<double> &lhs,
                                           const vil2_image_view_base &rhs_base)
{
  const unsigned ncomp =
    vil2_pixel_format_num_components(rhs_base.pixel_format());

  if (// rhs has just 1 plane
      rhs_base.nplanes() == 1 &&
      // both sides have equal component types
      vil2_pixel_format_component_format(rhs_base.pixel_format()) == VIL2_PIXEL_FORMAT_DOUBLE)
  {
    // cheat by casting to component type, not pixel type (because we don't know full pixel type at compile time.)
    const vil2_image_view<double> &rhs = static_cast<const vil2_image_view<double>&>(rhs_base);

    lhs = vil2_image_view<double>(rhs.memory_chunk(), rhs.top_left_ptr(),
                                      rhs.ni(),rhs.nj(),ncomp,
                                      rhs.istep()*ncomp,rhs.jstep()*ncomp,1);
    return true;
  }
  else
    return false;
} 


//: Create a copy of the data viewed by this, and return a view of copy.
// This function can be made a lot more powerful - to automatically convert between pixel types.
template<class T>
const vil2_image_view<T> & vil2_image_view<T>::operator= (const vil2_image_view_base & rhs)
{
  if (static_cast<const vil2_image_view_base*>(this) == &rhs)
    return *this;

  if (rhs.pixel_format() == pixel_format())
  {
    const vil2_image_view<T> &that = static_cast<const vil2_image_view<T>&>(rhs);
    ni_=that.ni_;
    nj_=that.nj_;
    nplanes_=that.nplanes_;
    istep_=that.istep_;
    jstep_=that.jstep_;
    planestep_=that.planestep_;
    top_left_=that.top_left_;
    ptr_=that.ptr_;
    return *this;
  }

  if (convert_components_from_planes(*this, rhs))
    return *this;

  if (convert_planes_from_components(*this, rhs))
    return *this;

  set_to_memory(0, 0, 0, 0, 0, 0, 0);
  return *this;
}


//=======================================================================

template<class T>
void vil2_image_view<T>::release_data()
{
  ptr_=0;
}

template<class T> vil2_image_view<T>::~vil2_image_view()
{
  // release_data();
}

//=======================================================================


template<class T>
void vil2_image_view<T>::resize(unsigned ni, unsigned nj)
{
  resize(ni,nj, nplanes_);
}

//: True if data all in one unbroken block
template<class T>
bool vil2_image_view<T>::is_contiguous() const
{
  // RRR GGG BBB
  if (planestep_==ni_*nj_)
  {
    if (istep_==1 && jstep_==ni_) return true;
    if (jstep_==1 && istep_==nj_) return true;
  }

  // RGBRGBRGB
  if (planestep_==1)
  {
    if (istep_==nplanes_ && jstep_==ni_*nplanes_) return true;
    if (jstep_==nplanes_ && istep_==nj_*nplanes_) return true;
  }

  // Note that there may be other weird combinations
  return false;
}

//=======================================================================

template<class T>
void vil2_image_view<T>::resize(unsigned ni, unsigned nj, unsigned nplanes)
{
  if (ni==ni_ && nj==nj_ && nplanes==nplanes_) return;

  release_data();

  ptr_ = new vil2_memory_chunk(sizeof(T)*nplanes*nj*ni);

  ni_ = ni;
  nj_ = nj;
  nplanes_ = nplanes;
  istep_ = 1;
  jstep_ = ni;
  planestep_ = ni*nj;

  top_left_ = (T*) ptr_->data();
}


//: Set this view to look at someone else's memory.
template<class T>
void vil2_image_view<T>::set_to_memory(const T* top_left,
                                       unsigned ni, unsigned nj, unsigned nplanes,
                                       int istep, int jstep, int planestep)
{
  release_data();
  top_left_ = (T*) top_left;  // Remove const, as view may end up manipulating data

  ni_ = ni;
  nj_ = nj;
  nplanes_ = nplanes;
  istep = istep;
  jstep_ = jstep;
  planestep_ = planestep;
}


//=======================================================================
//: Arrange that this is window on given image.
template<class T>
void vil2_image_view<T>::set_to_window(const vil2_image_view& im,
                                       unsigned i0, unsigned ni, unsigned j0,
                                       unsigned nj, unsigned p0, unsigned np)
{
  assert(this!=&im);

  assert(i0<im.ni()); assert(i0+ni<=im.ni());
  assert(j0<im.nj()); assert(j0+nj<=im.nj());
  assert(p0<im.nplanes()); assert(p0+np<=im.nplanes());

  release_data();

  // Take smart pointer to im's data to keep it in scope
  ptr_ = im.ptr_;

  ni_ = ni;
  nj_ = nj;
  nplanes_ = np;
  istep_ = im.istep();
  jstep_ = im.jstep();
  planestep_ = im.planestep();

  // Have to force the cast to avoid compiler warnings about const
  top_left_ = (T*) im.top_left_ptr() + i0*istep_ + j0*jstep_ + p0*planestep_;
}

//: Arrange that this is window on all planes of given image.
template<class T>
void vil2_image_view<T>::set_to_window(const vil2_image_view& im,
                                       unsigned i0, unsigned ni, unsigned j0, unsigned nj)
{
  set_to_window(im,i0,nj,j0,nj,0,im.nplanes());
}

//: Fill view with given value
template<class T>
void vil2_image_view<T>::fill(T value)
{
  T* plane = top_left_;
  for (unsigned int p=0;p<nplanes_;++p,plane += planestep_)
  {
    T* row = plane;
    for (int j=0;j<nj_;++j,row += jstep_)
    {
      T* p = row;
      for (int i=0;i<ni_;++i,p+=istep_) *p = value;
    }
  }
}

//=======================================================================

template<class T>
bool vil2_image_view<T>::is_class(vcl_string const& s) const
{
  return s==vil2_image_view<T>::is_a() || vil2_image_view_base::is_class(s);
}

//=======================================================================

template<class T>
void vil2_image_view<T>::print(vcl_ostream& os) const
{
  os<<nplanes_<<" planes, each "<<ni_<<" x "<<nj_;
}

//: print all data to os
template<class T>
void vil2_image_view<T>::print_all(vcl_ostream& os) const
{
  print(os);
  os<<"  istep: "<<istep_<<" jstep: "<<jstep_<<" planestep: "<<planestep_<<vcl_endl;
  vcl_cout<<"vil2_image_view<T>::print_all Not complete!"<<vcl_endl;
}


//=======================================================================
//: True if they share same view of same image data.
//  This does not do a deep equality on image data. If the images point
//  to different image data objects that contain identical images, then
//  the result will still be false.
template<class T>
bool vil2_image_view<T>::operator==(const vil2_image_view<T> &other) const
{
  return ptr_  == other.ptr_ &&
    top_left_  == other.top_left_ &&
    nplanes_   == other.nplanes_ &&
    ni_        == other.ni_ &&
    nj_        == other.nj_ &&
    planestep_ == other.planestep_ &&
    istep_      == other.istep_ &&
    jstep_     == other.jstep_;
}


#define VIL2_IMAGE_VIEW_INSTANTIATE(T) \
template class vil2_image_view<T >

#endif // vil2_image_view_txx_
