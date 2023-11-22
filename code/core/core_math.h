#ifndef CORE_MATH_H
#define CORE_MATH_H

#define PI 3.14159265359f
#define To_Radians(degrees) ((degrees) * (PI/180.0f)) 

union vec2 {
    f32 Data[2] = {};
    struct { f32 x, y; };

    inline vec2() { }
    inline vec2(f32 _x, f32 _y) : x(_x), y(_y) { }

    inline vec2 operator*(f32 S) const {
        return vec2(x*S, y*S);
    }

    static inline f32 Dot(const vec2& A, const vec2& B) {
        return A.x*B.x + A.y*B.y;
    }

    inline f32 Sq_Mag() const {
        return vec2::Dot(*this, *this);
    }

    inline vec2 Normalize() const {
        f32 SqLength = Sq_Mag();
        if(Equal_Zero_Eps_Sq(SqLength)) {
            return vec2();
        }

        f32 InvLength = 1.0f/Sqrt(SqLength);
        return *this * InvLength;
    }
};

union vec3 {
    f32 Data[3] = {};
    struct { f32 x, y, z; };
    struct { vec2 xy; f32 __unused0__; };

    inline vec3() { }
    inline vec3(f32 _x, f32 _y, f32 _z) : x(_x), y(_y), z(_z) { }
    inline vec3(vec2 _xy, f32 _z = 0.0f) : x(_xy.x), y(_xy.y), z(_z) { }

    inline vec3 operator*(f32 S) const {
        return vec3(x*S, y*S, z*S);
    }

    inline vec3& operator*=(f32 S) {
        x *= S;
        y *= S;
        z *= S;
        return *this;
    }

    inline vec3 operator+(const vec3& Other) const {
        return vec3(x+Other.x, y+Other.y, z+Other.z);
    }

    inline vec3& operator+=(const vec3& Other) {
        x += Other.x;
        y += Other.y;
        z += Other.z;
        return *this;
    }

    static inline f32 Dot(const vec3& A, const vec3& B) {
        return A.x*B.x + A.y*B.y + A.z*B.z;
    }

    static inline vec3 Cross(const vec3& A, const vec3& B) {
        return vec3(A.y*B.z - A.z*B.y, A.z*B.x - A.x*B.z, A.x*B.y - A.y*B.x);
    }
};

union vec4 {
    f32 Data[4] = {};
    struct { f32 x, y, z, w; };
    struct { vec3 xyz; f32 __unused0__; };

    inline vec4() { }
    inline vec4(f32 _x, f32 _y, f32 _z, f32 _w) : x(_x), y(_y), z(_z), w(_w) { }
};

union quat {
    f32 Data[4] = {0, 0, 0, 1};
    struct { f32 x, y, z, w; };
    struct { vec3 v; f32 s; };

    inline quat() {}
    inline quat(f32 _x, f32 _y, f32 _z, f32 _w) : x(_x), y(_y), z(_z), w(_w) { }
    inline quat(const vec3& _v, f32 _s) : x(_v.x), y(_v.y), z(_v.z), w(_s) { }

    inline quat operator*(f32 S) const {
        return quat(x*S, y*S, z*S, w*S);
    }

    inline quat operator*(const quat& Other) const {
        return quat(vec3::Cross(v, Other.v) + v*Other.s + Other.v*s, s*Other.s - vec3::Dot(v, Other.v));
    }

    static inline f32 Dot(const quat& A, const quat& B) {
        return A.x*B.x + A.y*B.y + A.z*B.z + A.w*B.w;
    }

    inline f32 Sq_Mag() const {
        return quat::Dot(*this, *this);
    }

    inline quat Normalize() const {
        f32 SqLength = Sq_Mag();
        if(Equal_Zero_Eps_Sq(SqLength)) {
            return quat();
        }

        f32 InvLength = 1.0f/Sqrt(SqLength);
        return *this * InvLength;
    }

    static inline quat RotateX(f32 Angle) {
        Angle *= 0.5f;
        return quat(Sin(Angle), 0.0f, 0.0f, Cos(Angle));
    }

    static inline quat RotateY(f32 Angle) {
        Angle *= 0.5f;
        return quat(0.0f, Sin(Angle), 0.0f, Cos(Angle));
    }

    static inline quat RotateZ(f32 Angle) {
        Angle *= 0.5f;
        return quat(0.0f, 0.0f, Sin(Angle), Cos(Angle));
    }

    static inline quat Normalize(const quat& Q) {
        return Q.Normalize();
    }
};

union matrix3 {
    f32 Data[9] = {
        1, 0, 0, 
        0, 1, 0, 
        0, 0, 1
    };

    vec3 Rows[3];
    struct {
        vec3 x;
        vec3 y;
        vec3 z;
    };

    struct {
        f32 m00, m01, m02;
        f32 m10, m11, m12;
        f32 m20, m21, m22;
    };

    inline matrix3() { }

    inline matrix3(const quat& Q) {
        f32 qxqy = Q.x*Q.y;
        f32 qwqz = Q.w*Q.z;
        f32 qxqz = Q.x*Q.z;
        f32 qwqy = Q.w*Q.y;
        f32 qyqz = Q.y*Q.z;
        f32 qwqx = Q.w*Q.x;
        
        f32 qxqx = Q.x*Q.x;
        f32 qyqy = Q.y*Q.y;
        f32 qzqz = Q.z*Q.z;

        Rows[0] = vec3(1 - 2*(qyqy+qzqz), 2*(qxqy+qwqz),     2*(qxqz-qwqy));
        Rows[1] = vec3(2*(qxqy-qwqz),     1 - 2*(qxqx+qzqz), 2*(qyqz+qwqx)); 
        Rows[2] = vec3(2*(qxqz+qwqy),     2*(qyqz-qwqx),     1 - 2*(qxqx+qyqy));
    }
};

union matrix4_affine {
    f32  Data[12] = {
        1, 0, 0, 
        0, 1, 0, 
        0, 0, 1, 
        0, 0, 0
    };

    vec3 Rows[4];
    struct {
        vec3 x;
        vec3 y;
        vec3 z;
        vec3 t;
    };

    struct {
        f32 m00, m01, m02;
        f32 m10, m11, m12;
        f32 m20, m21, m22;
        f32 m30, m31, m32;
    };

    inline matrix4_affine() {}

    inline matrix4_affine(const vec3& _t) {
        t = _t;
    }

    inline matrix4_affine(const vec3& _t, const vec3& s) {
        *this = {
            s.x, 0, 0, 
            0, s.y, 0, 
            0, 0, s.z, 
            _t.x, _t.y, _t.z
        };
    }

    inline matrix4_affine(std::initializer_list<f32> List) {
        Assert(List.size() <= 12, "Invalid size!");
        Memory_Copy(Data, List.begin(), List.size()*sizeof(f32));
    }

    inline matrix4_affine Transpose() const {
        matrix4_affine Result;

        Result.Data[0]  = m00;
        Result.Data[1]  = m10;
        Result.Data[2]  = m20;
        Result.Data[3]  = m30;

        Result.Data[4]  = m01;
        Result.Data[5]  = m11;
        Result.Data[6]  = m21;
        Result.Data[7]  = m31;
        
        Result.Data[8]  = m02;
        Result.Data[9]  = m12;
        Result.Data[10] = m22;
        Result.Data[11] = m32;

        return Result;
    }

    static inline matrix4_affine Inverse(const vec3& P, const matrix3& Q) {
        f32 tx = -vec3::Dot(P, Q.x);
        f32 ty = -vec3::Dot(P, Q.y);
        f32 tz = -vec3::Dot(P, Q.z);

        return {
            Q.m00, Q.m10, Q.m20,
            Q.m01, Q.m11, Q.m21,
            Q.m02, Q.m12, Q.m22,
            tx, ty, tz
        };
    }
};

union matrix4 {
    f32 Data[16] = {
        1, 0, 0, 0, 
        0, 1, 0, 0, 
        0, 0, 1, 0,
        0, 0, 0, 1
    };
    vec4 Rows[4];
    struct {
        vec3 x; f32 __unused0__;
        vec3 y; f32 __unused1__;
        vec3 z; f32 __unused2__;
        vec3 t; f32 __unused3__;
    };

    struct {
        f32 m00, m01, m02, m03;
        f32 m10, m11, m12, m13;
        f32 m20, m21, m22, m23;
        f32 m30, m31, m32, m33;
    };

    inline matrix4() { }

    inline f32& operator[](u32 Index) {
        Assert(Index < 16, "Invalid index!");
        return Data[Index];
    }

    inline matrix4 Transpose() const {
        matrix4 Result;

        Result.Data[0]  = m00;
        Result.Data[1]  = m10;
        Result.Data[2]  = m20;
        Result.Data[3]  = m30;

        Result.Data[4]  = m01;
        Result.Data[5]  = m11;
        Result.Data[6]  = m21;
        Result.Data[7]  = m31;
        
        Result.Data[8]  = m02;
        Result.Data[9]  = m12;
        Result.Data[10] = m22;
        Result.Data[11] = m32;

        Result.Data[12] = m03;
        Result.Data[13] = m13;
        Result.Data[14] = m23;
        Result.Data[15] = m33;

        return Result;
    }

    static inline matrix4 Zero() {
        matrix4 Result;
        Memory_Clear(&Result, sizeof(Result));
        return Result;
    }
};

inline matrix4 operator*(const matrix4_affine& A, const matrix4& B) {
    matrix4 Result;

    matrix4 BTransposed = B.Transpose();

    Result.Data[0]  = vec3::Dot(A.Rows[0], BTransposed.Rows[0].xyz);
    Result.Data[1]  = vec3::Dot(A.Rows[0], BTransposed.Rows[1].xyz);
    Result.Data[2]  = vec3::Dot(A.Rows[0], BTransposed.Rows[2].xyz);
    Result.Data[3]  = vec3::Dot(A.Rows[0], BTransposed.Rows[3].xyz);

    Result.Data[4]  = vec3::Dot(A.Rows[1], BTransposed.Rows[0].xyz);
    Result.Data[5]  = vec3::Dot(A.Rows[1], BTransposed.Rows[1].xyz);
    Result.Data[6]  = vec3::Dot(A.Rows[1], BTransposed.Rows[2].xyz);
    Result.Data[7]  = vec3::Dot(A.Rows[1], BTransposed.Rows[3].xyz);

    Result.Data[8]  = vec3::Dot(A.Rows[2], BTransposed.Rows[0].xyz);
    Result.Data[9]  = vec3::Dot(A.Rows[2], BTransposed.Rows[1].xyz);
    Result.Data[10] = vec3::Dot(A.Rows[2], BTransposed.Rows[2].xyz);
    Result.Data[11] = vec3::Dot(A.Rows[2], BTransposed.Rows[3].xyz);

    Result.Data[12] = vec3::Dot(A.Rows[3], BTransposed.Rows[0].xyz) + BTransposed.Rows[0].w;
    Result.Data[13] = vec3::Dot(A.Rows[3], BTransposed.Rows[1].xyz) + BTransposed.Rows[1].w;
    Result.Data[14] = vec3::Dot(A.Rows[3], BTransposed.Rows[2].xyz) + BTransposed.Rows[2].w;
    Result.Data[15] = vec3::Dot(A.Rows[3], BTransposed.Rows[3].xyz) + BTransposed.Rows[3].w;

    return Result;
}

struct range_u32 {
    u32 Offset;
    u32 Count;
};

#endif