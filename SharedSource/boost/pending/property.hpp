#ifndef BOOST_PROPERTY_HPP
#define BOOST_PROPERTY_HPP

#include <boost/pending/ct_if.hpp>

namespace boost {

  struct no_property { 
    typedef no_property tag_type;
    typedef no_property next_type;
    typedef no_property value_type;
  };
  template <class Tag, class T, class Base = no_property>
  struct property : public Base {
    typedef Base next_type;
    typedef Tag tag_type;
    typedef T value_type;
    property() { }
    property(const T& v) : m_value(v) { }
    property(const T& v, const Base& b) : Base(b), m_value(v) { }
    T m_value;
  };

  // The BGL properties just specialize property_kind and
  // property_num, and use enum's for the Property type (see
  // graph/properties.hpp), but the user may want to use a class
  // instead with a nested kind type and num.  Also, we may want to
  // switch BGL back to using class types for properties.

  template <class Property>
  struct property_kind {
    typedef typename Property::kind type;
  };

  template <class Property>
  struct property_num {
    enum { value = Property::num };
  };


} // namespace boost

#include <boost/pending/detail/property.hpp>

namespace boost {

  template <class Tag1, class Tag2, class T1, class T2, class Base>
  inline T2& 
  get_property_value(property<Tag1,T1,Base>& p, T2 t2, Tag2 tag2) {
    enum { match = int(property_num<Tag1>::value)
           == int(property_num<Tag2>::value) };
    typedef detail::property_value_dispatch<match> Dispatcher;
    return Dispatcher::get_value(p, t2, tag2);
  }
  template <class Tag1, class Tag2, class T1, class T2, class Base>
  inline const T2& 
  get_property_value(const property<Tag1,T1,Base>& p, T2 t2, Tag2 tag2) {
    enum { match = int(property_num<Tag1>::value) 
           == int(property_num<Tag2>::value) };
    typedef detail::property_value_dispatch<match> Dispatcher;
    return Dispatcher::const_get_value(p, t2, tag2);
  }

  template <class Property, class Tag>
  struct property_value {
#if !defined BOOST_NO_TEMPLATE_PARTIAL_SPECIALIZATION
    typedef typename detail::build_property_tag_value_alist<Property>::type AList;
    typedef typename detail::extract_value<AList,Tag>::type type;
#else
    typedef typename detail::build_property_tag_value_alist<Property>::type AList;
    typedef typename detail::ev_selector<AList>::type Extractor;
    typedef typename Extractor::template bind<AList,Tag>::type type;
#endif  
  };

} // namesapce boost

#endif /* BOOST_PROPERTY_HPP */
