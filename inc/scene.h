#pragma once

#include <GL/glu.h>
#include <stddef.h>

// metadata describing a scene, mainly GL data and callbacks
struct p_scene {
    const char* scene_name;

    GLuint program_id;
    GLuint vertex_shader;
    GLuint fragment_shader;

    GLuint vao;
    GLuint vbo;

    GLfloat* vertices;
    size_t sz_vertices;

    const GLchar* vertex_shader_source[1];
    const GLchar* fragment_shader_source[1];
};
