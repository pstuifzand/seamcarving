
template <typename F>
concept bool Field = requires (F x, F y) {
    { x + y } -> F;
    { x * y } -> F;
};

template <typename V, typename F>
concept bool VectorSpace = requires (V x, V y) {
    { x + y } -> V;
    { x * y } -> V;
};

struct v2 {
    v2() {} // partially formed
    v2(s32 x, s32 y) : x((float)x), y((float)y) {}
    v2(float x, float y) : x(x), y(y) {}
    union {
        struct { float x, y; };
        struct { float u, v; };
        struct { float s, t; };
        float e[2];
    };

    inline
    v2& operator+=(const v2& a)
    {
        *this = *this + a;
        return *this;
    }

    inline
    v2& operator-=(const v2& a)
    {
        *this = *this + -a;
        return *this;
    }

    friend
    inline
    v2 operator+(const v2& a, const v2& b)
    {
        return v2(a.x + b.x, a.y + b.y);
    }

    friend
    inline
    v2 operator-(const v2& a)
    {
        return v2(-a.x, -a.y);
    }

};

static
inline
v2 operator-(const v2& a, const v2& b)
{
    return v2(a.x - b.x, a.y - b.y);
}

static
inline
f32 inner(const v2& a, const v2& b)
{
    return a.x * b.x + a.y * b.y;
}

static
inline
v2 operator*(f32 s, const v2& a)
{
    return v2(s * a.x, s * a.y);
}

static
inline
v2 operator*(const v2& a, f32 s)
{
    return s * a;
}

static
inline
v2 recip(const v2& a)
{
    return v2{ 1 / a.x, 1 / a.y};
}

struct v2i {
    v2i() {}
    v2i(int x, int y) : x(x), y(y) {}
    union {
        struct {
            int x, y;
        };
        int e[2];
    };
};

static inline
v2i operator+(const v2i& a, const v2i& b)
{
    return v2i(a.x + b.x, a.y + b.y);
}

static inline
v2i operator-(const v2i& a, const v2i& b)
{
    return v2i(a.x - b.x, a.y - b.y);
}

static inline
int inner(const v2i& a, const v2i& b)
{
    return a.x * b.x + a.y * b.y;
}

static inline
v2i operator*(int s, const v2i& a)
{
    return v2i(s * a.x, s * a.y);
}

static inline
v2i operator*(const v2i& a, int s)
{
    return s * a;
}

static inline
v2i operator*(const v2i& a, const v2i& b)
{
    return v2i(a.x * b.x, a.y * b.y);
}

struct v3 {
    v3() {}
    v3(f32 x, f32 y, f32 z) : x(x), y(y), z(z) {}

    f32 x, y, z;
};

inline
v3 operator+(const v3& a, const v3& b)
{
    return v3(a.x + b.x, a.y + b.y, a.z + b.z);
}

inline
v3 operator-(const v3& a, const v3& b)
{
    return v3(a.x - b.x, a.y - b.y, a.z - b.z);
}

inline
f32 inner(const v3& a, const v3& b)
{
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

inline
v3 operator*(f32 s, const v3& a)
{
    return v3(s * a.x, s * a.y, s * a.z);
}

inline
v3 operator*(const v3& a, f32 s)
{
    return s * a;
}

inline
v3 operator*(const v3& a, const v3& b)
{
    return v3(a.x * b.x, a.y * b.y, a.z * b.z);
}

struct v3i {
    v3i() {}
    v3i(int x, int y, int z) : x(x), y(y), z(z) {}
    union {
        struct {
            int x, y, z;
        };
        struct {
            int r, g, b;
        };
        int e[3];
    };
};

inline
v3i operator+(const v3i& a, const v3i& b)
{
    return v3i(a.x + b.x, a.y + b.y, a.z + b.z);
}

inline
v3i operator-(const v3i& a, const v3i& b)
{
    return v3i(a.x - b.x, a.y - b.y, a.z - b.z);
}

inline
int inner(const v3i& a, const v3i& b)
{
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

inline
v3i operator*(int s, const v3i& a)
{
    return v3i(s * a.x, s * a.y, s * a.z);
}

inline
v3i operator*(const v3i& a, int s)
{
    return s * a;
}

inline
v3i operator*(const v3i& a, const v3i& b)
{
    return v3i(a.x * b.x, a.y * b.y, a.z * b.z);
}

struct v3u8 {
    u8 r, g, b;
};

struct v4 {
    v4() {}
    v4(f32 x, f32 y, f32 z, f32 w) :
        x(x), y(y), z(z), w(w) {}
    union {
        struct {
            f32 x, y, z, w;
        };
        struct {
            f32 r, g, b, a;
        };
    };
};

inline
v4 operator+(const v4& a, const v4& b)
{
    return v4(a.x + b.x, a.y + b.y, a.z + b.z, a.z + b.z);
}

inline
v4 operator-(const v4& a, const v4& b)
{
    return v4(a.x - b.x, a.y - b.y, a.z - b.z, a.z - b.z);
}

inline
f32 inner(const v4& a, const v4& b)
{
    return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
}

inline
v4 operator*(f32 s, const v4& a)
{
    return v4(s * a.x, s * a.y, s * a.z, s * a.w);
}

inline
v4 operator*(const v4& a, f32 s)
{
    return s * a;
}

inline
v4 operator*(const v4& a, const v4& b)
{
    return v4(a.x * b.x, a.y * b.y, a.z * b.z, a.w * b.w);
}

template <Field F, typename V>
inline
V Blend(V a, F r, V b)
{
    return (1.0f - r) * a + r * b;
}

struct rect2
{
    rect2() {} // partially formed
    rect2(v2 a, v2 b) : min(a), max(b) {}
    rect2(float x1, float y1, float x2, float y2) : min(x1,y1), max(x2,y2) {}
    v2 min;
    v2 max;
};

inline
rect2 RectCenterRadius(v2 center, v2 radius)
{
    return rect2{center-radius, center+radius};
}

template <typename T, int n, int m>
struct matrix {
    T d[n][m];
    matrix() {}
    matrix(std::initializer_list<T> l) {
        auto p = l.begin();
        for (int j = 0; j < m; ++j) {
            for (int i = 0; i < n; ++i) {
                d[j][i] = *p;
                ++p;
            }
        }
    }

    matrix(int i) {
        if(i==0) zero(*this);
        if(i==1) identity(*this);
    }
};

template <typename T, int m, int n, int l>
matrix<T, n, l>
operator*(const matrix<T,m,n>& a, const matrix<T,n,l>& b)
{
    matrix<T, m, l> r;

    for (int j = 0; j < m; ++j) {
        for (int i = 0; i < n; ++i) {
            for (int k = 0; k < l; ++k) {
                r.d[j][i] += a.d[k][i] * b.d[j][k];
            }
        }
    }

    return r;
}

template <typename T, int m, int n>
static inline
matrix<T, m, n>
operator*(T s, const matrix<T, m, n>& a)
{
    matrix<T, m, n> r;

    for (int j = 0; j < m; ++j) {
        for (int i = 0; i < n; ++i) {
            r.d[j][i] = s * a.d[j][i];
        }
    }

    return r;
}

static inline
v2
operator*(const matrix<f32, 2, 2>& a, const v2& b)
{
    v2 r;
    r.x = a.d[0][0] * b.x + a.d[0][1] * b.y;
    r.y = a.d[1][0] * b.x + a.d[1][1] * b.y;
    return r;
}

template <typename T, int m, int n>
matrix<T, m, n>
operator+(const matrix<T,m,n>& a, const matrix<T,m,n>& b)
{
    matrix<T, m, n> r;

    for (int j = 0; j < m; ++j) {
        for (int i = 0; i < n; ++i) {
            r.d[i][j] = a.d[i][j] + b.d[i][j];
        }
    }

    return r;
}

template <typename T, int m, int n>
void
zero(matrix<T,m,n>& a)
{
    for (int j = 0; j < m; ++j) {
        for (int i = 0; i < n; ++i) {
            a.d[i][j] = T(0);
        }
    }
}

template <typename T, int m, int n>
void
identity(matrix<T,m,n>& a)
{
    zero(a);
    for (int i = 0; i < std::min(m, n); ++i) {
        a.d[i][i] = T(1);
    }
}


