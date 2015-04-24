
//CAUTION: The content of this file is automatically generated by Emacs orgmode
//from the file caravaggio.org that should either be in this, or the parent
//directory. Consequently, any modifications made to this file will likely be
//ephemeral. Please edit caravaggio.org instead.
//----------
// sphere.cpp
//-----------
//================================================================================
// This file is part of project caravaggio, a simple demonstration of the OpenGL
// API, that generates several polyhedra, each decorated with a texture map
// that is derived from an input digital image.
//
// caravaggio is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// caravaggio is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with union-find.org.  If not, see <http://www.gnu.org/licenses/>.
//
// Copyright (C) 2014, 2015 Robert R. Snapp.
//================================================================================
// This file was automatically generated using an org-babel tangle operation with
// the file caravaggio.org. Thus, the latter file should be edited instead of
// this file.
//================================================================================
//
//-----------
#include "sphere.h"
#include <cmath>

GLuint Sphere::_base_offset[3] = {0,0,0};
GLuint Sphere::_element_buffer_object = 0;
GLuint Sphere::_vertex_array_object = 0;
GLuint Sphere::_vertex_buffer = 0;
GLint Sphere::_SL_AmbientRho = 0;
GLint Sphere::_SL_DiffuseRho = 0;
GLint Sphere::_SL_SpecularRho = 0;
GLint Sphere::_SL_Shininess = 0;
GLenum Sphere::_mode[3] = {GL_TRIANGLE_FAN, GL_TRIANGLE_STRIP, GL_TRIANGLE_FAN};
GLsizei Sphere::_count[3] = {_nWedges + 2, (_nSlices - 1)*(2*(_nWedges + 1) + 1) - 1, _nWedges + 2};
GLsizei Sphere::_nElements{_nSlices*(2*_nWedges + 3)};


void Sphere::initialize_class() {
    _base_offset[0] = 0;
    _base_offset[1] = _count[0];
    _base_offset[2] = _base_offset[1] + _count[1];
    GLushort element[_nElements];

    const GLuint nVertices = _nSlices*_nWedges + 2;
    double dTheta = M_PI/(_nSlices + 1);
    double dPhi   = 2*M_PI/_nWedges;

    GLfloat position[nVertices][3];

    position[0][0] = 0.0f;   // north pole
    position[0][1] = 0.0f;
    position[0][2] = 1.0f;

    int index = 1;
    for (int slice = 0; slice < _nSlices; slice++) {
        double theta = (slice + 1)*dTheta;
        for (int wedge = 0; wedge < _nWedges; wedge++) {
            double phi = wedge*dPhi;
            position[index][0]   = std::cos(phi) * std::sin(theta);
            position[index][1]   = std::sin(phi) * std::sin(theta);
            position[index++][2] = std::cos(theta);
        }
    }
    // index shoud now equal nSphereVertices - 1
    position[index][0] =  0.0f;   // south pole
    position[index][1] =  0.0f;
    position[index][2] = -1.0f;

    int element_index = 0;

    // Insert the element indices for the triangle fan at the north pole.
    for (int i = 0; i < _nWedges + 1; i++) {
        element[element_index++] = i;
    }
    element[element_index++] = 1;

    // Insert the element indicies for the nSlices - 1 triangle strips that
    // constitute the waist of the sphere.
    int lower_offset;
    int upper_offset= 1;
    for (int slice = 0; slice < _nSlices - 1; slice++) {
        lower_offset = upper_offset;
        upper_offset += _nWedges;
        for(int wedge = 0; wedge < _nWedges; wedge++) {
            element[element_index++] = wedge + lower_offset;
            element[element_index++] = wedge + upper_offset;
        }
        element[element_index++] = lower_offset;
        element[element_index++] = upper_offset;
        if (slice < _nSlices - 2) {
            element[element_index++] = _primitive_restart_index;
        }
    }

    // Insert the element indices for the triangle fan at the south pole.
    element[element_index++] = nVertices - 1;
    for(int i = 0; i < _nWedges; i++) {
        element[element_index++] = i + upper_offset;
    }
    element[element_index] = upper_offset;


    // Allocate GPU address space.
    glGenBuffers(1, &_element_buffer_object);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _element_buffer_object);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                 sizeof(element), element, GL_STATIC_DRAW);

    glGenVertexArrays(1, &_vertex_array_object);
    glBindVertexArray(_vertex_array_object);

    glGenBuffers(1, &_vertex_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, _vertex_buffer);

    // Here we will pack the vertex positions, and the normal vectors of each vertex into
    // the same vertex array buffer, with the position in location 0, and the normal in
    // position 1. Note that for a unit sphere, the position and normal vectors are equal.

    glBufferData(GL_ARRAY_BUFFER, 2*sizeof(position), NULL, GL_STATIC_DRAW);
    // For the position of each vetex.
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(position), position);

    // For the normal vector to each vertex
    glBufferSubData(GL_ARRAY_BUFFER, sizeof(position), sizeof(position), position);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (const GLvoid *) sizeof(position));

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);

}

void Sphere::draw_elements() {
   glUniform3fv(_SL_AmbientRho, 1, &_ambient_rho[0]);
   glUniform3fv(_SL_DiffuseRho, 1, &_diffuse_rho[0]);
   glUniform3fv(_SL_SpecularRho, 1, &_specular_rho[0]);
   glUniform1f(_SL_Shininess, _shininess);

   glEnable(GL_PRIMITIVE_RESTART);
   glPrimitiveRestartIndex(_primitive_restart_index);
   glBindVertexArray(_vertex_array_object);
   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _element_buffer_object);
   for (int i = 0; i < 3; i++) {
      glDrawElements(Sphere::_mode[i], Sphere::_count[i], GL_UNSIGNED_SHORT,
                     (const GLvoid *) (Sphere::_base_offset[i]*sizeof(GLushort)));
   }
}

glm::mat4 Sphere::model_transformation() {
   return _model_transformation;
}

glm::mat3 Sphere::vector_transformation() {
   return _vector_transformation;
}

glm::vec3 Sphere::ambient_rho() {
   return _ambient_rho;
}

void Sphere::set_ambient_rho(glm::vec3 rho) {
   _ambient_rho = rho;
}

void Sphere::set_ambient_rho_SL(GLint sl) {
   _SL_AmbientRho = sl;
}

glm::vec3 Sphere::diffuse_rho() {
   return _diffuse_rho;
}

void Sphere::set_diffuse_rho(glm::vec3 rho) {
   _diffuse_rho = rho;
}

void Sphere::set_diffuse_rho_SL(GLint sl) {
   _SL_DiffuseRho = sl;
}

glm::vec3 Sphere::specular_rho() {
   return _specular_rho;
}

void Sphere::set_specular_rho(glm::vec3 rho) {
   _specular_rho = rho;
}

void Sphere::set_specular_rho_SL(GLint sl) {
   _SL_SpecularRho = sl;
}

GLfloat Sphere::shininess() {
   return _shininess;
}

void Sphere::set_shininess(GLfloat s) {
   _shininess = s;
}

void Sphere::set_shininess_SL(GLint sl) {
   _SL_Shininess = sl;
}
