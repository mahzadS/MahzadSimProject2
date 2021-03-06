/*

Second Phase Description: The Warbird's camera, movement, and warp capabilities, gravity, missle sites,
and intelligens-semita missles are added to the simulation in this phase.

Third Phase Description: The star field and lights have been implemented, along with the vertex and fragment shaders for them.
Star Field: Implemented as 6 different planes that create a cube around the gameplay area.
The image we used was placed into IrfanView and converted to a 640x480 .raw file

465 utility include files: texture.hpp, glmUtils465.hpp, shader465.hpp, triModel465.hpp
Header files: Object3D.hpp

User commands:
'v' cycles to the next camera
'x' cycles to the previous camera
'w' warps the warbird to Unum, Duo, or the warbird's start state
't' changes the Time Quantum to either ACE, PILOT, TRAINEE, or DEBUG
'f' fires a missile from the warbird
'r' restarts the game
'g' toggle gravity on or off
't' cycle TQ value
's' cycle ship speed
'p' toggles point light
'h' toggles head lamp light
'up' ship moves forward
'down' ship moves backward
'left' ship "yaws" left
'right' ship "yaws" right
'ctrl up'ship "pitches" up
'ctrl down' ship "pitches" down
'ctrl left' ship "rolls" left
'ctrl right' ship "rolls" right
'r' restarts the game
*/

# define __Windows__
using namespace std;
# include <string>
# include <iostream>
# include <stdlib.h> //rand and srand
# include <stdio.h> 
# include <time.h>
# include <windows.h>
# include "../includes465/include465.hpp"
# include "Object3D.hpp"
# include "Warbird.hpp"
# include "Missile.hpp"


// Model and camera indexes:
const int
RUBERINDEX = 0,
UNUMINDEX = 1,
DUOINDEX = 2,
PRIMUSINDEX = 3,
SECUNDUSINDEX = 4,
SHIPINDEX = 5,
UNUMMISSLESILOINDEX = 6,
DUOMISSLESILOINDEX = 7,
SHIPMISSILEINDEX = 8,
UNUMMISSILEINDEX = 9,
DUOMISSILEINDEX = 10,
FRONTCAMERAINDEX = 0, TOPCAMERAINDEX = 1, SHIPCAMERAINDEX = 2, UNUMCAMERAINDEX = 3, DUOCAMERAINDEX = 4;

// Model Information
const int nModels = 11;  // number of models in this scene
GLuint vPosition[nModels], vColor[nModels], vNormal[nModels];   // vPosition, vColor, vNormal handles for models
char * modelFile[nModels] = {
	"ruber.tri",
	"unum.tri",
	"duo.tri",
	"primus.tri",
	"secundus.tri",
	"ship.tri",
	"site.tri",
	"site.tri",
	"Missile.tri",
	"Missile.tri",
	"Missile.tri"
};//modelFile
int nVertices[nModels] = {
	760 * 3,
	760 * 3,
	760 * 3,
	760 * 3,
	2072 * 3,
	936 * 3,
	760 * 3,
	760 * 3,
	1108 * 3,
	1108 * 3,
	1108 * 3 
};
float modelBR[nModels]; // model's bounding radius
GLuint VAO[nModels];    // Vertex Array Objects
						  //shader
GLuint shaderProgram;
char * vertexShaderFile = "simpleVertex.glsl";
char * fragmentShaderFile = "simpleFragment.glsl";
GLuint MVP;  // ModelViewProjection matrix handle
			 // model, view, projection matrices and values to create modelMatrix.
glm::mat4 modelMatrix[nModels]; // set in display()
glm::mat4 viewMatrix;
glm::mat4 projectionMatrix; // set in reshape()
glm::mat4 ModelViewProjectionMatrix; // set in display();
									 //vectors and values for look at
glm::vec3 eye, at, up;

float modelSize[nModels] = {
	2000.0f, // ruber
	200.0f,  // unum
	400.0f,  // duo
	100.0f,  // primus
	150.0f,  // secundus
	100.0f,  // ship
	100.0f,  // Primus missileSilo
	100.0f,  // Secundus missileSilo 
	25.0f,   // ship missile
	25.0f,   // Unum missile
	25.0f,   // Duo missile
}; //modelSize, size of model
   // The rotation amount (in radians) of each object
float rotationAmount[nModels] = { //checked
	0.0f,		// Ruber
	0.004f,		// Unum
	0.002f,		// Duo
	0.002f,		// Primus
	0.004f,		// Secundus
	0.0f,		// ship
	0.0f,		// Primus Missile Site
	0.0f,		// Secundus Missile Site
	0.0f,		// Missile
	0.0f,		// Unum Missile
	0.0f		// Duo Missile
}; //rotationAmount
glm::vec3 scale[nModels]; // set in init()
glm::mat4 translationMatrix[nModels];
glm::vec3 translatePosition[nModels] = {
	glm::vec3(0,0,0), // ruber
	glm::vec3(4000, 0, 0), // unum
	glm::vec3(9000, 0, 0), // duo
	glm::vec3(11000, 0, 0), // primus
	glm::vec3(13000,0,0), // secundus
	glm::vec3(15000, 0, 0), // warbird
	glm::vec3(14500, 0, 0), // Primus Missile Site
	glm::vec3(7250,0,0), // Secundus Missile Site
	glm::vec3(4900, 1000, 4850),  // missle
	glm::vec3(0,0,0),  // Unum missle
	glm::vec3(0,0,0)  // Duo missle
}; //translatePosition 
Object3D * object3D[nModels];
GLuint buffer[nModels];   // Vertex Buffer Objects

						  //Vectors and Cameras
glm::vec3 upVector(0.0f, 1.0f, 0.0f);
glm::vec3 topVector(1.0f, 0.0f, 0.0f);
glm::mat4 mainCamera;
glm::mat4 frontCamera;
glm::mat4 topCamera;
glm::mat4 shipCamera;
glm::mat4 unumCamera;
glm::mat4 duoCamera;
glm::vec3 frontCamEyePosition(0.0f, 10000.0f, 20000.0f);
glm::vec3 topCamEyePosition(0.0f, 20000.0f, 0.0f);
glm::vec3 shipCamEyePosition(0.0f, 300.0f, 1000.0f);
glm::vec3 planetCamEyePosition(-4000.0f, 0.0f, -4000.0f);
char * cameraNames[5] = { "Front Camera", "Top Camera", "Ship Camera", "Unum Camera", "Duo Camera" };
int currentCamera = 0;
static int maxCameras = 5;
//int maxCameras = sizeof(cameraNames);
glm::vec3 shipPosition;
glm::vec3 camPosition;

//Planet rotation variables
GLfloat rotateRadian = 0.0f;
float eyeDistanceMultiplier = 10.0f;
float eyeDistance;
glm::mat4 identityMatrix(1.0f); // initialized identity matrix.
float unumGravityVector = 1.11f;//////////Tom can you check our specs on this?////////??????????????????????????
float duoGravityVector = 5.63f;///////Tom can you check our specs on this?///////////??????????????????????????
glm::mat4 transformMatrix[nModels];
glm::vec3 rotationalAxis(0.0f, 1.0f, 0.0f);

int timerDelay = 25, frameCount = 0; // A delay of 5 milliseconds is 200 updates / second // changed from delay of 5
int timeQuantumState = 0;
double currentTime, lastTime, timeInterval;
bool idleTimerFlag = false;  // interval or idle timer ?
bool wireFrame = false; //initially show surfaces
						//Gravities
bool gravityState = 0;
const float gravity = 90000000.0f;
const float gravityFieldRuber = 5000; ///////Tom can you check our specs on this?///////////??????????????????????????
const float gravityFieldUnum = 6000; ///////Tom can you check our specs on this?///////////??????????????????????????
const float gravityFieldDuo = 2000; ////////Tom can you check our specs on this?//////////??????????????????????????

/* Warp Variables *////////Tom can you check our specs on this?///////////??????????????????????????
glm::mat4 shipCameraSave;
int warpit = 0;
const int maxWarpSpots = 3;
glm::vec3 warpPosition(1000, 0.0f, -3000);

/* Ship Global variables */
glm::mat4 shipOrientationMatrix;
glm::vec3 shipUp(0.0f, 1.0f, 0.0f);
glm::vec3 shipRight(1.0f, 0.0f, 0.0f);
glm::vec3 shipLookingAt(0.0f, 0.0f, -1.0f);
float shipGravityVector = 1.76f; ////////Tom can you check our specs on this?//////////??????????????????????????
int shipSpeedState = 0;
int totalSpeeds = 3;
Warbird * warbird;
float shipSpeed[3] = { 10.0f, 50.0f, 200.0f }; //(min, average, max)
int shipMissiles = 9;

// Ship Missle Variables 
float shipMissleSpeed = 10;
Missile * shipMissile;
Object3D * shipMissileTarget;

// General Missile Variables 
glm::mat4 missileLocation;
glm::mat4 targetLocation;
float length;
float unumLength;
float duoLength;
glm::vec3 targetPositionVector;
glm::vec3 missilePositionVector;
const int missileActivationTimer = 200; //missile doesn't detect for 200 updates
const int missleLifetime = 2000; //lives for 2000 frames
float detectionRadius = 10000.0f; 

								  // Missle Site Variables
int unumMissiles = 5;
int duoMissiles = 5;
float siteMissleSpeed = 5;
Missile * unumMissile;
Missile * duoMissile;
bool unumMissileSiloAlive = true;
bool duoMissileSiloAlive = true;

// Collision Variables 
glm::vec3 objectPosition;
glm::mat4 objectOrientationMatrix;

// Cadet, timer variables, update rate is based on time quantum (TQ)
//'t' key will sequence TQ selection from ace to debug then back to ace
//the TQ will be set by the user
int timeQuantum[4] = {
	5, //ace
	40, //pilot
	100, //traine
	500 //debug
};

/* Display state and "state strings" for title display */
int timerIndex = 0;
int gameState = 0;
bool hasRestarted = false;
const int start = 0, win = 1, lose = 2;
char titleStr[175];
char fpsStr[15];
char baseStr[45] = "Warbird Simulator: {v, x, s, t, f, g, w, r} ";
char warbirdMissleCount[14] = "| Warbird 9";
char unumMissleCount[11] = " | Unum 5";
char duoMissleCount[15] = " | Duo 5";
char cameraStr[30] = "| View: Front Camera ";
char * timerStr[4] = { " | U/S 200 ", " | U/S 25 ", " | U/S 10 ", " | U/S 2 " };
char winGameStr[29] = "Cadet passes flight training";
char loseGameStr[31] = "Cadet resigns from War College";

/* Vertex Buffer and Array Objects */
GLuint textIBO;
GLuint textBuf;
GLuint textVao;
glm::mat4 modelViewMatrix;
GLuint NormalMatrix;
GLuint ModelViewMatrix;
glm::mat3 normalMatrix;
glm::mat4 modelViewProjectionMatrix;


glm::mat4 squareRotation = glm::mat4();
glm::mat4 translateSquare = glm::mat4();

const int numberOfSquares = 6;
float squareRotationAmount = 0.0f;	// default rotation amount

static const unsigned int indices[] = {
	0, 1, 2,
	2, 3, 0
};

static const GLfloat squareVertices[16] = {
	-50000.0f, -50000.0f, 0.0f, 1.0f,	// BL
	50000.0f, -50000.0f, 0.0f, 1.0f,	// BR
	50000.0f, 50000.0f, 0.0f, 1.0f,		// TR
	-50000.0f, 50000.0f, 0.0f, 1.0f		// TL
};

/* Locations of each plane */
glm::vec3 squareTranslationAmounts[6] = {
	glm::vec3(0.0f, 0.0f, -50000.0f),
	glm::vec3(0.0f, 0.0f, 50000.0f),
	glm::vec3(0.0f, -50000.0f, 0.0f),
	glm::vec3(0.0f, 50000.0f, 0.0f),
	glm::vec3(-50000.0f, 0.0f, 0.0f),
	glm::vec3(50000.0f, 0.0f, 0.0f)
};

/* What axis to rotate each plane on */
glm::vec3 squareRotationAxis[6] = {
	glm::vec3(0.0f, 1.0f, 0.0f),
	glm::vec3(0.0f, 1.0f, 0.0f),
	glm::vec3(1.0f, 0.0f, 0.0f),
	glm::vec3(1.0f, 0.0f, 0.0f),
	glm::vec3(0.0f, 1.0f, 0.0f),
	glm::vec3(0.0f, 1.0f, 0.0f),

};

// To maximize efficiency, operations that only need to be called once are called in init().
void init()
{
	// Load the shader programs
	shaderProgram = loadShaders(vertexShaderFile, fragmentShaderFile);//check
	glUseProgram(shaderProgram);//check

								// Generate VAOs and VBOs
	glGenVertexArrays(nModels, VAO);
	glGenBuffers(nModels, buffer);

	// Load the buffers from the model files, generate VAOs and VBOs
	for (int i = 0; i < nModels; i++)
	{
		modelBR[i] = loadModelBuffer(modelFile[i], nVertices[i], VAO[i], buffer[i], shaderProgram,
			vPosition[i], vColor[i], vNormal[i], "vPosition", "vColor", "vNormal");

		if (modelBR[i] == -1.0f)
		{
			printf("loadModelBUffer error:  returned -1.0f \n");
			system("pause");
		}
		else
		{
			printf("loaded %s model with %7.2f bounding radius \n", modelFile[i], modelBR[i]);
		}

		// set scale for models given bounding radius  
		scale[i] = glm::vec3(modelSize[i] / modelBR[i]);
	}

	MVP = glGetUniformLocation(shaderProgram, "ModelViewProjection");
	ModelViewMatrix = glGetUniformLocation(shaderProgram, "ModelViewMatrix");
	NormalMatrix = glGetUniformLocation(shaderProgram, "NormalMatrix");

	// Set up vertex arrays (after shaders are loaded)
	//printf("Shader program variable locations:\n");
	//printf("  vPosition = %d  vColor = %d  vNormal = %d MVP = %d\n",
	glGetAttribLocation(shaderProgram, "vPosition"), // vPosition maps to shader var "vPosition"
		glGetAttribLocation(shaderProgram, "vColor"),
		//glGetAttribLocation(shaderProgram, "vNormal"), MVP);
		glGetAttribLocation(shaderProgram, "vNormal");


	frontCamera = glm::lookAt(
		frontCamEyePosition,  // eye position (in world space)
		translatePosition[RUBERINDEX],                   // look at position (looks at the origin)
		upVector); // up vect0r (head is up, set to 0,-1,0 to look upside down)

	topCamera = glm::lookAt(
		topCamEyePosition,  // eye position
		translatePosition[RUBERINDEX],                   // look at position
		topVector); // up vect0r

	unumCamera = glm::lookAt(
		planetCamEyePosition,  // eye position
		translatePosition[UNUMINDEX], // Looks at unum assuming [1] is unum
		upVector); // up vect0r

	duoCamera = glm::lookAt(
		planetCamEyePosition,  // eye position
		translatePosition[DUOINDEX], //Looks at duo, assuming [3] is duo
		upVector); // up vect0r

	shipCamera = glm::lookAt(
		shipCamEyePosition,  // eye position
		translatePosition[SHIPINDEX],           // look at position
		upVector); // up vect0r

	mainCamera = frontCamera;

	// set render state values
	glEnable(GL_DEPTH_TEST);
	glClearColor(0.7f, 0.7f, 0.7f, 1.0f); // Establishes what color the window will be cleared to.

										  // Create and set all the 3D objects:
	for (int i = 0; i < nModels; i++)
	{
		object3D[i] = new Object3D(modelSize[i], modelBR[i]);
		object3D[i]->setTranslationMatrix(translatePosition[i]);
		object3D[i]->setRotationAmount(rotationAmount[i]);
		// Set the planet flags:
		if (i == UNUMINDEX || i == DUOINDEX)
			object3D[i]->setOrbit();
	}

	// Create the warbird:
	warbird = new Warbird(modelSize[SHIPINDEX], modelBR[SHIPINDEX], translatePosition[SHIPINDEX]);
	warbird->setTranslationMatrix(translatePosition[SHIPINDEX]);
	warbird->setRotationAmount(rotationAmount[SHIPINDEX]);
	warbird->setPosition(translatePosition[SHIPINDEX]);

	// Create the ship missle:
	shipMissile = new Missile(modelSize[SHIPMISSILEINDEX], modelBR[SHIPMISSILEINDEX], shipMissleSpeed);

	// Create the Unum Missile:
	unumMissile = new Missile(modelSize[UNUMMISSILEINDEX], modelBR[UNUMMISSILEINDEX], siteMissleSpeed);

	// Create the Duo Missile:
	duoMissile = new Missile(modelSize[DUOMISSILEINDEX], modelBR[DUOMISSILEINDEX], siteMissleSpeed);

	// set up the indices buffer
	glGenBuffers(1, &textIBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, textIBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	// set up the vertex attributes
	glGenVertexArrays(1, &textVao);
	glBindVertexArray(textVao);

	//  initialize a buffer object
	glGenBuffers(1, &textBuf);
	glBindBuffer(GL_ARRAY_BUFFER, textBuf);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(squareVertices), squareVertices);

	//get ellapsed time
	lastTime = glutGet(GLUT_ELAPSED_TIME);
	// srand is needed in order to generate new random values everytime rand() is called.
	srand(time(NULL));
}


// Indicates what action should be taken when the window is resized.
void reshape(int width, int height)
{
	float aspectRatio = (GLfloat)width / (GLfloat)height;
	float FOVY = glm::radians(60.0f);

	glViewport(0, 0, width, height);
	projectionMatrix = glm::perspective(FOVY, aspectRatio, 1.0f, 100000.0f);
	printf("reshape: FOVY = %5.2f, width = %4d height = %4d aspect = %5.2f \n",
		FOVY, width, height, aspectRatio);
}

// update and display animation state in window title
void updateTitle()
{
	strcpy(titleStr, baseStr);
	strcat(titleStr, warbirdMissleCount);
	strcat(titleStr, unumMissleCount);
	strcat(titleStr, duoMissleCount);
	strcat(titleStr, timerStr[timerIndex]);
	strcat(titleStr, fpsStr);
	strcat(titleStr, cameraStr);
	glutSetWindowTitle(titleStr);
}

// Method to handle the logic for when the ship fires a missle.
void fireShipMissile()
{
	if (shipMissile->hasFired() == false)
	{
		if (shipMissiles > 0)
		{
			// Set the orientation of the missile
			shipMissile->setTranslationMatrix(warbird->getTranslationMatrix());
			shipMissile->setRotationMatrix(warbird->getRotationMatrix());
			shipMissile->setDirection(getIn(warbird->getRotationMatrix()));

			shipMissile->fireMissile();
			shipMissiles--;

			// Update title string for missle count
			strcpy(warbirdMissleCount, "| Warbird ");
			strcat(warbirdMissleCount, std::to_string(shipMissiles).c_str());
		}
	}

}

// Handle the lose game state
void gameLose()
{
	gameState = lose;
	strcpy(titleStr, loseGameStr);
	glutSetWindowTitle(titleStr);
}

// Handle the win game state
void gameWin()
{
	gameState = win;
	strcpy(titleStr, winGameStr);
	glutSetWindowTitle(titleStr);
	if (hasRestarted == false)
	{
		hasRestarted = true;
	}
}

void gravitySwitch()
{
	gravityState = !gravityState;
	printf("Gravity State: %d\n", gravityState);
}

/*
Display() callback is required by freeglut.
It is invoked whenever OpenGL determines a window has to be redrawn.
*/
void display()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Actually clears the window to color specified in glClearColor().

														// Associate shader variables with vertex arrays:
	for (int index = 0; index < nModels; index++)
	{
		switch (index)
		{
		case UNUMINDEX: // If it's planet Unum (planet closest to Ruber with no moons):
			transformMatrix[index] = object3D[index]->getOrientationMatrix();

			// Update Unum's Camera:
			unumCamera = glm::lookAt(getPosition(glm::translate(transformMatrix[index], planetCamEyePosition)), getPosition(transformMatrix[index]), upVector);
			if (currentCamera == UNUMCAMERAINDEX) // Update Unum's Camera:
				mainCamera = unumCamera;
			break;

		case DUOINDEX: // If it's planet Duo (planest farthest from Ruber with moons Secundus and Primus):

			transformMatrix[index] = object3D[index]->getOrientationMatrix();

			// Update Duo's Camera:
			duoCamera = glm::lookAt(getPosition(glm::translate(transformMatrix[index], planetCamEyePosition)), getPosition(object3D[index]->getOrientationMatrix()), upVector);
			if (currentCamera == DUOCAMERAINDEX)
				mainCamera = duoCamera;
			break;

		case PRIMUSINDEX: // If its Primus, one of the moons, orbit around planet Duo.
			transformMatrix[index] = transformMatrix[DUOINDEX] * object3D[index]->getRotationMatrix() * glm::translate(identityMatrix, (translatePosition[index] - translatePosition[DUOINDEX]));
			object3D[index]->setOrientationMatrix(transformMatrix[index]);

			// For Debugging:
			//showMat4("rotation", moonRotationMatrix);
			//showMat4("transform", transformMatrix[index]);
			break;

		case SECUNDUSINDEX: // If its Secundus, one of the moons, orbit around planet Duo.
			transformMatrix[SECUNDUSINDEX] = transformMatrix[DUOINDEX] * object3D[index]->getRotationMatrix() * glm::translate(identityMatrix, (translatePosition[SECUNDUSINDEX] - translatePosition[DUOINDEX]));
			object3D[index]->setOrientationMatrix(transformMatrix[index]);

			break;

		case SHIPINDEX:
			object3D[SHIPINDEX]->setTranslationMatrix(warbird->getTranslationMatrix());
			object3D[SHIPINDEX]->setRotationMatrix(warbird->getRotationMatrix());
			object3D[SHIPINDEX]->setRotationAmount(warbird->getRotationAmount());
			object3D[SHIPINDEX]->setOrientationMatrix(warbird->getOrientationMatrix());

			modelMatrix[index] = object3D[index]->getModelMatrix();
			shipOrientationMatrix = object3D[index]->getOrientationMatrix();

			// Update Ship's Camera:
			camPosition = getPosition(glm::translate(object3D[index]->getModelMatrix(), shipCamEyePosition));
			shipPosition = getPosition(shipOrientationMatrix);
			shipCamera = glm::lookAt(camPosition, glm::vec3(shipPosition.x, shipPosition.y, shipPosition.z), upVector);
			if (currentCamera == SHIPCAMERAINDEX) //If we're on ship camera
				mainCamera = shipCamera;
			break;

		case UNUMMISSLESILOINDEX:
			break;

		case DUOMISSLESILOINDEX:
			break;

		case SHIPMISSILEINDEX:
			object3D[SHIPMISSILEINDEX]->setTranslationMatrix(shipMissile->getTranslationMatrix());
			object3D[SHIPMISSILEINDEX]->setRotationMatrix(shipMissile->getRotationMatrix());
			object3D[SHIPMISSILEINDEX]->setOrientationMatrix(shipMissile->getOrientationMatrix());
			break;

		case UNUMMISSILEINDEX:
			object3D[UNUMMISSILEINDEX]->setTranslationMatrix(unumMissile->getTranslationMatrix());
			object3D[UNUMMISSILEINDEX]->setRotationMatrix(unumMissile->getRotationMatrix());
			object3D[UNUMMISSILEINDEX]->setOrientationMatrix(unumMissile->getOrientationMatrix());
			break;
		case DUOMISSILEINDEX:
			object3D[DUOMISSILEINDEX]->setTranslationMatrix(duoMissile->getTranslationMatrix());
			object3D[DUOMISSILEINDEX]->setRotationMatrix(duoMissile->getRotationMatrix());
			object3D[DUOMISSILEINDEX]->setOrientationMatrix(duoMissile->getOrientationMatrix());
			break;

		default:
			break;
		}

		viewMatrix = mainCamera;
		ModelViewProjectionMatrix = projectionMatrix * viewMatrix * object3D[index]->getModelMatrix();
		glUniformMatrix4fv(MVP, 1, GL_FALSE, glm::value_ptr(ModelViewProjectionMatrix));
		modelViewMatrix = viewMatrix * object3D[index]->getModelMatrix();
		normalMatrix = glm::mat3(modelViewMatrix);
		glUniformMatrix3fv(NormalMatrix, 1, GL_FALSE, glm::value_ptr(normalMatrix));
		glUniformMatrix4fv(ModelViewMatrix, 1, GL_FALSE, glm::value_ptr(modelViewMatrix));
		
		glBindVertexArray(VAO[index]); // set model for its instance. Have to rebind everytime its changed.
		glDrawArrays(GL_TRIANGLES, 0, nVertices[index]);  // Initializes vertex shader, for contiguous groups of vertices.
														  //not sure about this being correct. 
	}

	for (int i = 0; i < numberOfSquares; i++) {
		if (i > 1) {
			squareRotationAmount = PI / 2;
		}
		else {
			squareRotationAmount = 0.0f;
		}
		modelViewMatrix = viewMatrix * glm::translate(translateSquare, squareTranslationAmounts[i])
			* glm::rotate(squareRotation, squareRotationAmount, squareRotationAxis[i]);

		glUniformMatrix4fv(ModelViewMatrix, 1, GL_FALSE, glm::value_ptr(modelViewMatrix));
		normalMatrix = glm::mat3(modelViewMatrix);
		glUniformMatrix3fv(NormalMatrix, 1, GL_FALSE, glm::value_ptr(normalMatrix));
		ModelViewProjectionMatrix = projectionMatrix * modelViewMatrix;
		glUniformMatrix4fv(MVP, 1, GL_FALSE, glm::value_ptr(ModelViewProjectionMatrix));
		glBindVertexArray(textVao);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, textIBO);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, BUFFER_OFFSET(0));
	}

	glutSwapBuffers();

	frameCount++;
	// see if a second has passed to set estimated fps information
	currentTime = glutGet(GLUT_ELAPSED_TIME);  // get elapsed system time
	timeInterval = currentTime - lastTime;
	if (timeInterval >= 1000)
	{
		sprintf(fpsStr, "| F/S %4d ", (int)(frameCount / (timeInterval / 1000.0f)));
		lastTime = currentTime;
		frameCount = 0;

		if (gameState == start)
			updateTitle();
	}
}

void collisionCheck()
{
	objectPosition = getPosition(warbird->getOrientationMatrix());

	if (warbird->isAlive())
	{
		// Check if the warbird collides with planetary bodies:
		for (int index = 0; index < 5; index++)
		{
			length = distance(objectPosition, getPosition(object3D[index]->getOrientationMatrix()));

			if (length < (modelBR[SHIPINDEX] + 10.0f + modelSize[index]))
			{
				// If there is a collision with a planet the warbird gets destroyed
				// The camera view is set to front camera
				warbird->destroy();
				printf("Warbird Hit Planetary Body \n");
				currentCamera = 0;
			}
		}

		// Check if the warbird collides with ship missile:
		if (shipMissile->isSmart()) // We only check for the collision when the missile becomes smart
		{
			length = distance(objectPosition, getPosition(shipMissile->getOrientationMatrix()));

			if (length < (modelBR[SHIPINDEX] + 10.0f + modelBR[SHIPMISSILEINDEX]))
			{
				warbird->destroy();
				shipMissile->destroy();
				printf("Ship Missile %d hit warbird \n", shipMissiles);
				currentCamera = 0;
			}
		}

		// Check if the warbird collides with Unum missile:
		if (unumMissile->isSmart()) // We only check for the collision when the missile becomes smart
		{
			length = distance(objectPosition, getPosition(unumMissile->getOrientationMatrix()));

			if (length < (modelBR[SHIPINDEX] + 10.0f + modelBR[UNUMMISSILEINDEX]))
			{
				warbird->destroy();
				unumMissile->destroy();
				printf("Unum Missile %d is gone \n", unumMissiles);
				currentCamera = 0;
			}
		}

		// Check if the warbird collides with Duo missile:
		if (duoMissile->isSmart()) // We only check for the collision when the missile becomes smart
		{
			length = distance(objectPosition, getPosition(duoMissile->getOrientationMatrix()));

			if (length < (modelBR[SHIPINDEX] + modelBR[DUOMISSILEINDEX] + 10.0f))
			{
				warbird->destroy();
				duoMissile->destroy();
				printf("Duo Missile %d is gone \n", duoMissiles);
				currentCamera = 0;
			}
		}

		// Check if the warbird collides with Unum Missile Site:
		length = distance(objectPosition, getPosition(object3D[UNUMMISSLESILOINDEX]->getOrientationMatrix()));

		if (length < (modelBR[SHIPINDEX] + modelBR[UNUMMISSLESILOINDEX] + 10.0f))
		{
			warbird->destroy();
			printf("Warbird Hit Unum Missile Site \n");
			currentCamera = 0;
		}

		// Check if the warbird collides with Duo Missile Site:
		length = distance(objectPosition, getPosition(object3D[DUOMISSLESILOINDEX]->getOrientationMatrix()));

		if (length < (modelBR[SHIPINDEX] + modelBR[DUOMISSLESILOINDEX] + 10.0f))
		{
			warbird->destroy();
			printf("Warbird Hit Duo Missile Site \n");
			currentCamera = 0;
		}
	}

	////////////////////////////////////////////////////
	// Missiles Collision Detection:
	////////////////////////////////////////////////////

	objectPosition = getPosition(shipMissile->getOrientationMatrix());

	// Check ship missile for collision:
	if (shipMissile->isSmart()) // We only check for the collision when the missile becomes smart
	{
		// Check if it collides with a missile site:

		// Unum Missile Site:
		length = distance(objectPosition, getPosition(object3D[UNUMMISSLESILOINDEX]->getOrientationMatrix()));

		if (length < (modelBR[UNUMMISSLESILOINDEX] + modelBR[SHIPMISSILEINDEX] + 10.0f))
		{
			shipMissile->destroy();
			unumMissileSiloAlive = false;
			printf("Unum Missile Silo is dead \n");
		}

		// Duo Missile Site:
		length = distance(objectPosition, getPosition(object3D[DUOMISSLESILOINDEX]->getOrientationMatrix()));

		if (length < (modelBR[DUOMISSLESILOINDEX] + modelBR[SHIPMISSILEINDEX] + 10.0f))
		{
			shipMissile->destroy();
			duoMissileSiloAlive = false;
			printf("Duo Missile Silo is dead \n");
		}

		// Check if it collides with a planetary body:
		for (int index = 0; index < 5; index++)
		{
			length = distance(objectPosition, getPosition(object3D[index]->getOrientationMatrix()));

			if (length < (modelBR[index] + modelBR[SHIPMISSILEINDEX] + 10.0f)) {

				// If there is a collision with a planet, the missile is destroyed
				shipMissile->destroy();
				printf("Ship Missile %d hit planet \n", shipMissiles);
			}
		}
	}

	objectPosition = getPosition(unumMissile->getOrientationMatrix());

	// Check Unum missile for collision:
	if (unumMissile->isSmart()) // We only check for the collision when the missile becomes smart
	{
		// Check if it collides with Unum missile site:
		length = distance(objectPosition, getPosition(object3D[UNUMMISSLESILOINDEX]->getOrientationMatrix()));

		if (length < (modelBR[UNUMMISSLESILOINDEX] + modelBR[UNUMMISSILEINDEX] + 10.0f))
		{
			unumMissile->destroy();
			printf("Unum Missile %d is gone \n", unumMissiles);
			unumMissileSiloAlive = false;
			printf("Unum Missile Silo is dead \n");
		}

		// Check if it collides with Duo missile site:
		length = distance(objectPosition, getPosition(object3D[DUOMISSLESILOINDEX]->getOrientationMatrix()));

		if (length < (modelBR[DUOMISSLESILOINDEX] + modelBR[UNUMMISSILEINDEX] + 10.0f))
		{
			unumMissile->destroy();
			printf("Unum Missile %d is gone \n", unumMissiles);
			duoMissileSiloAlive = false;
			printf("Duo Missile Silo is dead \n");
		}

		// Check if it collides with a planetary body:
		for (int index = 0; index < 5; index++)
		{
			length = distance(objectPosition, getPosition(object3D[index]->getOrientationMatrix()));

			if (length < (modelBR[index] + modelBR[UNUMMISSILEINDEX] + 10.0f)) {

				// If there is a collision with a planet, the missile is destroyed
				unumMissile->destroy();
				printf("Unum Missile %d is gone \n", unumMissiles);
			}
		}
	}

	objectPosition = getPosition(duoMissile->getOrientationMatrix());

	// Check Duo missile for collision:
	if (duoMissile->isSmart()) // We only check for the collision when the missile becomes smart
	{
		// Check if it collides with Unum missile site:
		length = distance(objectPosition, getPosition(object3D[UNUMMISSLESILOINDEX]->getOrientationMatrix()));

		if (length < modelBR[UNUMMISSLESILOINDEX] + modelBR[DUOMISSILEINDEX] + 10.0f)
		{
			duoMissile->destroy();
			printf("Duo Missile %d is gone \n", duoMissiles);
			unumMissileSiloAlive = false;
			printf("Unum Missile Silo is dead \n");
		}

		//// Check if it collides with Duo missile site:
		length = distance(objectPosition, getPosition(object3D[DUOMISSLESILOINDEX]->getOrientationMatrix()));

		if (length < (modelBR[DUOMISSLESILOINDEX] + modelBR[DUOMISSILEINDEX] + 10.0f))
		{
			duoMissile->destroy();
			printf("Duo Missile %d is gone \n", duoMissiles);
			duoMissileSiloAlive = false;
			printf("Duo Missile Silo is dead \n");
		}

		// Check if it collides with a planetary body:
		for (int index = 0; index < 5; index++)
		{
			length = distance(objectPosition, getPosition(object3D[index]->getOrientationMatrix()));

			if (length < (modelBR[index] + modelBR[DUOMISSILEINDEX] + 10.0f)) {

				// If there is a collision with a planet, the missile is destroyed
				duoMissile->destroy();
				printf("Duo Missile %d is gone \n", duoMissiles);
			}
		}
	}
}

void handleMissiles()
{
	// Check to see the update count of the ship missile before activating smart.
	if (shipMissile->hasFired())
	{
		if (shipMissile->getUpdateFrameCount() > missileActivationTimer)
		{
			shipMissile->activateSmart();
		}
	}

	if (unumMissile->hasFired())
	{
		if (unumMissile->getUpdateFrameCount() > missileActivationTimer)
		{
			unumMissile->activateSmart();
		}
	}

	if (duoMissile->hasFired())
	{
		if (duoMissile->getUpdateFrameCount() > missileActivationTimer)
		{
			duoMissile->activateSmart();
		}
	}

	// If the ship missile has been fired and is smart it needs to find a target
	if (shipMissile->hasFired())
	{
		if (shipMissile->isSmart())
		{
			// If it doesn't have a target we need to find one for it:
			if (!shipMissile->isTargetLocked())
			{
				// Determine the closest target for the warbirds missile

				// Get the distance from Unum
				missileLocation = shipMissile->getOrientationMatrix();
				targetLocation = object3D[UNUMMISSLESILOINDEX]->getOrientationMatrix();
				missilePositionVector = getPosition(missileLocation);
				targetPositionVector = getPosition(targetLocation);
				unumLength = distance(missilePositionVector, targetPositionVector);

				// Get the distance from Duo
				missileLocation = shipMissile->getOrientationMatrix();
				targetLocation = object3D[DUOMISSLESILOINDEX]->getOrientationMatrix();
				missilePositionVector = getPosition(missileLocation);
				targetPositionVector = getPosition(targetLocation);
				duoLength = distance(missilePositionVector, targetPositionVector);

				// The target will be one of the missile sites, the closest
				// one to the missile that is within the missiles detection range.
				if (unumLength <= duoLength)
				{
					shipMissileTarget = object3D[UNUMMISSLESILOINDEX];
					shipMissile->setTargetLocation(shipMissileTarget->getOrientationMatrix());
					printf("Ship Missile Target is UNUM Missile Site \n");
				}
				else if (unumLength > duoLength)
				{
					shipMissileTarget = object3D[DUOMISSLESILOINDEX];
					shipMissile->setTargetLocation(shipMissileTarget->getOrientationMatrix());
					printf("Ship Missile Target is DUO Missile Site \n");
				}
			}

			// Update the missiles knowledge of its targets location.
			else
			{
				shipMissile->setTargetLocation(shipMissileTarget->getOrientationMatrix());
			}
		}
	}

	else // The ship missile hasn't been fired yet so keep it next to the warbird
	{
		shipMissile->setOrientationMatrix(glm::translate(warbird->getOrientationMatrix(), glm::vec3(-33, 0, -30)));
	}

	/* UNUM MISSILE SITE MISSILE: */

	if (!unumMissile->hasFired())
	{
		// Set position of the missile if it has not been fired to the missile silo
		unumMissile->setOrientationMatrix(object3D[UNUMMISSLESILOINDEX]->getOrientationMatrix());

		// Check to see if the warbird is in the detection radius by getting the distance between the two objects:
		missileLocation = unumMissile->getOrientationMatrix();
		targetLocation = warbird->getOrientationMatrix();
		missilePositionVector = getPosition(missileLocation);
		targetPositionVector = getPosition(targetLocation);
		length = distance(missilePositionVector, targetPositionVector);

		if (length <= detectionRadius && unumMissiles > 0)
		{
			unumMissile->fireMissile();
			unumMissile->setTargetLocation(warbird->getOrientationMatrix());
			unumMissiles--;

			// Update title string for the Unum Missile Site missile count
			strcpy(unumMissleCount, " | Unum ");
			strcat(unumMissleCount, std::to_string(unumMissiles).c_str());
		}
	}

	// Once the missile becomes smart keep updating it to get the warbird's location.
	if (unumMissile->isSmart())
	{
		unumMissile->setTargetLocation(warbird->getOrientationMatrix());
	}

	/* DUO MISSILE SITE MISSILE: */

	if (!duoMissile->hasFired())
	{
		// Set position of the missile if it has not been fired to the missile silo
		duoMissile->setOrientationMatrix(object3D[DUOMISSLESILOINDEX]->getOrientationMatrix());

		// Check to see if the warbird is in the detection radius by getting the distance between the two objects:
		missileLocation = duoMissile->getOrientationMatrix();
		targetLocation = warbird->getOrientationMatrix();
		missilePositionVector = getPosition(missileLocation);
		targetPositionVector = getPosition(targetLocation);
		length = distance(missilePositionVector, targetPositionVector);

		if (length <= detectionRadius && duoMissiles > 0)
		{
			duoMissile->fireMissile();
			duoMissile->setTargetLocation(warbird->getOrientationMatrix());
			duoMissiles--;

			// Update title string for the Duo Missile Site missile count
			strcpy(duoMissleCount, " | Duo ");
			strcat(duoMissleCount, std::to_string(duoMissiles).c_str());
		}
	}

	// Once the missile becomes smart keep updating it to get the warbird's location.
	if (duoMissile->isSmart())
	{
		duoMissile->setTargetLocation(warbird->getOrientationMatrix());
	}

	// Update all the missiles:
	shipMissile->update();
	unumMissile->update();
	duoMissile->update();
}

// Animate scene objects by updating their transformation matrices
// for use with Idle and intervalTimer functions to set rotation
void update(int i)
{
	glutTimerFunc(timeQuantum[timeQuantumState], update, 1); // glutTimerFunc(time, fn, arg). This sets fn() to be called after time millisecond with arg as an argument to fn().

															 // Update all of the object3D's
	for (int index = 0; index < nModels; index++)
	{
		object3D[index]->update();
	}

	// Update the missile silo orientation matrices

	// Update the Unum Missile Silo
	object3D[UNUMMISSLESILOINDEX]->setTranslationMatrix(object3D[UNUMINDEX]->getTranslationMatrix());
	object3D[UNUMMISSLESILOINDEX]->setOrientationMatrix(object3D[UNUMINDEX]->getRotationMatrix());
	object3D[UNUMMISSLESILOINDEX]->setTranslationMatrix(glm::translate(object3D[UNUMINDEX]->getTranslationMatrix(), glm::vec3(0, 135, 0)));
	transformMatrix[UNUMMISSLESILOINDEX] = glm::translate(object3D[UNUMINDEX]->getOrientationMatrix(), glm::vec3(0, 135, 0));
	object3D[UNUMMISSLESILOINDEX]->setOrientationMatrix(transformMatrix[UNUMMISSLESILOINDEX]);

	// Update the Duo Missile Silo
	object3D[DUOMISSLESILOINDEX]->setTranslationMatrix(object3D[DUOINDEX]->getTranslationMatrix());
	object3D[DUOMISSLESILOINDEX]->setOrientationMatrix(object3D[DUOINDEX]->getRotationMatrix());
	object3D[DUOMISSLESILOINDEX]->setTranslationMatrix(glm::translate(object3D[DUOINDEX]->getTranslationMatrix(), glm::vec3(0, 400, 0)));
	transformMatrix[DUOMISSLESILOINDEX] = glm::translate(object3D[DUOINDEX]->getOrientationMatrix(), glm::vec3(0, 400, 0));
	object3D[DUOMISSLESILOINDEX]->setOrientationMatrix(transformMatrix[DUOMISSLESILOINDEX]);

	// Update the warbird object
	warbird->update();

	// Update all the missiles
	handleMissiles();

	// Check for any collisions:
	collisionCheck();

	// Update Gravity:
	if (gravityState == true)
	{
		glm::vec3 shipPosition = getPosition(warbird->getTranslationMatrix());

		//Check distance to Ruber
		glm::vec3 vectorPointingFromShipToRuber = translatePosition[RUBERINDEX] - shipPosition;
		float distanceToRuber = glm::length(vectorPointingFromShipToRuber);

		if (distanceToRuber < gravityFieldRuber) {
			//normalize the vector, this is now gravity
			glm::vec3 gravity = (vectorPointingFromShipToRuber / distanceToRuber);
			warbird->setTranslationMatrix(gravity * glm::vec3(0.8f, 0.8f, 0.8f));
		}
	}

	// Check if the player won the game:
	if (unumMissileSiloAlive == false && duoMissileSiloAlive == false)
	{
		gameWin();
	}
	// Check if the player lost the game:
	if (warbird->isAlive() == false || (shipMissiles == 0 && (unumMissileSiloAlive == true || duoMissileSiloAlive == true)))
	{
		gameLose();
	}
	glutPostRedisplay();
}

// Estimate FPS, use for fixed interval timer driven animation
void intervalTimer(int i)
{
	glutTimerFunc(timeQuantum[timeQuantumState], intervalTimer, 1);
	if (!idleTimerFlag)
	{
		update(1);  // fixed interval timer
	}
}

void switchCamera(int camera)
{
	switch (camera)
	{
	case FRONTCAMERAINDEX:
		mainCamera = frontCamera;
		break;
	case TOPCAMERAINDEX:
		mainCamera = topCamera;
		break;
	case SHIPCAMERAINDEX:
		mainCamera = shipCamera;
		break;
	case UNUMCAMERAINDEX:
		mainCamera = unumCamera;
		break;
	case DUOCAMERAINDEX:
		mainCamera = duoCamera;
		break;
	default:
		return;
	}
	sprintf(cameraStr, "| View: %s", cameraNames[camera]);
	printf("Current Camera: %s\n", cameraNames[camera]);
	display();
}

void keyboard(unsigned char key, int x, int y)
{
	switch (key)
	{
	case 'v': case 'V':
		currentCamera = (currentCamera + 1) % maxCameras;
		switchCamera(currentCamera);
		break;
	case 'x': case 'X':
		if (currentCamera == 0)
			currentCamera = 5;
		currentCamera = (currentCamera - 1) % maxCameras;
		switchCamera(currentCamera);
		break;
	case 's': case'S':
		shipSpeedState = (shipSpeedState + 1) % totalSpeeds;
		warbird->setSpeed(shipSpeed[shipSpeedState]);
		printf("Current Ship Speed: %.2f \n", warbird->getSpeed());
		break;
	case 't': case'T':
		if (timeQuantumState >= 3)
		{
			timeQuantumState = 0;
			timerIndex = 0;
		}
		else
		{
			timeQuantumState++;
			timerIndex++;
		}
		break;
	case 'f': case'F':
		fireShipMissile();
		break;
	case 'g': case'G':
		gravitySwitch();
		break;
	case 'w': case'W':
		warpit = (warpit + 1) % maxWarpSpots;

		switch (warpit)
		{
		case 0:
			warbird->setTranslationMatrix(glm::translate(identityMatrix, translatePosition[SHIPINDEX]));
			warbird->setRotationMatrix(glm::rotate(identityMatrix, 0.0f, glm::vec3(0, 1, 0)));
			printf("Ship Warped back to original position\n");
			break;

		case 1:
			warbird->setTranslationMatrix(glm::translate(identityMatrix, getPosition(glm::translate
			(transformMatrix[UNUMINDEX], planetCamEyePosition))));
			warbird->setRotationMatrix(glm::rotate(object3D[UNUMINDEX]->getRotationMatrix(), PI, glm::vec3(0, 1, 0)));

			printf("Ship Warped to Unum\n");
			break;

		case 2:
			warbird->setTranslationMatrix(glm::translate(identityMatrix, getPosition(glm::translate
			(transformMatrix[DUOINDEX], planetCamEyePosition))));
			warbird->setRotationMatrix(glm::rotate(object3D[DUOINDEX]->getRotationMatrix(), PI, glm::vec3(0, 1, 0)));

			printf("Ship Warped to Duo\n");
			break;
		}
		break;
	case 'r': case'R':
		// Reset Unum Missile Site:
		unumMissiles = 5;
		unumMissileSiloAlive = true;

		// Reset Duo Missile Site:
		duoMissiles = 5;
		duoMissileSiloAlive = true;

		// Reset Warbird:
		shipMissiles = 9;
		warbird->restart();

		// Reset the game state flag:
		gameState = start;
		hasRestarted = false;
		currentCamera = SHIPCAMERAINDEX;
		break;
	}
}

void handleSpecialKeypress(int key, int x, int y)
{
	switch (key)
	{
	case GLUT_KEY_UP:
		if (glutGetModifiers() == GLUT_ACTIVE_CTRL)
		{
			warbird->setPitch(1);
		}
		else // ship forward = positive step on "at" vector
		{
			warbird->setMove(-1);
		}
		break;
	case GLUT_KEY_DOWN:
		if (glutGetModifiers() == GLUT_ACTIVE_CTRL)
		{
			warbird->setPitch(-1);
		}
		else // ship backward = negative step on "at" vector
		{
			warbird->setMove(1);
		}
		break;
	case GLUT_KEY_LEFT:
		if (glutGetModifiers() == GLUT_ACTIVE_CTRL)
		{
			warbird->setRoll(1);
		}
		else // ship yaws "right" = rotate 0.02 radians on "up" vector
		{
			warbird->setYaw(1);
		}
		break;
	case GLUT_KEY_RIGHT:
		if (glutGetModifiers() == GLUT_ACTIVE_CTRL)
		{
			warbird->setRoll(-1);
		}
		else // ship yaws "left" = rotate -0.02 radians on "up" vector
		{
			warbird->setYaw(-1);
		}
		break;
	}
}//handleSpecialKeypress

 /*
 The main() has a number of tasks:
 * Initialize and open a window
 * Initialize the buffers and parameters by calling init()
 * Specify the callback functions for events
 * Enter an infinite event loop
 */
int main(int argc, char** argv) {
	glutInit(&argc, argv); // Initializes GLUT.
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH); // Configure  GLUT options.
	glutInitWindowSize(1024, 800);
	glutInitContextVersion(3, 3);
	glutInitContextProfile(GLUT_CORE_PROFILE);

	// set OpenGL and GLSL versions to 3.3 for Comp 465/L, comment to see highest versions.
	glutInitContextVersion(3, 3);
	glutInitContextProfile(GLUT_CORE_PROFILE);
	glutCreateWindow(titleStr);

	/*
	GLEW manages function pointers for OpenGL so we want to
	initialize GLEW before we call any OpenGL functions.
	*/
	glewExperimental = GL_TRUE;  // Set true to use more modern techniques for managing OpenGL functionality.
	GLenum err = glewInit(); // Initialize GLEW
	if (GLEW_OK != err) // Check for any errors. (Must be done after GLUT has been inititalized)
	{
		printf("GLEW Error: %s \n", glewGetErrorString(err));
	}
	else
	{
		printf("Using GLEW %s \n", glewGetString(GLEW_VERSION));
		printf("OpenGL %s, GLSL %s\n",
			glGetString(GL_VERSION),
			glGetString(GL_SHADING_LANGUAGE_VERSION));
	}

	// initialize scene
	init();

	// set glut callback functions
	glutDisplayFunc(display); // Continuously called for interacting with the window. 
	glutReshapeFunc(reshape);
	glutKeyboardFunc(keyboard);
	glutSpecialFunc(handleSpecialKeypress);

	glutIdleFunc(NULL); // start with intervalTimer
	glutTimerFunc(timeQuantum[timeQuantumState], update, 1); // glutTimerFunc(time, fn, arg). This sets fn() to be called after time millisecond with arg as an argument to fn().

	glutMainLoop();  // This call passes control to enter GLUT event processing cycle.

	printf("done\n");
	return 0;
}//main