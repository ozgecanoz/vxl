// This is prip/vdtop/vdtop_4_lower_homotopic_kernel.h
#ifndef vdtop_4_lower_homotopic_kernel_h_
#define vdtop_4_lower_homotopic_kernel_h_
//:
// \file
// \brief Provides a function for computing a 4-connected lower homotopic kernel of Bertrand and al
// ( G. Bertrand, J. C. Everat and M. Couprie, "Image segmentation through operators based upon topology",
//   Journal of Electronic Imaging, Vol. 6, N. 4, 395-405, 1997).
// \author Jocelyn Marchadier
// \date 06 May 2004
//
// \verbatim
//  Modifications
//   06 May 2004 Jocelyn Marchadier
// \endverbatim

#include "vdtop_kernel.h"

template <class T>
class vdtop_4_lower_homotopic
{
 protected:
  vdtop_pixel<T> pixel_ ;
  vdtop_4_neighborhood<T> neighborhood_ ;
 public:
  vdtop_4_lower_homotopic(vil_image_view<T> & arg) :pixel_(arg, arg.begin()) {}

  typedef typename vdtop_4_neighborhood<T>::const_iterator iterator ;

  void set_position(typename vil_image_view<T>::iterator arg)
  {
    pixel_.set_position(arg) ;
  }

  bool can_remove()
  {
    return pixel_.is_4_destructible() ;
  }

  void remove()
  {
    pixel_.destruct_8() ;
  }

  iterator begin_next()
  {
    neighborhood_.set_center(pixel_) ;
    return neighborhood_.begin() ;
  }
  iterator end_next()
  {
    return neighborhood_.end() ;
  }
};

//: computes the 4 lower homotopic kernel.
template <class T>
void vdtop_4_lower_homotopic_kernel(vil_image_view<T> & arg)
{
  vdtop_4_lower_homotopic<T> predicate(arg) ;
  vdtop_kernel(arg, predicate) ;
}

#endif
