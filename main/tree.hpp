#ifndef CUBE_HPP_6874B39C_112D_4D34_BD85_AB81A730955B
#define CUBE_HPP_6874B39C_112D_4D34_BD85_AB81A730955B

#include "simple_mesh.hpp"

using namespace std;

struct Vaos{
	int num = 0;
	vector<GLuint> vaos;
	vector<int> textured;
	vector<size_t> vao_size;
};

struct Trees {
	int num = 0;
	vector<SimpleMeshData> tree;
	vector<int> textured;
};

struct posVaos {
	SimpleMeshData tree;
	SimpleMeshData leaves;
	Vec3f pos = {0, 0, 0};
};

struct Turtle {
	float x;
	float y;
	float z;
	
	float angleX;
	float angleY;
	float angleZ;

	float length;
	float radius1;
	float radius2;
};

SimpleMeshData make_cylinder(float length, float radius1, float radius2, Vec3f Colour, float angleX, float angleY, float angleZ, Vec3f initialPos, int n);

posVaos drawBranch(float length, float radius1, float radius2, Vec3f Colour, float angleX, float angleY, float angleZ, Vec3f initialPos, posVaos vaos, int n, int treeIndex);

posVaos drawTree(float length, float radius1, float radius2, Vec3f Colour, float angleX, float angleY, float angleZ, Vec3f initialPos, posVaos vaos, vector<string> Bstrings, string Bstring, vector<Turtle> branchPos, int n, int treeIndex, float leafLength, float leafWidth);

posVaos drawLeaf(float length, float radius1, float radius2, Vec3f Colour, float angleX, float angleY, float angleZ, Vec3f initialPos, posVaos vaos, int treeIndex, float leafLength, float leafWidth);
#endif // CUBE_HPP_6874B39C_112D_4D34_BD85_AB81A730955B
