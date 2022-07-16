/** \file
gui.c --- gui for optical-flow

Copyright (C) 2021 Roman V. Prikhodchenko

Author: Roman V. Prikhodchenko <chujoii@gmail.com>


    This file is part of optical-flow.

    optical-flow is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    optical-flow is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with optical-flow.  If not, see <http://www.gnu.org/licenses/>.



Keywords: capture video4linux v4l2

Usage:

History:
    based on:
    https://learnopengl.com/

Code:
*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "gui-type.h"
#include "gui.h"
#include "const.h"
#include "util.h"

int space_step = false;



int init_gui (unsigned int window_width, unsigned int window_height)
{
	extern GLFWwindow* window; // fixme: global variable
	/* Initialize the library */
	if (!glfwInit()) {
		printf("Initialization of GLFW failed\n");
		return -1;
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

	/* Create a windowed mode window and its OpenGL context */
	window = glfwCreateWindow(window_width, window_height, "optical flow", NULL, NULL);
	if (!window) {
		printf("Failed to create GLFW window");
		glfwTerminate();
		return -1;
	}

	/* Make the window's context current */
	glfwMakeContextCurrent(window);

	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);


	glfwSetKeyCallback(window, key_callback);

	return 0;
}


/**

return video_texture
*/
unsigned int init_shader_video(void)
{
	extern GLFWwindow* window; // fixme: global variable

	// build and compile our shader program
	// ------------------------------------

	// set up vertex data (and buffer(s)) and configure vertex attributes
	// ------------------------------------------------------------------
	float vertices[] = {
		// positions          // colors           // texture coords
		1.0f,  1.0f, 0.0f,   1.0f, 0.0f, 0.0f,   1.0f, 1.0f, // top right
		1.0f, -1.0f, 0.0f,   0.0f, 1.0f, 0.0f,   1.0f, 0.0f, // bottom right
		-1.0f, -1.0f, 0.0f,   0.0f, 0.0f, 1.0f,   0.0f, 0.0f, // bottom left
		-1.0f,  1.0f, 0.0f,   1.0f, 1.0f, 0.0f,   0.0f, 1.0f  // top left
	};
	unsigned int indices[] = {
		0, 1, 3, // first triangle
		1, 2, 3  // second triangle
	};

	extern unsigned int VBOs[NUM_OF_SHADER]; // fixme: global variable
	extern unsigned int VAOs[NUM_OF_SHADER]; // fixme: global variable
	extern unsigned int EBOs[NUM_OF_SHADER]; // fixme: global variable

	glGenVertexArrays(NUM_OF_SHADER, VAOs);
	glGenBuffers(NUM_OF_SHADER, VBOs);
	glGenBuffers(NUM_OF_SHADER, EBOs);

	// bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
	glBindVertexArray(VAOs[SHADER_V]);

	glBindBuffer(GL_ARRAY_BUFFER, VBOs[SHADER_V]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBOs[SHADER_V]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	// position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	// color attribute
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	// texture coord attribute
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
	glEnableVertexAttribArray(2);

	unsigned int video_texture;
	glGenTextures(1, &video_texture); // This creates a new texture object (not need create every frame). The entire texture creation block up to and including the glTexImage2D() call should be done once in main(), but you should still call glBindTexture() in update().
	return video_texture;
}



void use_shader_video ()
{
	extern unsigned int VAOs[NUM_OF_SHADER]; // fixme: global variable

	int success;
	char infoLog[512];

	// vertex shader
	const char *vertexShaderSource =
		#include "GLSL/video.vert.quoted"
		;
	unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
	glCompileShader(vertexShader);
	// check for shader compile errors
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
	if(!success) {
		glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
		printf ("ERROR::SHADER::VERTEX::COMPILATION_FAILED\n%s\n", infoLog);
	}

	// fragment shader
	const char *fragmentShaderSource =
		#include "GLSL/video.frag.quoted"
		;
	unsigned int fragmentShader;
	fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
	glCompileShader(fragmentShader);
	// check if compilation was successful
	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
	if(!success) {
		glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
		printf ("ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n%s\n", infoLog);
	}

	// link shaders
	extern unsigned int shaderProgram_video; // fixme: global variable
	shaderProgram_video = glCreateProgram();
	glAttachShader(shaderProgram_video, vertexShader);
	glAttachShader(shaderProgram_video, fragmentShader);
	glLinkProgram(shaderProgram_video);
	// check for linking errors
	glGetProgramiv(shaderProgram_video, GL_LINK_STATUS, &success);
	if (!success) {
		glGetProgramInfoLog(shaderProgram_video, 512, NULL, infoLog);
		printf("ERROR::SHADER::PROGRAM::LINKING_FAILED %s\n", infoLog);
	}
	// linked shader objects into the program object, so it no longer need:
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);


	// draw our first triangle
	glUseProgram(shaderProgram_video);

	glBindVertexArray(VAOs[SHADER_V]);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}



void init_shader_widget(unsigned long polylini_size, float *polyline)
{
	extern GLFWwindow* window; // fixme: global variable

	extern unsigned int VBOs[NUM_OF_SHADER]; // fixme: global variable
	extern unsigned int VAOs[NUM_OF_SHADER]; // fixme: global variable
	extern unsigned int EBOs[NUM_OF_SHADER]; // fixme: global variable

	// ---------------------
	glBindVertexArray(VAOs[SHADER_W]);	// note that we bind to a different VAO now
	glBindBuffer(GL_ARRAY_BUFFER, VBOs[SHADER_W]);	// and a different VBO
	glBufferData(GL_ARRAY_BUFFER, polylini_size, polyline, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0); // because the vertex data is tightly packed we can also specify 0 as the vertex attribute's stride to let OpenGL figure it out
	glEnableVertexAttribArray(0);
	// glBindVertexArray(0); // not really necessary as well, but beware of calls that could affect VAOs while this one is bound (like binding element buffer objects, or enabling/disabling vertex attributes)
}



/** display widget: line [with points]

    \param size_point size of point in line graph: if size > 0, draw square at data points
*/
void use_shader_widget (unsigned long int num_components, unsigned long int sizeof_polyline, float *polyline, int size_point, int color_type)
{
	extern unsigned int VAOs[NUM_OF_SHADER]; // fixme: global variable

	int success;
	char infoLog[512];

	// vertex shader
	const char *vertexShaderSource_widget =
		#include "GLSL/widget.vert.quoted"
		;
	unsigned int vertexShader_widget = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader_widget, 1, &vertexShaderSource_widget, NULL);
	glCompileShader(vertexShader_widget);
	// check for shader compile errors
	glGetShaderiv(vertexShader_widget, GL_COMPILE_STATUS, &success);
	if(!success) {
		glGetShaderInfoLog(vertexShader_widget, 512, NULL, infoLog);
		printf ("ERROR::SHADER::VERTEX:widget:COMPILATION_FAILED\n%s\n", infoLog);
	}

	// fragment shader
	const char *fragmentShaderSource_widget =
		#include "GLSL/widget.frag.quoted"
		;

	// fixme: replace to strstr, memcpy (but char * == const string)
	const char *fragmentShaderSource_fire =
		#include "GLSL/fire.frag.quoted"
		;
	unsigned int fragmentShader_widget;
	fragmentShader_widget = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader_widget, 1, (color_type) ? &fragmentShaderSource_widget : &fragmentShaderSource_fire, NULL);
	glCompileShader(fragmentShader_widget);
	// check if compilation was successful
	glGetShaderiv(fragmentShader_widget, GL_COMPILE_STATUS, &success);
	if(!success) {
		glGetShaderInfoLog(fragmentShader_widget, 512, NULL, infoLog);
		printf ("ERROR::SHADER::FRAGMENT:widget:COMPILATION_FAILED\n%s\n", infoLog);
	}

	// link shaders
	extern unsigned int shaderProgram_widget; // fixme: global variable
	shaderProgram_widget = glCreateProgram();
	glAttachShader(shaderProgram_widget, vertexShader_widget);
	glAttachShader(shaderProgram_widget, fragmentShader_widget);
	glLinkProgram(shaderProgram_widget);
	// check for linking errors
	glGetProgramiv(shaderProgram_widget, GL_LINK_STATUS, &success);
	if (!success) {
		glGetProgramInfoLog(shaderProgram_widget, 512, NULL, infoLog);
		printf("ERROR::SHADER::PROGRAM::LINKING_FAILED %s\n", infoLog);
	}
	// linked shader objects into the program object, so it no longer need:
	glDeleteShader(vertexShader_widget);
	glDeleteShader(fragmentShader_widget);


	// draw our first triangle
	glUseProgram(shaderProgram_widget);

	unsigned long int number_vertices_polyline = sizeof_polyline / sizeof(polyline[0]) / num_components;

	glBindVertexArray(VAOs[SHADER_W]);
	glDrawArrays(GL_LINE_STRIP, 0, number_vertices_polyline);	// this call should output an widget
	if (size_point > 0) {
		glPointSize(size_point);
		glDrawArrays(GL_POINTS, 0, number_vertices_polyline);	// this call should output an widget
	}
}










/**

   \return press_key
*/
void render_loop (struct imgRawImage* gui_image, unsigned int video_texture)
{
	extern GLFWwindow* window; // fixme: global variable
	extern unsigned int shaderProgram_video; // fixme: global variable
	extern unsigned int shaderProgram_widget; // fixme: global variable
	extern unsigned int VBOs[NUM_OF_SHADER]; // fixme: global variable
	extern unsigned int VAOs[NUM_OF_SHADER]; // fixme: global variable
	extern unsigned int EBOs[NUM_OF_SHADER]; // fixme: global variable



	// load and create a texture
	// -------------------------

	glBindTexture(GL_TEXTURE_2D, video_texture); // all upcoming GL_TEXTURE_2D operations now have effect on this texture object
	// set the texture wrapping parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	// set texture filtering parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	// load image, create texture and generate mipmaps
	if (gui_image->lpData) // fixme: work?
	{
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, gui_image->width, gui_image->height, 0, GL_RGB, GL_UNSIGNED_BYTE, gui_image->lpData);
		//glGenerateMipmap(GL_TEXTURE_2D);
	}
	else
	{
		printf("Failed to load texture\n");
	}




	// render loop
	// -----------
	if (!glfwWindowShouldClose(window)) {






		// render
		// ------
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		// bind Texture
		glBindTexture(GL_TEXTURE_2D, video_texture);

		// render container
		use_shader_video();


		// glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
		// -------------------------------------------------------------------------------
		glfwSwapBuffers(window);
		glfwPollEvents();
	}
}



void deallocate_resources (void)
{
	extern GLFWwindow* window; // fixme: global variable
	extern unsigned int shaderProgram_video; // fixme: global variable
	extern unsigned int shaderProgram_widget; // fixme: global variable
	extern unsigned int VBOs[NUM_OF_SHADER]; // fixme: global variable
	extern unsigned int VAOs[NUM_OF_SHADER]; // fixme: global variable
	extern unsigned int EBOs[NUM_OF_SHADER]; // fixme: global variable
	// optional: de-allocate all resources once they've outlived their purpose:
	// ------------------------------------------------------------------------
	glDeleteVertexArrays(NUM_OF_SHADER, VAOs);
	glDeleteBuffers(NUM_OF_SHADER, VBOs);
	glDeleteBuffers(NUM_OF_SHADER, EBOs);

	// fixme: need?
	glDeleteProgram(shaderProgram_video);
	glDeleteProgram(shaderProgram_widget);


	// glfw: terminate, clearing all previously allocated GLFW resources.
	// ------------------------------------------------------------------
	glfwSetWindowShouldClose(window, true);
	printf("close window.\n");
	printf("terminate.\n");
	glfwTerminate();
}


/* if window change size */
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	extern struct imgRawImage* gui_image; // fixme: global variable
	extern float glob_zoom_ratio;
	extern float glob_canvas_shift_y;

	(void)window; // suppress "unused parameter" warnings

	float i_proportion = (float)gui_image->width / (float)gui_image->height;
	float c_proportion = (float)width / (float)height;

	int proportional_height = height;
	// resize viewport for image
	if (i_proportion < c_proportion) {
		int proportional_width = i_proportion*height;
		glViewport(0, 0, proportional_width, height);
	} else {
		proportional_height = width/i_proportion;
		glViewport(0, 0, width, proportional_height);
	}

	glob_zoom_ratio = get_zoom_ratio (width, height, gui_image->width, gui_image->height);
	glob_canvas_shift_y = -(height - proportional_height) / glob_zoom_ratio;
}


/* input events */
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	(void)window; // suppress "unused parameter" warnings
	(void)scancode; // suppress "unused parameter" warnings
	(void)mods; // suppress "unused parameter" warnings

	extern int hide_static_block;
	extern int escape_status;
	extern int verbose_type;

	const char* keyName;

	switch (key) {
	case GLFW_KEY_ESCAPE:
		escape_status = true;
		break;
	case GLFW_KEY_SPACE:
		if (action == GLFW_PRESS || action == GLFW_REPEAT) {
			hide_static_block ^= 1;
		}
		break;
	default:
		keyName = glfwGetKeyName(key, 0);
		if (keyName == NULL) return;
		if( 'v' == keyName[0] && 0 == keyName[1] && action == GLFW_PRESS) {
			verbose_type++;
			switch (verbose_type) {
			case VERBT_GRADER:
				printf("VERBT_GRADER\n");
				break;
			case VERBT_FILTER:
				printf("VERBT_FILTER\n");
				break;
			case VERBT_SECONDDERIVATIVE:
				printf("VERBT_SECONDDERIVATIVE\n");
				break;
			case VERBT_LAST:
				verbose_type = VERBT_GRADER;
				printf("VERBT_GRADER\n");
				break;
			}
		}
	}
}

//int processInput(GLFWwindow *window)
//{
//printf("aa\n");
/*
	if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
			printf("bb\n");

		glfwSetWindowShouldClose(window, true);
		return GLFW_KEY_ESCAPE;
	}
*/
//	return 0;
//}



void * start_gui_process(void *arguments)
{
	extern GLFWwindow* window; // fixme: global variable
	int index = *((int *)arguments);
	printf("gui started. thread: %d\n", index);

	/* Loop until the user closes the window */
	while (!glfwWindowShouldClose(window)) {
		// Render here
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);
		//render_loop();

		// Swap front and back buffers
		glfwSwapBuffers(window);

		// Poll for and process events
		glfwPollEvents();
	}
	return NULL;
}



/* draw directly on image */
void draw_crosshair (struct imgRawImage* image)
{
	unsigned long int raw_coord;
	unsigned long int shift_x = image->width / 2;
	unsigned long int shift_y = image->height / 2;
	unsigned char color;
	struct coord_2Du coord;

	// horizontal rule
	coord.y = shift_y;
	for (coord.x = 0; coord.x < image->width; coord.x++) {
		color = (coord.x % 2 == 0) ? 0xff : 0x00;
		raw_coord = coord_to_raw_chunk(image, coord);
		image->lpData[raw_coord]     = color;
		image->lpData[raw_coord + 1] = color;
		image->lpData[raw_coord + 2] = color;
	}

	// vertical rule
	coord.x = shift_x;
	for (coord.y = 0; coord.y < image->height; coord.y++) {
		color = (coord.y % 2 == 0) ? 0xff : 0x00;
		raw_coord = coord_to_raw_chunk(image, coord);
		image->lpData[raw_coord]     = color;
		image->lpData[raw_coord + 1] = color;
		image->lpData[raw_coord + 2] = color;
	}
}



/** draw directly on image

    color = 0, 1, 2
 */
void draw_graph (struct imgRawImage* draw_image, int len_data, float * data, int shift_x, int shift_y, int size_x, int size_y, int color)
{
	extern int verbose;
	extern int escape_status;
	extern unsigned int video_texture;

	struct coord_2Du coord;

	float min_val = data[0];
	float max_val = data[0];

	if (color < 0 || color > 2) return;

	for (int i = 0; i < len_data; i++) {
		min_val = MIN(min_val, data[i]);
		max_val = MAX(max_val, data[i]);
	}

	float scale_x = size_x / len_data;
	float scale_y = size_y / (max_val - min_val);

	for (int i = 0; i < len_data; i++) {
		coord.x = shift_x + (len_data - i - 1)*scale_x;
		coord.y = shift_y + (data[i]-min_val)*scale_y;
		unsigned long int raw_coord = coord_to_raw_chunk(draw_image, coord);
		if (raw_coord < draw_image->dwBufferBytes) {
			unsigned int old_color = draw_image->lpData[raw_coord + color] + 0xf0;
			old_color = (old_color > 0xff) ? 0xff - old_color : old_color;
			draw_image->lpData[raw_coord + color] = old_color;
		}
	}

	if (verbose & VERBOSE_STEP_BY_STEP)
		space_step = true;
	while (space_step && !escape_status) {
		glfwWaitEvents(); // glfwWaitEventsTimeout(1.0);
		glfwPollEvents();
	}

	if (verbose & VERBOSE_VIDEO && verbose & VERBOSE_STEP_BY_STEP) {
		render_loop (draw_image, video_texture);
	}

}



int convert_i_c (int ai, int shift, float zoom)
{
        // convert image coordinates to canvas coordinates
	return (int)((ai - shift) * zoom);
}



int convert_c_i (int ac, int shift, float zoom)
{
	// convert canvas coordinates to image coordinates
	return (int)(ac/zoom + shift);
}


float get_zoom_ratio (int canvas_width, int canvas_height, int image_width, int image_height)
{
	float test_scale_x, test_scale_y;

	test_scale_x = (float)canvas_width / (float)image_width;
	test_scale_y = (float)canvas_height / (float)image_height;

	return fmin(test_scale_x, test_scale_y);
}
