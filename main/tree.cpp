#define _USE_MATH_DEFINES

#include "../third_party/glad/include/glad.h"
#include "../third_party/glfw/include/GLFW/glfw3.h"

#include <typeinfo>
#include <stdexcept>

#include <cmath>
#include <cstdio>
#include <cstdlib>

#include "../support/error.hpp"
#include "../support/program.hpp"
#include "../support/checkpoint.hpp"
#include "../support/debug_output.hpp"

#include "../vmlib/vec4.hpp"
#include "../vmlib/mat44.hpp"

#include "defaults.hpp"
#include "tree.hpp"
#include <iostream>
#include <random>
#include <omp.h>

using namespace std;

SimpleMeshData make_cylinder(float length, float radius1, float radius2, Vec3f Colour, float angleX, float angleY, float angleZ, Vec3f initialPos, int n) {
	vector<Vec3f> pos;
	vector<Vec3f> norms;
	vector<Vec2f> textcoords;

	float prevY = cos(0.f);
	float prevZ = sin(0.f);
	float j = 0;

	int N = 64 / pow(2,(5 - n));
	if (N < 2) {
		N = 2;
	}

	for (int i = 0; i < N; ++i) {
		float const subDivAngle = (i + 1) / float(N) * 2.f * M_PI;

		float y = cos(subDivAngle);
		float z = sin(subDivAngle);

		pos.emplace_back(Vec3f{ 0.f, prevY * radius1, prevZ * radius1});
		pos.emplace_back(Vec3f{ 0.f, y * radius1, z * radius1 });
		pos.emplace_back(Vec3f{ 1.f, prevY * radius2, prevZ * radius2 });

		pos.emplace_back(Vec3f{ 0.f, y * radius1, z * radius1 });
		pos.emplace_back(Vec3f{ 1.f, y * radius2, z * radius2 });
		pos.emplace_back(Vec3f{ 1.f, prevY * radius2, prevZ * radius2 });

		pos.emplace_back(Vec3f{ 0.f, prevY * radius1, prevZ * radius1 });
		pos.emplace_back(Vec3f{ 0.f, 0, 0 });
		pos.emplace_back(Vec3f{ 0.f, y * radius1, z * radius1 });

		pos.emplace_back(Vec3f{ 1.f, prevY * radius2, prevZ * radius2 });
		pos.emplace_back(Vec3f{ 1.f, y * radius2, z * radius2 });
		pos.emplace_back(Vec3f{ 1.f, 0, 0 });

		Vec3f normal1 = cross(Vec3f{ 1.f, prevY * radius2, prevZ * radius2 } - Vec3f{ 0.f, prevY * radius1, prevZ * radius1 }, Vec3f{ 1.f, prevY * radius2, prevZ * radius2 } - Vec3f{ 0.f, y * radius1, z * radius1 });

		norms.emplace_back(normal1);
		norms.emplace_back(normal1);
		norms.emplace_back(normal1);

		norms.emplace_back(normal1);
		norms.emplace_back(normal1);
		norms.emplace_back(normal1);

		Vec3f normal2 = cross(Vec3f{ 0.f, y * radius1, z * radius1 } - Vec3f{ 0.f, prevY * radius1, prevZ * radius1 }, Vec3f{ 0.f, y * radius1, z * radius1 } - Vec3f{ 0.f, 0, 0 });

		norms.emplace_back(normal2);
		norms.emplace_back(normal2);
		norms.emplace_back(normal2);

		Vec3f normal3 = cross(Vec3f{ 1.f, 0, 0 } - Vec3f{ 1.f, prevY * radius2, prevZ * radius2 }, Vec3f{ 1.f, 0, 0 } - Vec3f{ 1.f, y * radius2, z * radius2 });

		norms.emplace_back(normal3);
		norms.emplace_back(normal3);
		norms.emplace_back(normal3);

		textcoords.emplace_back(Vec2f{ j / N, 0 });
		textcoords.emplace_back(Vec2f{ (j + 1) / N, 0 });
		textcoords.emplace_back(Vec2f{ j / N, 1 });

		textcoords.emplace_back(Vec2f{ (j + 1) / N, 0 });
		textcoords.emplace_back(Vec2f{ (j + 1) / N, 1 });
		textcoords.emplace_back(Vec2f{ j / N, 1 });

		textcoords.emplace_back(Vec2f{ 0, 0 });
		textcoords.emplace_back(Vec2f{ 1, 0 });
		textcoords.emplace_back(Vec2f{ 0, 1 });

		textcoords.emplace_back(Vec2f{ 1, 0 });
		textcoords.emplace_back(Vec2f{ 1, 1 });
		textcoords.emplace_back(Vec2f{ 0, 1 });
		
		prevY = y;
		prevZ = z;
		j++;
	}

	Mat44f Transformation = make_translation(initialPos) * make_rotation_x(angleX) * make_rotation_z(angleZ) * make_rotation_y(angleY) * make_rotation_z(M_PI/2) * make_scaling(length, 1, 1);

	for (auto& p : pos) {
		Vec4f p4{ p.x,p.y,p.z,1.f };
		Vec4f t = Transformation * p4;
		t /= t.w;

		p = Vec3f{ t.x,t.y,t.z };
	}

	vector<Vec3f> col;
	for (int i = 0; i < pos.size(); i++) {
		col.emplace_back(Colour);
	}

	return SimpleMeshData{ std::move(pos), std::move(col), std::move(norms), std::move(textcoords)};
}

posVaos drawBranch(float length, float radius1, float radius2, Vec3f Colour, float angleX, float angleY, float angleZ, Vec3f initialPos, posVaos vaosPos, int n, int treeIndex) {
	SimpleMeshData mesh = make_cylinder(length, radius1, radius2, Colour, angleX, angleY, angleZ, initialPos, n);
	vaosPos.tree = concatenate(vaosPos.tree, mesh);

	float diff = 0;

	if (angleX > angleZ) {
		diff = radius2 * sin(abs(angleX));
	}
	else {
		diff = radius2 * sin(abs(angleZ - M_PI/2));
	}

	Vec4f test = make_rotation_x(angleX) * make_rotation_z(angleZ) * make_rotation_y(angleY) * make_rotation_z(M_PI / 2) * make_scaling(length, 1, 1) * Vec4f { 1.0f, 0.0f, 0.0f, 1.f };
	vaosPos.pos = { initialPos.x + test.x, initialPos.y + test.y, initialPos.z + test.z };

	return vaosPos;
}

posVaos drawLeaf(float length, float radius1, float radius2, Vec3f Colour, float angleX, float angleY, float angleZ, Vec3f initialPos, posVaos vaosPos, int treeIndex, float leafLength, float leafWidth) {
	vector<Vec3f> pos1;
	vector<Vec3f> norms;
	vector<Vec2f> textcoords;

	pos1.emplace_back(Vec3f{ 0.f, 0.f, -0.5f });
	pos1.emplace_back(Vec3f{ 1.f, 0.f, -0.5f });
	pos1.emplace_back(Vec3f{ 1.f, 0.f, 0.5f });

	pos1.emplace_back(Vec3f{ 0.f, 0.f, -0.5f });
	pos1.emplace_back(Vec3f{ 0.f, 0.f, 0.5f });
	pos1.emplace_back(Vec3f{ 1.f, 0.f, 0.5f });

	pos1.emplace_back(Vec3f{ 1.f, 0.f, 0.5f });
	pos1.emplace_back(Vec3f{ 1.f, 0.f, -0.5f });
	pos1.emplace_back(Vec3f{ 0.f, 0.f, -0.5f });

	pos1.emplace_back(Vec3f{ 1.f, 0.f, 0.5f });
	pos1.emplace_back(Vec3f{ 0.f, 0.f, 0.5f });
	pos1.emplace_back(Vec3f{ 0.f, 0.f, -0.5f });

	Vec3f normal1 = cross(Vec3f{ 0.5f, 0.f, 1.f } - Vec3f{ -0.5f, 0.f, 0.f }, Vec3f{ 0.5f, 0.f, 1.f } - Vec3f{ 0.5f, 0.f, 0.f });
	Vec3f normal2 = cross(Vec3f{ 0.5f, 0.f, 1.f } - Vec3f{ -0.5f, 0.f, 0.f }, Vec3f{ 0.5f, 0.f, 1.f } - Vec3f{ -0.5f, 0.f, 1.f });
	Vec3f normal3 = cross(Vec3f{ -0.5f, 0.f, 0.f } - Vec3f{ 0.5f, 0.f, 1.f }, Vec3f{ -0.5f, 0.f, 0.f } - Vec3f{ 0.5f, 0.f, 0.f });
	Vec3f normal4 = cross(Vec3f{ -0.5f, 0.f, 0.f } - Vec3f{ 0.5f, 0.f, 1.f }, Vec3f{ -0.5f, 0.f, 0.f } - Vec3f{ -0.5f, 0.f, 1.f });

	norms.emplace_back(normal1);
	norms.emplace_back(normal1);
	norms.emplace_back(normal1);

	norms.emplace_back(normal2);
	norms.emplace_back(normal2);
	norms.emplace_back(normal2);

	norms.emplace_back(normal3);
	norms.emplace_back(normal3);
	norms.emplace_back(normal3);

	norms.emplace_back(normal4);
	norms.emplace_back(normal4);
	norms.emplace_back(normal4);

	textcoords.emplace_back(Vec2f{ 0, 0 });
	textcoords.emplace_back(Vec2f{ 1, 0 });
	textcoords.emplace_back(Vec2f{ 1, 1 });

	textcoords.emplace_back(Vec2f{ 0, 0 });
	textcoords.emplace_back(Vec2f{ 0, 1 });
	textcoords.emplace_back(Vec2f{ 1, 1 });

	textcoords.emplace_back(Vec2f{ 1, 1 });
	textcoords.emplace_back(Vec2f{ 1, 0 });
	textcoords.emplace_back(Vec2f{ 0, 0 });

	textcoords.emplace_back(Vec2f{ 1, 1 });
	textcoords.emplace_back(Vec2f{ 0, 1 });
	textcoords.emplace_back(Vec2f{ 0, 0 });

	if (angleX == 0) {
		angleY = 0;
	}
	else if (angleZ == 0 && angleX != 0) {
		angleY = 90;
	}
	else if (angleZ < 0 && angleX < 0) {
		angleY = -angleX;
	}
	else if (angleZ < 0) {
		angleY = -angleX;
	}
	else {
		angleY = angleX;
	}

	Mat44f Transformation = make_translation(initialPos) * make_rotation_x(angleX) * make_rotation_z(angleZ) * make_rotation_y(angleY) * make_rotation_z(M_PI / 2) * make_scaling(length*leafLength, 1, radius2*leafWidth);

	for (auto& p : pos1) {
		Vec4f p4{ p.x,p.y,p.z,1.f };
		Vec4f t = Transformation * p4;
		t /= t.w;

		p = Vec3f{ t.x,t.y,t.z };
	}

	Colour = { 0.2f, 0.5f, 0.f };
	vector col(pos1.size(), Colour);

	SimpleMeshData mesh = { std::move(pos1), std::move(col), std::move(norms), std::move(textcoords)};
	vaosPos.leaves = concatenate(vaosPos.leaves, mesh);
	vaosPos.pos = { initialPos.x, initialPos.y, initialPos.z };
	
	return vaosPos;
}

float evaluate_expression(string s, string definitions[100][2]) {
	float evaluation;
	string temps[100];
	float nums[100];
	int tempsIndex = 0;
	int j = 0;
	for (int i = 0; i < s.length(); i++) {
		while (s[i] != '*' && s[i] != '+' && s[i] != '-' && s[i] != '/' && s[i] != '^' && s[i] != ')') {
			temps[tempsIndex].push_back(s[i]);
			j++;
			i++;
		}

		for (int k = 0; k < 12; k++) {
			if (definitions[k][0] == temps[tempsIndex]) {
				nums[tempsIndex] = stof(definitions[k][1]);
				break;
			}
		}

		tempsIndex++;
		j = 0;

		if (s[i] == '*') {
			i++;
			while (s[i] != '*' && s[i] != '+' && s[i] != '-' && s[i] != '/' && s[i] != '^' && s[i] != ')') {
				temps[tempsIndex].push_back(s[i]);
				j++;
				i++;
			}

			for (int k = 0; k < 12; k++) {
				if (definitions[k][0] == temps[tempsIndex]) {
					nums[tempsIndex] = stof(definitions[k][1]);
					break;
				}
			}

			nums[0] = nums[tempsIndex] * nums[tempsIndex - 1];
		}

		else if (s[i] == '/') {
			i++;
			while (s[i] != '*' && s[i] != '+' && s[i] != '-' && s[i] != '/' && s[i] != '^' && s[i] != ')') {
				temps[tempsIndex].push_back(s[i]);
				j++;
				i++;
			}

			for (int k = 0; k < 12; k++) {
				if (definitions[k][0] == temps[tempsIndex]) {
					nums[tempsIndex] = stof(definitions[k][1]);
					break;
				}
			}

			nums[0] = nums[tempsIndex - 1] / nums[tempsIndex];
		}
		return nums[0];
	}
}

posVaos drawTree(float length, float radius1, float radius2, Vec3f Colour, float angleX, float angleY, float angleZ, Vec3f initialPos, posVaos vaosPos, vector<string> Bstrings, string Bstring, vector<Turtle> branchPos, int n, int treeIndex, float leafLength, float leafWidth) {
	int i = 0;
	string l;
	string r;
	while(i < Bstring.length()) {
		if (Bstring[i] == 'F') {
			l = to_string(length);
			r = to_string(radius1);
			string definitions[100][2] = { {"1","0.1"} , {"2","0.2"} , {"3","0.3"} , {"4","0.4"} , {"5","0.5"} , {"6","0.6"}, {"7","0.7"} , {"8","0.8"} ,
				{"9","0.9"} , {"u","1"}, {"l", l}, {"r", r} };

			if (Bstring[i + 1] == '(') {
				i = i + 2;
				string tempString1;
				while (Bstring[i] != ',') {
					tempString1 = tempString1 + Bstring[i];
					i++;
				}
				tempString1 = tempString1 + ')';
				length = evaluate_expression(tempString1, definitions);
				string tempString3;
				i++;
				while (Bstring[i] != ')') {
					tempString3 = tempString3 + Bstring[i];
					i++;
				}
				tempString3 = tempString3 + ')';
				radius1 = evaluate_expression(tempString3, definitions);
			}
			radius2 = radius1 * 0.9;

			vaosPos = drawBranch(length, radius1, radius2, Colour, angleX, angleY, angleZ, initialPos, vaosPos, n, treeIndex);
			initialPos = vaosPos.pos;
		}
		else if (Bstring[i] == '+') {
			angleZ = angleZ + (15 * M_PI ) / 180;
		}
		else if (Bstring[i] == '-') {
			angleZ = angleZ - (15 * M_PI) / 180;
		}
		else if (Bstring[i] == '&') {
			angleY = angleY + (15 * M_PI) / 180;
		}
		else if (Bstring[i] == '^') {
			angleY = angleY - (15 * M_PI) / 180;
		}
		else if (Bstring[i] == '|') {
			angleX = angleX + (15 * M_PI) / 180;
		}
		else if (Bstring[i] == '/') {
			angleX = angleX - (15 * M_PI) / 180;
		}
		else if (Bstring[i] == '[') {
			Turtle tempTurtle = { initialPos.x, initialPos.y, initialPos.z, angleX, angleY, angleZ, length, radius1, radius2 };
			branchPos.push_back(tempTurtle);
		}
		else if (Bstring[i] == ']') {
			Turtle tempTurtle = branchPos.back();
			branchPos.pop_back();
			initialPos.x = tempTurtle.x;
			initialPos.y = tempTurtle.y;
			initialPos.z = tempTurtle.z;
			angleX = tempTurtle.angleX;
			angleY = tempTurtle.angleY;
			angleZ = tempTurtle.angleZ;
			length = tempTurtle.length;
			radius1 = tempTurtle.radius1;
			radius2 = tempTurtle.radius2;
		}
		else if (isdigit(Bstring[i]) && n != 0) {
			int next_str = int(Bstring[i]) - 48;
			random_device dev;
			mt19937 rng(dev());
			uniform_int_distribution<mt19937::result_type> dist3(0, 4);
			int x = dist3(rng);
			next_str = next_str * 5 + x;
			l = to_string(length);
			r = to_string(radius1);
			string definitions[100][2] = { {"1","0.1"} , {"2","0.2"} , {"3","0.3"} , {"4","0.4"} , {"5","0.5"} , {"6","0.6"}, {"7","0.7"} , {"8","0.8"} , {"9","0.9"}, {"l", l}, {"r", r} };
			radius1 = radius2;

			if (Bstring[i + 1] == '(') {
				i = i + 2;
				string tempString1;
				while (Bstring[i] != ',') {
					tempString1 = tempString1 + Bstring[i];
					i++;
				}
				tempString1 = tempString1 + ')';
				length = evaluate_expression(tempString1, definitions);
				string tempString3;
				i++;
				while (Bstring[i] != ')') {
					tempString3 = tempString3 + Bstring[i];
					i++;
				}
				tempString3 = tempString3 + ')';
				radius2 = evaluate_expression(tempString3, definitions);
			}

			vaosPos = drawTree(length, radius1, radius2, Colour, angleX, angleY, angleZ, initialPos, vaosPos, Bstrings, Bstrings[next_str], branchPos, n-1, treeIndex, leafLength, leafWidth);
			initialPos = vaosPos.pos;
		}
		else if (isdigit(Bstring[i]) && n == 0) {
			l = to_string(length);
			r = to_string(radius1);
			string definitions[100][2] = { {"1","0.1"} , {"2","0.2"} , {"3","0.3"} , {"4","0.4"} , {"5","0.5"} , {"6","0.6"}, {"7","0.7"} , {"8","0.8"} , {"9","0.9"}, {"l", l}, {"r", r} };
			radius1 = radius2;

			if (Bstring[i + 1] == '(') {
				i = i + 2;
				string tempString1;
				while (Bstring[i] != ',') {
					tempString1 = tempString1 + Bstring[i];
					i++;
				}
				tempString1 = tempString1 + ')';
				length = evaluate_expression(tempString1, definitions);
				string tempString3;
				i++;
				while (Bstring[i] != ')') {
					tempString3 = tempString3 + Bstring[i];
					i++;
				}
				tempString3 = tempString3 + ')';
				radius2 = evaluate_expression(tempString3, definitions);
			}
			radius2 = radius1 * 0.95;

			vaosPos = drawBranch(length, radius1, radius2, Colour, angleX, angleY, angleZ, initialPos, vaosPos, n, treeIndex);
			initialPos = vaosPos.pos;
		}
		else if (Bstring[i] == 'L') {
			vaosPos = drawLeaf(length, radius1, radius2, Colour, angleX, angleY, angleZ, initialPos, vaosPos, treeIndex, leafLength, leafWidth);
			initialPos = vaosPos.pos;
		}
		i++;
	}
	return vaosPos;
}