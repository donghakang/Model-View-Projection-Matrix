// This code was written by Matt Overby while a TA for CSci5607

// The loaders are included by glfw3 (glcorearb.h) if we are not using glew.
#ifdef USE_GLEW
#include <GL/glew.h>
#endif
#include <GLFW/glfw3.h>

// Includes
#include "trimesh.hpp"
#include "shader.hpp"
#include <cstring> // memcpy


#include <iostream>
using namespace std;

// Constants
#define WIN_WIDTH 500
#define WIN_HEIGHT 500
#define PI 3.14159265359

class Mat4x4 {
public:

	float m[16];

	Mat4x4(){ // Default: Identity
		m[0] = 1.f;  m[4] = 0.f;  m[8]  = 0.f;  m[12] = 0.f;
		m[1] = 0.f;  m[5] = 1.f;  m[9]  = 0.f;  m[13] = 0.f;
		m[2] = 0.f;  m[6] = 0.f;  m[10] = 1.f;  m[14] = 0.f;
		m[3] = 0.f;  m[7] = 0.f;  m[11] = 0.f;  m[15] = 1.f;
	}

	void make_identity(){
		m[0] = 1.f;  m[4] = 0.f;  m[8]  = 0.f;  m[12] = 0.f;
		m[1] = 0.f;  m[5] = 1.f;  m[9]  = 0.f;  m[13] = 0.f;
		m[2] = 0.f;  m[6] = 0.f;  m[10] = 1.f;  m[14] = 0.f;
		m[3] = 0.f;  m[7] = 0.f;  m[11] = 0.f;  m[15] = 1.f;
	}

	void print(){
		std::cout << m[0] << ' ' <<  m[4] << ' ' <<  m[8]  << ' ' <<  m[12] << "\n";
		std::cout << m[1] << ' ' <<   m[5] << ' ' <<  m[9]  << ' ' <<   m[13] << "\n";
		std::cout << m[2] << ' ' <<   m[6] << ' ' <<  m[10] << ' ' <<   m[14] << "\n";
		std::cout << m[3] << ' ' <<   m[7] << ' ' <<  m[11] << ' ' <<   m[15] << "\n";
	}

	void make_scale(float x, float y, float z){
		make_identity();
		m[0] = x; m[5] = y; m[10] = x;
	}

	// void rotate_x


};

static inline const Vec3f operator*(const Mat4x4 &m, const Vec3f &v){
	Vec3f r( m.m[0]*v[0]+m.m[4]*v[1]+m.m[8]*v[2],
		m.m[1]*v[0]+m.m[5]*v[1]+m.m[9]*v[2],
		m.m[2]*v[0]+m.m[6]*v[1]+m.m[10]*v[2] );
	return r;
}


//
//	Global state variables
//
namespace Globals {
	double cursorX, cursorY; // cursor positions
	float win_width, win_height; // window size
	float aspect;
	GLuint verts_vbo[1], colors_vbo[1], normals_vbo[1], faces_ibo[1], tris_vao;
	TriMesh mesh;

	float left, right, top, bottom, near, far;

	//  eye position, view direction, up direction
	float eye_position;
	int   view_position;

	Vec3f camera;
	Vec3f up_direction;
	Vec3f view_direction;

	//  Model, view and projection matrices, initialized to the identity
	Mat4x4 model;
	Mat4x4 view;
	Mat4x4 projection;
}


// ViewMatrix
// eye is eye position, up is up direction, view is view direction
Mat4x4 ViewMatrix (Vec3f eye, Vec3f up, Vec3f view) {
	Vec3f n;
	n.data[0] = -view.data[0];
	n.data[1] = -view.data[1];
	n.data[2] = -view.data[2];
	n.normalize();
	Vec3f u = up.cross(n);
	u.normalize();
	Vec3f v = n.cross(u);
	v.normalize();

	float dx = -(eye.dot(u));
	float dy = -(eye.dot(v));
	float dz = -(eye.dot(n));

	Mat4x4 m;
	m.m[0] = u[0];  m.m[4] = u[1];  m.m[8]  = u[2];  m.m[12] = dx;
	m.m[1] = v[0];  m.m[5] = v[1];  m.m[9]  = v[2];  m.m[13] = dy;
	m.m[2] = n[0];  m.m[6] = n[1];  m.m[10] = n[2];  m.m[14] = dz;
	m.m[3] =  0.f;  m.m[7] =  0.f;  m.m[11] =  0.f;  m.m[15] = 1.f;

	return m;
}

Vec3f rotateLeft (Vec3f view_dir) {
	Mat4x4 m;
	float theta = (10 % 360) / 180.f * PI;
	m.m[0] = cos(theta);  m.m[4] = 0.f;  m.m[8]  = sin(theta);  m.m[12] = 0.f;
	m.m[1] = 0.f; 			  m.m[5] = 1.f;  m.m[9]  = 0.f;  				m.m[13] = 0.f;
	m.m[2] = -sin(theta); m.m[6] = 0.f;  m.m[10] = cos(theta);  m.m[14] = 0.f;
	m.m[3] = 0.f;  				m.m[7] = 0.f;  m.m[11] = 0.f;  				m.m[15] = 1.f;
	return m * view_dir;
}

Vec3f rotateRight (Vec3f view_dir) {
	Mat4x4 m;
	float theta = (-10 % 360) / 180.f * PI;
	m.m[0] = cos(theta);  m.m[4] = 0.f;  m.m[8]  = sin(theta);  m.m[12] = 0.f;
	m.m[1] = 0.f; 			  m.m[5] = 1.f;  m.m[9]  = 0.f;  				m.m[13] = 0.f;
	m.m[2] = -sin(theta); m.m[6] = 0.f;  m.m[10] = cos(theta);  m.m[14] = 0.f;
	m.m[3] = 0.f;  				m.m[7] = 0.f;  m.m[11] = 0.f;  				m.m[15] = 1.f;
	return m * view_dir;
}

// Projection Matrix
Mat4x4 ProjectionMatrix (float near, float far, float left, float right,
												 float bottom, float top) {
  Mat4x4 m;
	m.m[0] = (2.0 * near)/(right-left);  m.m[4] = 0.f;  m.m[8] = (right+left)/(right-left);  m.m[12] = 0.f;
	m.m[1] = 0.f;  m.m[5] = (2.0 * near)/(top-bottom);  m.m[9] = (top+bottom)/(top-bottom);  m.m[13] = 0.f;
	m.m[2] = 0.f;  m.m[6] = 0.f;  m.m[10] = -(far+near)/(far-near);  m.m[14] = (-2.0f * far * near)/(far-near);
	m.m[3] = 0.f;  m.m[7] =  0.f;  m.m[11] =  -1.f;  m.m[15] = 0.f;
	return m;
}



//
//	Callbacks
//
static void error_callback(int error, const char* description){ fprintf(stderr, "Error: %s\n", description); }

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods){
	// Close on escape or Q
	if( action == GLFW_PRESS ){
		switch ( key ) {
			case GLFW_KEY_ESCAPE: glfwSetWindowShouldClose(window, GL_TRUE); break;
			case GLFW_KEY_Q: glfwSetWindowShouldClose(window, GL_TRUE); break;
			// change view direction
			case GLFW_KEY_LEFT:
				Globals::view_direction = rotateLeft(Globals::view_direction);
				Globals::view = ViewMatrix(Globals::camera, Globals::up_direction, Globals::view_direction);
				break;
			case GLFW_KEY_RIGHT:
				Globals::view_direction = rotateRight(Globals::view_direction);
				Globals::view = ViewMatrix(Globals::camera, Globals::up_direction, Globals::view_direction);
				break;
			case GLFW_KEY_UP:
				// Globals::eye_position += 1;
				// Globals::camera[2] += 0.1f;
				// Globals::view = ViewMatrix(Globals::camera, Globals::up_direction, Globals::view_direction);

				Globals::near += 0.2f;
				Globals::far  += 0.2f;
				Globals::projection = ProjectionMatrix(Globals::near, Globals::far, Globals::right, Globals::left,
																							 Globals::bottom, Globals::top);
				break;
			case GLFW_KEY_DOWN:
				// Globals::camera[2] -= 0.1f;
				// Globals::view = ViewMatrix(Globals::camera, Globals::up_direction, Globals::view_direction);
				Globals::near -= 0.2f;
				Globals::far  -= 0.2f;
				Globals::projection = ProjectionMatrix(Globals::near, Globals::far, Globals::right, Globals::left,
																							 Globals::bottom, Globals::top);
				break;
		}
	}
}

static void framebuffer_size_callback(GLFWwindow* window, int width, int height){
	Globals::win_width = float(width);
	Globals::win_height = float(height);
  Globals::aspect = Globals::win_width/Globals::win_height;

	if (Globals::aspect > 1.f) {
		// width is longer
		float aspect_flip = 1.f / Globals::aspect;
		Globals::projection = ProjectionMatrix(Globals::near, Globals::far, Globals::right/aspect_flip , Globals::left/aspect_flip ,
																					 Globals::bottom, Globals::top);
	} else {
		// height is longer
		Globals::projection = ProjectionMatrix(Globals::near, Globals::far, Globals::right, Globals::left,
																					 Globals::bottom/Globals::aspect, Globals::top/Globals::aspect);
	}

  glViewport(0,0,width,height);
}


// Function to set up geometry
void init_scene();

//
//	Main
//
int main(int argc, char *argv[]){


	// Load the mesh
	std::stringstream obj_file; obj_file << MY_DATA_DIR << "sibenik/sibenik.obj";
	if( !Globals::mesh.load_obj( obj_file.str() ) ){ return 0; }
	Globals::mesh.print_details();

	// Scale to fit in (-1,1): a temporary measure to allow the entire model to be visible
    	// Should be replaced by the use of an appropriate projection matrix
    	// Original model dimensions: center = (0,0,0); height: 30.6; length: 40.3; width: 17.0
    	float min, max, scale;
   	 min = Globals::mesh.vertices[0][0]; max = Globals::mesh.vertices[0][0];
	for( int i=0; i<Globals::mesh.vertices.size(); ++i ){
           if (Globals::mesh.vertices[i][0] < min) min = Globals::mesh.vertices[i][0];
           else if (Globals::mesh.vertices[i][0] > max) max = Globals::mesh.vertices[i][0];
           if (Globals::mesh.vertices[i][1] < min) min = Globals::mesh.vertices[i][1];
           else if (Globals::mesh.vertices[i][1] > max) max = Globals::mesh.vertices[i][1];
           if (Globals::mesh.vertices[i][2] < min) min = Globals::mesh.vertices[i][2];
           else if (Globals::mesh.vertices[i][2] > max) max = Globals::mesh.vertices[i][2];
    	}
    	if (min < 0) min = -min;
    	if (max > min) scale = 1/max; else scale = 1/min;

	Mat4x4 mscale; mscale.make_scale( scale, scale, scale );
	for( int i=0; i<Globals::mesh.vertices.size(); ++i ){
           Globals::mesh.vertices[i] = mscale*Globals::mesh.vertices[i];
    	}

	// Set up window
	GLFWwindow* window;
	glfwSetErrorCallback(&error_callback);

	// Initialize the window
	if( !glfwInit() ){ return EXIT_FAILURE; }

	// Ask for OpenGL 3.2
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

	// Create the glfw window
	Globals::win_width = WIN_WIDTH;
	Globals::win_height = WIN_HEIGHT;
	window = glfwCreateWindow(int(Globals::win_width), int(Globals::win_height), "HW2c", NULL, NULL);
	if( !window ){ glfwTerminate(); return EXIT_FAILURE; }

	// Bind callbacks to the window
	glfwSetKeyCallback(window, &key_callback);
	glfwSetFramebufferSizeCallback(window, &framebuffer_size_callback);

	// Make current
	glfwMakeContextCurrent(window);
	glfwSwapInterval(1);

	// Initialize glew AFTER the context creation and before loading the shader.
	// Note we need to use experimental because we're using a modern version of opengl.
	#ifdef USE_GLEW
		glewExperimental = GL_TRUE;
		glewInit();
	#endif

	// Initialize the shader (which uses glew, so we need to init that first).
	// MY_SRC_DIR is a define that was set in CMakeLists.txt which gives
	// the full path to this project's src/ directory.
	mcl::Shader shader;
	std::stringstream ss; ss << MY_SRC_DIR << "shader.";
	shader.init_from_files( ss.str()+"vert", ss.str()+"frag" );

	// Initialize the scene
	// IMPORTANT: Only call after gl context has been created
	init_scene();
	framebuffer_size_callback(window, int(Globals::win_width), int(Globals::win_height));

	// Initialize OpenGL
	glEnable(GL_DEPTH_TEST);
	glClearColor(1.f,1.f,1.f,1.f);

	// Enable the shader, this allows us to set uniforms and attributes
	shader.enable();


	Vec3f eye 		 = Globals::camera;
	Vec3f up_dir 	 = Globals::up_direction;
	Vec3f view_dir = Globals::view_direction;

	// update view matrix.
	Globals::view = ViewMatrix(eye, up_dir, view_dir);


	// Bind buffers
	glBindVertexArray(Globals::tris_vao);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, Globals::faces_ibo[0]);

	// Game loop
	while( !glfwWindowShouldClose(window) ){

		// Clear screen
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Send updated info to the GPU
		glUniformMatrix4fv( shader.uniform("model"), 1, GL_FALSE, Globals::model.m  ); // model transformation
		glUniformMatrix4fv( shader.uniform("view"), 1, GL_FALSE, Globals::view.m  ); // viewing transformation
		glUniformMatrix4fv( shader.uniform("projection"), 1, GL_FALSE, Globals::projection.m ); // projection matrix
		glUniform3f( shader.uniform("eye"), eye[0], eye[1], eye[2] ); // used in fragment shader

		// Draw
		glDrawElements(GL_TRIANGLES, Globals::mesh.faces.size()*3, GL_UNSIGNED_INT, 0);

		// Finalize
		glfwSwapBuffers(window);
		glfwPollEvents();

	} // end game loop

	// Unbind
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	// Disable the shader, we're done using it
	shader.disable();

	return EXIT_SUCCESS;
}


void init_scene(){
	using namespace Globals;

	// initializing
  Globals::eye_position = 0.0f;

	Globals::model.make_scale(1,1,1);

	Globals::camera 				= Vec3f(0.f, 0.f, -5.f);
	Globals::up_direction   = Vec3f(0.f, 1.f, 0.f);
  Globals::view_direction = Vec3f(0.f, 0.f, 1.f);
	Globals::view = ViewMatrix(Globals::camera, Globals::up_direction, Globals::view_direction);


	Globals::left = -1.f;
	Globals::right = 1.f;
	Globals::bottom = -1.f;
	Globals::top = 1.f;
	Globals::far =  10.f;
	Globals::near = 1.f;
	Globals::projection = ProjectionMatrix(Globals::near, Globals::far, Globals::right, Globals::left,
																				 Globals::bottom, Globals::top);

	// Create the buffer for vertices
	glGenBuffers(1, verts_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, verts_vbo[0]);
	glBufferData(GL_ARRAY_BUFFER, mesh.vertices.size()*sizeof(mesh.vertices[0]), &mesh.vertices[0][0], GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// Create the buffer for colors
	glGenBuffers(1, colors_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, colors_vbo[0]);
	glBufferData(GL_ARRAY_BUFFER, mesh.colors.size()*sizeof(mesh.colors[0]), &mesh.colors[0][0], GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// Create the buffer for normals
	glGenBuffers(1, normals_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, normals_vbo[0]);
	glBufferData(GL_ARRAY_BUFFER, mesh.normals.size()*sizeof(mesh.normals[0]), &mesh.normals[0][0], GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// Create the buffer for indices
	glGenBuffers(1, faces_ibo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, faces_ibo[0]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh.faces.size()*sizeof(mesh.faces[0]), &mesh.faces[0][0], GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	// Create the VAO
	glGenVertexArrays(1, &tris_vao);
	glBindVertexArray(tris_vao);

	int vert_dim = 3;

	// location=0 is the vertex
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, verts_vbo[0]);
	glVertexAttribPointer(0, vert_dim, GL_FLOAT, GL_FALSE, sizeof(mesh.vertices[0]), 0);

	// location=1 is the color
	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, colors_vbo[0]);
	glVertexAttribPointer(1, vert_dim, GL_FLOAT, GL_FALSE, sizeof(mesh.colors[0]), 0);

	// location=2 is the normal
	glEnableVertexAttribArray(2);
	glBindBuffer(GL_ARRAY_BUFFER, normals_vbo[0]);
	glVertexAttribPointer(2, vert_dim, GL_FLOAT, GL_FALSE, sizeof(mesh.normals[0]), 0);

	// Done setting data for the vao
	glBindVertexArray(0);

}
