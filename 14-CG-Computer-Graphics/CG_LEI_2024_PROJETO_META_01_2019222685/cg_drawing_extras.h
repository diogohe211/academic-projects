#pragma once
#define _USE_MATH_DEFINES
#include "ofMain.h"
#include "cg_extras.h"
#include <math.h>

//desenha ponto 3D na origem
inline void drawPoint() {
	glBegin(GL_POINTS);
	glVertex3f(0, 0, 0);
	glEnd();
}

//desenha axis 3D
inline void axis3d() {
	glBegin(GL_LINES);
	glColor3f(1, 0, 0);
	glVertex3f(0, 0, 0);
	glVertex3f(1, 0, 0);
	glColor3f(0, 1, 0);
	glVertex3f(0, 0, 0);
	glVertex3f(0, 1, 0);
	glColor3f(1, 1, 1);
	glVertex3f(0, 0, 0);
	glVertex3f(0, 0, 1);
	glEnd();
}

//função que desenha quadrado unitário 
//centrado na origem e preenchido
inline void rectFill_unit() {
	glBegin(GL_QUADS);
	glVertex3d(-0.5, -0.5, 0.);
	glVertex3d(-0.5, 0.5, 0.);
	glVertex3d(0.5, 0.5, 0.);
	glVertex3d(0.5, -0.5, 0.);
	glEnd();
}

//função que desenha malha unitária com resolução mxn
inline void malha_unit(GLint m, GLint n) {
	GLfloat x_start = -0.5;
	GLfloat y_start = -0.5;
	GLfloat x_step = 1.0 / GLfloat(m);
	GLfloat y_step = 1.0 / GLfloat(n);

	glBegin(GL_QUADS);
	for (int i = 0; i < m; i++) {
		for (int j = 0; j < n; j++) {
			glVertex2d(i * x_step + x_start, j * y_step + y_start);
			glVertex2d(i * x_step + x_start, (j + 1) * y_step + y_start);
			glVertex2d((i + 1) * x_step + x_start, (j + 1) * y_step + y_start);
			glVertex2d((i + 1) * x_step + x_start, j * y_step + y_start);
		}
	}
	glEnd();
}


inline void cube_unit() {
	GLfloat p = 0.5;
	glBegin(GL_QUADS);

	//frente
	//glColor3f(1, 0, 0);
	glVertex3f(-p, -p, p);
	glVertex3f(-p, p, p);
	glVertex3f(p, p, p);
	glVertex3f(p, -p, p);

	//tras
	//glColor3f(0, 1, 0);
	glVertex3f(-p, -p, -p);
	glVertex3f(p, -p, -p);
	glVertex3f(p, p, -p);
	glVertex3f(-p, p, -p);

	//glColor3f(1, 1, 1);
	//cima
	glVertex3f(-p, -p, -p);
	glVertex3f(-p, -p, p);
	glVertex3f(p, -p, p);
	glVertex3f(p, -p, -p);

	//baixo
	glVertex3f(-p, p, p);
	glVertex3f(-p, p, -p);
	glVertex3f(p, p, -p);
	glVertex3f(p, p, p);

	//esq
	glVertex3f(-p, -p, p);
	glVertex3f(-p, -p, -p);
	glVertex3f(-p, p, -p);
	glVertex3f(-p, p, p);

	//dir
	glVertex3f(p, -p, p);
	glVertex3f(p, p, p);
	glVertex3f(p, p, -p);
	glVertex3f(p, -p, -p);

	glEnd();

}

//funcao que desenha cubo unit�rio com malha_unit()
inline void cube_malha_unit(GLint m, GLint n) {


	//tras
	glColor3f(0, 1, 0);
	glPushMatrix();
	glTranslated(0., 0., -0.5);
	glRotated(180, 0, 1, 0);
	malha_unit(m, n);
	glPopMatrix();

	//cima
	glColor3f(0, 1, 1);
	glPushMatrix();
	glTranslated(0., -0.5, 0.);
	glRotated(90, 1, 0, 0);
	malha_unit(m, n);
	glPopMatrix();

	//baixo
	glColor3f(1, 0, 0);
	glPushMatrix();
	glTranslated(0., 0.5, 0.);
	glRotated(-90, 1, 0, 0);
	malha_unit(m, n);
	glPopMatrix();

	//esquerda
	glColor3f(1, 0, 1);
	glPushMatrix();
	glTranslated(-0.5, 0., 0.);
	glRotated(-90, 0, 1, 0);
	malha_unit(m, n);
	glPopMatrix();

	//direita
	glColor3f(1, 1, 0);
	glPushMatrix();
	glTranslated(0.5, 0., 0.);
	glRotated(90, 0, 1, 0);
	malha_unit(m, n);
	glPopMatrix();

	//frente
	glColor3f(0, 0, 1);
	glPushMatrix();
	glTranslated(0., 0., 0.5);
	malha_unit(m, n);
	glPopMatrix();


}


inline void drawCylinder(float radius, float height, int slices) {
	float angleStep = 2.0f * M_PI / slices;

	// Desenhar as tampas do cilindro
	glBegin(GL_TRIANGLE_FAN);
	glNormal3f(0.0f, 1.0f, 0.0f);
	glVertex3f(0.0f, height / 2.0f, 0.0f); // centro do topo
	for (int i = 0; i <= slices; ++i) {
		float angle = i * angleStep;
		glVertex3f(radius * cos(angle), height / 2.0f, radius * sin(angle));
	}
	glEnd();

	glBegin(GL_TRIANGLE_FAN);
	glNormal3f(0.0f, -1.0f, 0.0f);
	glVertex3f(0.0f, -height / 2.0f, 0.0f); // centro da base
	for (int i = 0; i <= slices; ++i) {
		float angle = i * angleStep;
		glVertex3f(radius * cos(angle), -height / 2.0f, radius * sin(angle));
	}
	glEnd();

	// Desenhar a lateral do cilindro
	glBegin(GL_QUAD_STRIP);
	for (int i = 0; i <= slices; ++i) {
		float angle = i * angleStep;
		float x = cos(angle);
		float z = sin(angle);

		glNormal3f(x, 0.0f, z);
		glVertex3f(radius * x, height / 2.0f, radius * z);  // topo
		glVertex3f(radius * x, -height / 2.0f, radius * z); // base
	}
	glEnd();
}