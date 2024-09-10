#include"pch.h"
#include"FLOAT3.h"
FLOAT3::FLOAT3( float x, float y, float z ){
    this->x = x;    this->y = y;    this->z = z;
}
void FLOAT3::set(float x, float y, float z){
    this->x = x;    this->y = y;    this->z = z;
}
FLOAT3 FLOAT3::operator-( const FLOAT3& v ) const{
    return FLOAT3( x - v.x,  y - v.y,  z - v.z );
}
FLOAT3 FLOAT3::operator+( const FLOAT3& v ) const{
    return FLOAT3( x + v.x,  y + v.y,  z + v.z );
}
FLOAT3 FLOAT3::operator*( float f ) const{
    return FLOAT3( x * f, y * f, z * f );
}
FLOAT3 FLOAT3::operator/( float f ) const{
    return FLOAT3( x / f, y / f, z / f );
}
FLOAT3 FLOAT3::operator-() const{
    return FLOAT3( -x, -y, -z );
}
void FLOAT3::operator+=( const FLOAT3& v ){
    x += v.x;   y += v.y;   z += v.z;
}
void FLOAT3::operator-=( const FLOAT3& v ){
    x -= v.x;    y -= v.y;    z -= v.z;
}
void FLOAT3::operator*=( float f ){
    x *= f;    y *= f;    z *= f;
}
void FLOAT3::operator/=( float f ){
    x /= f;    y /= f;    z /= f;
}
float FLOAT3::sqMag() const{
    return x * x + y * y + z * z;
}
float FLOAT3::magSq() const{
    return x * x + y * y + z * z;
}
float FLOAT3::mag() const{
    return sqrtf(x * x + y * y + z * z);
}
FLOAT3 FLOAT3::setMag(float mag){
    float len = sqrtf(x * x + y * y + z * z);
    if (len > 0) {
        mag /= len;
        x *= mag;
        y *= mag;
        z *= mag;
    }
    return FLOAT3(x, y, z);
}
FLOAT3 FLOAT3::limmit(float maxMag) {
    float len = sqrtf(x * x + y * y + z * z);
    if (len > maxMag) {
        maxMag /= len;
        x *= maxMag; 
        y *= maxMag; 
        z *= maxMag;
    }
    return FLOAT3(x, y, z);
}
FLOAT3 FLOAT3::normalize(){
    float len = sqrtf(x * x + y * y + z * z);
    if (len > 0) {
        x /= len;
        y /= len;
        z /= len;
    }
    return FLOAT3(x, y, z);
}
float FLOAT3::dot(const FLOAT3& v) const{
    return x * v.x + y * v.y + z * v.z;
}
FLOAT3 FLOAT3::cross(const FLOAT3& v) const{
    return FLOAT3(
        y * v.z - z * v.y,
        z * v.x - x * v.z,
        x * v.y - y * v.x
    );
}
float FLOAT3::crossZ(const FLOAT3& v) const {
    return x * v.y - y * v.x;
}
float FLOAT3::crossY(const FLOAT3& v) const {
    return x * v.z - z * v.x;
}

//‚RŽŸŒ³ƒxƒNƒgƒ‹‚Ìˆê”ÊŠÖ”-------------------------------------------------------
FLOAT3 operator*(float f, const FLOAT3& v) {
    return FLOAT3(f * v.x, f * v.y, f * v.z);
}
//”ñ”j‰ónoramalize
FLOAT3 normalize( const FLOAT3& a ){
    float l = sqrtf( a.x * a.x + a.y * a.y + a.z * a.z );
    if (l > 0) {
        return FLOAT3( a.x / l, a.y / l, a.z / l);
    }
    return a;
}
float dot( const FLOAT3& a, const FLOAT3& b){
    return a.x * b.x + a.y * b.y + a.z * b.z;
}
FLOAT3 cross( const FLOAT3& a, const FLOAT3& b ){
    return FLOAT3(
        a.y * b.z - a.z * b.y,
        a.z * b.x - a.x * b.z,
        a.x * b.y - a.y * b.x
    );
}
float crossZ(const FLOAT3& a, const FLOAT3& b) {
    return a.x * b.y - a.y * b.x;
}
float crossY(const FLOAT3& a, const FLOAT3& b) {
    return a.x * b.z - a.z * b.x;
}
