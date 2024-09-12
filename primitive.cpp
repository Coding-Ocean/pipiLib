#include"pch.h"
#include<cmath>
#include<vector>
#include<fstream>
#include"primitive.h"

struct VERTEX {
	float x = 0, y = 0, z = 0; // 頂点座標 
	float nx = 0, ny = 0, nz = -1;//法線ベクトル
	float u = 0, v = 0;    //　テクスチャ座標
};

void createVtxSquare(float l)
{
	VERTEX vertices[] = {
		-l, l, 0, 0,0,1, 0,0,
		-l,-l, 0, 0,0,1, 0,1,
		 l,-l, 0, 0,0,1, 1,1,
		 l, l, 0, 0,0,1, 1,0,
	};
	//return createVertexBuffer(vertices, _countof(vertices));
}
void createIdxSquare()
{
	unsigned short indices[] = {
		0,1,2,
		0,2,3,
	};
	//return createIndexBuffer(indices, _countof(indices));
}

void createVtxCircle(float radius, int numCorners)
{
	std::vector<VERTEX> vertices;
	float divAngle = 3.1415926f * 2 / (numCorners);
	VERTEX temp;
	for (int i = 0; i < numCorners; i++) {
		temp.x = cos(divAngle * i) * radius;
		temp.y = sin(divAngle * i) * radius;
		vertices.emplace_back(temp);
	}
	//return createVertexBuffer(vertices.data(), numCorners);
}

void createVtxCube(float l)
{
	VERTEX vertices[] = {
		//正面
		-l, l, l, 0,0,1, 0,0,
		-l,-l, l, 0,0,1, 0,1,
		 l,-l, l, 0,0,1, 1,1,
		 l, l, l, 0,0,1, 1,0,
		 //裏面
		  l, l, -l, 0,0,-1, 0,0,
		  l,-l, -l, 0,0,-1, 0,1,
		 -l,-l, -l, 0,0,-1, 1,1,
		 -l, l, -l, 0,0,-1, 1,0,
		 //右面
		  l, l, l, 1,0,0, 0,0,
		  l,-l, l, 1,0,0, 0,1,
		  l,-l, -l, 1,0,0, 1,1,
		  l, l, -l, 1,0,0, 1,0,
		  //左面
		  -l, l, -l, -1,0,0, 0,0,
		  -l,-l, -l, -1,0,0, 0,1,
		  -l,-l, l, -1,0,0, 1,1,
		  -l, l, l, -1,0,0, 1,0,
		  //上面
		  -l, l, -l, 0,1,0, 0,0,
		  -l, l,l, 0,1,0, 0,1,
		   l, l,l, 0,1,0, 1,1,
		   l, l, -l, 0,1,0, 1,0,
		   //下面
		   -l,-l,l, 0,-1,0, 0,0,
		   -l,-l, -l, 0,-1,0, 0,1,
			l,-l, -l, 0,-1,0, 1,1,
			l,-l,l, 0,-1,0, 1,0,
	};

	size_t n = _countof(vertices);
	std::ofstream fout("assets/cube.txt");
	fout << "v cube pnt " << n << std::endl;
	for (auto& v : vertices) {
		fout << v.x << ' ' << v.y << ' ' << v.z << ' '
			<< v.nx << ' ' << v.ny << ' ' << v.nz << ' '
			<< v.u << ' ' << v.v << std::endl;
	}

	createIdxCube();
	//return createVertexBuffer(vertices, _countof(vertices));
}
void createIdxCube()
{
	unsigned short indices[] = {
		0,1,2,
		0,2,3,
		4,5,6,
		4,6,7,
		8,9,10,
		8,10,11,
		12,13,14,
		12,14,15,
		16,17,18,
		16,18,19,
		20,21,22,
		20,22,23,
	};

	std::ofstream fout("assets/cube.txt", std::ios::app);
	size_t n = _countof(indices);
	fout << "i cube " << n << std::endl;
	int cnt = 0;
	for (auto& i : indices) {
		fout << i;
		if (++cnt >= 3) {
			cnt = 0;
			fout << std::endl;
		}
		else {
			fout << ' ';
		}
	}
	fout << "x banana assets/strawberry.png";

	//return createIndexBuffer(indices, _countof(indices));
}

void createVtxSphere(float radius, int numCorners)
{
	std::vector<VERTEX> vertices;
	//北極南極点以外の頂点
	float divAngle = 3.141592f * 2 / numCorners;
	VERTEX v;
	float r;
	for (int j = 1; j < numCorners / 2; j++) {
		v.ny = cos(divAngle * j);
		v.y = v.ny * radius;
		r = sin(divAngle * j);
		v.v = (1.0f - asin(v.ny) / (3.141592f / 2.0f)) / 2.0f;
		for (int i = 0; i <= numCorners; i++) {
			v.nx = cos(divAngle * i) * r;
			v.x = v.nx * radius;
			v.nz = sin(divAngle * i) * r;
			v.z = v.nz * radius;
			v.u = atan2(v.z, v.x) / (3.141592f * 2);
			if (i > numCorners / 2)	v.u += 1.0f;
			v.u *= -1;
			vertices.push_back(v);
		}
	}
	//北極点
	v.x = 0; v.y = radius; v.z = 0; v.nx = 0, v.ny = 1, v.nz = 0;
	v.u = 0.5f; v.v = 0;
	vertices.push_back(v);
	//南極点
	v.y = -radius; v.ny = -1;
	v.u = 0.5f; v.v = 1;
	vertices.push_back(v);

	size_t n = vertices.size();
	std::ofstream fout("../pipiApp/assets/sphere.txt");
	fout << "v sphere pnt " << n << std::endl;
	for (auto& v : vertices) {
		fout << std::fixed << v.x << ' ' << v.y << ' ' << v.z << ' '
			<< v.nx << ' ' << v.ny << ' ' << v.nz << ' '
			<< v.u << ' ' << v.v << std::endl;
	}

	createIdxSphere(numCorners);
	//return createVertexBuffer(vertices.data(), (int)vertices.size());
}
void createIdxSphere(int numCorners)
{
	std::vector<unsigned short> indices;
	int stride = numCorners + 1;
	for (int j = 0; j < numCorners / 2 - 2; j++) {
		for (int i = 0; i < numCorners; i++) {
			int k = i + stride * j;
			indices.push_back(k);
			indices.push_back(k + stride + 1);
			indices.push_back(k + stride);
			indices.push_back(k);
			indices.push_back(k + 1);
			indices.push_back(k + stride + 1);
		}
	}
	{
		//北極点のインデックスn
		int n = (numCorners + 1) * (numCorners / 2 - 1);
		for (int i = 0; i < numCorners; i++) {
			indices.push_back(n);
			indices.push_back(i + 1);
			indices.push_back(i);
		}
	}
	{
		//南極点のインデックスs
		int s = (numCorners + 1) * (numCorners / 2 - 1) + 1;
		int j = numCorners / 2 - 2;
		for (int i = 0; i < numCorners; i++) {
			int k = i + (numCorners + 1) * j;
			indices.push_back(k);
			indices.push_back(k + 1);
			indices.push_back(s);
		}
	}

	std::ofstream fout("../pipiApp/assets/sphere.txt", std::ios::app);
	size_t n = indices.size();
	fout << "i sphere " << n << std::endl;
	int cnt = 0;
	for (auto& i : indices) {
		fout << i;
		if (++cnt >= 3) {
			cnt = 0;
			fout << std::endl;
		}
		else {
			fout << ' ';
		}
	}
	fout << "x earth assets/earth.png";

	//return createIndexBuffer(indices.data(), (int)indices.size());
}

void createVtxCylinder(float radius, int numCorners, float low, float high)
{
	std::vector<VERTEX> vertices;
	float divAngle = 3.141592f * 2 / numCorners;
	VERTEX v;

	for (int j = 0; j < 2; j++) {
		v.y = j == 0 ? low : high;
		v.v = 1.0f - j;
		for (int i = 0; i <= numCorners; i++) {
			float angle = divAngle * i;
			v.nx = cos(angle);
			v.x = v.nx * radius;

			v.nz = sin(angle);
			v.z = v.nz * radius;

			v.u = angle / (3.141592f * 2);

			vertices.push_back(v);
		}
	}
	for (int j = 0; j < 2; j++) {
		v.y = j == 0 ? low : high;
		v.v = 1.0f - j;
		v.nx = 0;
		v.ny = -1.0f + j * 2;
		v.nz = 0;
		for (int i = 0; i < numCorners; i++) {
			float angle = divAngle * i;
			v.x = cos(angle) * radius;
			v.z = sin(angle) * radius * v.ny;

			v.u = angle / (3.141592f * 2);

			vertices.push_back(v);
		}
	}
	//return createVertexBuffer(vertices.data(), (int)vertices.size());
}
void createVtxCylinderAxisX(float radius, int numCorners, float low, float high)
{
	std::vector<VERTEX> vertices;
	float divAngle = 3.141592f * 2 / numCorners;
	VERTEX v;

	for (int j = 0; j < 2; j++) {
		v.x = j == 0 ? low : high;
		v.v = 1.0f - j;
		v.nx = 0;
		for (int i = 0; i <= numCorners; i++) {
			float angle = divAngle * i;
			v.nz = cos(angle);
			v.z = v.nz * radius;

			v.ny = sin(angle);
			v.y = v.ny * radius;

			v.u = angle / (3.141592f * 2);

			vertices.push_back(v);
		}
	}
	for (int j = 0; j < 2; j++) {
		v.x = j == 0 ? low : high;
		v.v = 1.0f - j;
		v.nx = -1.0f + j * 2;
		v.ny = 0;
		v.nz = 0;
		for (int i = 0; i < numCorners; i++) {
			float angle = divAngle * i;
			v.z = cos(angle) * radius;
			v.y = sin(angle) * radius * v.nx;

			v.u = angle / (3.141592f * 2);

			vertices.push_back(v);
		}
	}
	//return createVertexBuffer(vertices.data(), (int)vertices.size());
}

void createIdxCylinder(int numCorners)
{
	std::vector<unsigned short> indices;
	for (int i = 0; i < numCorners; i++) {
		;
		indices.push_back(i);
		indices.push_back(i + 1);
		indices.push_back(i + numCorners + 2);
		indices.push_back(i);
		indices.push_back(i + numCorners + 2);
		indices.push_back(i + numCorners + 1);
	}
	unsigned short start = (numCorners + 1) * 2;
	for (int i = 0; i < numCorners - 2; i++) {
		indices.push_back(start);
		indices.push_back(start + i + 1);
		indices.push_back(start + i + 2);
	}
	start += numCorners;
	for (int i = 0; i < numCorners - 2; i++) {
		indices.push_back(start);
		indices.push_back(start + i + 1);
		indices.push_back(start + i + 2);
	}
	//return createIndexBuffer(indices.data(), (int)indices.size());
}
