#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
	ofSetFrameRate(60);
	glEnable(GL_DEPTH_TEST);

	glPointSize(200);
	glLineWidth(3);

	viewMode = 0; //starts with topdown view 0-top down 1-perspective 2-FP
	debugHitboxes = false;

	//cam variable
	lensAngle = 60;
	alpha = 10;
	beta = 1000;
	perfectDist = gh() * .5 / (tanf(lensAngle * 0.5 * PI / 180.0)); //pode ser usado para o zoom pois determina a distância da câmera.
	camPos = ofVec3f(gw() / 2.0, -gh() *.65, gh());  // Elevated position
	lookatPos = ofVec3f(gw() / 2.0, gh() / 2.0, 0);   // Look at the center of the world
	camUp = ofVec3f(0, 0, 1);                        // Up vector

	//World  Dimensions
	roadWidth = gw() * 0.8;
	riverWidth = gw() * 0.8;
	laneHeight = gh() / 11.0; // Divide the screen into 11 lanes
	laneWidth = gw() / 11.0; // Grid cell size is based on screen width
	//Car Dimensions 
	carBaseWidth = gw() * .1;
	carBaseHeight = gh() * .05;
	carBaseDepth = 30;
	cockpitDepth = carBaseDepth * .5;
	cockpitWidth = carBaseWidth * .6;
	cockpitHeight = carBaseHeight;
	tireHeight = carBaseWidth * .1;
	tireRadius = carBaseDepth * .35;
	tireSlices = 50;

	//Log Dimensions
	logRadius = carBaseHeight*.75;
	logHeight = carBaseWidth;
	logSlices = 25;

	//Frog Dimensions
	frogBodyWidth = gw()*.05;
	frogBodyHeight = gw()*.05;
	frogBodyDepth = gh() * .025;
	frogLegWidth = frogBodyWidth*.5;
	frogLegHeight = frogBodyHeight*.75;
	frogLegDepth = frogBodyDepth*.25;
	frogEyeWidth = frogBodyWidth*.25;
	frogEyeHeight = frogBodyHeight * .25;
	frogEyeDepth = frogBodyDepth*.25;
	
	//world dimensions
	worldWidth = gw();
	worldHeight= gh();

	// Set frog starting position
	frogGridPosX = 5;
	frogGridPosY = 0;
	frogPosX = gw() *.5;              // Center horizontally
	frogPosY = laneHeight*.5;     // Bottom lane
	frogPosZ = frogBodyDepth*.5;                     // Flat on the screen

	// Colors
	roadColor = ofColor(50, 50, 50);
	riverColor = ofColor(0, 0, 255);
	grassColor = ofColor(0, 255, 0);
	
	//Game states
	frogOnLog = false;
	playerWin = false;
	playerLoss = false;

	}


//--------------------------------------------------------------
void ofApp::update() {

	// Spawn cars
	carSpawnTimer += ofGetLastFrameTime();
	if (carSpawnTimer >= carSpawnInterval) {
		carSpawnTimer = 0;

		// Add a new car only if there is enough space
		float laneY = laneHeight * int((3 + ofRandom(3))) - laneHeight * 0.5; // Random lane: 3, 4, or 5
		bool canSpawn = true;

		// Check existing cars for overlap
		for (const auto& car : carPositions) {
			if (abs(car.x + carBaseWidth) < laneWidth * 2) {
				canSpawn = false;
				break;
			}
		}

		if (canSpawn) {
			carPositions.push_back(ofVec3f(-carBaseWidth, laneY, 0)); // Spawn off-screen left

			// Limit the number of cars
			if (carPositions.size() > maxCars) {
				carPositions.erase(carPositions.begin());
			}
		}
	}

	// Spawn Logs
	logSpawnTimer += ofGetLastFrameTime();
	if (logSpawnTimer >= logSpawnInterval) {
		logSpawnTimer = 0;

		// Add a new log only if there is enough space
		int laneIndex = (ofRandom(1.0) < 0.5) ? 7 : 9;
		float laneY = laneHeight * laneIndex - laneHeight * 0.5;
		bool canSpawn = true;

		// Check existing logs for overlap
		for (const auto& log : logPositions) {
			if (abs(log.y - laneY) < laneHeight && abs(log.x - gw()) < laneWidth * 2) {
				canSpawn = false;
				break;
			}
		}

		if (canSpawn) {
			logPositions.push_back(ofVec3f(gw() + logHeight, laneY, 0)); // Spawn off-screen right

			// Limit the number of logs
			if (logPositions.size() > maxLogs) {
				logPositions.erase(logPositions.begin());
			}
		}
	}
	
	// Update car positions DONE
	if (playerLoss == false&&playerWin==false) {
		for (auto& pos : carPositions) {
			pos.x += laneWidth / 10.0; // Move by a fraction of the grid cell size

		}
	}

	//check for car collisions  DONE
	for (const auto& pos : carPositions) {
		if (checkCollision(frogPosX, frogPosY, frogBodyWidth, frogBodyHeight,
			pos.x, pos.y, carBaseWidth, carBaseHeight)) {
			// Handle collision
			playerLoss = true;
			playerLossCar = true;
		
			break;
		}
	}

	// Update log positions DONE
	if (playerLoss == false&&playerWin==false) {
		for (auto& pos : logPositions) {
			pos.x -= laneWidth / 20.0; // Move by a fraction of the grid cell size
		}
	}
	//Check if on top of log else frog dead
	if (checkIfOnRiver(frogPosX, frogPosY)) {
		playerLoss = true;
		playerLossRiver=true;
		frogOnLog = false;
	}

	if (playerLoss) {
		for (int i = 0; i < 100; i++)
		{

			Particle aux;
			aux.setup(
				frogPosX - frogBodyWidth * 0.5, frogPosX + frogBodyWidth * 0.5, // X range around the frog
				frogPosY - frogBodyHeight * 0.5, frogPosY + frogBodyHeight * 0.5); // Y range around the frog
			
			 explosion.push_back(aux);

		}
		for (int i = 0; i < explosion.size(); i++)
		{
			explosion[i].update();
			if (explosion[i].dead) {
				explosion.erase(explosion.begin() + i);
			}
		}
	}

	if (frogOnLog) { 
		frogPosX = frogPosX - laneWidth / 20.0; 
		frogGridPosX = frogPosX/laneWidth;
	}

	if (frogGridPosY == 9) frogOnLog = false; //the frog reaches the other side of river stop moving with log
	
	// Prevent frog from leaving the screen
	if (frogPosX < laneWidth) frogPosX = laneWidth*.5;
	if (frogPosX > worldWidth-laneWidth) frogPosX = worldWidth - laneWidth *.5;
	if (frogPosY < laneHeight*.5) frogPosY = laneHeight*.5;	
	if (frogPosY > worldHeight - laneHeight * .5) { 
		frogPosY =worldHeight - laneHeight * .5; 
		playerWin = true; //WIN
	}
}

//--------------------------------------------------------------
void ofApp::draw() {

	// Setup camera view
	glViewport(0, 0, gw(), gh());

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	if (viewMode == 0) {
		// Top-down view
		perspective(60, alpha, beta);
		lookat(
			gw() / 2.0, gh() / 2.0, perfectDist+200,  // Camera above the world
			gw() / 2.0, gh() / 2.0, 0.0,         // Look at the center
			0.0, 1.0, 0.0                        // Up vector
		);

		if (playerWin) {
			ofSetColor(255, 0, 0);
			ofDrawBitmapString("Congratulations! You Won!\nPRESS ""R"" TO PLAY AGAIN", gw() / 2 - 100, gh() - 50);
		}
		if (playerLossRiver) {
			ofSetColor(50, 50, 255);

			for (int i = 0; i < explosion.size(); i++) {
				explosion[i].draw();
			}

			ofSetColor(255, 0, 0);
			ofDrawBitmapString("Frog drowned in the river\nPRESS ""R"" TO TRY AGAIN ", gw() / 2 - 100, gh() - 50);
			glColor3f(173 / 255.0, 216 / 255.0, 255 / 255.0);

		}
		if (playerLossCar) {
			ofSetColor(255, 50, 50);


			for (int i = 0; i < explosion.size(); i++) {
				explosion[i].draw();
			}

			ofSetColor(255, 0, 0);
			ofDrawBitmapString("Frog died in a car accident\nPRESS ""R"" TO TRY AGAIN ", gw() / 2 - 100, gh() - 50);

		}
	}
	else if (viewMode == 1) {
		// Perspective view
		perspective(lensAngle, alpha, beta);
		lookat(
			camPos.x, camPos.y, camPos.z,        // Perspective camera position
			lookatPos.x, lookatPos.y, lookatPos.z, // Look at the world center
			camUp.x, camUp.y, camUp.z           // Up vector

		);

		if (playerWin) {
			ofSetColor(255, 0, 0);
			ofDrawBitmapString("Congratulations! You Won!\nPRESS ""R"" TO PLAY AGAIN", gw() / 2 - 100, gh() - 50);
		}
		if (playerLossRiver) {
			ofSetColor(50, 50, 255);

			for (int i = 0; i < explosion.size(); i++) {
				explosion[i].draw();
			}

			ofSetColor(255, 0, 0);
			ofDrawBitmapString("Frog drowned in the river\nPRESS ""R"" TO TRY AGAIN ", gw() / 2 - 100, gh() - 50);
			glColor3f(173 / 255.0, 216 / 255.0, 255 / 255.0);

		}
		if (playerLossCar) {
			ofSetColor(255, 50, 50);


			for (int i = 0; i < explosion.size(); i++) {
				explosion[i].draw();
			}

			ofSetColor(255, 0, 0);
			ofDrawBitmapString("Frog died in a car accident\nPRESS ""R"" TO TRY AGAIN ", gw() / 2 - 100, gh() - 50);

		}
	}
	else if (viewMode == 2) {
		// First-person view (frog POV)
		perspective(90, alpha, beta);
		lookat(
			frogPosX, frogPosY+frogEyeHeight, frogPosZ + frogBodyHeight+frogEyeDepth, // Camera at frog's "head"
			frogPosX, frogPosY +100.0f, frogPosZ+frogBodyDepth+frogEyeDepth,           // Look slightly ahead of the frog
			0.0, 1.0, 0.0                                 // Up vector
		);

		if (playerWin) {
			ofSetColor(255, 0, 0);
			ofDrawBitmapString("Congratulations! You Won!\nPRESS ""R"" TO PLAY AGAIN", frogPosX, frogPosY + 200);
		}
		if (playerLossRiver) {
			ofSetColor(50, 50, 255);

			for (int i = 0; i < explosion.size(); i++) {
				explosion[i].draw();
			}

			ofSetColor(255, 0, 0);
			ofDrawBitmapString("Frog drowned in the river\nPRESS ""R"" TO TRY AGAIN ", frogPosX, frogPosY+200);
			glColor3f(173 / 255.0, 216 / 255.0, 255 / 255.0);

		}
		if (playerLossCar) {
			ofSetColor(255, 50, 50);


			for (int i = 0; i < explosion.size(); i++) {
				explosion[i].draw();
			}

			ofSetColor(255, 0, 0);
			ofDrawBitmapString("Frog died in a car accident\nPRESS ""R"" TO TRY AGAIN ", frogPosX, frogPosY + 200);

		}
	}

	// Draw world
	draw_world();

	// Draw cars
	for (auto& pos : carPositions) {
		draw_car(pos.x, pos.y, pos.z+carBaseDepth);
		
		if (pos.x > worldWidth) pos.x = -worldWidth; // Loop cars if car pos is out of the world 

		
	}
	if (debugHitboxes) {
		for (const auto& car : carPositions) {
			glColor3f(1.0, 0.0, 1.0); //
			glPushMatrix();
			// Use the exact car position and dimensions
			glTranslatef(car.x, car.y, car.z+carBaseDepth);
			glScalef(carBaseWidth, carBaseHeight, carBaseDepth+cockpitDepth+tireRadius);
			cube_unit();
			glPopMatrix();
		}
	}
	//// Draw logs
	for (auto& pos : logPositions) {
		draw_wood_log(pos.x, pos.y, pos.z);
		if (pos.x < -worldWidth) pos.x = worldWidth; // Loop logs
	}


	// Draw frog
	draw_frog(frogPosX, frogPosY, frogPosZ);

}

void ofApp::draw_car(float carPosX, float carPosY, float carPosZ)
{
	glPushMatrix();//master car push
	glTranslatef(carPosX , carPosY, carPosZ); 
		glPushMatrix();//car base push
			glPushMatrix();//carbase
			glScalef(carBaseWidth, carBaseHeight, carBaseDepth);
			glColor3f(1, 0, 0);
			cube_unit();
			glPopMatrix();
			
			glPushMatrix();//cockpit push
			glTranslatef(0, 0, carBaseDepth*.5+cockpitDepth*.5);
				glPushMatrix();
				glScalef(cockpitWidth, cockpitHeight, cockpitDepth);
				glColor3f(1, 0, 0);
				cube_unit();
				glPopMatrix();
			glPopMatrix();//cockpit pop

		glPopMatrix();

		glColor3f(0.41, 0.41, 0.41);
		glPushMatrix();//front left tire pop 	
			glTranslatef(tireRadius-carBaseWidth*.45,-carBaseHeight*.5+ tireHeight*.4, -carBaseDepth*.5-tireRadius*.5);
			drawCylinder(tireRadius, tireHeight, tireSlices);
		glPopMatrix();//front left tire pop 

		glPushMatrix();//front right tire pop 
			glTranslatef(tireRadius-carBaseWidth * .45, carBaseHeight * .5 - tireHeight*.4 , -carBaseDepth*.5-tireRadius*.5);
			drawCylinder(tireRadius, tireHeight, tireSlices);
		glPopMatrix();//front right tire pop 

		glPushMatrix();//back left tire pop 
			glTranslatef(-tireRadius+ carBaseWidth * .45, -carBaseHeight * .5 + tireHeight * .4, -carBaseDepth*.5-tireRadius*.5);
			drawCylinder(tireRadius, tireHeight, tireSlices);
		glPopMatrix();//back left tire pop 

		glPushMatrix();//back right tire pop 
			glTranslatef(-tireRadius+ carBaseWidth * .45, carBaseHeight * .5 - tireHeight * .4,-carBaseDepth*.5 -tireRadius*.5);
			drawCylinder(tireRadius, tireHeight, tireSlices);
		glPopMatrix();//back right tire pop 
		
	glPopMatrix();//master car pop
}

void ofApp::draw_wood_log(float logPosX,float logPosY,float logPosZ)
{
	glPushMatrix();
	glColor3f(0.55, 0.27, 0.07);
	glTranslatef(logPosX , logPosY, logPosZ-logRadius*.8); 
	glRotatef(90, 0, 0, 1);
	drawCylinder(logRadius, logHeight, logSlices);
	glPopMatrix();
}

void ofApp::draw_frog(float frogPosX, float frogPosY, float frogPosZ) {
	glColor3f(0.2, 0.55, 0.2); // Frog body color

	glPushMatrix(); // Frog master push
	glTranslatef(frogPosX, frogPosY, frogPosZ); // Position the frog

	// Draw frog body
	glPushMatrix(); // Frog body push
	glScalef(frogBodyWidth, frogBodyHeight, frogBodyDepth);
	cube_unit();
	glPopMatrix(); // Frog body pop

	// Draw left eye
	glPushMatrix(); // Frog left eye push
	glColor3f(0.95, 0.92, 0.84); // White color for eye
	glTranslatef(-frogBodyWidth * 0.3, frogBodyHeight * 0.5, frogBodyDepth * 0.5);

	// Eye white
	glPushMatrix();
	glScalef(frogEyeWidth, frogEyeHeight, frogEyeDepth);
	cube_unit();
	glPopMatrix();

	// Iris
	glColor3f(0, 0, 0); // Black color for iris
	glPushMatrix();
	glTranslatef(0, 0, frogBodyDepth * .25);
	glScalef(frogEyeWidth * 0.5, frogEyeHeight * 0.5, frogEyeDepth * 0.5);
	cube_unit();
	glPopMatrix();
	glPopMatrix(); // Frog left eye pop

	// Draw right eye
	glPushMatrix(); // Frog right eye push
	glColor3f(0.95, 0.92, 0.84); // White color for eye
	glTranslatef(frogBodyWidth * 0.3, frogBodyHeight * 0.5, frogBodyDepth * 0.5); // Ensure it's relative to frog body

	// Eye white
	glPushMatrix();
	glScalef(frogEyeWidth, frogEyeHeight, frogEyeDepth);
	cube_unit();
	glPopMatrix();

	// Iris
	glColor3f(0, 0, 0); // Black color for iris
	glPushMatrix();
	glTranslatef(0, 0, frogBodyDepth*.25);
	glScalef(frogEyeWidth * 0.5, frogEyeHeight * 0.5, frogEyeDepth * 0.5);
	cube_unit();
	glPopMatrix();
	glPopMatrix(); // Frog right eye pop

	glPopMatrix(); // Frog master pop
}

void ofApp::draw_world() {
	// Grass Background
	glColor3f(grassColor.r / 255.0, grassColor.g / 255.0, grassColor.b / 255.0); // Grass color
	glPushMatrix();
	glTranslatef(worldWidth*.5 , worldHeight*.5, 0); // Center the world
	glScalef(worldWidth, worldHeight, 1);            // Scale to fit the screen
	malha_unit(11, 11);                 // Grid overlay
	glPopMatrix();

	//// Road Section (covering lanes 3, 4, 5)
	glColor3f(roadColor.r / 255.0, roadColor.g / 255.0, roadColor.b / 255.0); // Road color
	glPushMatrix();
	glTranslatef(gw() / 2, laneHeight * 4 - laneHeight * .5, 0.01); // Center river section
	glScalef(gw(), laneHeight * 3., 1);          // Same width as grass, height = 3 lanes
	malha_unit(11, 11);                         // Grid overlay
	glPopMatrix();

	//// River Section (covering lanes 7, 8, 9)
	glColor3f(riverColor.r / 255.0, riverColor.g / 255.0, riverColor.b / 255.0); // River color
	glPushMatrix();
	glTranslatef(gw() / 2, laneHeight * 8-laneHeight*.5, 0.01); // Center river section
	glScalef(gw(), laneHeight*3. , 1);          // Same width as grass, height = 3 lanes
	malha_unit(11, 11);                         // Grid overlay
	glPopMatrix();

	for (int i = 0; i < 6; i++) { // Adjust the number of lily pads
		float x = laneWidth * .5 + 4 * i * laneWidth * .5;  // Spaced across the lane
		float y = laneHeight * 8 - laneHeight * 0.5;
		draw_lily_pad(x, y, 0.02); // Slightly above the river
	}
}

void ofApp::draw_lily_pad(float x, float y, float z) {
	glPushMatrix();
	glTranslatef(x, y, z);
	glColor3f(0.0, 0.5, 0.0); // Green for lily pad
	glScalef(laneWidth * .8, laneHeight * .8, 0);
	malha_unit(5, 5); // Use a cylinder for the lily pad
	glPopMatrix();
}

bool ofApp::checkCollision(float x1, float y1, float w1, float h1, float x2, float y2, float w2, float h2) {
	float halfW1 = w1 * 0.5, halfH1 = h1 * 0.5;
	float halfW2 = w2 * 0.5, halfH2 = h2 * 0.5;

	// Check overlap in X and Y axes
	return (abs(x1 - x2) < (halfW1 + halfW2)) && (abs(y1 - y2) < (halfH1 + halfH2));
}

bool ofApp::checkIfOnRiver(float frogX, float frogY) {
	// Determine if the frog is in a river lane
	int laneIndex = frogY / laneHeight;
	if (laneIndex >= 6 && laneIndex <= 8) {
		// Check if the frog is on a log
		for (const auto& log : logPositions) {
			if (checkCollision(frogX, frogY, frogBodyWidth, frogBodyHeight,
				log.x, log.y, logHeight, logRadius * 2)) {
				frogOnLog = true;
				return false; // Frog is on a log
			}
		}
		// Check if the frog is on a lily pad
		for (int i = 0; i < 6; i++) { // Assuming 6 lily pads
			float lilyPadX = laneWidth * 0.5 + 4 * i * laneWidth * 0.5;
			float lilyPadY = laneHeight * 8 - laneHeight * 0.5; // Assuming lane 8
			if (checkCollision(frogX, frogY, frogBodyWidth, frogBodyHeight,
				lilyPadX, lilyPadY, laneWidth * 0.8, laneHeight * 0.8)) {
				frogOnLog = false;
				return false; // Frog is on a lily pad, safe
			}
		}
		return true; // Frog is in the river and not on a log
	}
	return false; // Frog is not in the river
}

void ofApp::reset_game() {
	playerWin = false;
	playerLoss = false;
	playerLossCar = false;
	playerLossRiver = false;
	frogOnLog = false;
	// Clear all cars and logs
	explosion.clear(); // Clear all particles
	carPositions.clear();
	logPositions.clear();
	frogPosX = gw() * 0.5;
	frogPosY = laneHeight * 0.5;
	frogGridPosX = 5;
	frogGridPosY = 0;
	ofLog() << "Game reset!";
}
//--------------------------------------------------------------
void ofApp::keyPressed(int key){
	switch (key) {
	case 'g':
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		break;
	case 'f':
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

		break;

	case 'v': // Switch view
		viewMode += 1; // Cycle between 0 (top-down), 1 (perspective), and 2 (first-person)
		if (viewMode > 2) viewMode = 0;
		break;
	
	case OF_KEY_UP: 
		if (frogGridPosY < 11 && playerWin==false && playerLoss==false)
		{
			frogGridPosY += 1;
		}

		frogPosY = laneHeight*.5+ frogGridPosY * laneHeight;
		break;

	case OF_KEY_DOWN: 
		if (frogGridPosY>0 && playerWin == false && playerLoss == false)
		{
			frogGridPosY -= 1;
		}
		
		frogPosY = laneHeight*.5 + frogGridPosY* laneHeight;
		break;
	case OF_KEY_LEFT: 
		if (frogGridPosX > 0 && playerWin == false && playerLoss == false)
		{
			frogGridPosX -= 1;
		}

		frogPosX = laneWidth*.5+ frogGridPosX * laneWidth;
		break;
	case OF_KEY_RIGHT: 
		if (frogGridPosX < 11 && playerWin == false && playerLoss == false)
		{
			frogGridPosX += 1;
		}
		frogPosX = laneWidth*.5+ frogGridPosX * laneWidth;
		break;
	case 'b':
		debugHitboxes = !debugHitboxes;
		break;
	case 'r':
		reset_game();
		break;
	}
	
	}
	

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){
	
}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y){

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h) {
	// Update world dimensions
	worldWidth = gw();
	worldHeight = gh();

	// Update lane dimensions
	laneHeight = gh() / 11.0; // Divide the screen into 11 lanes
	laneWidth = gw() / 11.0;  // Update lane width if needed

	camPos = ofVec3f(gw() / 2.0, -gh() * .65, gh());  // Elevated position
	lookatPos = ofVec3f(gw() / 2.0, gh() / 2.0, 0);   // Look at the center of the world

	// Adjust existing positions for cars and logs based on new dimensions
	for (auto& car : carPositions) {
		car.y = laneHeight * int((3 + ofRandom(3))) - laneHeight * .5; // Keep cars in correct lanes
	}
	for (auto& log : logPositions) {
		// Snap logs to the nearest valid lane (7 or 9, excluding 8)
		int laneIndex = (ofRandom(1.0) < 0.5) ? 7 : 9;
		float laneY = laneHeight * laneIndex - laneHeight * 0.5;
		log.y = laneHeight * laneIndex - laneHeight * 0.5;
	}
	frogPosX = laneWidth * .5 + frogGridPosX * laneWidth;
	frogPosY = laneHeight * .5 + frogGridPosY * laneHeight;

	// Note: Frog position is preserved to avoid resetting on window resize.
}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}
