#ifndef SIMD_H
#define SIMD_H

#include<iostream>
#include<string>
#include<memory>


namespace ASC_HPC
{


#ifdef __AVX__
  constexpr size_t DefaultSimdSizeBytes = 32;
#else
  constexpr size_t DefaultSimdSizeBytes = 16;
#endif

  
  template <typename T, size_t S = DefaultSimdSizeBytes/sizeof(T)> class SIMD;
  
  
  constexpr size_t LargestPowerOfTwo (size_t x)
  {
    size_t y = 1;
    while (2*y <= x) y *= 2;
    return y;
  }

  
  class mask64
  {
    int64_t mask;
  public:
    mask64 (bool b)
      : mask{ b ? -1 : 0 } { }
    // auto Val() const { return mask; }
    operator bool() { return bool(mask); }
  };
  
  inline std::ostream & operator<< (std::ostream & ost, mask64 m)
  {
    ost << (m ? 't' : 'f');
    return ost;
  }

  namespace detail {
    template <typename T, size_t N, size_t... I>
    auto array_range_impl(std::array<T, N> const& arr, size_t first,
                          std::index_sequence<I...>) {
      return std::array<T, sizeof...(I)>{arr[first + I]...};
    }
    
    template <size_t FIRST, size_t NEXT, typename T, size_t N>
    auto array_range(std::array<T, N> const& arr) {
      return array_range_impl(arr, FIRST, std::make_index_sequence<NEXT-FIRST>{});
    }
  } // namespace detail

  
  
  template <typename T, size_t S>
  class SIMD
  {
  protected:
    static constexpr size_t S1 = LargestPowerOfTwo(S-1);
    static constexpr size_t S2 = S-S1;

    SIMD<T,S1> lo;
    SIMD<T,S2> hi;
  public:
    SIMD() = default;

    explicit SIMD (T val)
      : lo(val), hi(val) { }

    explicit SIMD (SIMD<T,S1> _lo, SIMD<T,S2> _hi)
      : lo(_lo), hi(_hi) { }
    
    explicit SIMD (std::array<T, S> arr)
      : lo(detail::array_range<0, S1>(arr)),
        hi(detail::array_range<S1, S>(arr))
      {}

    template <typename ...T2>
    explicit SIMD (T val0, T2... vals)
      : SIMD(std::array<T, S>{val0, vals...}) { }

    explicit SIMD (T * ptr)
      : lo(ptr), hi(ptr+S1) { }
    
    explicit SIMD (T * ptr, SIMD<mask64,S> m)
      : lo(ptr, m.Lo()), hi(ptr+S1, m.Hi()) { }
    
    
    static constexpr int Size() { return S; }    
    auto & Lo() { return lo; }
    auto & Hi() { return hi; }

    const T * Ptr() const { return lo.Ptr(); }
    T operator[] (size_t i) const { return Ptr()[i]; }
  };


  
  template <typename T>
  class SIMD<T,1>
  {
    T val;
  public:
    SIMD() = default;
    SIMD(T _val) : val(_val) { }
    SIMD(std::array<T,1> vals) : val(vals[0]) { }

    explicit SIMD (T * ptr)
      : val{*ptr} { } 

    auto Val() const { return val; }
    
    explicit SIMD (T * ptr, SIMD<mask64,1> m)
      : val{ m.Val() ? *ptr : T(0)} { }

    constexpr size_t Size() { return 1; }
    const T * Ptr() const { return &val; }    
    T operator[] (size_t i) const { return val; }
  };


  template <typename T, size_t S>  
  std::ostream & operator<< (std::ostream & ost, SIMD<T,S> simd)
  {
    for (size_t i = 0; i < S-1; i++)
      ost << simd[i] << ", ";
    ost << simd[S-1];
    return ost;
  }



  template <typename T, size_t S>
  auto operator+ (SIMD<T,S> a, SIMD<T,S> b) { return SIMD<T,S> (a.Lo()+b.Lo(), a.Hi()+b.Hi()); }
  template <typename T>
  auto operator+ (SIMD<T,1> a, SIMD<T,1> b) { return SIMD<T,1> (a.Val()+b.Val()); }


  template <typename T, size_t S>
  auto operator* (SIMD<T,S> a, SIMD<T,S> b) { return SIMD<T,S> (a.Lo()*b.Lo(), a.Hi()*b.Hi()); }
  template <typename T>
  auto operator* (SIMD<T,1> a, SIMD<T,1> b) { return SIMD<T,1> (a.Val()*b.Val()); }
  
  template <typename T, size_t S>
  auto operator* (double a, SIMD<T,S> b) { return SIMD<T,S> (a*b.Lo(), a*b.Hi()); }
  template <typename T>
  auto operator* (double a, SIMD<T,1> b) { return SIMD<T,1> (a*b.Val()); }

  template <typename T, size_t S>
  auto FMA(SIMD<T,S> a, SIMD<T,S> b, SIMD<T,S> c)
  { return SIMD<T,S> (FMA(a.Lo(),b.Lo(),c.Lo()), FMA(a.Hi(),b.Hi(),c.Hi())); }    
  template <typename T>
  auto FMA(SIMD<T,1> a, SIMD<T,1> b, SIMD<T,1> c)
  { return SIMD<T,1> (a.Val()*b.Val()+c.Val()); }
  


  
  template <typename T, size_t S, size_t first=0>
  class IndexSequence : public SIMD<T,S>
  {
    using SIMD<T,S>::S1;
    using SIMD<T,S>::S2;
  public:
    IndexSequence()
      : SIMD<T,S> (IndexSequence<T,S1,first>(),
                   IndexSequence<T,S2,first+S1>())
    { }
  };

  template <typename T, size_t first>
  class IndexSequence<T,1,first> : public SIMD<T,1>
  {
  public:
    IndexSequence() : SIMD<T,1> (first) { }
  };


  template <typename T, size_t S>
  auto operator>= (SIMD<T,S> a, SIMD<T,S> b)
  { return SIMD<mask64,S>(a.Lo()>=b.Lo(), a.Hi()>=b.Hi()); }

  template <typename T>
  auto operator>= (SIMD<T,1> a, SIMD<T,1> b)
  { return SIMD<mask64,1>(a.Val()>=b.Val()); }

  template <typename TA, typename T, size_t S>
  auto operator>= (TA a, const SIMD<T,S> & b)
  { return SIMD<T,S>(a) >= b; }
  
}
  
#endif
  
  
  
#ifdef __AVX__
#include "simd_avx.h"
#endif

#if defined(__aarch64__) || defined(_M_ARM64)
#include "simd_arm64.h"
#endif