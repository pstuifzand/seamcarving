
#define STB_IMAGE_IMPLEMENTATION
#include "../stb/stb_image.h"
#include <iostream>
#include <cassert>
#include <limits>
#include <queue>
#include <unordered_map>
#include <utility>

typedef uint32_t u32;
typedef int32_t  s32;
typedef uint32_t b32;
typedef uint8_t u8;
typedef float f32;

const int WINDOW_WIDTH = 1920;
const int WINDOW_HEIGHT = 1080;

#include <SDL.h>
#include "math.hpp"

//#include "buffer.hpp"
#include "tbuffer.hpp"

int load_image(buffer<u8>& ret, const char* filename)
{
    int x, y, cif, dc=0;

    stbi_uc* image;
    image = stbi_load(filename, &x, &y, &cif, dc);
    if (!image) {
        std::cout << "ERROR\n";
        return 1;
    }

    std::cout << "x:" << x << ", y: " << y << ", cif: " << cif << "\n";

    buffer<u8> b{x, y, x*cif, cif};

    size_t buffer_size = (size_t)(pitch(b)*height(b));
    std::cout << "buffer_size: " << buffer_size << "\n";
    std::copy_n((u8*)image, buffer_size, row(b, u32(0)));

    stbi_image_free(image);

    ret = b;

    return 0;
}

struct game_memory {
    float time;
    int running;
    int remove;
    rect2 viewport;
    v2 scale;
    buffer<u8> original;
    buffer<f32> edges;
    buffer<u8> choice;
    u32 last_x;
};

// should return sum of squares of differences between channels
// (R(p1) - R(p0))^2 + (G(p1) - G(p0))^2 + (B(p1) - B(p0))^2
f32 edge(u8 x, u8 y)
{
    f32 a = (f32)x;
    f32 b = (f32)y;
    return (b-a) * (b-a);
}

template <typename T>
int smallest(T a, T b, T c)
{
    if (b < a) {
        if (c < b) {
            return 2;
        } else {
            return 1;
        }
    } else {
        if (c < a) {
            return 2;
        } else {
            return 0;
        }
    }
}

template <typename I>
struct energy {
    int n;
    energy(int n) : n(n) {}
    f32 operator()(I f0, I f1, I f2)
    {
        f32 x = 0.0f;
        for (int i = 0; i < n; ++i) {
            x += edge(*(f0+i), *(f2+i));
        }
        return x;
    }
};

template <typename I, typename B, typename N, typename O, typename Op>
O op_row3_n(I f0, I f1, I f2, B f_s, N n, O out, B o_s, Op op)
{
    while (n--) {
        *out += op(f0, f1, f2);
        f0 += f_s;
        f1 += f_s;
        f2 += f_s;
        out += o_s;
    }

    return out;
}

template <typename T>
void edge_detect_h(const buffer<T>& in, buffer<f32>& out)
{
    // edge detect
    // energy differentiation

    const u8* src0 = row(in, 0);
    const u8* src1 = row(in, 1);
    const u8* src2 = row(in, 2);
    f32* dst = row(out, 1);

    assert(height(in) > 2);
    assert(width(in) > 2);

    const u32 h = height(in) - 2;

    for (u32 y = 1; y < h; ++y) {
        std::fill(dst, dst+pitch(out), 0);
        dst = op_row3_n(src0, src1, src2, bpp(in), width(in), dst, bpp(out), energy<const u8*>{bpp(in)});

        src0 = src1;
        src1 = src2;
        src2 = row(in, y+1);

        dst += pitch(out) - width(out);
    }

}
template <typename T>
void edge_detect_w(const buffer<T>& in, buffer<f32>& out)
{
    // edge detect
    // energy differentiation

    const u8* src0 = row(in, 0);
    const u8* src1 = row(in, 0)+1;
    const u8* src2 = row(in, 0)+2;

    const u32 w = width(in) - 2;

    for (u32 x = 1; x < w; ++x) {
        f32* dst = row(out, 0) + (x+1) * bpp(out);

        op_row3_n(src0, src1, src2, pitch(in), height(in), dst, pitch(out), energy<const u8*>{bpp(in)});

        src0 = src1;
        src1 = src2;
        src2 = row(in, 0) + (x+1) * bpp(in);
    }
}

template <typename T>
void edge_detect(const buffer<T>& in, buffer<f32>& out)
{
    // edge detect
    // energy differentiation
    edge_detect_h(in, out);
    edge_detect_w(in, out);
}

void find_path(const buffer<f32>& energies, const buffer<u8>& choice, u32 x, f32* sum)
{
    u32 y = 0;

    while (y < height(energies)) {
        const f32* e_row = row(energies, y);
        const u8* c_row = row(choice, y);
        *sum += e_row[x];

        // next x;
        if (x == 0 && c_row[x] == 0) x = 0;
        else if (x == width(choice)-1 && c_row[x] == 2) x = width(choice)-1;
        else {
            x += c_row[x];
            x--;
        }

        // next y
        y++;
    }
}

void remove_path(buffer<u8>& image, const buffer<u8>& choice, u32 x)
{
    u32 y = 0;

    while (y < height(image)) {
        u8* i_row = row(image, y);
        const u8* c_row = row(choice, y);

        // copy row
        auto f0 = i_row + x * bpp(image);
        auto f1 = i_row + (x + 1) * bpp(image);
        auto l  = i_row + pitch(image);

        std::copy(f1, l, f0);

        //assert(c_row[x] == 0 || c_row[x] == 1 || c_row[x] == 2);

        // next x;
        if (x == 0 && c_row[x] == 0) x = 0;
        else if (x == width(choice)-1 && c_row[x] == 2) x = width(choice)-1;
        else {
            x += c_row[x];
            x--;
        }

        // next y
        y++;
    }
}

void calculate_paths(const buffer<f32>& in, buffer<u8>& out)
{
    // path of smallest energy
    for (u32 y = 1; y < height(in)-2; ++y) {
        const f32* src0 = row(in, y-1);
        u8* dst = row(out, y) + 1;

        for (u32 x = 1; x < width(out)-2; ++x) {
            u8 v = smallest(*src0, *(src0+1), *(src0+2));
            assert(v == 0 || v == 1 || v == 2);
            *dst = v; // 0, 1, 2
            src0+=bpp(in);
            dst+=bpp(out);
        }
    }
}

u32 find_minimum_path(const buffer<f32>& in, buffer<u8>& out)
{
    buffer<float> sum{width(in), 1, width(in), 1};

    for (u32 x = 0; x < width(out); ++x) {
        find_path(in, out, x, &row(sum, 0)[x]);
    }

    auto f = row(sum, 0);
    auto l = row(sum, 0) + width(sum);

    auto m = std::min_element(f, l);
    u32 x = m - f;
    return x;
}

void GameUpdateAndRender(game_memory* memory, float delta, buffer<u8>& screen)
{
    auto& edges = memory->edges;
    auto& choice = memory->choice;

    if (memory->remove >= 0) {
        std::fill(begin(edges), end(edges), 0);

        edge_detect(memory->original, edges);

        u32 y=0;
        while (y < height(edges)) {
            row(edges,y++)[0] = std::numeric_limits<float>::max();
            row(edges,y++)[width(edges)] = std::numeric_limits<float>::max();
        }

        calculate_paths(edges, choice);
        u32 x = find_minimum_path(edges, choice);
        remove_path(memory->original, choice, x);

        // decrease width of images (pitch stays the same)
        set_width(memory->original, width(memory->original)-1);
        set_width(edges, width(edges)-1);
        set_width(choice, width(choice)-1);

        memory->remove--;
    }

    for (u32 y = 0; y < height(screen); ++y) {
        u8* dst = row(screen, y);
        for (u32 x = 0; x < width(screen); ++x) {
            *(dst) = 0;
            *(dst+1) = 0;
            *(dst+2) = 0;
            *(dst+3) = 0;
            dst += bpp(screen);
        }
    }

    // draw
    for (u32 y = 0; y < std::min(height(memory->original), height(screen)); ++y) {
        const u8* src = row(memory->original, y);
        u8* dst = row(screen, y);
        for (u32 x = 0; x < std::min(width(memory->original), width(screen)); ++x) {

            *(dst)   = *(src+2);
            *(dst+1) = *(src+1);
            *(dst+2) = *(src+0);
            *(dst+3) = 255;

            /* u8 c = *src; */
            /* u8 g = 0; */

            /* if (c == 0) g = 0; */
            /* if (c == 1) g = 128; */
            /* if (c == 2) g = 255; */

            /* *(dst)   = g; */
            /* *(dst+1) = g; */
            /* *(dst+2) = g; */

            /* *(dst+3) = 255; */

            src += bpp(memory->original);
            dst += bpp(screen);
        }
    }
}

int main(int argc, const char* argv[])
{
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
        fprintf(stderr, "Unable to initialize SDL:  %s\n", SDL_GetError());
        return 1;
    }

    SDL_Window *window;
    SDL_Renderer *renderer;

    // Create an application window with the following settings:
    window = SDL_CreateWindow(
        "Animator",                        // window title
        SDL_WINDOWPOS_UNDEFINED,           // initial x position
        SDL_WINDOWPOS_UNDEFINED,           // initial y position
        WINDOW_WIDTH,                      // width, in pixels
        WINDOW_HEIGHT,                     // height, in pixels
        SDL_WINDOW_FULLSCREEN_DESKTOP      // flags - see below
    );

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED|SDL_RENDERER_PRESENTVSYNC);

    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");  // make the scaled rendering look smoother.
    SDL_RenderSetLogicalSize(renderer, WINDOW_WIDTH, WINDOW_HEIGHT);

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
    SDL_RenderPresent(renderer);

    int key_count = 0;
    const u8* keyboardState = SDL_GetKeyboardState(&key_count);

    SDL_Texture* texture = SDL_CreateTexture(renderer,
            SDL_PIXELFORMAT_ARGB8888,
            SDL_TEXTUREACCESS_STREAMING,
            WINDOW_WIDTH, WINDOW_HEIGHT);

    u32 screen_pitch = WINDOW_WIDTH * sizeof(u32);
    buffer<u8> screen{WINDOW_WIDTH, WINDOW_HEIGHT, screen_pitch, 4};

    float frameRate = 60.0f;
    float frameMs = 1.0f / frameRate;

    u32 start = SDL_GetTicks();

    game_memory memory;
    memory.time = 0.0f;
    memory.running = 1;
    memory.remove = 0;
    memory.last_x = 0;

    SDL_Rect w;
    SDL_RenderGetViewport(renderer, &w);
    memory.viewport.min.x = w.x;
    memory.viewport.min.y = w.y;
    memory.viewport.max.x = w.w + w.x;
    memory.viewport.max.y = w.h + w.y;
    SDL_RenderGetScale(renderer, &memory.scale.x, &memory.scale.y);

    printf("Viewport: %f %f %f %f\n",
            memory.viewport.min.x, memory.viewport.min.y,
            memory.viewport.max.x, memory.viewport.max.y);
    printf("Scale: %f %f\n", memory.scale.x, memory.scale.y);

    v2 a = recip(memory.scale);
    matrix<f32,2,2> scale_inv = {a.x,0,0,a.y};

    v2 b = memory.scale;
    matrix<f32,2,2> scale = {b.x,0,0,b.y};

    v2 ul = scale*memory.viewport.min;
    v2 dr = scale*memory.viewport.max;

    printf("Viewport: %f %f %f %f\n", ul.x, ul.y, dr.x, dr.y);

    const char* filename = argv[1];
    load_image(memory.original, filename);

    buffer<f32> edges{
        width(memory.original),
        height(memory.original),
        1*width(memory.original),
        1
    };
    memory.edges = edges;

    buffer<u8> choice{
        width(memory.edges),
        height(memory.edges),
        1 * width(memory.edges),
        1,
    };

    memory.choice = choice;

    while (memory.running) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_QUIT:
                    /* Quit */
                    memory.running = 0;
                    break;
            }
        }

        if (keyboardState[SDL_SCANCODE_ESCAPE]) {
            memory.running = 0;
        }
        if (keyboardState[SDL_SCANCODE_R]) {
            memory.remove = 50;
        }

        int mouse_x;
        int mouse_y;
        /*u32 mouse_mask = */SDL_GetMouseState(&mouse_x, &mouse_y);

        v2 pos = {(float)mouse_x, (float)mouse_y};
        pos = scale_inv * pos - memory.viewport.min;

        u32 now = SDL_GetTicks();
        f32 delta = (float)(now-start)/1000.0f;
        start = now;

        GameUpdateAndRender(&memory, delta, screen);

        SDL_UpdateTexture(texture, NULL, pixels(screen), pitch(screen));
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, texture, 0, 0);
        SDL_RenderPresent(renderer);

        now = SDL_GetTicks();
        delta = (float)(now-start)/1000.0f;
        float frameDelta = frameMs - delta;
        if (frameDelta > 0) {
            SDL_Delay((u32)(frameDelta*1000.0f));
        }
    }

    SDL_DestroyWindow(window);

    SDL_Quit();

    return 0;
}
