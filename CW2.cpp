#define FREEGLUT_STATIC
#include <GL/freeglut.h>
#include <math.h>
#include <stdio.h> 
#include <stdlib.h> 
#include "vector" 
#include <iostream>
#define M_PI 3.14159265359

using namespace std;

GLint imagewidth0, imagewidth1;
GLint imageheight0, imageheight1;
GLint pixellength0, pixellength1;
vector<GLubyte*>p;  // Similar to GLubyte* for program 3 but for 2 images (so a vector) 
GLuint texture[2];

float planeWheelSize = 2.0f;
float planeSizeX = 20.0f; 
float planeSizeY = 2.0f;
float planeSizeZ = planeSizeY;
float planeCenterX = 0.0f;
float planeCenterY = 0.0f;
float planeCenterZ = 0.0f;
float planeXspeed = 30.0f;
float planeYspeed = 0.0f;
float planeZspeed = 0.0f;
bool planeTurnLeft = false;
bool planeTurnRight = false;
bool crashed = false;
float startHeight = 88.0f;

float groundX = 50000.0f;
float groundY = 0.2f;
float groundZ = 50000.0f;

int frameRate = 60; // Desired frame rate (frames per second)

float fltFOV; //Field Of View
float fltZoom; //Zoom amount
float fltX0; //Camera position
float fltY0;
float fltZ0;
float fltXRef; //Look At reference point
float fltYRef;
float fltZRef;
float fltXUp; //Up vector
float fltYUp;
float fltZUp;
float fltViewingAngle;
int fltMode;

void ReadImage(const char path[256], GLint& imagewidth, GLint& imageheight, GLint& pixellength) {
    GLubyte* pixeldata;
    FILE* pfile;
    fopen_s(&pfile, path, "rb");
    if (pfile == 0) exit(0);
    fseek(pfile, 0x0012, SEEK_SET);
    fread(&imagewidth, sizeof(imagewidth), 1, pfile);
    fread(&imageheight, sizeof(imageheight), 1, pfile);
    pixellength = imagewidth * 3;
    while (pixellength % 4 != 0)pixellength++;
    pixellength *= imageheight;
    pixeldata = (GLubyte*)malloc(pixellength);
    if (pixeldata == 0) exit(0);
    fseek(pfile, 54, SEEK_SET);
    fread(pixeldata, pixellength, 1, pfile);
    p.push_back(pixeldata);
    fclose(pfile);
}


void setupMaterial(float red, float green, float blue) {

    // Set material properties for the object
    GLfloat material_ambient[] = { red, green, blue, 1.0 };
    GLfloat material_diffuse[] = { red, green, blue, 1.0 };
    GLfloat material_specular[] = { 0.0, 0.0, 0.0, 1.0 };
    GLfloat material_shininess[] = { 50.0 };

    glMaterialfv(GL_FRONT, GL_AMBIENT, material_ambient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, material_diffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR, material_specular);
    glMaterialfv(GL_FRONT, GL_SHININESS, material_shininess);
}

void initLighting() {
    glEnable(GL_LIGHTING);

    GLfloat light_position0[] = { 100.0, 100.0, -100.0, 1.0 };
    GLfloat diffuse_specular_light0[] = { 0.8, 0.8, 0.8, 1.0 };
    GLfloat ambient_light0[] = { 1.0, 1.0, 1.0, 1.0 };

    glLightfv(GL_LIGHT0, GL_POSITION, light_position0);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse_specular_light0);
    glLightfv(GL_LIGHT0, GL_SPECULAR, diffuse_specular_light0);
    glLightfv(GL_LIGHT0, GL_AMBIENT, ambient_light0);

    glEnable(GL_LIGHT0);


    GLfloat light_position1[] = { 1.0, 1.0, -0.0, 1.0 };
    GLfloat diffuse_specular_light1[] = { 0.0, 0.0, 0.0, 1.0 };

    glLightfv(GL_LIGHT0, GL_POSITION, light_position1);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse_specular_light1);
    glLightfv(GL_LIGHT0, GL_SPECULAR, diffuse_specular_light1);

    glEnable(GL_LIGHT1);
}

void drawCylinder(float down_radius, float top_radius, float height, float centerX, float centerY, float centerZ, float xAngle, float yAngle, float zAngle, float red, float green, float blue, GLuint Texture) {
    setupMaterial(red, green, blue);
    glPushMatrix();
    glTranslatef(centerX, centerY, centerZ);
    glRotatef(xAngle / M_PI * 180, 1.0f, 0.0f, 0.0f);
    glRotatef(yAngle / M_PI * 180, 0.0f, 1.0f, 0.0f);
    glRotatef(zAngle / M_PI * 180, 0.0f, 0.0f, 1.0f);

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, Texture);

    GLUquadricObj* cylinder = gluNewQuadric();
    gluQuadricNormals(cylinder, GLU_SMOOTH);
    gluQuadricOrientation(cylinder, GLU_OUTSIDE);
    gluQuadricTexture(cylinder, GL_TRUE);
    gluQuadricDrawStyle(cylinder, GLU_FILL);
    gluCylinder(cylinder, down_radius, top_radius, height, 20, 20);

    glDisable(GL_TEXTURE_2D);

    glPopMatrix();
}

void drawCube(float X_length, float Y_length, float Z_length, float centerX, float centerY, float centerZ, float xAngle, float yAngle, float zAngle, float red, float green, float blue, GLuint Texture) {
    setupMaterial(red, green, blue);

    float halfWidth = X_length / 2.0;
    float halfHeight = Y_length / 2.0;
    float halfDepth = Z_length / 2.0;

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
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, Texture);
    glBegin(GL_QUADS);

    // Front face
    glTexCoord2f(0.0, 0.0);
    glVertex3fv(vertices[0]);
    glTexCoord2f(1.0, 0.0);
    glVertex3fv(vertices[1]);
    glTexCoord2f(1.0, 1.0);
    glVertex3fv(vertices[2]);
    glTexCoord2f(0.0, 1.0);
    glVertex3fv(vertices[3]);

    // Back face
    glTexCoord2f(0.0, 0.0);
    glVertex3fv(vertices[4]);
    glTexCoord2f(1.0, 0.0);
    glVertex3fv(vertices[5]);
    glTexCoord2f(1.0, 1.0);
    glVertex3fv(vertices[6]);
    glTexCoord2f(0.0, 1.0);
    glVertex3fv(vertices[7]);

    // Top face
    glTexCoord2f(0.0, 0.0);
    glVertex3fv(vertices[3]);
    glTexCoord2f(1.0, 0.0);
    glVertex3fv(vertices[2]);
    glTexCoord2f(1.0, 1.0);
    glVertex3fv(vertices[6]);
    glTexCoord2f(0.0, 1.0);
    glVertex3fv(vertices[5]);

    // Bottom face
    glTexCoord2f(0.0, 0.0);
    glVertex3fv(vertices[0]);
    glTexCoord2f(1.0, 0.0);
    glVertex3fv(vertices[4]);
    glTexCoord2f(1.0, 1.0);
    glVertex3fv(vertices[7]);
    glTexCoord2f(0.0, 1.0);
    glVertex3fv(vertices[1]);

    // Right face
    glTexCoord2f(0.0, 0.0);
    glVertex3fv(vertices[1]);
    glTexCoord2f(1.0, 0.0);
    glVertex3fv(vertices[7]);
    glTexCoord2f(1.0, 1.0);
    glVertex3fv(vertices[6]);
    glTexCoord2f(0.0, 1.0);
    glVertex3fv(vertices[2]);

    // Left face
    glTexCoord2f(0.0, 0.0);
    glVertex3fv(vertices[0]);
    glTexCoord2f(1.0, 0.0);
    glVertex3fv(vertices[3]);
    glTexCoord2f(1.0, 1.0);
    glVertex3fv(vertices[5]);
    glTexCoord2f(0.0, 1.0);
    glVertex3fv(vertices[4]);

    glEnd();
    glDisable(GL_TEXTURE_2D);	
    glBindTexture(GL_TEXTURE_2D, 0);
}

void drawEZCube(float width, float height, float depth, float centerX, float centerY, float centerZ, float red, float green, float blue, GLuint texture) {
    drawCube(width, height, depth, centerX, centerY, centerZ, 0.0, 0.0, 0.0, red, green, blue, texture);
}

void drawPlane(float size, float centerX, float centerY, float centerZ, float red, float green, float blue) {
    setupMaterial(red, green, blue);

    float wheel_size = 1.0 * size;
    float distance = planeCenterX;

    glEnable(GL_LIGHTING);

    // Plane body
    //drawCube(planeSizeX * size, planeSizeY * size, planeSizeZ * size, centerX, centerY, centerZ, 0.0, 0.0, 0.0, red, green, blue, texture[0]);
    drawCylinder(planeSizeY, 0, planeSizeX * 0.6, planeSizeX, 0.0, 0.0, 0.0, M_PI / 2, 0.0, 0.0, 0.0, 0.0, texture[0]);
    drawCylinder(planeSizeY, planeSizeY, planeSizeX, 0.0, 0.0, 0.0, 0.0, M_PI / 2, 0.0, 0.0, 0.0, 0.0, texture[0]);
    drawCube(planeSizeX * 0.1 * size, planeSizeY * 0.2 * size, planeSizeX * 0.66 * size, centerX + planeSizeX * 0.5, centerY, centerZ - planeSizeZ * 1.7, 0.0, M_PI / 4, 0.0, red, green, blue, texture[1]);
    drawCube(planeSizeX * 0.1 * size, planeSizeY * 0.2 * size, planeSizeX * 0.66 * size, centerX + planeSizeX * 0.5, centerY, centerZ + planeSizeZ * 1.7, 0.0, -M_PI / 4, 0.0, red, green, blue, texture[1]);

    drawCube(planeSizeX * 0.1 * size, planeSizeY * 0.2 * size, planeSizeX * 0.4 * size, centerX + planeSizeX * 0.1, centerY, centerZ - planeSizeZ * 1.0, 0.0, M_PI / 4, 0.0, red, green, blue, texture[1]);
    drawCube(planeSizeX * 0.1 * size, planeSizeY * 0.2 * size, planeSizeX * 0.4 * size, centerX + planeSizeX * 0.1, centerY, centerZ + planeSizeZ * 1.0, 0.0, -M_PI / 4, 0.0, red, green, blue, texture[1]);

    //drawCube(planeSizeX * 0.1 * size, planeSizeY * 0.2 * size, planeSizeX * size, centerX + planeSizeX * 0.3, centerY, centerZ, 0.0, M_PI / 4, 0.0, red, green, blue, 0);
    //drawCube(planeSizeX * 0.05 * size, planeSizeY * 0.2 * size, planeSizeX * 0.3 * size, centerX - planeSizeX * 0.45, centerY, centerZ, 0.0, 0.0, 0.0, red, green, blue, 0);
    //drawCube(planeSizeX * 0.05 * size, planeSizeY * 1.0 * size, planeSizeZ * 0.3 * size, centerX - planeSizeX * 0.45, centerY + planeSizeY * 0.5, centerZ, 0.0, 0.0, 0.0, red, green, blue, 0);


    // Draw wheels in cube
    float rotateZ = 1.0;
    float rotateYangle = 0.0;
    bool onGroundEZ = (planeCenterY <= -startHeight + groundY / 2.0f + planeWheelSize / 1.0f + planeSizeY * 1.5f);
    if (planeTurnLeft == true && planeTurnRight == false && onGroundEZ) {
        rotateYangle = M_PI / 4;
        rotateZ = 0.0;
    };
    if (planeTurnLeft == false && planeTurnRight == true && onGroundEZ) {
        rotateYangle = -M_PI / 4;
        rotateZ = 0.0;
    };
    if (planeTurnLeft == false && planeTurnRight == false && onGroundEZ) {
        rotateYangle = 0.0;
        rotateZ = 1.0;
    };
    if (!onGroundEZ) {
        rotateYangle = 0.0;
        rotateZ = 0.0;
    }

    drawCube(planeWheelSize * size, planeWheelSize * size, 0.5 * size, +planeSizeX * 0.55 * size + centerX, -planeSizeY * 0.5 * size - planeWheelSize + centerY, -planeSizeX * 0.25 * size + centerZ, 0.0, rotateYangle, (-distance / planeWheelSize) * rotateZ, 0.5, 0.5, 0.5, 0);
    drawCube(planeWheelSize * size, planeWheelSize * size, 0.5 * size, +planeSizeX * 0.55 * size + centerX, -planeSizeY * 0.5 * size - planeWheelSize + centerY, planeSizeX * 0.25 * size + centerZ, 0.0, rotateYangle, (-distance / planeWheelSize) * rotateZ, 0.5, 0.5, 0.5, 0);
    drawCube(planeWheelSize * size, planeWheelSize * size, 0.5 * size, planeSizeX * 0.99 * size + centerX, -(planeSizeY) * 0.5 * size - planeWheelSize + centerY, 0.0, 0.0, 0.0, (-distance / planeWheelSize) * rotateZ, 0.5, 0.5, 0.5, 0);

    glEnd();
}

void drawGround(float centerX, float centerY, float centerZ) {
    float r = 0.10;
    float g = 0.10;
    float b = 0.10;
    drawCube(groundX, groundY, groundZ, centerX, centerY, centerZ, 0.0, 0.0, 0.0, r, g, b, -1);
}

void drawDashedLine(float centerX, float centerY, float centerZ) {
    float LineX = 6.0;
    float LineY = 1.1;
    float LineZ = 0.5;
    float r = 0.9;
    float g = 0.9;
    float b = 0.9;
    for (float X = 0.0f; X < groundX / 2.0f; X += 15.0f) {
        drawCube(LineX, LineY, LineZ, X + centerX, centerY, centerZ, 0.0, 0.0, 0.0, r, g, b, 0);
    }
}

void drawScene(float centerX, float centerY, float centerZ) {
    float groundX = centerX;
    float groundY = centerY - startHeight;
    float groundZ = centerZ;

    drawGround(groundX, groundY, groundZ);
    drawDashedLine(groundX, groundY, groundZ - planeSizeZ * 3);
    drawDashedLine(groundX, groundY, groundZ + planeSizeZ * 3);


    drawEZCube(2.0, 2.0, 2.0, groundX, groundY, groundZ, 0.0, 0.0, 1.0, 0); // For background Test
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
        planeXspeed += 0.2f;
        planeXspeed = min(planeXspeed, 2.0f);
        break;
    case 'd':
        planeXspeed -= 0.2f;
        planeXspeed = max(planeXspeed, -2.0f);
        break;
    case 's':
        planeZspeed -= 0.02f;
        planeZspeed = min(planeZspeed, 2.0f);
        break;
    case 'f':
        planeZspeed += 0.02f;
        planeZspeed = max(planeZspeed, -2.0f);
        break;
    case 'a':
        if (planeXspeed >= 0) {
            planeYspeed += 0.01f;
        }
        break;
    case 'z':
        planeYspeed -= 0.1f;
        break;
    case ' ':
        handlefltMode();
        break;
    }

    glutPostRedisplay();
}

void update(int value) {
    // Update the cube's position based on speed
    if (planeXspeed > 0 && planeCenterY > -startHeight + planeSizeY * 2.0f) {
        planeXspeed -= 0.005f * abs(planeXspeed);
    }

    if (planeXspeed > 0 && planeCenterY <= -startHeight + planeSizeY * 2.0f) {
        planeXspeed -= 0.01f * abs(planeXspeed);
    }

    if (planeXspeed < 0 && planeCenterY > -startHeight + planeSizeY * 2.0f) {
        planeXspeed += 0.005f * abs(planeXspeed);
    }

    if (planeXspeed < 0 && planeCenterY <= -startHeight + planeSizeY * 2.0f) {
        planeXspeed += 0.01f * abs(planeXspeed);
    }

    if (planeZspeed > 0.005f) {
        planeZspeed -= 0.05f * abs(planeZspeed);
        planeTurnRight = true;
        planeTurnLeft = false;
    }

    if (planeZspeed < 0.005f) {
        planeZspeed += 0.05f * abs(planeZspeed);
        planeTurnRight = false;
        planeTurnLeft = true;
    }

    if (planeCenterY > -startHeight + groundY / 2.0f + planeWheelSize / 1.0f + planeSizeY / 1.0f) {
        planeYspeed -= 0.001f;
    }

    if (planeCenterY < -startHeight + groundY / 2.0f + planeWheelSize / 1.0f + planeSizeY / 1.0f) {
        planeCenterY = -startHeight + groundY / 2.0f + planeWheelSize / 1.0f + planeSizeY / 1.0f;
        if (abs(planeYspeed) >= 0.5f) {
            crashed = true;
        }
        planeYspeed = -0.1 * planeYspeed;
    }

    if (planeXspeed <= 2.0f && planeXspeed > 0.0 && planeCenterY > -startHeight + groundY / 2.0f + planeWheelSize / 1.0f + planeSizeY * 2.0f) {
        std::cout << "Low Speed!!!" << endl;
    }

    if (planeXspeed <= 0.7f && planeCenterY > -startHeight + groundY / 2.0f + planeWheelSize / 1.0f + planeSizeY * 2.0f) {
        crashed = true;
    }

    if (abs(planeYspeed) >= 0.4f) {
        crashed = true;
    }

    if (abs(planeXspeed) < 0.05f) { 
        planeXspeed = 0; 
    };

    if (abs(planeZspeed) < 0.005f) {
        planeZspeed = 0;
        planeTurnRight = false;
        planeTurnLeft = false;
    };

    if (abs(planeYspeed) < 0.1 * 0.001f) { 
        planeYspeed = 0; 
    };

    planeCenterX += planeXspeed;
    planeCenterZ += planeZspeed;
    planeCenterY += planeYspeed;

    if (crashed == true) {
        std::cout << "Crashed!!!!!!!!!IDIOT" << endl;
    }
    //std::cout << "__________" << endl;
    //std::cout << planeXspeed << endl;
    //std::cout << planeZspeed << endl;
    //std::cout << planeYspeed << endl;

    switch (fltMode) {
    case 0:
        fltFOV = 70; //Field Of View
        fltZoom = 1.0; //Zoom amount
        fltX0 = -20.0; //Camera position
        fltY0 = 20.0;
        fltZ0 = -20.0;
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

    drawScene(0.0 - planeCenterX, 0.0 - planeCenterY, 0.0 - planeCenterZ);
    drawPlane(1.0, 0.0, 0.0, 0.0, 0.5, 0.5, 0.5);
    drawEZCube(2.0, 2.0, 2.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(fltFOV, 1, 1, 100);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(fltX0 * fltZoom, fltY0 * fltZoom, fltZ0 * fltZoom, fltXRef, fltYRef, fltZRef, fltXUp, fltYUp, fltZUp);

    glutSwapBuffers();
}

void init() {
    glClearColor(0.0, 0.0, 0.15, 1.0);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glShadeModel(GL_FLAT);
    glEnable(GL_TEXTURE_2D);
    ReadImage("plane1.bmp", imagewidth0, imageheight0, pixellength0);
    ReadImage("plane.bmp", imagewidth1, imageheight1, pixellength1);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);  // set pixel storage modes (in the memory) 
    glGenTextures(2, &texture[0]);  // number of texture names to be generated and an array of texture names
    glBindTexture(GL_TEXTURE_2D, texture[0]);  // target to which texture is bound and name of a texture 
    glTexImage2D(GL_TEXTURE_2D, 0, 3, imagewidth0, imageheight0, 0, GL_BGR_EXT, GL_UNSIGNED_BYTE, p[0]);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glBindTexture(GL_TEXTURE_2D, texture[1]);
    glTexImage2D(GL_TEXTURE_2D, 0, 3, imagewidth1, imageheight1, 0, GL_BGR_EXT, GL_UNSIGNED_BYTE, p[1]);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(800, 600);
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
// X轴上可以考虑用engine实现，不用一直按着
