#define FREEGLUT_STATIC
#include <GL/freeglut.h>
#include <math.h>
#define M_PI 3.14159265359

#include <iostream>
using namespace std;

float carCenterX = 0.0f;
float carCenterZ = 0.0f;
float carXspeed = 0.0f;
float carZspeed = 0.0f;
bool carTurnLeft = false;
bool carTurnRight = false;
int frameRate = 60; // Desired frame rate (frames per second)

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
    //setupLightingAndMaterial(red, green, blue);

    float wheel_size = 1.0 * size;
    float distance = centerX;

    glEnable(GL_LIGHTING);
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

void initLighting() {
    // First point light source
    GLfloat light_position0[] = { 1.0, 1.0, 1.0, 1.0 };
    GLfloat light_diffuse0[] = { 1.0, 1.0, 1.0, 1.0 }; // White light
    glEnable(GL_LIGHT0);
    glLightfv(GL_LIGHT0, GL_POSITION, light_position0);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse0);

    // Second point light source
    GLfloat light_position1[] = { -2.0, 2.0, 0.0, 1.0 };
    GLfloat light_diffuse1[] = { 0.0, 1.0, 0.0, 1.0 }; // Green light
    glEnable(GL_LIGHT1);
    glLightfv(GL_LIGHT1, GL_POSITION, light_position1);
    glLightfv(GL_LIGHT1, GL_DIFFUSE, light_diffuse1);

    glEnable(GL_LIGHTING);
}

void handleKeypress(unsigned char key, int x, int y) {
    switch (key) {
    case 'w':
        carXspeed += 0.02f;
        carXspeed = min(carXspeed, 0.2f);
        break;
    case 's':
        carXspeed -= 0.02f;
        carXspeed = max(carXspeed, -0.2f);
        break;
    case 'a':
        carZspeed -= 0.02f;
        carZspeed = min(carZspeed, 0.2f);
        break;
    case 'd':
        carZspeed += 0.02f;
        carZspeed = max(carZspeed, -0.2f);
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

    if (abs(carXspeed) < 0.005f) { carXspeed = 0; };
    if (abs(carZspeed) < 0.005f) { 
        carZspeed = 0; 
        carTurnRight = false;
        carTurnLeft = false;
    };

    carCenterX += carXspeed;
    carCenterZ += carZspeed;


    cout << "__________" << endl;
    cout << carXspeed << endl;
    cout << carZspeed << endl;

    glutPostRedisplay();
    glutTimerFunc(1000 / frameRate, update, 0); // Call update function after 1/frameRate seconds
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    drawCar(1.0, carCenterX, 0.0, carCenterZ, 0.5, 0.5, 0.5);
    drawEZCube(0.2, 0.2, 0.2, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0);

    glutSwapBuffers();
}

void init() {
    glClearColor(0.0, 0.0, 0.15, 1.0);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0, 1.0, 1.0, 100.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(-3.0, 3.0, -3.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);
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
