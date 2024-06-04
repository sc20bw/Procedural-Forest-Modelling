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

#include "../third_party/stb/include/stb_image.h"
#include "../third_party/stb/include/stb_image_write.h"

#include "../vmlib/vec4.hpp"
#include "../vmlib/mat44.hpp"

#include "defaults.hpp"
#include "tree.hpp"
#include <iostream>
#include <random>
#include <stdio.h>
#include <stdlib.h>
#include <chrono>
#include <omp.h>

GLuint load_texture_2d(char const* aPath);
int createForest(int treesRow, int const treesCol, int treeType, float branchlength, float radius1, int n);
