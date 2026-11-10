#pragma once

#include "ofMain.h"
#include "cg_extras.h"
#include "cg_drawing_extras.h"
#include "cg_cam_extras.h"
#include "cg_game_particle.h"

class ofApp : public ofBaseApp{

	public:
		void setup();
		void update();
		void draw();
		
		void keyPressed(int key);
		void keyReleased(int key);
		void mouseMoved(int x, int y );
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		void mouseEntered(int x, int y);
		void mouseExited(int x, int y);
		void windowResized(int w, int h);
		void dragEvent(ofDragInfo dragInfo);
		void gotMessage(ofMessage msg);
	

		//Camera variables
		int viewMode;

		GLfloat lensAngle;
		GLfloat alpha, beta;
		GLfloat perfectDist;

		ofVec3f camPos;
		ofVec3f lookatPos;
		ofVec3f camUp;
		
		
		//models
		float carPosX, carPosY, carPosZ;
		float carBaseWidth, carBaseHeight, carBaseDepth, cockpitDepth, cockpitWidth, cockpitHeight, tireRadius, tireHeight;
		int tireSlices;
		float logPosX, logPosY, logPosZ;
		float logHeight, logRadius;
		int logSlices;
		int frogGridPosX, frogGridPosY;
		float frogPosX, frogPosY, frogPosZ;
		float frogBodyWidth, frogBodyHeight, frogBodyDepth, frogFootWidth, frogFootHeight, frogFootDepth, frogEyeWidth, frogEyeHeight, frogEyeDepth, frogLegDepth, frogLegWidth, frogLegHeight;
		float worldHeight, worldWidth;
		// Colors
		ofColor roadColor, riverColor, grassColor;

		float roadWidth, riverWidth, laneHeight;
		vector<ofVec3f> logPositions;
		vector<ofVec3f> carPositions;
	
		// Spawn timers
		float carSpawnTimer = 0;
		float logSpawnTimer = 0;

		// Spawn intervals (in seconds)
		float carSpawnInterval = 0.5f;
		float logSpawnInterval = 0.5f;

		// Maximum number of cars and logs
		int maxCars = 20;
		int maxLogs = 10;

		float laneWidth; // Size of each grid cell

		vector<Particle> explosion;
		bool playerWin;
		bool playerLoss;
		bool playerLossRiver;
		bool playerLossCar;
		bool frogOnLog;
		bool debugHitboxes;

		bool checkIfOnRiver(float frogX, float frogY);
		bool checkCollision(float x1, float y1, float w1, float h1, float x2, float y2, float w2, float h2);
		void draw_car(float carPosX, float carPosY, float carPosZ);
		void draw_wood_log(float logPosX, float logPosY, float logPosZ);
		void draw_frog(float frogPosX, float frogPosY, float frogPosZ);
		void draw_world();
		void draw_lily_pad(float x, float y, float z);
		void reset_game();
};
