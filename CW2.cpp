#define FREEGLUT_STATIC
#include <GL/freeglut.h>
#include <math.h>
#define M_PI 3.14159265359

#include <iostream>
using namespace std;

float carCenterX = 0.0f;
float carCenterY = 0.0f;
float carCenterZ = 0.0f;
float carXspeed = 0.0f;
float carYspeed = 0.0f;
float carZspeed = 0.0f;
bool carTurnLeft = false;
bool carTurnRight = false;
bool crashed = false;
float startHeight = 10.0;

int frameRate = 60; // Desired frame rate (frames per second)

float fltFOV = 70; //Field Of View
float fltZoom = 1.0; //Zoom amount
float fltX0 = -3.0; //Camera position
float fltY0 = 3.0;
float fltZ0 = -3.0;
float fltXRef = 0.0; //Look At reference point
float fltYRef = 0.0;
float fltZRef = 0.0;
float fltXUp = 0.0; //Up vector
float fltYUp = 1.0;
float fltZUp = 0.0;
float fltViewingAngle = 1;
int fltMode = 0;


void setupLightingAndMaterial(float red, float green, float blue) {
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);

    // Set light source position
    GLfloat light_position[] = { 1.0, 1.0, 1.0, 0.0 };  // Position above the scene
    glLightfv(GL_LIGHT0, GL_POSITION, light_position);

    // Set light source properties
    GLfloat light_ambient[] = { 0.2, 0.2, 0.2, 1.0 };
    GLfloat light_diffuse[] = { 1.0, 1.0, 1.0, 1.0 };
    GLfloat light_specular[] = { 1.0, 1.0, 1.0, 1.0 };

    glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);

    // Set material properties for the object
    GLfloat material_ambient[] = { red, green, blue, 1.0 };
    GLfloat material_diffuse[] = { red, green, blue, 1.0 };
    GLfloat material_specular[] = { 1.0, 1.0, 1.0, 1.0 };
    GLfloat material_shininess[] = { 50.0 };

    glMaterialfv(GL_FRONT, GL_AMBIENT, material_ambient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, material_diffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR, material_specular);
    glMaterialfv(GL_FRONT, GL_SHININESS, material_shininess);
}

void drawCube(float width, float height, float depth, float centerX, float centerY, float centerZ, float xAngle, float yAngle, float zAngle, float red, float green, float blue) {
    setupLightingAndMaterial(red, green, blue);

    float halfWidth = width / 2.0;
    float halfHeight = height / 2.0;
    float halfDepth = depth / 2.0;

    // Calculate the vertices of the parallelepiped based on the center position
    float vertices[8][3] = {
        {centerX - halfWidth, centerY - halfHeight, centerZ + halfDepth},
        {centerX + halfWidth, centerY - halfHeight, centerZ + halfDepth},
        {centerX + halfWidth, centerY + halfHeight, centerZ + halfDepth},
        {centerX - halfWidth, centerY + halfHeight, centerZ + halfDepth},
        {centerX - halfWidth, centerY - halfHeight, centerZ - halfDepth},
        {centerX - halfWidth, centerY + halfHeight, centerZ - halfDepth},
        {centerX + halfWidth, centerY + halfHeight, centerZ - halfDepth},
        {centerX + halfWidth, centerY - halfHeight, centerZ - halfDepth}
    };

    // Apply rotation around x-axis
    for (int i = 0; i < 8; i++) {
        float y = vertices[i][1] - centerY;
        float z = vertices[i][2] - centerZ;

        // Rotate the points around the x-axis
        float rotatedY = y * cos(xAngle) - z * sin(xAngle);
        float rotatedZ = y * sin(xAngle) + z * cos(xAngle);

        // Update the rotated vertices
        vertices[i][1] = rotatedY + centerY;
        vertices[i][2] = rotatedZ + centerZ;
    }

    // Apply rotation around y-axis
    for (int i = 0; i < 8; i++) {
        float x = vertices[i][0] - centerX;
        float z = vertices[i][2] - centerZ;

        // Rotate the points around the y-axis
        float rotatedX = x * cos(yAngle) + z * sin(yAngle);
        float rotatedZ = -x * sin(yAngle) + z * cos(yAngle);

        // Update the rotated vertices
        vertices[i][0] = rotatedX + centerX;
        vertices[i][2] = rotatedZ + centerZ;
    }

    // Apply rotation around z-axis
    for (int i = 0; i < 8; i++) {
        float x = vertices[i][0] - centerX;
        float y = vertices[i][1] - centerY;

        // Rotate the points around the z-axis
        float rotatedX = x * cos(zAngle) - y * sin(zAngle);
        float rotatedY = x * sin(zAngle) + y * cos(zAngle);

        // Update the rotated vertices
        vertices[i][0] = rotatedX + centerX;
        vertices[i][1] = rotatedY + centerY;
    }

    // Draw colored faces of the parallelepiped
    glBegin(GL_QUADS);

    // Front face
    glVertex3fv(vertices[0]);
    glVertex3fv(vertices[1]);
    glVertex3fv(vertices[2]);
    glVertex3fv(vertices[3]);

    // Back face
    glVertex3fv(vertices[4]);
    glVertex3fv(vertices[5]);
    glVertex3fv(vertices[6]);
    glVertex3fv(vertices[7]);

    // Top face
    glVertex3fv(vertices[3]);
    glVertex3fv(vertices[2]);
    glVertex3fv(vertices[6]);
    glVertex3fv(vertices[5]);

    // Bottom face
    glVertex3fv(vertices[0]);
    glVertex3fv(vertices[4]);
    glVertex3fv(vertices[7]);
    glVertex3fv(vertices[1]);

    // Right face
    glVertex3fv(vertices[1]);
    glVertex3fv(vertices[7]);
    glVertex3fv(vertices[6]);
    glVertex3fv(vertices[2]);

    // Left face
    glVertex3fv(vertices[0]);
    glVertex3fv(vertices[3]);
    glVertex3fv(vertices[5]);
    glVertex3fv(vertices[4]);

    glEnd();
}

void drawEZCube(float width, float height, float depth, float centerX, float centerY, float centerZ, float red, float green, float blue) {
    drawCube(width, height, depth, centerX, centerY, centerZ, 0.0, 0.0, 0.0, red, green, blue);
}

void drawCar(float size, float centerX, float centerY, float centerZ, float red, float green, float blue) {
    setupLightingAndMaterial(red, green, blue);

    float wheel_size = 1.0 * size;
    float distance = carCenterX;

    glEnable(GL_LIGHTING);

    // Car body
    drawEZCube(1.0 * size, 0.2 * size, 0.6 * size, centerX, centerY, centerZ, red, green, blue);

    // Draw wheels in cube
    float rotateZ = 1.0;
    float rotateYangle = 0.0;
    if (carTurnLeft == true && carTurnRight == false) {
        rotateYangle = M_PI / 4;
        rotateZ = 0.0;
    };
    if (carTurnLeft == false && carTurnRight == true) {
        rotateYangle = -M_PI / 4;
        rotateZ = 0.0;
    };
    if (carTurnLeft == false && carTurnRight == false) {
        rotateYangle = 0.0;
        rotateZ = 1.0;
    };
    drawCube(0.15 * size, 0.15 * size, 0.05 * size, -0.3 * size + centerX, -0.15 * size + centerY, -0.3 * size + centerZ, 0.0, rotateYangle, (-distance / 0.15) * rotateZ, 0.2, 0.2, 0.2);
    drawCube(0.15 * size, 0.15 * size, 0.05 * size, -0.3 * size + centerX, -0.15 * size + centerY, +0.3 * size + centerZ, 0.0, rotateYangle, (-distance / 0.15) * rotateZ, 0.2, 0.2, 0.2);
    drawCube(0.15 * size, 0.15 * size, 0.05 * size, +0.3 * size + centerX, -0.15 * size + centerY, -0.3 * size + centerZ, 0.0, rotateYangle, (-distance / 0.15) * rotateZ, 0.2, 0.2, 0.2);
    drawCube(0.15 * size, 0.15 * size, 0.05 * size, +0.3 * size + centerX, -0.15 * size + centerY, +0.3 * size + centerZ, 0.0, rotateYangle, (-distance / 0.15) * rotateZ, 0.2, 0.2, 0.2);

    glDisable(GL_LIGHTING);

    glEnd();
}

void drawGround(float centerX, float centerY, float centerZ) {
    float groundWidth = 500.0;
    float groundHeight = 0.5;
    float groundDepth = 500.0;
    float r = 0.15;
    float g = 0.1;
    float b = 0.15;
    drawCube(groundWidth, groundHeight, groundDepth, centerX, centerY, centerZ, 0.0, 0.0, 0.0, r, g, b);
}

void drawDashedLine(float centerX, float centerY, float centerZ) {
    float LineWidth = 0.6;
    float LineHeight = 0.1;
    float LineDepth = 0.05;
    float r = 0.9;
    float g = 0.9;
    float b = 0.9;
    for (float X = -250.0; X < 250.0; X += 1.5) {
        drawCube(LineWidth, LineHeight, LineDepth, X + centerX, centerY - LineHeight / 2, centerZ, 0.0, 0.0, 0.0, r, g, b);
    }
}

void drawScene(float centerX, float centerY, float centerZ) {
    float groundX = centerX;
    float groundY = centerY - startHeight;
    float groundZ = centerZ;

    drawGround(groundX, groundY, groundZ);
    drawDashedLine(groundX, groundY, groundZ - 1.0);
    drawDashedLine(groundX, groundY, groundZ + 1.0);


    drawEZCube(0.2, 0.2, 0.2, groundX, groundY, groundZ, 0.0, 0.0, 1.0); // For background Test
}

void initLighting() {
    // First point light source
    GLfloat light_position0[] = { -1.0, 1.0, 0.0, 1.0 };
    GLfloat light_diffuse0[] = { 0.0, 1.0, 0.0, 1.0 };
    glEnable(GL_LIGHT0);
    glLightfv(GL_LIGHT0, GL_POSITION, light_position0);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse0);

    // Second point light source
    GLfloat light_position1[] = { -1.0, 1.0, 0.0, 1.0 };
    GLfloat light_diffuse1[] = { 0.0, 1.0, 0.0, 1.0 };
    glEnable(GL_LIGHT1);
    glLightfv(GL_LIGHT1, GL_POSITION, light_position1);
    glLightfv(GL_LIGHT1, GL_DIFFUSE, light_diffuse1);

    glEnable(GL_LIGHTING);
}

void handlefltMode() {
    switch (fltMode) {
    case 0:
        fltMode = 1;
        break;
    case 1:
        fltMode = 0;
        break;
    }
}

void handleKeypress(unsigned char key, int x, int y) {
    switch (key) {
    case 'e':
        carXspeed += 0.02f;
        carXspeed = min(carXspeed, 0.2f);
        break;
    case 'd':
        carXspeed -= 0.02f;
        carXspeed = max(carXspeed, -0.2f);
        break;
    case 's':
        carZspeed -= 0.02f;
        carZspeed = min(carZspeed, 0.2f);
        break;
    case 'f':
        carZspeed += 0.02f;
        carZspeed = max(carZspeed, -0.2f);
        break;
    case 'a':
        carYspeed += 0.0005f;
        break;
    case 'z':
        carYspeed -= 0.005f;
        break;
    case ' ':
        handlefltMode();
        break;
    }

    glutPostRedisplay();
}

void update(int value) {
    // Update the cube's position based on speed
    if (carXspeed > 0) {
        carXspeed -= 0.005f;
    }

    if (carXspeed < 0) {
        carXspeed += 0.005f;
    }

    if (carZspeed > 0) {
        carZspeed -= 0.005f;
        carTurnRight = true;
        carTurnLeft = false;
    }

    if (carZspeed < 0) {
        carZspeed += 0.005f;
        carTurnRight = false;
        carTurnLeft = true;
    }

    if (carCenterY > -startHeight + 0.1) {
        carYspeed -= 0.00005f;
    }

    if (carCenterY < -startHeight + 0.1) {
        carCenterY = -startHeight + 0.1;
        carYspeed = - 0.1 * carYspeed;
    }

    if (abs(carXspeed) < 0.005f) { carXspeed = 0; };
    if (abs(carZspeed) < 0.005f) {
        carZspeed = 0;
        carTurnRight = false;
        carTurnLeft = false;
    };
    if (abs(carYspeed) < 0.1 * 0.00005f) { carYspeed = 0; };

    carCenterX += carXspeed;
    carCenterZ += carZspeed;
    carCenterY += carYspeed;

    std::cout << "__________" << endl;
    std::cout << carXspeed << endl;
    std::cout << carZspeed << endl;
    std::cout << carYspeed << endl;

    switch (fltMode) {
    case 0:
        fltFOV = 70; //Field Of View
        fltZoom = 1.0; //Zoom amount
        fltX0 = -3.0; //Camera position
        fltY0 = 3.0;
        fltZ0 = -3.0;
        fltXRef = 0.0; //Look At reference point
        fltYRef = 0.0;
        fltZRef = 0.0;
        fltXUp = 0.0; //Up vector
        fltYUp = 1.0;
        fltZUp = 0.0;
        fltViewingAngle = 1;
        break;
    case 1:
        fltFOV = 70; //Field Of View
        fltZoom = 1.0; //Zoom amount
        fltX0 = 0.5; //Camera position
        fltY0 = 0.5;
        fltZ0 = 0.0;
        fltXRef = 500.0; //Look At reference point
        fltYRef = 0.5;
        fltZRef = 0.0;
        fltXUp = 0.0; //Up vector
        fltYUp = 1.0;
        fltZUp = 0.0;
        fltViewingAngle = 1;
        break;
    }


    glutPostRedisplay();
    glutTimerFunc(1000 / frameRate, update, 0); // Call update function after 1/frameRate seconds
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    drawScene(0.0 - carCenterX, 0.0 - carCenterY, 0.0 - carCenterZ);
    drawCar(1.0, 0.0, 0.0, 0.0, 0.5, 0.5, 0.5);
    drawEZCube(0.2, 0.2, 0.2, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(fltFOV, 1, 1, 100);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(fltX0 * fltZoom, fltY0 * fltZoom, fltZ0 * fltZoom, fltXRef, fltYRef, fltZRef, fltXUp, fltYUp, fltZUp);
    //glEnable(GL_LIGHTING);
    //glEnable(GL_LIGHT0);
    //glEnable(GL_LIGHT1);

    glutSwapBuffers();
}

void init() {
    glClearColor(0.0, 0.0, 0.15, 1.0);
    //glMatrixMode(GL_PROJECTION);
    //glLoadIdentity();
    //gluPerspective(45.0, 1.0, 1.0, 100.0);
    //glMatrixMode(GL_MODELVIEW);
    //glLoadIdentity();
    //gluLookAt(-3.0, 3.0, 0.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(500, 500);
    glutCreateWindow("CW2");
    glutDisplayFunc(display);
    glutKeyboardFunc(handleKeypress);

    init();
    initLighting(); // Initialize lighting with two point light sources

    glutTimerFunc(0, update, 0); // Start the update function immediately

    glutMainLoop();

    return 0;
}

// 光照
// 坠毁测试
// 背景换位置，开始降落总不能在跑道上
// 仪表盘，旋转长方形就行
// 飞机，可考虑螺旋桨，不用画圆
