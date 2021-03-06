#include "gl_calls.h"

#include <stdio.h>
#include <stdlib.h>


void check_error(int condition, const char* error_text)
{
	if (!condition)
	{
		fprintf(stderr, "%s\n", error_text);
		exit(EXIT_FAILURE);
	}
}

void gl_check_error(const char* error_text)
{
	GLenum error = glGetError();

	if (error != GL_NO_ERROR)
	{
		fprintf(stderr, "GLError: %s (%d)\n", error_text, error);
		exit(EXIT_FAILURE);
	}
}

// Calc the x postion of vertex
GLfloat calc_X(GLfloat part_angle, int iteration)
{
	return 0.5f * cos(part_angle * iteration);
}

// Calc the y position of vertex
GLfloat calc_Y(GLfloat part_angle, int iteration)
{
	return 0.5f * sin(part_angle * iteration);
}

// Write all vertex position data in buffer
void calc_pos_vertex(int corners, vertex_data_t* vertex_data)
{
	GLfloat part_angle = 2 * M_PI / corners;

	vertex_data[0].position[0] = 0.0f;
	vertex_data[0].position[1] = 0.0f;
	vertex_data[0].position[2] = 0.0f;

	for(int i = 1; i < corners +2; i++)
	{
		vertex_data[i].position[0] = calc_X(part_angle, i-1);
		vertex_data[i].position[1] = calc_Y(part_angle, i-1);
		vertex_data[i].position[2] = 0 ;
	}

}

// Calc the RGB color values dependent from 
void calc_RGB_value(GLfloat part_angle, int iteration, GLubyte* R, GLubyte* G, GLubyte* B)
{
	
	int C = 1;
	float X, H, H_dash;

	H = part_angle * iteration;
	H_dash = H / (M_PI / 3.0f);

	float val_abs = fmod(H_dash,  2.0f);
	X = C * (1 - fabsf((val_abs) - 1));
	
	if( H_dash >= 0 && H_dash <= 1 )
	{
		*R = 255 * C;
		*G = 255 * X;
		*B = 0;
	}
	else if( H_dash > 1 && H_dash <= 2 )
	{
		*R = 255 * X;
		*G = 255 * C;
		*B = 0;
	}
	else if( H_dash > 2 && H_dash <= 3 )
	{
		*R = 0;
		*G = 255 * C;
		*B = 255 * X;
	}
	else if( H_dash > 3 && H_dash <= 4 )
	{
		*R = 0;
		*G = 255 * X;
		*B = 255 * C;
	}
	else if( H_dash > 4 && H_dash <= 5)
	{
		*R = 255 * X;
		*G = 0;
		*B = 255 * C;
	}
	else if( H_dash > 5 && H_dash <= 6)
	{
		*R = 255 * C;
		*G = 0;
		*B = 255 * X;
	}

}

// Write all vertex color values in buffer
void calc_color_vertex(int corners, vertex_data_t* vertex_data)
{
	GLubyte R = 0;
	GLubyte G = 0;
	GLubyte B = 0;
	GLfloat part_angle = (2 * M_PI) / corners;


	vertex_data[0].color[0] = 255;
	vertex_data[0].color[1] = 255;
	vertex_data[0].color[2] = 255;


	for(int i = 1; i < corners + 2; i++)
	{
		calc_RGB_value(part_angle, i-1, &R, &G, &B);

		vertex_data[i].color[0] = R;
		vertex_data[i].color[1] = G;
		vertex_data[i].color[2] = B;
	}

}


char* read_shader_source_from_file(const char* path)
{
	// Open the file:
	FILE* file = fopen(path , "rb");
	check_error(file != NULL, "Failed to open file.");

	// Seek to the end:
	int success = fseek(file, 0, SEEK_END);
	check_error(success == 0, "Failed to forward file pointer.");

	// Obtain the size of the file from the position of the file pointer:
	long size = ftell(file);
	check_error(size >= 0, "Failed to determine file size.");

	// Rewind the file pointer back to the start:
	rewind(file);

	// Allocate the output buffer:
	char* buffer = malloc(size + 1);
	check_error(buffer != NULL, "Failed to allocate buffer.");

	// Read into it:
	size_t blocks_read = fread(buffer, size, 1, file);
	check_error(blocks_read == 1, "Failed to read file.");

	// Close the file:
	success = fclose(file);
	check_error(success == 0, "Failed to close file.");

	// Append a null-terminator and return the buffer:
	buffer[size] = '\0';

	return buffer;
}

GLuint compile_shader(GLenum type, const char* shader_path, const char* shader_tag)
{
	// Create an empty shader:
	GLuint shader = glCreateShader(type);
	gl_check_error("glCreateShader");

	// Read and specify the source code:
	char* shader_source = read_shader_source_from_file(shader_path);

	glShaderSource(shader, 1, (const char**)&shader_source, NULL);
	gl_check_error("glShaderSource");

	free(shader_source);

	// Compile the shader:
	glCompileShader(shader);
	gl_check_error("glCompileShader");

	// Check the compilation status:
	GLint success;

	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
	gl_check_error("glGetShaderiv");

	if (success)
	{
		return shader;
	}

	// Extract the length of the error message (including '\0'):
	GLint info_length = 0;

	glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &info_length);
	gl_check_error("glGetShaderiv");

	if (info_length > 1)
	{
		// Extract the message itself:
		char* info = malloc(info_length);
		check_error(info != NULL, "Failed to allocate memory for error message.");

		glGetShaderInfoLog(shader, info_length, NULL, info);
		gl_check_error("glGetShaderInfoLog");

		fprintf(stderr, "Error compiling shader (%s): %s\n", shader_tag, info);
		free(info);
	}
	else
	{
		fprintf(stderr, "No info log from the shader compiler :(\n");
	}

	exit(EXIT_FAILURE);
}

void init_shader_program(user_data_t* user_data)
{
	// Create the vertex shader:
	printf("Compiling vertex shader ...\n");
	GLuint vertex_shader = compile_shader(GL_VERTEX_SHADER, "shader/vertex.glsl", "Vertex shader");

	// Create the fragment shader:
	printf("Compiling fragment shader ...\n");
	GLuint fragment_shader = compile_shader(GL_FRAGMENT_SHADER, "shader/fragment.glsl", "Fragment shader");

	// We can now release the shader compiler.
	glReleaseShaderCompiler();
	gl_check_error("glReleaseShaderCompiler");

	// Create an empty shader program:
	printf("Creating shader program ...\n");

	GLuint shader_program = glCreateProgram();
	gl_check_error("glCreateProgram");

	// Attach both shaders to the program:
	glAttachShader(shader_program, vertex_shader);
	gl_check_error("glAttachShader [vertex]");

	glAttachShader(shader_program, fragment_shader);
	gl_check_error("glAttachShader [fragment]");

	// Link the shader program:
	glLinkProgram(shader_program);
	gl_check_error("glLinkProgram");

	// Detach the shaders after linking:
	glDetachShader(shader_program, vertex_shader);
	gl_check_error("glDetachShader [vertex]");

	glDetachShader(shader_program, fragment_shader);
	gl_check_error("glDetachShader [fragment]");

	// Delete the shaders:
	glDeleteShader(vertex_shader);
	gl_check_error("glDeleteShader [vertex]");

	glDeleteShader(fragment_shader);
	gl_check_error("glDeleteShader [fragment]");

	// Check the link status:
	GLint success;

	glGetProgramiv(shader_program, GL_LINK_STATUS, &success);
	gl_check_error("glGetProgramiv");

	if (success)
	{
		// Use the program from now on:
		glUseProgram(shader_program);
		gl_check_error("glUseProgram");

		// Store it inside our user data struct:
		user_data->shader_program = shader_program;

		return;
	}

	// Extract the length of the error message (including '\0'):
	GLint info_length = 0;
	glGetProgramiv(shader_program, GL_INFO_LOG_LENGTH, &info_length);
	gl_check_error("glGetProgramiv");

	if (info_length > 1)
	{
		// Extract the message itself:
		char* info = malloc(info_length);
		check_error(info != NULL, "Failed to allocate memory for error message.");

		glGetProgramInfoLog(shader_program, info_length, NULL, info);
		gl_check_error("glGetProgramInfoLog");

		fprintf(stderr, "Error linking shader program: %s\n", info);
		free(info);
	}
	else
	{
		fprintf(stderr, "No info log from the shader compiler :(\n");
	}

	exit(EXIT_FAILURE);
}


void init_vertex_data(user_data_t* user_data, int corners)
{
	// Polygon Data:
	vertex_data_t* vertex_data = (vertex_data_t*)malloc(sizeof(vertex_data_t) * (corners + 2));
	calc_pos_vertex(corners, vertex_data);
	calc_color_vertex(corners, vertex_data);
	


	// TODO: blackbox! Create a VAO.
	GLuint vao;

	glGenVertexArrays(1, &vao);
	gl_check_error("glGenVertexArrays");

	glBindVertexArray(vao);
	gl_check_error("glBindVertexArray");

	// Store the VAO inside our user data:
	user_data->vao = vao;

	// Generate and bind a vertex buffer object:
	GLuint vbo;

	glGenBuffers(1, &vbo);
	gl_check_error("glGenBuffers");

	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	gl_check_error("glBindBuffer");

	// Upload the vertex data to the GPU:
	glBufferData(GL_ARRAY_BUFFER, (corners + 2)  * sizeof(vertex_data_t), (const GLvoid*)vertex_data, GL_STATIC_DRAW);
	gl_check_error("glBufferData");

	// Position attribute:
	// Number of attribute, number of components, data type, normalize, stride, pointer (= offset)
	glVertexAttribPointer(ATTRIB_POSITION, 3, GL_FLOAT, GL_FALSE, sizeof(vertex_data_t), (GLvoid*)offsetof(vertex_data_t, position));
	gl_check_error("glVertexAttribPointer [position]");

	glVertexAttribPointer(ATTRIB_COLOR, 3, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(vertex_data_t), (GLvoid*)offsetof(vertex_data_t, color));
	gl_check_error("glVertexAttribPointer [color]");

	// Enable the attributes:
	glEnableVertexAttribArray(ATTRIB_POSITION);
	gl_check_error("glEnableVertexAttribArray [position]");

	glEnableVertexAttribArray(ATTRIB_COLOR);
	gl_check_error("glEnableVertexAttribArray [color]");

	// Store the VBO inside our user data:
	user_data->vbo = vbo;

	free(vertex_data);
}

void init_gl(GLFWwindow* window, int corners)
{
	user_data_t* user_data = glfwGetWindowUserPointer(window);

	// Initialize our shader program:
	init_shader_program(user_data);

	// Initialize our vertex data:
	init_vertex_data(user_data, corners);

	// Obtain the internal size of the framebuffer:
	int fb_width, fb_height;
	glfwGetFramebufferSize(window, &fb_width, &fb_height);

	// Align the viewport to the framebuffer:
	glViewport(0, 0, fb_width, fb_height);
	gl_check_error("glViewport");

	// Specify the clear color:
	glClearColor(0.1, 0.1, 0.1, 1);
	gl_check_error("glClearColor");
}

void draw_gl(GLFWwindow* window, int corners)
{
	// Clear the color buffer -> background color:
	glClear(GL_COLOR_BUFFER_BIT);
	gl_check_error("glClear");

	// Draw our stuff!
	// Parameters: primitive type, start index, count
	glDrawArrays(GL_TRIANGLE_FAN, 0, corners + 2);
	gl_check_error("glDrawArrays");
}

void teardown_gl(GLFWwindow* window)
{
	printf("Tearing down ...\n");
	user_data_t* user_data = glfwGetWindowUserPointer(window);

	// Delete the shader program:
	glDeleteProgram(user_data->shader_program);
	gl_check_error("glDeleteProgram");

	// Delete the VAO:
	glDeleteVertexArrays(1, &user_data->vao);
	gl_check_error("glDeleteVertexArrays");

	// Delete the VBO:
	glDeleteBuffers(1, &user_data->vbo);
	gl_check_error("glDeleteBuffers");
}
