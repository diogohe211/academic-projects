#pragma once
#include "ofMain.h"
#include "cg_extras.h"
#include "cg_drawing_extras.h"
#include "cg_cam_extras.h"

class Particle{
    public:

    void setup(float xmin, float xmax, float ymin, float ymax);
    void update();
    void draw();

    ofVec3f pos;
    ofVec3f speed;
    int lifeTime;
    int age;

    float size;

    bool dead;

};