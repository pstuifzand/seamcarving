#ifndef BUFFER_HPP
#define BUFFER_HPP

#include <algorithm>

typedef uint32_t u32;
typedef int32_t  s32;
typedef uint32_t b32;
typedef uint8_t u8;
typedef float f32;

const int WINDOW_WIDTH = 1920;
const int WINDOW_HEIGHT = 1080;

template <typename B>
concept bool WriteBuffer = requires (B& b) {
    { height(b) } -> u32;
    { width(b) } -> u32;
    { pitch(b) } -> u32;
    { pixels(b) } -> u8*;
};

template <typename B>
concept bool ReadBuffer = requires (const B& b) {
    { height(b) } -> u32;
    { width(b) } -> u32;
    { pitch(b) } -> u32;
    { pixels(b) } -> const u8*;
};

class buffer {
public:
     buffer() : pixels(0) {}
     buffer(u32 w, u32 h, u32 p, u32 s) :
         w(w), h(h), p(p), s(s), pixels(new u8[h*p]) {
        auto first = pixels;
        auto last = pixels + p * h;
        std::fill(first, last, 255);
    }

    buffer(const buffer& b) :
        w(b.w), h(b.h), p(b.p), s(b.s), pixels(nullptr)
    {
        pixels = new u8[h*p];
        std::copy_n(b.pixels, h*p, pixels);
    }

    buffer& operator=(const buffer& b)
    {
        u8* tmp = new u8[b.h*b.p];
        std::copy_n(b.pixels, b.h*b.p, tmp);
        w = b.w;
        h = b.h;
        p = b.p;
        s = b.s;
        pixels = tmp;
        return *this;
    }

    buffer(buffer&& b) : w(b.w), h(b.h), p(b.p), pixels(b.pixels) {
        b.pixels = 0;
    }
    buffer& operator=(buffer&& b) {
        w = b.w;
        h = b.h;
        p = b.p;
        pixels = b.pixels;
        return *this;
    }

    ~buffer() { delete [] pixels; }

    friend constexpr inline u32 height(const buffer& b) { return b.h; }
    friend constexpr inline u32 width(const buffer& b) { return b.w; }
    friend constexpr inline u32 pitch(const buffer& b) { return b.p; }
    friend constexpr inline u32 bpp(const buffer& b) { return b.s; }
    friend constexpr inline u8* pixels(buffer& b) { return b.pixels; }
private:
    u32 w; // width
    u32 h; // height
    u32 p; // pitch
    u32 s; // bytes per pixel
    u8* pixels;
};

template <WriteBuffer B>
constexpr inline u8* row(B& b, u32 y) {
    return pixels(b) + y * pitch(b);
}
template <ReadBuffer B>
constexpr inline const u8* row(const B& b, u32 y) {
    return pixels(b) + y * pitch(b);
}

class buffer_view {
public:
    buffer_view(buffer& b, u32 x, u32 y, u32 w, u32 h)
        : w(w), h(h), p(pitch(b)), s(bpp(b)), pixels(row(b,y) + bpp(b) * x) {
    }

    buffer_view(u8* pixels, u32 w, u32 h, u32 p, u32 s) :
        w(w), h(h), p(p), s(s), pixels(pixels) {}

    friend constexpr inline u32 height(const buffer_view& b) { return b.h; }
    friend constexpr inline u32 width(const buffer_view& b) { return b.w; }
    friend constexpr inline u32 pitch(const buffer_view& b) { return b.p; }
    friend constexpr inline u32 bpp(const buffer_view& b) { return b.s; }
    friend constexpr inline u8* pixels(const buffer_view& b) { return b.pixels; }

private:
    u32 w;
    u32 h;
    u32 p;
    u32 s; // bytes per pixel
    u8* pixels;
};


#endif
