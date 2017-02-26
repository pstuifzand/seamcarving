#ifndef TBUFFER_HPP
#define TBUFFER_HPP

#include <algorithm>

template <typename B>
concept bool WriteBuffer = requires (B& b) {
    typename B::value_type;

    { height(b) } -> u32;
    { width(b) } -> u32;
    { pitch(b) } -> u32;

    { pixels(b) } -> typename B::value_type*;
};

template <typename B>
concept bool ReadBuffer = requires (const B& b) {
    typename B::value_type;

    { height(b) } -> u32;
    { width(b) } -> u32;
    { pitch(b) } -> u32;
    { pixels(b) } -> const typename B::value_type*;
};

template <typename T>
class buffer {
public:
     buffer() : data(0) {}
     buffer(u32 w, u32 h, u32 p, u32 s) :
         w(w), h(h), p(p), s(s), data(new T[h*p]) {
        auto first = data;
        auto last = data + p * h;
        std::fill(first, last, 1);
    }

    buffer(const buffer& b) :
        w(b.w), h(b.h), p(b.p), s(b.s), data(nullptr)
    {
        data = new T[h*p];
        std::copy_n(b.data, h*p, data);
    }

    buffer& operator=(const buffer& b)
    {
        T* tmp = new T[b.h*b.p];
        std::copy_n(b.data, b.h*b.p, tmp);
        w = b.w;
        h = b.h;
        p = b.p;
        s = b.s;
        data = tmp;
        return *this;
    }

    buffer(buffer&& b) : w(b.w), h(b.h), p(b.p), data(b.data) {
        b.data = 0;
    }
    buffer& operator=(buffer&& b) {
        w = b.w;
        h = b.h;
        p = b.p;
        data = b.data;
        return *this;
    }

    ~buffer() { delete [] data; }

    friend constexpr inline u32 height(const buffer& b) { return b.h; }
    friend constexpr inline u32 width(const buffer& b) { return b.w; }
    friend constexpr inline u32 pitch(const buffer& b) { return b.p; }
    friend constexpr inline void set_pitch(buffer& b, u32 pitch) { b.p=pitch; }
    friend constexpr inline void set_width(buffer& b, u32 width) { b.w=width; }
    friend constexpr inline u32 bpp(const buffer& b) { return b.s; }

public:
    typedef T value_type;
public:
    u32 w; // width
    u32 h; // height
    u32 p; // pitch
    u32 s; // bytes per pixel
    value_type* data;
};

template <typename U> inline
U* begin(buffer<U>& b) { return pixels(b); }

template <typename U> inline
U* end(buffer<U>& b) { return pixels(b) + (pitch(b)*height(b)); }

template <typename U> inline
U* pixels(buffer<U>& b) { return b.data; }

template <typename U> inline
const U* pixels(const buffer<U>& b) { return b.data; }


template <WriteBuffer B>          inline       typename B::value_type* row(B& b, u32 y) { return pixels(b) + y * pitch(b); }
template <ReadBuffer B> constexpr inline const typename B::value_type* row(const B& b, u32 y) { return pixels(b) + y * pitch(b); }

template <typename T>
class buffer_view {
public:
    buffer_view(buffer<T>& b, u32 x, u32 y, u32 w, u32 h)
        : w(w), h(h), p(pitch(b)), s(bpp(b)), pixels(row(b,y) + bpp(b) * x) {
    }

    buffer_view(T* pixels, u32 w, u32 h, u32 p, u32 s) :
        w(w), h(h), p(p), s(s), pixels(pixels) {}

    friend constexpr inline u32 height(const buffer_view& b) { return b.h; }
    friend constexpr inline u32 width(const buffer_view& b) { return b.w; }
    friend constexpr inline u32 pitch(const buffer_view& b) { return b.p; }
    friend constexpr inline u32 bpp(const buffer_view& b) { return b.s; }
    template <typename U> friend constexpr inline U* pixels(const buffer_view<U>& b) { return b.pixels; }

public:
    typedef T value_type;

private:
    u32 w;
    u32 h;
    u32 p;
    u32 s; // bytes per pixel
    u8* pixels;
};


#endif

