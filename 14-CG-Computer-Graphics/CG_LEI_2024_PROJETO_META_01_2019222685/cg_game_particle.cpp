#include "cg_game_particle.h"

void Particle::setup(float xmin, float xmax, float ymin, float ymax){
    float x = ofRandom(xmin, xmax);
    float y = ofRandom(ymin, ymax);

    pos = ofVec3f(x, y, 0);
    
    age = 0.;
    lifeTime = int(ofRandom(5, 30));

    speed = ofVec3f(ofRandom(-2, 2.), ofRandom(-2, 2.), ofRandom(-2, 2.));

    size = gw()*0.01;
    dead = false;
}
void Particle::update(){
    pos += speed;
    age++;
    if(age > lifeTime){
        dead = true;
    }
}
void Particle::draw(){
    if(!dead){      
        glPushMatrix();
        glTranslatef(pos.x, pos.y, pos.z);
        glScalef(size, size, size);
        cube_unit();
        glPopMatrix();
    }
}