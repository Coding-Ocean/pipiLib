#pragma once
#pragma once
void createVtxSquare(float l = 0.5f);
void createIdxSquare();
void createVtxCube(float length = 0.5f);
void createIdxCube();
void createVtxSphere(float radius = 0.5f, int numCorners = 30);
void createIdxSphere(int numCorners = 30);
void createVtxCylinder(float radius = 0.5f, int numCorners = 30, float low = -0.5f, float high = 0.5f);
void createVtxCylinderAxisX(float radius = 0.5f, int numCorners = 30, float low = -0.5f, float high = 0.5f);
void createIdxCylinder(int numCorners = 30);
void createVtxCircle(float radius = 0.5f, int numCorners = 30);
