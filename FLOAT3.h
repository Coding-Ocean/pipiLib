#pragma once
class FLOAT3{
public:
    float x, y, z;
    FLOAT3( float x=0, float y=0, float z=0 );
    void set(float x, float y, float z=0);
    FLOAT3 operator+( const FLOAT3& v ) const;
    FLOAT3 operator-( const FLOAT3& v ) const;
    FLOAT3 operator*( float f ) const;
    FLOAT3 operator/( float f ) const;
    FLOAT3 operator-() const;
    void operator+=( const FLOAT3& v );
    void operator-=( const FLOAT3& v );
    void operator*=( float f );
    void operator/=( float f );
    float sqMag() const;
    float magSq() const;//ã@î\ÇÕsqMagÇ∆ìØÇ∂
    float mag() const;
    FLOAT3 setMag(float mag);
    FLOAT3 limmit(float maxMag);
    FLOAT3 normalize();
    float dot(const FLOAT3& v)const;
    FLOAT3 cross(const FLOAT3& v)const;
    float crossZ(const FLOAT3& v)const;//äOêœZê¨ï™ÇÃÇ›åvéZ(ÇQÇcóp)
    float crossY(const FLOAT3& v)const;//äOêœYê¨ï™ÇÃÇ›åvéZ(ÇQÇcóp)
};

FLOAT3 operator* (float f, const FLOAT3& v);// FLOAT3 = float * FLOAT3
FLOAT3 normalize( const FLOAT3& a );
float dot( const FLOAT3& a, const FLOAT3& b );
FLOAT3 cross( const FLOAT3& a, const FLOAT3& b );
float crossZ(const FLOAT3& a, const FLOAT3& b);
float crossY(const FLOAT3& a, const FLOAT3& b);
