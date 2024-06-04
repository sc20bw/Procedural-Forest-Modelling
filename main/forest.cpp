#include "forest.hpp"
#include<iostream>
#include<fstream>
#include<omp.h>
//#include <debugapi.h>

using namespace std::chrono;
using namespace std;

namespace
{
	constexpr char const* kWindowTitle = "Procedural Forest Modeller";

	constexpr float kPi_ = 3.1415926f;

	constexpr float kMovementPerSecond_ = 5.f; // units per second
	constexpr float kMouseSensitivity_ = 0.01f; // radians per pixel

	struct State_
	{
		ShaderProgram* prog;

		struct CamCtrl_
		{
			bool cameraActive;
			bool actionZoomIn, actionZoomOut, moveLeft, moveRight, moveUp, moveDown, fast;

			float phi, theta;
			float x, y, z, speed;

			float lastX, lastY;
		} camControl;
	};

	void glfw_callback_error_( int, char const* );

	void glfw_callback_key_( GLFWwindow*, int, int, int, int );
	void glfw_callback_motion_( GLFWwindow*, double, double );

	struct GLFWCleanupHelper
	{
		~GLFWCleanupHelper();
	};
	struct GLFWWindowDeleter
	{
		~GLFWWindowDeleter();
		GLFWwindow* window;
	};
}

GLuint load_texture_2d(char const* aPath)
{
	assert(aPath);
	stbi_set_flip_vertically_on_load(true);
	int w, h, channels;
	stbi_uc* ptr = stbi_load(aPath, &w, &h, &channels, 4);
	if (!ptr)
		throw Error("Unable to load image �%s�\n", aPath);
	// Generate texture object and initialize texture with image
	GLuint tex = 0;
	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB8_ALPHA8, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, ptr);
	stbi_image_free(ptr);
	// Generate mipmap hierarchy
	glGenerateMipmap(GL_TEXTURE_2D);
	// Configure texture
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
	return tex;
}

int createForest(int treesRow, int const treesCol, int treeType, float branchlength, float radius1, int n) try
{
	// Initialize GLFW
	if( GLFW_TRUE != glfwInit() )
	{
		char const* msg = nullptr;
		int ecode = glfwGetError( &msg );
		throw Error( "glfwInit() failed with '%s' (%d)", msg, ecode );
	}

	// Ensure that we call glfwTerminate() at the end of the program.
	GLFWCleanupHelper cleanupHelper;

	// Configure GLFW and create window
	glfwSetErrorCallback( &glfw_callback_error_ );

	glfwWindowHint( GLFW_SRGB_CAPABLE, GLFW_TRUE );
	glfwWindowHint( GLFW_DOUBLEBUFFER, GLFW_TRUE );

	//glfwWindowHint( GLFW_RESIZABLE, GLFW_FALSE );

	glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, 4 );
	glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, 3 );
	glfwWindowHint( GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE );
	glfwWindowHint( GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE );

	glfwWindowHint( GLFW_DEPTH_BITS, 24 );

#	if !defined(NDEBUG)
	// When building in debug mode, request an OpenGL debug context. This
	// enables additional debugging features. However, this can carry extra
	// overheads. We therefore do not do this for release builds.
	glfwWindowHint( GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE );
#	endif // ~ !NDEBUG

	GLFWwindow* window = glfwCreateWindow(
		1280,
		720,
		kWindowTitle,
		nullptr, nullptr
	);

	if( !window )
	{
		char const* msg = nullptr;
		int ecode = glfwGetError( &msg );
		throw Error( "glfwCreateWindow() failed with '%s' (%d)", msg, ecode );
	}

	GLFWWindowDeleter windowDeleter{ window };

	// Set up event handling
	State_ state{};

	glfwSetWindowUserPointer( window, &state );

	glfwSetKeyCallback( window, &glfw_callback_key_ );
	glfwSetCursorPosCallback( window, &glfw_callback_motion_ );

	// Set up drawing stuff
	glfwMakeContextCurrent( window );
	glfwSwapInterval( 1 ); // V-Sync is on.

	// Initialize GLAD
	// This will load the OpenGL API. We mustn't make any OpenGL calls before this!
	if( !gladLoadGLLoader( (GLADloadproc)&glfwGetProcAddress ) )
		throw Error( "gladLoaDGLLoader() failed - cannot load GL API!" );

	std::printf( "RENDERER %s\n", glGetString( GL_RENDERER ) );
	std::printf( "VENDOR %s\n", glGetString( GL_VENDOR ) );
	std::printf( "VERSION %s\n", glGetString( GL_VERSION ) );
	std::printf( "SHADING_LANGUAGE_VERSION %s\n", glGetString( GL_SHADING_LANGUAGE_VERSION ) );

	// Ddebug output
#	if !defined(NDEBUG)
	setup_gl_debug_output();
#	endif // ~ !NDEBUG

	// Global GL state
	OGL_CHECKPOINT_ALWAYS();

	glEnable(GL_FRAMEBUFFER_SRGB);
	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

	OGL_CHECKPOINT_ALWAYS();

	// Get actual framebuffer size.
	// This can be different from the window size, as standard window
	// decorations (title bar, borders, ...) may be included in the window size
	// but not be part of the drawable surface area.
	int iwidth, iheight;
	glfwGetFramebufferSize( window, &iwidth, &iheight );

	glViewport( 0, 0, iwidth, iheight );

	// Load shader program
	ShaderProgram prog( {
		{ GL_VERTEX_SHADER, "assets/default.vert" },
		{ GL_FRAGMENT_SHADER, "assets/default.frag" }
	} );

	state.prog = &prog;
	//state.camControl.radius = 1000.f;
	state.camControl.phi = M_PI*2/3;
	state.camControl.x = -20.f;
	state.camControl.z = 20.f;
	state.camControl.y = -20.f;
	state.camControl.speed = 3.f;

	// Animation state
	auto last = Clock::now();

	float angle = 0.f;
   
	Trees trees;

	Turtle turtle;

	turtle.x = 0;
	turtle.y = 0;
	turtle.z = 0;
	turtle.angleX = 0;
	turtle.angleY = 0;
	turtle.angleZ = 0;
	turtle.length = 0;
	turtle.radius1 = 0;
	turtle.radius2 = 0;

	vector<string> stringTree1 = { 
		"FF(l*9,r*9)[+1(l*9,r*7)]-F(l*9,r*9)0(l*9,r*9)L",
		"F[++1(l*9,r*7)]-F(l*9,r*9)[//1(l*9,r*7)]F(l*9,r*9)0(l*9,r*9)L",
		"FF(l*9,r*9)[||1(l*9,r*7)]//F(l*9,r*9)0(l*9,r*9)L",
		"F[//1(l*9,r*7)][++|1(l*9,r*7)]--|F(l*9,r*9)0(l*9,r*9)L",
		"-/F|F(l*9,r*9)[++++1(l*9,r*7)][/-1(l*9,r*7)]F(l*9,r*9)0(l*9,r*9)L",

		"F[//2(l*9,r*5)]-F(l*9,r*9)[+|2(l*9,r*5)]-F(l*9,r*9)1(l*9,r*9)L",
		"F[++2(l*9,r*5)][-|2(l*9,r*5)][-/2(l*9,r*5)]F(l*9,r*9)1(l*9,r*9)L",
		"F-F(l*9,r*9)[2(l*9,r*9)]|+F(l*9,r*7)F(l*9,r*9)1(l*9,r*9)L",
		"F|||F(l*9,r*9)[/+2(l*9,r*5)]F(l*9,r*9)[--2(l*9,r*5)]/F(l*9,r*9)1(l*9,r*9)L",
		"F[++2(l*9,r*5)][//2(l*9,r*5)]F(l*9,r*9)[--2(l*9,r*5)][||2(l*9,r*5)]1(l*9,r*9)L",

		"F[/++1(l*9,r*5)L][1(l*9,r*5)L][--1(l*9,r*5)L]",
		"F[+++1(l*9,r*5)L][-//1(l*9,r*5)L][-||1(l*9,r*5)L]",  
		"F[///1(l*9,r*5)L][|||1(l*9,r*5)L]",
		"[|++2(l*9,r*5)L][||-2(l*9,r*5)L]F1(l*9,r*9)L",
		"F[----1(l*9,r*5)L][++++1(l*9,r*5)L][////1(l*9,r*5)L][||||1(l*9,r*5)L]"
	};

	vector<string> stringTree2 = {
		"1(l*9,r*7)1(l*9,r*7)1(l*9,r*7)0(l*9,r*7)",
		"1(l*9,r*7)1(l*9,r*7)1(l*9,r*7)0(l*9,r*7)",
		"1(l*9,r*7)1(l*9,r*7)1(l*9,r*7)0(l*9,r*7)",
		"1(l*9,r*7)1(l*9,r*7)1(l*9,r*7)0(l*9,r*7)",
		"1(l*9,r*7)1(l*9,r*7)1(l*9,r*7)0(l*9,r*7)",

		"F[++++L][////++L][||||--L][////--L][||||++L][----L]",
		"F[++++L][////++L][||||--L][////--L][||||++L][----L]",
		"F[++++L][////++L][||||--L][////--L][||||++L][----L]",
		"F[++++L][////++L][||||--L][////--L][||||++L][----L]",
		"F[++++L][////++L][||||--L][////--L][||||++L][----L]"
	};

	GLuint barkTexture;
	GLuint leafTexture;
	vector<string> strings;
	Vec3f colour;
	float leafLength;
	float leafWidth;
	float radius;

	if (treeType == 1) {
		barkTexture = load_texture_2d("./textures/tree_bark_brown.png");
		leafTexture = load_texture_2d("./textures/leaf.png");
		strings = stringTree1;
		colour = { 0.051f, 0.023f, 0.002f };
		leafLength = 1.2;
		leafWidth = 3;
		radius = branchlength;
		for (int i = 0; i < n-1; i++) {
			radius = radius + (radius * 0.45);
		}
	}
	else if (treeType == 2) {
		barkTexture = load_texture_2d("./textures/birch_tree_texture.jpg");
		leafTexture = load_texture_2d("./textures/leaf.png");
		strings = stringTree1;
		colour = { 0.9f, 0.9f, 0.9f };
		leafLength = 1.2;
		leafWidth = 3;
		radius = branchlength;
		for (int i = 0; i < n - 1; i++) {
			radius = radius + (radius * 0.45);
		}
	}
	else if (treeType == 3) {
		barkTexture = load_texture_2d("./textures/tree_bark_brown.png");
		leafTexture = load_texture_2d("./textures/fern_leaf.png");
		strings = stringTree2;
		colour = { 0.051f, 0.023f, 0.002f };
		leafLength = 2;
		leafWidth = 5;
		radius = 5*(radius1+0.15)*0.9;
	}

	ofstream myfile;
	myfile.open("Test.txt");

	float forestLength = round(round(sqrt(treesCol * treesRow) + 1) * radius * 2.5);
	vector<Vec3f> initialTreePositions;
	random_device dev;
	mt19937 rng(dev());
	uniform_int_distribution<mt19937::result_type> dist5(0, forestLength*100);
	float tempx = dist5(rng), tempy = dist5(rng);
	tempx = tempx / 100;
	tempy = tempy / 100;
	initialTreePositions.push_back(Vec3f{ tempx, 0.0, tempy });

	for (int i = 0; i < treesCol * treesRow; i++) {
		int j = 0;
		while (j == 0) {
			tempx = dist5(rng), tempy = dist5(rng);
			tempx = tempx / 100;
			tempy = tempy / 100;
			for (int k = 0; k < initialTreePositions.size(); k++) {
				float x = tempx - initialTreePositions.at(k).x;
				float y = tempy - initialTreePositions.at(k).z;
				float x2 = pow(x,2);
				float y2 = pow(y,2);
				float root2 = sqrt(x2+y2);
				float rad2 = (2 * radius);
				if (root2 < rad2) {
					j = 0;
					break;
				}
				else {
					j = 1;
				}
			}
			if (j == 1) {
				initialTreePositions.push_back(Vec3f{ tempx, 0.0, tempy });
			}
		}
	}

	float radius1lower = (radius1 - 0.1) * 100;
	float radius1upper = (radius1 + 0.3) * 100;
	float bllower = (branchlength - 0.5) * 100;
	float blupper = (branchlength + 1) * 100;

	uniform_int_distribution<mt19937::result_type> dist2(radius1lower, radius1upper);
	uniform_int_distribution<mt19937::result_type> dist1(bllower, blupper);

	vector<Turtle> branchPos;
	float radius2 = radius1 * 0.9;
	trees.num = treesCol * treesRow * 2;
	SimpleMeshData baseMesh = make_cylinder(0, 0, 0, { 0.051f, 0.023f, 0.002f }, 0, 0, 0, { 0,0,0 }, 0);

	auto start = high_resolution_clock::now();

	Vaos vaos;
	vaos.num = trees.num;

	posVaos tempTreeMeshes[900];

	for (int i = 0; i < treesCol * treesRow; i++) {
		posVaos tempMesh;
		uniform_int_distribution<mt19937::result_type> dist3(0, 4);
		int x = dist3(rng);

		string Bstring = strings[x];
		radius1 = dist2(rng);
		radius1 = radius1 / 100;
		branchlength = dist1(rng);
		branchlength = branchlength / 100;
		turtle.length = branchlength;
		turtle.radius1 = radius1;
		turtle.radius2 = radius2;

		int treeIndex = 0;
		tempMesh = drawTree(branchlength, radius1, radius2, colour, turtle.angleX, turtle.angleY, turtle.angleZ, initialTreePositions.at(i), tempMesh, strings, Bstring, branchPos, n, treeIndex, leafLength, leafWidth);
		tempTreeMeshes[i] = tempMesh;
	}

	for (int j = 0; j < treesCol*treesRow; j++) {
		trees.tree.push_back(tempTreeMeshes[j].tree);
		trees.textured.push_back(1);
		trees.tree.push_back(tempTreeMeshes[j].leaves);
		trees.textured.push_back(2);
	}

	auto stop = high_resolution_clock::now();
	auto duration = duration_cast<microseconds>(stop - start);

	cout << "Time taken by function: "
		<< duration.count() << " microseconds" << endl;


	myfile << ((float)duration.count())/1000000 << " s\n";
	myfile.close();

	for (int i = 0; i < trees.num; i++) {
		vaos.vaos.push_back(create_vao(trees.tree.at(i)));
		vaos.textured.push_back(trees.textured.at(i));
		vaos.vao_size.push_back(trees.tree.at(i).positions.size());
	}

	SimpleMeshData ground = make_cylinder(forestLength, forestLength/2, forestLength/2, {0.659, 0.9, 0.416}, M_PI / 2, 0, 0, { forestLength / 2, 0, 0 }, 0);
	vaos.vaos.push_back(create_vao(ground));
	vaos.textured.push_back(0);
	vaos.vao_size.push_back(ground.positions.size());
	vaos.num++;

	// Main loop
	while( !glfwWindowShouldClose( window ) )
	{
		// Let GLFW process events
		glfwPollEvents();
		
		// Check if window was resized.
		float fbwidth, fbheight;
		{
			int nwidth, nheight;
			glfwGetFramebufferSize( window, &nwidth, &nheight );

			fbwidth = float(nwidth);
			fbheight = float(nheight);

			if( 0 == nwidth || 0 == nheight )
			{
				// Window minimized? Pause until it is unminimized.
				// This is a bit of a hack.
				do
				{
					glfwWaitEvents();
					glfwGetFramebufferSize( window, &nwidth, &nheight );
				} while( 0 == nwidth || 0 == nheight );
			}

			glViewport( 0, 0, fbwidth, fbheight );
		}

		// Update state
		auto const now = Clock::now();
		float dt = std::chrono::duration_cast<Secondsf>(now-last).count();
		last = now;


		angle += dt * kPi_ * 0.3f;
		if( angle >= 2.f*kPi_ )
			angle -= 2.f*kPi_;

		// Update camera state
		if (state.camControl.fast) {
			state.camControl.speed = 15.0;
		}
		else {
			state.camControl.speed = 3.0;
		}

		if (state.camControl.actionZoomIn) {
			state.camControl.x -= sin(-state.camControl.phi) * cos(-state.camControl.theta) * state.camControl.speed * dt;
			state.camControl.y -= cos(-state.camControl.phi) * cos(-state.camControl.theta) * state.camControl.speed * dt;
			state.camControl.z += sin(-state.camControl.theta) * state.camControl.speed * dt;
			if (state.camControl.z < 1)
				state.camControl.z = 1.0;
		}
		if (state.camControl.actionZoomOut) {
			state.camControl.x += sin(-state.camControl.phi) * cos(-state.camControl.theta) * state.camControl.speed * dt;
			state.camControl.y += cos(-state.camControl.phi) * cos(-state.camControl.theta) * state.camControl.speed * dt;
			state.camControl.z -= sin(-state.camControl.theta) * state.camControl.speed * dt;
			if (state.camControl.z < 1)
				state.camControl.z = 1.0;
		}

		if (state.camControl.moveUp) {
			state.camControl.x -= cos(M_PI / 2 - state.camControl.phi) * cos(-state.camControl.theta + M_PI / 2) * state.camControl.speed * dt;
			state.camControl.y -= sin(M_PI / 2 - state.camControl.phi) * cos(-state.camControl.theta + M_PI / 2) * state.camControl.speed * dt;
			state.camControl.z += sin(state.camControl.theta + M_PI / 2) * state.camControl.speed * dt;
			if (state.camControl.z < 1)
				state.camControl.z = 1.0;
		}
		if (state.camControl.moveDown) {
			state.camControl.x += cos(M_PI / 2 - state.camControl.phi) * cos(-state.camControl.theta + M_PI / 2) * state.camControl.speed * dt;
			state.camControl.y += sin(M_PI / 2 - state.camControl.phi) * cos(-state.camControl.theta + M_PI / 2) * state.camControl.speed * dt;
			state.camControl.z -= sin(state.camControl.theta + M_PI / 2) * state.camControl.speed * dt;
			if (state.camControl.z < 1)
				state.camControl.z = 1.0;
		}

		if (state.camControl.moveLeft) {
			state.camControl.y -= sin(state.camControl.phi) * state.camControl.speed * dt;
			state.camControl.x -= cos(state.camControl.phi) * state.camControl.speed * dt;
		}
		if (state.camControl.moveRight) {
			state.camControl.y += sin(state.camControl.phi) * state.camControl.speed * dt;
			state.camControl.x += cos(state.camControl.phi) * state.camControl.speed * dt;
		}

		// Update: compute matrices
		//Mat44f model2world = make_rotation_y(angle);

		Mat44f Rx = make_rotation_x(state.camControl.theta); 
		Mat44f Ry = make_rotation_y(state.camControl.phi); 


		Vec4f t2 = { -state.camControl.x, -state.camControl.z, -state.camControl.y, 0 };
		Vec3f t3 = { t2.x, t2.y, t2.z };
		Mat44f T = make_translation(t3);

		Mat44f world2camera = Rx * Ry * T;

		Mat44f projection = make_perspective_projection(90.f * 3.1415926f / 180.f,  fbwidth/float(fbheight), 0.1f, 100.0f);

		Mat44f projCameraWorld = projection * world2camera;

		//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

		// Draw scene
		OGL_CHECKPOINT_DEBUG();

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glUseProgram(prog.programId());

		Vec3f lightPos = { -20.f, 100.f, -20.f };
		Vec3f lightColour = { 1.f, 1.f, 1.f };

		glUniform3f(1, lightPos.x, lightPos.y, lightPos.z);
		glUniform3f(2, lightColour.x, lightColour.y, lightColour.z);

		for (int i = 0; i < vaos.num; i++) {
			if (vaos.textured.at(i) == 1) {
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, barkTexture);
				glUniformMatrix4fv(0, 1, GL_TRUE, projCameraWorld.v);
				glUniform1i(3, 1);
				glBindVertexArray(vaos.vaos.at(i));
				glDrawArrays(GL_TRIANGLES, 0, vaos.vao_size.at(i));
				glBindTexture(GL_TEXTURE_2D, 0);
			}
			else if (vaos.textured.at(i) == 2) {
				glUniform1i(3, 2);
				glBindTexture(GL_TEXTURE_2D, leafTexture);
				glUniformMatrix4fv(0, 1, GL_TRUE, projCameraWorld.v);
				glBindVertexArray(vaos.vaos.at(i));
				glDrawArrays(GL_TRIANGLES, 0, vaos.vao_size.at(i));
				glBindTexture(GL_TEXTURE_2D, 0);
			}
			else if (vaos.textured.at(i) == 0) {
				glUniform1i(3, 0);
				glUniformMatrix4fv(0, 1, GL_TRUE, projCameraWorld.v);
				glBindVertexArray(vaos.vaos.at(i));
				glDrawArrays(GL_TRIANGLES, 0, vaos.vao_size.at(i));
			}
		}

		glBindVertexArray(0);
		glUseProgram(0);

		OGL_CHECKPOINT_DEBUG();

		// Display results
		glfwSwapBuffers( window );
	}

	// Cleanup.
	state.prog = nullptr;

	//TODO: additional cleanup
	https://www.google.com/url?sa=i&url=https%3A%2F%2Fwww.quora.com%2FWhat-is-the-value-of-sin-45&psig=AOvVaw03mN4jDG6COWxE0zSY9l_f&ust=1672486326273000&source=images&cd=vfe&ved=0CA8QjRxqFwoTCPDey5yfofwCFQAAAAAdAAAAABAE
	return 0;
}
catch( std::exception const& eErr )
{
	std::fprintf( stderr, "Top-level Exception (%s):\n", typeid(eErr).name() );
	std::fprintf( stderr, "%s\n", eErr.what() );
	std::fprintf( stderr, "Bye.\n" );
	return 1;
}


namespace
{
	void glfw_callback_error_(int aErrNum, char const* aErrDesc)
	{
		std::fprintf(stderr, "GLFW error: %s (%d)\n", aErrDesc, aErrNum);
	}

	void glfw_callback_key_(GLFWwindow* aWindow, int aKey, int, int aAction, int)
	{
		if (GLFW_KEY_ESCAPE == aKey && GLFW_PRESS == aAction)
		{
			glfwSetWindowShouldClose(aWindow, GLFW_TRUE);
			return;
		}

		if (auto* state = static_cast<State_*>(glfwGetWindowUserPointer(aWindow)))
		{
			// R-key reloads shaders.
			if (GLFW_KEY_R == aKey && GLFW_PRESS == aAction)
			{
				if (state->prog)
				{
					try
					{
						state->prog->reload();
						std::fprintf(stderr, "Shaders reloaded and recompiled.\n");
					}
					catch (std::exception const& eErr)
					{
						std::fprintf(stderr, "Error when reloading shader:\n");
						std::fprintf(stderr, "%s\n", eErr.what());
						std::fprintf(stderr, "Keeping old shader.\n");
					}
				}
			}

			// Space toggles camera
			if (GLFW_KEY_SPACE == aKey && GLFW_PRESS == aAction)
			{
				state->camControl.cameraActive = !state->camControl.cameraActive;

				if (state->camControl.cameraActive)
					glfwSetInputMode(aWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
				else
					glfwSetInputMode(aWindow, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
			}

			// Camera controls if camera is active
			if (state->camControl.cameraActive)
			{
				if (GLFW_KEY_W == aKey)
				{
					if (GLFW_PRESS == aAction)
						state->camControl.actionZoomIn = true;
					else if (GLFW_RELEASE == aAction)
						state->camControl.actionZoomIn = false;
				}
				else if (GLFW_KEY_S == aKey)
				{
					if (GLFW_PRESS == aAction)
						state->camControl.actionZoomOut = true;
					else if (GLFW_RELEASE == aAction)
						state->camControl.actionZoomOut = false;
				}
				else if (GLFW_KEY_A == aKey)
				{
					if (GLFW_PRESS == aAction)
						state->camControl.moveLeft = true;
					else if (GLFW_RELEASE == aAction)
						state->camControl.moveLeft = false;
				}
				else if (GLFW_KEY_D == aKey)
				{
					if (GLFW_PRESS == aAction)
						state->camControl.moveRight = true;
					else if (GLFW_RELEASE == aAction)
						state->camControl.moveRight = false;
				}
				else if (GLFW_KEY_Q == aKey)
				{
					if (GLFW_PRESS == aAction)
						state->camControl.moveDown = true;
					else if (GLFW_RELEASE == aAction)
						state->camControl.moveDown = false;
				}
				else if (GLFW_KEY_E == aKey)
				{
					if (GLFW_PRESS == aAction)
						state->camControl.moveUp = true;
					else if (GLFW_RELEASE == aAction)
						state->camControl.moveUp = false;
				}
				else if (GLFW_KEY_LEFT_SHIFT == aKey)
				{
					if (GLFW_PRESS == aAction)
						state->camControl.fast = true;
					else if (GLFW_RELEASE == aAction)
						state->camControl.fast = false;
				}
			}
		}
	}

	void glfw_callback_motion_(GLFWwindow* aWindow, double aX, double aY)
	{
		if (auto* state = static_cast<State_*>(glfwGetWindowUserPointer(aWindow)))
		{
			if (state->camControl.cameraActive)
			{
				auto const dx = float(-(aX - state->camControl.lastX));
				auto const dy = float((aY - state->camControl.lastY));

				state->camControl.phi -= dx * kMouseSensitivity_;

				state->camControl.theta += dy * kMouseSensitivity_;

				if (state->camControl.theta > M_PI / 2.f)
					state->camControl.theta = M_PI / 2.f;
				else if (state->camControl.theta < -M_PI / 2.f)
					state->camControl.theta = -M_PI / 2.f;
			}

			state->camControl.lastX = float(aX);
			state->camControl.lastY = float(aY);
		}
	}
}

namespace
{
	GLFWCleanupHelper::~GLFWCleanupHelper()
	{
		glfwTerminate();
	}

	GLFWWindowDeleter::~GLFWWindowDeleter()
	{
		if( window )
			glfwDestroyWindow( window );
	}
}

