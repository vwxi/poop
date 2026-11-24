/*
 * poop - a demo for 3dt
 *
 * this project uses MODPlay by prochazkaml. get it here:
 * https://codeberg.org/prochazkaml/MODPlay
 *
 * the demo is coordinated via a tracker module that contains special commands to
 * trigger events at different times
 *
 * return convention: 0 is OK, anything under 0 is an error
 */

#include <SDL_timer.h>
#define DEBUG

#include "glad/glad.h"

#include <GL/gl.h>
#include <GL/glext.h>
#include <GL/glu.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include <SDL_events.h>
#include <SDL_video.h>
#include <stdio.h>

#include "modplay.h"
#include "scene.h"

#define WIDTH 640
#define HEIGHT 480

#ifdef DEBUG
#   define GLE(x) do { \
        x; \
        GLenum e; \
        while((e = glGetError()) != GL_NO_ERROR) { \
            fprintf(stderr, "[%s:%d]\n\tcall: %s\n\tglGetError: %d\n", __FILE_NAME__, __LINE__, #x, e); \
        } \
    } while(0);
#else
#   define GLE(x) x
#endif

#define NUM_SCENES 1

const static GLfloat RESOLUTION[2] = { WIDTH, HEIGHT };

static GLfloat TRIANGLE[] = {
    -0.5f, -0.5f, 0.0f,
    0.5f, -0.5f, 0.0f,
    0.0f,  0.5f, 0.0f
};

static struct p_scene SCENES[NUM_SCENES] = {
    /*
     * plasma scene - mapping the plasma feel onto an object bouncing around and maybe unfurling the model
     * not sure yet
     */
    {
        .scene_name = "plasma",
        .vertex_shader_source = {
            "#version 330 core\n \
            in vec3 position;\n\
            void main() {\n \
                gl_Position = vec4(position,1.0);\n \
            } \
            "
        },
        .fragment_shader_source = {
            "#version 330 core\n \
            #define PI 3.14159265\n\
            uniform float time;\n\
            uniform vec2 res;\n\
            out vec4 FragColor;\n \
            void main() {\n \
                vec2 coord = gl_FragCoord.xy / res.xy;\n\
                float v = sin(coord.x * 10.0 + time);\n\
                v += -sin(3.0 * (coord.x * sin(time) + coord.y * cos(time)) + time);\n\
                float cx = coord.x + 0.5 * sin(time / 5.0);\n\
                float cy = coord.y + 0.5 * cos(time / 3.0);\n\
                v += sin(sqrt(10.0 * (cx*cx + cy*cy) + 1.0) + time);\n\
                vec3 color = vec3(\n\
                    sin(v*PI),\n\
                    sin(v*PI+2.0*PI/3.0),\n\
                    sin(v*PI+4.0*PI/3.0)\n\
                );\n\
                FragColor = vec4(color*0.5 + 0.5, 1.0);\n \
            } \
            "
        },
        .vertices = &TRIANGLE[0],
        .sz_vertices = sizeof(TRIANGLE),
    }
};

SDL_Window* window;
SDL_GLContext gl_context;

int p_init();
int p_setup();
void p_die();
void p_log_shader(GLuint);
void p_run_scene(size_t);
void p_destroy_scene(size_t);
void p_render(size_t);

void p_log_shader(GLuint shader) {
    char log[1024];
    glGetShaderInfoLog(shader, sizeof(log), NULL, (GLchar*)log);

    fprintf(stderr, "log_shader: %s\n", log);
}

int p_init() {
    int x, y;
    int err = 0;

    if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
        fprintf(stderr, "failed to init SDL: %s\n", SDL_GetError());
        return -1;
    }

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 6);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

	window = SDL_CreateWindow(
	    "poop",
		SDL_WINDOWPOS_CENTERED,
		SDL_WINDOWPOS_CENTERED,
		WIDTH,
		HEIGHT,
		SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN
	);

	if(!window) {
	    fprintf(stderr, "failed to init SDL window: %s\n", SDL_GetError());
		return -1;
	}

	gl_context = SDL_GL_CreateContext(window);
	if(!gl_context) {
	    fprintf(stderr, "failed to init GL for SDL window: %s\n", SDL_GetError());
		return -1;
	}

	gladLoadGLLoader(SDL_GL_GetProcAddress);

	SDL_GL_SetSwapInterval(1);

	SDL_GetWindowPosition(window, &x, &y);
	glViewport(0, 0, WIDTH, HEIGHT);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	if((err = p_setup()) < 0) {
	    return err;
	}

    return 0;
}

void p_die() {
    struct p_scene* s;
    GLint state;

    for(size_t i = 0; i < NUM_SCENES; i++) {
        p_destroy_scene(i);
    }

    SDL_GL_DeleteContext(gl_context);
    SDL_DestroyWindow(window);

    window = NULL;

    SDL_Quit();
}

int p_setup() {
    int err = 0;
    struct p_scene* s;
    GLint state;

    for(int i = 0; i < NUM_SCENES; i++) {
        s = &SCENES[i];
        state = GL_FALSE;

        s->program_id = glCreateProgram();
        s->vertex_shader = glCreateShader(GL_VERTEX_SHADER);
        s->fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);

        // load the vertex shader

        GLE(glShaderSource(s->vertex_shader, 1, &s->vertex_shader_source[0], NULL));
        GLE(glCompileShader(s->vertex_shader));

        #ifdef DEBUG
        glGetShaderiv(s->vertex_shader, GL_COMPILE_STATUS, &state);
        if(state == GL_FALSE) {
            fprintf(stderr, "scene %s failed to compile vertex shader\n", s->scene_name);
            p_log_shader(s->vertex_shader);
            return -1;
        }
        #endif

        GLE(glAttachShader(s->program_id, s->vertex_shader));

        // load the fragment shader

        GLE(glShaderSource(s->fragment_shader, 1, &s->fragment_shader_source[0], NULL));
        GLE(glCompileShader(s->fragment_shader));

        #ifdef DEBUG
        GLE(glGetShaderiv(s->fragment_shader, GL_COMPILE_STATUS, &state));
        if(state == GL_FALSE) {
            fprintf(stderr, "scene %s failed to compile fragment shader\n", s->scene_name);
            p_log_shader(s->fragment_shader);
            return -1;
        }
        #endif

        GLE(glAttachShader(s->program_id, s->fragment_shader));

        GLE(glBindFragDataLocation(s->program_id, 0, "FragColor"));

        GLE(glLinkProgram(s->program_id));

        GLE(glDeleteShader(s->vertex_shader));
        GLE(glDeleteShader(s->fragment_shader));

        #ifdef DEBUG
        GLE(glGetProgramiv(s->program_id, GL_LINK_STATUS, &state));
        if(state == GL_FALSE) {
            fprintf(stderr, "scene %s failed to link GL program\n", s->scene_name);
            return -1;
        }
        #endif

        // setup VAO, VBO

        GLE(glGenVertexArrays(1, &s->vao));
        GLE(glGenBuffers(1, &s->vbo));

        GLE(glBindVertexArray(s->vao));

        GLE(glBindBuffer(GL_ARRAY_BUFFER, s->vbo));
        GLE(glBufferData(GL_ARRAY_BUFFER, s->sz_vertices, s->vertices, GL_STATIC_DRAW));

        // setup attributes

        GLint pos_attrib = glGetAttribLocation(s->program_id, "position");
        GLE(glVertexAttribPointer(pos_attrib, 3, GL_FLOAT, GL_FALSE, 0, 0));
        GLE(glEnableVertexAttribArray(pos_attrib));

        // reset for good measure

        GLE(glBindBuffer(GL_ARRAY_BUFFER, 0));
        GLE(glBindVertexArray(0));

        fprintf(stdout, "scene %s initialized\n", s->scene_name);
    }

    return 0;
}

void p_destroy_scene(size_t scene) {
    fprintf(stdout, "destroying scene %s\n", SCENES[scene].scene_name);
    glDeleteBuffers(1, &SCENES[scene].vbo);
    glDeleteVertexArrays(1, &SCENES[scene].vao);
}

void p_render(size_t scene) {
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    GLE(glUseProgram(SCENES[scene].program_id));
    GLE(glBindVertexArray(SCENES[scene].vao));

    float time = (float)SDL_GetTicks64() * 0.0005;
    GLE(glUniform1f(glGetUniformLocation(SCENES[scene].program_id, "time"), time));
    GLE(glUniform2fv(glGetUniformLocation(SCENES[scene].program_id, "res"), 1, &RESOLUTION[0]));

    GLE(glDrawArrays(GL_TRIANGLES, 0, 3));
    GLE(glBindVertexArray(0));
}

int main() {
    int quit = 0;
    SDL_Event e;

    if(p_init() < 0) {
        return -1;
    }

    while(!quit) {
        while(SDL_PollEvent(&e) != 0) {
            if(e.type == SDL_QUIT) {
                quit = 1;
            }
        }

        p_render(0);

        SDL_GL_SwapWindow(window);
    }

    p_die();

    return 0;
}
