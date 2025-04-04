// Aseprite Document Library
// Copyright (c) 2001-2015 David Capello
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#pragma once

#include "doc/color.h"
#include "doc/image_traits.h"
#include "doc/primitives_fast.h"
#include "gfx/point.h"
#include "gfx/rect.h"

#include <cstdlib>
#include <iterator>

#include <iostream>

namespace doc {

  class Image;

  template<typename ImageTraits,
           typename PointerType,
           typename ReferenceType>
  class ImageIteratorT {
  public:

    typedef typename ImageTraits::pixel_t value_type;
    typedef ptrdiff_t                     difference_type;
    typedef PointerType                   pointer;
    typedef ReferenceType                 reference;
    typedef std::forward_iterator_tag     iterator_category;

    ImageIteratorT() = default;

    ImageIteratorT(const ImageIteratorT& other) :
      m_image(other.m_image),
      m_ptr(other.m_ptr),
      m_x(other.m_x),
      m_y(other.m_y),
      m_xbegin(other.m_xbegin),
      m_xend(other.m_xend)
    {
    }

    ImageIteratorT(const Image* image, const gfx::Rect& bounds, int x, int y) :
      m_image(const_cast<Image*>(image)),
      m_ptr(get_pixel_address_fast<ImageTraits>(image, x, y)),
      m_x(x),
      m_y(y),
      m_xbegin(bounds.x),
      m_xend(bounds.x + bounds.w)
    {
      ASSERT(bounds.contains(gfx::Point(x, y)));
      ASSERT(image->bounds().contains(bounds));
    }

    ImageIteratorT& operator=(const ImageIteratorT& other) {
      m_image = other.m_image;
      m_ptr = other.m_ptr;
      m_x = other.m_x;
      m_y = other.m_y;
      m_xbegin = other.m_xbegin;
      m_xend = other.m_xend;
      return *this;
    }

    bool operator==(const ImageIteratorT& other) const {
      if (m_ptr == other.m_ptr) {
        ASSERT(m_x == other.m_x && m_y == other.m_y);
      }
      else {
        ASSERT(m_x != other.m_x || m_y != other.m_y);
      }
      return m_ptr == other.m_ptr;
    }
    bool operator!=(const ImageIteratorT& other) const {
      if (m_ptr != other.m_ptr) {
        ASSERT(m_x != other.m_x || m_y != other.m_y);
      }
      else {
        ASSERT(m_x == other.m_x && m_y == other.m_y);
      }
      return m_ptr != other.m_ptr;
    }
    bool operator<(const ImageIteratorT& other) const { return m_ptr < other.m_ptr; }
    bool operator>(const ImageIteratorT& other) const { return m_ptr > other.m_ptr; }
    bool operator<=(const ImageIteratorT& other) const { return m_ptr <= other.m_ptr; }
    bool operator>=(const ImageIteratorT& other) const { return m_ptr >= other.m_ptr; }

    ImageIteratorT& operator++() {
      ASSERT(m_image->bounds().contains(gfx::Point(m_x, m_y)));

      ++m_ptr;
      ++m_x;

      if (m_x == m_xend) {
        m_x = m_xbegin;
        ++m_y;

        if (m_y < m_image->height())
          m_ptr = get_pixel_address_fast<ImageTraits>(m_image, m_x, m_y);
      }

      return *this;
    }

    ImageIteratorT& operator+=(int diff) {
      while (diff-- > 0)
        operator++();
      return *this;
    }

    ImageIteratorT operator++(int) {
      ImageIteratorT tmp(*this);
      operator++();
      return tmp;
    }

    reference operator*() { return *m_ptr; }

  private:
    Image* m_image{};
    pointer m_ptr{};
    int m_x{}, m_y{};
    int m_xbegin{};
    int m_xend{};
  };

  template<typename ImageTraits>
  class ImageIterator : public ImageIteratorT<ImageTraits,
                                              typename ImageTraits::pixel_t *,
                                              typename ImageTraits::pixel_t&> {
  public:
    // GCC 4.6 needs these re-definitions here.
    typedef typename ImageTraits::pixel_t* pointer;
    typedef typename ImageTraits::pixel_t& reference;

    ImageIterator() {
    }

    ImageIterator(const Image* image, const gfx::Rect& bounds, int x, int y) :
      ImageIteratorT<ImageTraits,
                     pointer,
                     reference>(image, bounds, x, y) {
    }
  };

  template<typename ImageTraits>
  class ImageConstIterator : public ImageIteratorT<ImageTraits,
                                                   typename ImageTraits::pixel_t const *,
                                                   typename ImageTraits::pixel_t const &> {
  public:
    // GCC 4.6 needs these re-definitions here.
    typedef typename ImageTraits::pixel_t const* pointer;
    typedef typename ImageTraits::pixel_t const& reference;

    ImageConstIterator() {
    }

    ImageConstIterator(const Image* image, const gfx::Rect& bounds, int x, int y) :
      ImageIteratorT<ImageTraits,
                     pointer,
                     reference>(image, bounds, x, y) {
    }
  };

  //////////////////////////////////////////////////////////////////////
  // Iterator for BitmapTraits

  class BitPixelAccess {
  public:
    BitPixelAccess() :
      m_ptr(NULL),
      m_bit(0) {
    }

    void reset(BitmapTraits::address_t ptr, int bit) {
      m_ptr = ptr;
      m_bit = bit;
    }

    void reset(BitmapTraits::pixel_t const* ptr, int bit) {
      m_ptr = const_cast<BitmapTraits::address_t>(ptr);
      m_bit = bit;
    }

    operator color_t() const {
      return (*m_ptr & m_bit) ? 1: 0;
    }

    BitPixelAccess& operator=(color_t value) {
      if (value)
        *m_ptr |= m_bit;
      else
        *m_ptr &= ~m_bit;
      return *this;
    }

    // It doesn't copy the BitPixelAccess, it must copy the bit from
    // "other" to "this".
    BitPixelAccess& operator=(const BitPixelAccess& other) {
      return this->operator=((color_t)other);
    }

    bool operator==(int b) const {
      return (color_t(*this) == color_t(b));
    }

    bool operator==(color_t b) const {
      return (color_t(*this) == b);
    }

    bool operator==(const BitPixelAccess& b) const {
      return (color_t(*this) == color_t(b));
    }

    bool operator!=(int b) const {
      return (color_t(*this) != color_t(b));
    }

    bool operator!=(color_t b) const {
      return (color_t(*this) != b);
    }

    bool operator!=(const BitPixelAccess& b) const {
      return (color_t(*this) != color_t(b));
    }

  private:
    // Non-copyable by copy constructor.
    BitPixelAccess(const BitPixelAccess& other);

    BitmapTraits::address_t m_ptr;
    int m_bit;
  };

  inline bool operator==(int a, const BitPixelAccess& b) {
    return (color_t(a) == color_t(b));
  }

  inline bool operator==(color_t a, const BitPixelAccess& b) {
    return (a == color_t(b));
  }

  inline bool operator!=(int a, const BitPixelAccess& b) {
    return (color_t(a) != color_t(b));
  }

  inline bool operator!=(color_t a, const BitPixelAccess& b) {
    return (a != color_t(b));
  }

  template<typename PointerType,
           typename ReferenceType>
  class ImageIteratorT<BitmapTraits, PointerType, ReferenceType> {
  public:
    // std::iterator<std::forward_iterator_tag, BitmapTraits::pixel_t, ptrdiff_t, PointerType, ReferenceType>
    typedef std::forward_iterator_tag iterator_category;
    typedef BitmapTraits::pixel_t value_type;
    typedef ptrdiff_t difference_type;
    typedef PointerType pointer;
    typedef ReferenceType reference;

    enum { pixels_per_byte = BitmapTraits::pixels_per_byte };

    ImageIteratorT() : m_ptr(NULL) {
    }

    ImageIteratorT(const ImageIteratorT& other) :
      m_image(other.m_image),
      m_ptr(other.m_ptr),
      m_x(other.m_x),
      m_y(other.m_y),
      m_subPixel(other.m_subPixel),
      m_xbegin(other.m_xbegin),
      m_xend(other.m_xend)
    {
    }

    ImageIteratorT(const Image* image, const gfx::Rect& bounds, int x, int y) :
      m_image(const_cast<Image*>(image)),
      m_ptr(get_pixel_address_fast<BitmapTraits>(image, x, y)),
      m_x(x),
      m_y(y),
      m_subPixel(x % 8),
      m_xbegin(bounds.x),
      m_xend(bounds.x + bounds.w)
    {
      ASSERT(bounds.contains(gfx::Point(x, y)));
    }

    ImageIteratorT& operator=(const ImageIteratorT& other) {
      m_image = other.m_image;
      m_ptr = other.m_ptr;
      m_x = other.m_x;
      m_y = other.m_y;
      m_subPixel = other.m_subPixel;
      m_xbegin = other.m_xbegin;
      m_xend = other.m_xend;
      return *this;
    }

    bool operator==(const ImageIteratorT& other) const {
      if (m_ptr == other.m_ptr &&
          m_subPixel == other.m_subPixel) {
        ASSERT(m_x == other.m_x && m_y == other.m_y);
      }
      else {
        ASSERT(m_x != other.m_x || m_y != other.m_y);
      }
      return m_ptr == other.m_ptr;
    }
    bool operator!=(const ImageIteratorT& other) const {
      if (m_ptr != other.m_ptr ||
          m_subPixel != other.m_subPixel) {
        ASSERT(m_x != other.m_x || m_y != other.m_y);
      }
      else {
        ASSERT(m_x == other.m_x && m_y == other.m_y);
      }
      return m_ptr != other.m_ptr;
    }

    ImageIteratorT& operator++() {
      ASSERT(m_image->bounds().contains(gfx::Point(m_x, m_y)));

      ++m_x;
      ++m_subPixel;

      if (m_x == m_xend) {
        m_x = m_xbegin;
        m_subPixel = m_x % 8;
        ++m_y;

        if (m_y < m_image->height())
          m_ptr = get_pixel_address_fast<BitmapTraits>(m_image, m_x, m_y);
        else
          ++m_ptr;
      }
      else if (m_subPixel == 8) {
        m_subPixel = 0;
        ++m_ptr;
      }

      return *this;
    }

    ImageIteratorT& operator+=(int diff) {
      while (diff-- > 0)
        operator++();
      return *this;
    }

    ImageIteratorT operator++(int) {
      ImageIteratorT tmp(*this);
      operator++();
      return tmp;
    }

    reference operator*() const {
      m_access.reset(m_ptr, 1 << m_subPixel);
      return m_access;
    }

    reference operator*() {
      m_access.reset(m_ptr, 1 << m_subPixel);
      return m_access;
    }

  private:
    Image* m_image;
    pointer m_ptr;
    int m_x, m_y;
    int m_subPixel;
    int m_xbegin;
    int m_xend;
    mutable BitPixelAccess m_access;
  };

  template<>
  class ImageIterator<BitmapTraits> : public ImageIteratorT<BitmapTraits,
                                                            uint8_t*,
                                                            BitPixelAccess&> {
  public:
    typedef ImageIteratorT<BitmapTraits,
                           uint8_t*,
                           BitPixelAccess&> Base;

    ImageIterator() {
    }

    ImageIterator(const Image* image, const gfx::Rect& bounds, int x, int y) :
      Base(image, bounds, x, y) {
    }
  };

  template<>
  class ImageConstIterator<BitmapTraits> : public ImageIteratorT<BitmapTraits,
                                                                 uint8_t const*,
                                                                 const BitPixelAccess&> {
  public:
    typedef ImageIteratorT<BitmapTraits,
                           uint8_t const*,
                           const BitPixelAccess&> Base;

    ImageConstIterator() {
    }

    ImageConstIterator(const Image* image, const gfx::Rect& bounds, int x, int y) :
      Base(image, bounds, x, y) {
    }
  };

} // namespace doc
