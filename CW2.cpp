#define FREEGLUT_STATIC
#include <GL/freeglut.h>
#include <math.h>
#include <stdio.h> 
#include <stdlib.h> 
#include <vector>
#include <iostream>
#define M_PI 3.14159265359

using namespace std;

GLint imagewidth0, imagewidth1, imagewidth2;
GLint imageheight0, imageheight1, imageheight2;
GLint pixellength0, pixellength1, pixellength2;
vector<GLubyte*>p;
GLuint texture[3];

float planeWheelSize = 2.0;
float planeSizeX = 20.0f; // planeSize
float planeSizeY = 2.0f;
float planeSizeZ = planeSizeY;
float planeCenterX = 0.0f; // plane center point
float planeCenterY = 0.0f;
float planeCenterZ = 0.0f;
float planeXspeed = 20.0f; // plane speed
float planeYspeed = 0.0f;
float planeZspeed = 0.0f;
float lowXSpeed = 4.0f; // The plane is about to stall.
float crashXSpeed = 0.7f; // Aircraft stall speed
float startHeight = 88.0f;
bool planeTurnLeft = false;
bool planeTurnRight = false;
bool crashed = false;
bool crashReported = false;
bool flyPastReported = false;
bool succeedReported = false;

float groundSizeX = 20000.0f;
float groundSizeY = 0.5f;
float zMaxLimit = 150.0f;
float groundSizeZ = planeSizeZ * zMaxLimit;
float AirstripSpacingFactor = 10.0f;
float AirstripStartX = 1660.0f;

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

int GameMode = 0;
int frameCounter = 0;
int RestartTimes = 0;
int crashedTimes = 0;

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

void drawDashboard_X(float size, float centerX, float centerY, float centerZ, float NumMin, float NumMax, float dangerNum, float crashNum, float* par) {
    float var = *par;
    bool dangerLine = false;
    bool crashLine = false;
    float angle_danger, angle_crash;
    float LineXY = 0.03f;


    if (NumMin >= NumMax) {
        std::cout << "DEBUGGER: Check dashboard!" << endl;
    }

    float angle = M_PI * max(min(var, NumMax * 0.99f), NumMin) / (NumMax - NumMin);
    if (dangerNum > NumMin && dangerNum < NumMax) {
        angle_danger = M_PI * dangerNum / (NumMax - NumMin);
        dangerLine = true;
    }
    if (crashNum > NumMin && crashNum < NumMax) {
        angle_crash = M_PI * crashNum / (NumMax - NumMin);
        crashLine = true;
    }

    drawCylinder(1.0f * size, 1.0f * size, 0.2f * size, centerX, centerY, centerZ, 0.0, M_PI / 2, 0.0, 0.5, 0.5, 0.5, 0);
    drawCube(0.2f * size, 1.0f * size, 2.0f * size, centerX, centerY - 0.5f * size, centerZ, 0.0, 0.0, 0.0, 0.3, 0.3, 0.3, 0);
    drawCube(LineXY * size, LineXY * size, 2.0f * size, centerX, centerY, centerZ, angle, 0.0, 0.0, 0.4, 0.8, 0.0, 0);

    if (dangerLine) {
        drawCube(LineXY * size, LineXY * size, 2.0f * size, centerX, centerY, centerZ, angle_danger, 0.0, 0.0, 0.8, 0.8, 0.0, 0);
    }
    if (crashLine) {
        drawCube(LineXY * size, LineXY * size, 2.0f * size, centerX, centerY, centerZ, angle_crash, 0.0, 0.0, 0.8, 0.0, 0.0, 0);
    }

}

void drawPlane(float size, float centerX, float centerY, float centerZ, float red, float green, float blue, bool is_static) {
    setupMaterial(red, green, blue);

    float wheel_size = 1.0 * size;
    float distance = planeCenterX;

    glEnable(GL_LIGHTING);

    // Plane body
    drawCylinder(planeSizeY, 0, planeSizeX * 0.6, centerX + planeSizeX * 0.77, centerY, centerZ, 0.0, M_PI / 2, 0.0, 0.0, 0.0, 0.0, texture[0]);
    drawCylinder(planeSizeY, planeSizeY, planeSizeX, centerX, centerY, centerZ, 0.0, M_PI / 2, 0.0, 0.0, 0.0, 0.0, texture[0]);
    drawCube(planeSizeX * 0.1 * size, planeSizeY * 0.2 * size, planeSizeX * 0.66 * size, centerX + planeSizeX * 0.5, centerY, centerZ - planeSizeZ * 1.7, 0.0, M_PI / 4, 0.0, red, green, blue, texture[0]);
    drawCube(planeSizeX * 0.1 * size, planeSizeY * 0.2 * size, planeSizeX * 0.66 * size, centerX + planeSizeX * 0.5, centerY, centerZ + planeSizeZ * 1.7, 0.0, -M_PI / 4, 0.0, red, green, blue, texture[0]);

    drawCube(planeSizeX * 0.1 * size, planeSizeY * 0.2 * size, planeSizeX * 0.4 * size, centerX + planeSizeX * 0.1, centerY, centerZ - planeSizeZ * 1.0, 0.0, M_PI / 4, 0.0, red, green, blue, texture[0]);
    drawCube(planeSizeX * 0.1 * size, planeSizeY * 0.2 * size, planeSizeX * 0.4 * size, centerX + planeSizeX * 0.1, centerY, centerZ + planeSizeZ * 1.0, 0.0, -M_PI / 4, 0.0, red, green, blue, texture[0]);
    drawCube(planeSizeX * 0.1 * size, planeSizeY * 0.2 * size, planeSizeX * 0.2 * size, centerX + planeSizeX * 0.1, centerY + planeSizeX * 0.15, centerZ, M_PI / 2, 0.0, 0.0, red, green, blue, texture[0]);

    // Draw wheels in cube
    float rotateZ = 0.0;
    float rotateZ_first = 0.0;
    float rotateYangle = 0.0;
    bool onGroundEZ = (planeCenterY <= -startHeight + groundSizeY / 2.0f + planeWheelSize / 1.0f + planeSizeY * 1.5f);
    if (planeTurnLeft == true && planeTurnRight == false && onGroundEZ && !is_static) {
        rotateYangle = M_PI / 6;
        rotateZ = 0.0;
        rotateZ_first = 1.0;
    };
    if (planeTurnLeft == false && planeTurnRight == true && onGroundEZ && !is_static) {
        rotateYangle = -M_PI / 6;
        rotateZ = 0.0;
        rotateZ_first = 1.0;
    };
    if (planeTurnLeft == false && planeTurnRight == false && onGroundEZ && !is_static) {
        rotateYangle = 0.0;
        rotateZ = 1.0;
        rotateZ_first = 1.0;
    };
    if (!onGroundEZ) {
        rotateYangle = 0.0;
        rotateZ = 0.0;
        rotateZ_first = 0.0;
    }

    drawCube(planeWheelSize * size, planeWheelSize * size, 0.5 * size, +planeSizeX * 0.55 * size + centerX, -planeSizeY * 0.4 * size - planeWheelSize + centerY, -planeSizeX * 0.25 * size + centerZ, 0.0, rotateYangle, (-distance / planeWheelSize) * rotateZ, 0.5, 0.5, 0.5, 0);
    drawCube(planeWheelSize * size, planeWheelSize * size, 0.5 * size, +planeSizeX * 0.55 * size + centerX, -planeSizeY * 0.4 * size - planeWheelSize + centerY, planeSizeX * 0.25 * size + centerZ, 0.0, rotateYangle, (-distance / planeWheelSize) * rotateZ, 0.5, 0.5, 0.5, 0);

    drawCube(planeWheelSize * size, planeWheelSize * size, 0.5 * size, +planeSizeX * 0.99 * size + centerX, -planeSizeY * 0.4 * size - planeWheelSize + centerY, centerZ, 0.0, 0.0, (-distance / planeWheelSize) * rotateZ_first, 0.5, 0.5, 0.5, 0);


    // Fire
    if (planeXspeed > 0.114514f && !is_static) {
        float fireSize = float(planeSizeY);
        float disappearDistance = planeSizeX * planeXspeed / 30.0f;
        for (float i = 1.0f; i >= 0; i -= 0.05f) {
            drawEZCube(fireSize * i, fireSize * i, fireSize * i, disappearDistance * (i - 1.0f), 0.0, 0.0, 1.0, 0.7 * i, 0.0, 0);
        }
    }

    drawDashboard_X(1.0f, planeSizeX * 1.0f, planeSizeY * 1.0f, 0.0f, 0.0f, 20.0f, lowXSpeed, crashXSpeed, &planeXspeed);

    if (crashed && !succeedReported && !is_static) {
        drawCube(0.5f, planeSizeX, 0.5f, planeSizeX / 2.0f + centerX, centerY, centerZ, M_PI / 4.0f, 0.0, 0.0, 1.0, 0.0, 0.0, 0);
        drawCube(0.5f, planeSizeX, 0.5f, planeSizeX / 2.0f + centerX, centerY, centerZ, -M_PI / 4.0f, 0.0, 0.0, 1.0, 0.0, 0.0, 0);
    }

    glEnd();
}

void drawGround(float centerX, float centerY, float centerZ, float groundR, float groundG, float groundB) {
    drawCube(groundSizeX, groundSizeY, groundSizeZ, centerX, centerY, centerZ, 0.0, 0.0, 0.0, groundR, groundG, groundB, -1);
}

void drawBeforeGreenGround(float centerX, float centerY, float centerZ) {
    float r = 0.0;
    float g = 0.5;
    float b = 0.0;
    drawCube(AirstripStartX, groundSizeY * 1.1f, groundSizeZ, centerX + AirstripStartX / 2.0f, centerY, centerZ, 0.0, 0.0, 0.0, r, g, b, -1);
}

void drawLine(float centerX, float centerY, float centerZ, float DashedLineX, float DashedLineY, float DashedLineZ, bool isDashedLine) {
    float r = 0.9;
    float g = 0.9;
    float b = 0.9;
    if (isDashedLine) {
        for (float X = AirstripStartX; X < groundSizeX / 2.0f; X += 15.0f) {
            drawCube(DashedLineX, DashedLineY, DashedLineZ, X + centerX, centerY, centerZ, 0.0, 0.0, 0.0, r, g, b, 0);
        }
    }
    else {
        for (float X = AirstripStartX; X < groundSizeX / 2.0f; X += DashedLineX * 1.0f) {
            drawCube(DashedLineX, DashedLineY, DashedLineZ, X + centerX, centerY, centerZ, 0.0, 0.0, 0.0, r, g, b, 0);
        }
    }

}

void drawGrassland(float centerX, float centerY, float centerZ , float GrassLandX, float GrassLandY, float GrassLandZ, float GrassIntervalX, float GrassIntervalZ, float numberOfGrassLandZ) {
    float r = 0.0;
    float g = 0.5;
    float b = 0.0;
    for (float X = AirstripStartX; X < groundSizeX / 2.0f; X += GrassIntervalX) {
        for (float Z = -GrassIntervalZ * (numberOfGrassLandZ + 0.5); Z < GrassIntervalZ * (numberOfGrassLandZ + 0.5); Z += GrassIntervalZ) {
            drawCube(GrassLandX, GrassLandY, GrassLandZ, X + centerX, centerY, Z + centerZ, 0.0, 0.0, 0.0, r, g, b, texture[2]);
        }
    }
}

void drawClouds(float centerX, float centerY, float centerZ) {
    float cloudSizeX = 50.0f;
    float cloudSizeY = 2.0f;
    float cloudSizeZ = 50.0f;
    float interval = 2 * cloudSizeX;
    float r = 0.9;
    float g = 0.9;
    float b = 0.9;

    for (float X = 0.0f; X < groundSizeX / 2.0f; X += interval) {
        for (float Z = -planeSizeZ * zMaxLimit; Z < planeSizeZ * zMaxLimit; Z += interval) {
            // Like a stack
            drawCube(cloudSizeX * 1.00f, cloudSizeY, cloudSizeZ * 1.00f, X + centerX, centerY + cloudSizeY * 0.0f, Z + centerZ, 0.0, 0.0, 0.0, r * 0.85f, g * 0.85f, b * 0.85f, 0);
            drawCube(cloudSizeX * 0.87f, cloudSizeY, cloudSizeZ * 0.87f, X + centerX, centerY + cloudSizeY * 1.0f, Z + centerZ, 0.0, 0.0, 0.0, r * 0.9f, g * 0.9f, b * 0.9f, 0);
            drawCube(cloudSizeX * 0.71f, cloudSizeY, cloudSizeZ * 0.71f, X + centerX, centerY + cloudSizeY * 2.0f, Z + centerZ, 0.0, 0.0, 0.0, r * 0.95f, g * 0.95f, b * 0.95f, 0);
            drawCube(cloudSizeX * 0.50f, cloudSizeY, cloudSizeZ * 0.50f, X + centerX, centerY + cloudSizeY * 3.0f, Z + centerZ, 0.0, 0.0, 0.0, r, g, b, 0);
        }
    }
}

void drawScene(float centerX, float centerY, float centerZ) {
    float groundX = centerX;
    float groundY = centerY - startHeight;
    float groundZ = centerZ;

    float DashedLineX = 6.0f;
    float DashedLineY = 0.55f;
    float DashedLineZ = 0.5f;

    float GrassLandX = 100.0f;
    float GrassLandY = 0.55f;
    float GrassLandZ = 50.0f;
    float GrassIntervalX = GrassLandX + planeSizeZ * AirstripSpacingFactor;
    float GrassIntervalZ = GrassLandZ + DashedLineZ + 2 * planeSizeZ * AirstripSpacingFactor;
    float numberOfGrassLandZ = 2.0f;

    drawGround(groundX, groundY, groundZ, 0.1, 0.1, 0.1);
    drawCube(groundSizeX * 100.0f, groundSizeY * 0.5f, groundSizeZ * 10.0f, groundX, groundY - groundSizeY * 10.0f, groundZ, 0.0, 0.0, 0.0, 0.0, 0.5, 0.0, -1);
    drawBeforeGreenGround(groundX, groundY, groundZ);
    drawGrassland(groundX, groundY, groundZ, GrassLandX, GrassLandY, GrassLandZ, GrassIntervalX, GrassIntervalZ, numberOfGrassLandZ);

    // Central Airstrip
    drawLine(groundX, groundY, groundZ, DashedLineX, DashedLineY, DashedLineZ, true);
    drawLine(groundX, groundY, groundZ - planeSizeZ * AirstripSpacingFactor, DashedLineX, DashedLineY, DashedLineZ, false);
    drawLine(groundX, groundY, groundZ + planeSizeZ * AirstripSpacingFactor, DashedLineX, DashedLineY, DashedLineZ, false);

    // -1 Airstrip
    drawLine(groundX, groundY, groundZ - (planeSizeZ * AirstripSpacingFactor * 2.0f + GrassLandZ + DashedLineZ) * 1.0f, DashedLineX, DashedLineY, DashedLineZ, true);
    drawLine(groundX, groundY, groundZ - (planeSizeZ * AirstripSpacingFactor * 2.0f + GrassLandZ + DashedLineZ) * 1.0f - planeSizeZ * AirstripSpacingFactor, DashedLineX, DashedLineY, DashedLineZ, false);
    drawLine(groundX, groundY, groundZ - (planeSizeZ * AirstripSpacingFactor * 2.0f + GrassLandZ + DashedLineZ) * 1.0f + planeSizeZ * AirstripSpacingFactor, DashedLineX, DashedLineY, DashedLineZ, false);

    // +1 Airstrip
    drawLine(groundX, groundY, groundZ + (planeSizeZ * AirstripSpacingFactor * 2.0f + GrassLandZ + DashedLineZ) * 1.0f, DashedLineX, DashedLineY, DashedLineZ, true);
    drawLine(groundX, groundY, groundZ + (planeSizeZ * AirstripSpacingFactor * 2.0f + GrassLandZ + DashedLineZ) * 1.0f - planeSizeZ * AirstripSpacingFactor, DashedLineX, DashedLineY, DashedLineZ, false);
    drawLine(groundX, groundY, groundZ + (planeSizeZ * AirstripSpacingFactor * 2.0f + GrassLandZ + DashedLineZ) * 1.0f + planeSizeZ * AirstripSpacingFactor, DashedLineX, DashedLineY, DashedLineZ, false);

    drawClouds(groundX, groundY + startHeight * 0.6f, groundZ);

    // -1 Airstrip static plane
    for (float X = groundX + AirstripStartX + 2000.0f; X <= groundX + AirstripStartX + 3000.0f; X += 100.0f) {
        drawPlane(1.0, X, groundY + groundSizeY / 2.0f + planeWheelSize / 1.0f + planeSizeY / 1.0f, groundZ - (planeSizeZ * AirstripSpacingFactor * 2.0f + GrassLandZ + DashedLineZ) * 1.0f, 0.5, 0.5, 0.5, true);
    }

    // +1 Airstrip staticplane
    for (float X = groundX + AirstripStartX + 100.0f; X <= groundX + AirstripStartX + 1000.0f; X += 100.0f) {
        drawPlane(1.0, X, groundY + groundSizeY / 2.0f + planeWheelSize / 1.0f + planeSizeY / 1.0f, groundZ + (planeSizeZ * AirstripSpacingFactor * 2.0f + GrassLandZ + DashedLineZ) * 1.0f, 0.5, 0.5, 0.5, true);
    }
}

void handleGameMode() {
    switch (GameMode) {
    case 0:
        GameMode = 1;
        break;
    case 1:
        GameMode = 2;
        break;
    case 2:
        GameMode = 3;
        break;
    case 3:
        GameMode = 1;
        break;
    }
}

void Restart() {
    GameMode = 0;
    frameCounter = 0;

    planeCenterX = 0.0f;
    planeCenterY = 0.0f;
    planeCenterZ = 0.0f;
    planeXspeed = 20.0f;
    planeYspeed = 0.0f;
    planeZspeed = 0.0f;
    planeTurnLeft = false;
    planeTurnRight = false;
    crashed = false;
    crashReported = false;
    flyPastReported = false;
    succeedReported = false;
    startHeight = 88.0f;

    RestartTimes += 1;
    std::cout << "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~" << endl;
    std::cout << "You've already RESTART " << RestartTimes << " times." << std::endl;
    std::cout << "Now you can press the SPACE key to start the game." << endl;
    std::cout << "___________________________________________" << endl;
}

void handleKeypress(unsigned char key, int x, int y) {
    switch (key) {
    case 'E':
    case 'e':
        if (planeCenterY > -startHeight + groundSizeY / 2.0f + planeWheelSize / 1.0f + planeSizeY * 2.0f) {
            planeXspeed += 0.3f;
            planeXspeed = min(planeXspeed, 30.0f);
        }
        else {
            planeXspeed += 0.15f;
            planeXspeed = min(planeXspeed, 10.0f);
        }
        break;
    case 'D':
    case 'd':
        planeXspeed -= 0.15f;
        planeXspeed = max(planeXspeed, -10.0f);
        break;
    case 'S':
    case 's':
        planeZspeed -= 0.02f;
        planeZspeed = min(planeZspeed, 2.0f);
        break;
    case 'F':
    case 'f':
        planeZspeed += 0.02f;
        planeZspeed = max(planeZspeed, -2.0f);
        break;
    case 'A':
    case 'a':
        if (planeXspeed >= 0) {
            planeYspeed += 0.01f;
        }
        break;
    case 'Z':
    case 'z':
        planeYspeed -= 0.1f;
        break;
    case ' ':
        handleGameMode();
        break;
    case 'Q':
    case 'q':
        glutFullScreenToggle();
        break;
    case 'R':
    case 'r':
        Restart();
        break;
    default:
        break;
    }

    glutPostRedisplay();
}

void updatePlane() {
    // Update the cube's position based on speed
    if (planeXspeed > 0 && planeCenterY > -startHeight + planeSizeY * 2.0f) {
        planeXspeed -= 0.01f * abs(planeXspeed);
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

    if (planeCenterY > -startHeight + groundSizeY / 2.0f + planeWheelSize / 1.0f + planeSizeY / 1.0f) {
        planeYspeed -= 0.001f;
    }

    if (planeCenterY < -startHeight + groundSizeY / 2.0f + planeWheelSize / 1.0f + planeSizeY / 1.0f) {
        planeCenterY = -startHeight + groundSizeY / 2.0f + planeWheelSize / 1.0f + planeSizeY / 1.0f;
        if (abs(planeYspeed) >= 0.4f && !succeedReported) {
            crashed = true;
        }
        planeYspeed = -0.1 * planeYspeed;
    }

    if (planeXspeed <= lowXSpeed &&
        planeXspeed > crashXSpeed &&
        planeCenterY > -startHeight + groundSizeY / 2.0f + planeWheelSize / 1.0f + planeSizeY * 2.0f &&
        frameCounter % (frameRate / 6) == 0 && 
        !succeedReported) {
        if (crashed == false && !crashReported) {
            std::cout << "Low Speed! (Almost Crash!)" << endl;
        }
        else {
            std::cout << "Low Speed! (You can continue playing but you've already CRASHED once)" << endl;
        }

    }

    if (planeXspeed <= crashXSpeed && 
        planeCenterY > -startHeight + groundSizeY / 2.0f + planeWheelSize / 1.0f + planeSizeY * 2.0f && 
        !succeedReported) {
        crashed = true;
    }

    if (abs(planeYspeed) >= 0.7f && !succeedReported) {
        crashed = true;
    }

    if ((abs(planeCenterX) >= groundSizeX / 2 || abs(planeCenterZ) >= groundSizeZ) && !succeedReported) {
        if (!flyPastReported) {
            std::cout << "You flew past the airport! Fail!" << std::endl;
        }
        crashed = true;
        flyPastReported = true;
    }

    if (abs(planeCenterZ) >= planeSizeZ * AirstripSpacingFactor && 
        planeCenterY < -startHeight + groundSizeY / 2.0f + planeWheelSize / 1.0f + planeSizeY * 2.0f && 
        !succeedReported) {
        if (!flyPastReported) {
            std::cout << "You flew off the airstrip! Fail!" << std::endl;
        }
        crashed = true;
        flyPastReported = true;
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

    if (crashed == true && !crashReported) {
        crashedTimes += 1;
        std::cout << "Crash!!! You've already crashed " << crashedTimes << " times." << std::endl;
        std::cout << "Now you may press R to RESTART." << endl;
        crashReported = true;
    }

    if (planeCenterY == -startHeight + groundSizeY / 2.0f + planeWheelSize / 1.0f + planeSizeY / 1.0f &&
        !crashed &&
        !succeedReported) {
        std::cout << "///////////////////////////////////////////" << endl;
        std::cout << "You succeed! Congratulations!" << endl;
        std::cout << "You cleared it in only " << RestartTimes + 1 << " try!" << endl;
        std::cout << "You may fly freely to explore." << endl;
        std::cout << "Please give the game a thumbs up!" << endl;
        std::cout << "b（￣▽￣）d" << endl;
        
        succeedReported = true;
    }
}

void update(int value) {

    if (GameMode != 0) {
        updatePlane();
    }

    switch (GameMode) {
    case 0:
        fltFOV = 100.0; //Field Of View
        fltZoom = 1.0; //Zoom amount
        fltX0 = -20.0; //Camera position
        fltY0 = 20.0;
        fltZ0 = -0.0;
        fltXRef = 0.0; //Look At reference point
        fltYRef = 0.0;
        fltZRef = 0.0;
        fltXUp = -0.5; //Up vector
        fltYUp = 1.0;
        fltZUp = -0.0;
        fltViewingAngle = 1;
        break;
    case 1:
        fltFOV = 100.0; //Field Of View
        fltZoom = 1.0; //Zoom amount
        fltX0 = planeSizeX * 0.8; //Camera position
        fltY0 = planeSizeY * 1.5;
        fltZ0 = 0.0;
        fltXRef = groundSizeX; //Look At reference point
        fltYRef = 0.5;
        fltZRef = 0.0;
        fltXUp = -0.5; //Up vector
        fltYUp = 1.0;
        fltZUp = -0.0;
        fltViewingAngle = 1;
        break;
    case 2:
        fltFOV = 100.0; //Field Of View
        fltZoom = 1.0; //Zoom amount
        fltX0 = -20.0; //Camera position
        fltY0 = 20.0;
        fltZ0 = -0.0;
        fltXRef = 0.0; //Look At reference point
        fltYRef = 0.0;
        fltZRef = 0.0;
        fltXUp = -0.5; //Up vector
        fltYUp = 1.0;
        fltZUp = -0.0;
        fltViewingAngle = 1;
        break;
    case 3:
        fltFOV = 100.0; //Field Of View
        fltZoom = 1.0; //Zoom amount
        fltX0 = -25.0; //Camera position
        fltY0 = 20.0;
        fltZ0 = -15.0;
        fltXRef = 0.0; //Look At reference point
        fltYRef = 0.0;
        fltZRef = 0.0;
        fltXUp = 0.0; //Up vector
        fltYUp = 1.0;
        fltZUp = 0.0;
        fltViewingAngle = 1;
        break;
    }

    frameCounter += 1;
    if (frameCounter > 8192) {
        frameCounter = 0;
    }
    glutPostRedisplay();
    glutTimerFunc(1000 / frameRate, update, 0); // Call update function after 1/frameRate seconds
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    drawScene(0.0 - planeCenterX, 0.0 - planeCenterY, 0.0 - planeCenterZ);
    drawPlane(1.0, 0.0, 0.0, 0.0, 0.5, 0.5, 0.5, false);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(fltFOV, 1, 1, 100);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(fltX0 * fltZoom, fltY0 * fltZoom, fltZ0 * fltZoom, fltXRef, fltYRef, fltZRef, fltXUp, fltYUp, fltZUp);

    glutSwapBuffers();
}

void consoleInit() {
    std::cout << "Please Read the following instruction:" << endl;
    std::cout << "Your mission is to land a plane smoothly on the airport airstrip." << endl;
    std::cout << "If not handled properly, the plane will crash." << endl;
    std::cout << "(For example, forward speed was too slow, landing speed was too high, or it flew off the airstrip.)" << endl;
    std::cout << "(For your own good experience, you can continue to fly freely even if you crash.)" << endl;
    std::cout << "___________________________________________" << endl;
    std::cout << "Note: " << endl;
    std::cout << "Use the ESDF keys to control the acceleration of the FRONT, BACK, LEFT, and RIGHT directions." << endl;
    std::cout << "Not WASD!!!" << endl;
    std::cout << "Use A and Z to control the plane UP and DOWN." << endl;
    std::cout << "The Q key is used to toggle FULL SCREEN, and the R key is used to TRY AGAIN." << endl;
    std::cout << "The SPACE key is used to START the game or SWITCH to another person's perspective." << endl;
    std::cout << " " << endl;
    std::cout << "There is a dashboard under the first-person view to visually display the forward speed." << endl;
    std::cout << "The red line on the instrument panel represents the stall speed, the yellow line represents the impending stall warning, and the green line represents the current speed." << endl;
    std::cout << "My suggestion: try both perspectives, the third person may be easier." << endl;
    std::cout << "Tip: You can actually land smoothly by just pressing the E and A keys intermittently. But it's always good to try." << endl;
    std::cout << "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~" << endl;
    std::cout << "Now you can press the SPACE key to start the game." << endl;
    std::cout << "___________________________________________" << endl;
}

void init() {
    consoleInit();
    glClearColor(173.0f / 256.0f, 216.0f / 256.0f, 235.0 / 256.0f, 1.0);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glShadeModel(GL_FLAT);
    glEnable(GL_TEXTURE_2D);
    ReadImage("plane1.bmp", imagewidth0, imageheight0, pixellength0);
    ReadImage("plane.bmp", imagewidth1, imageheight1, pixellength1);
    ReadImage("grass.bmp", imagewidth2, imageheight2, pixellength2);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);  // set pixel storage modes (in the memory) 
    glGenTextures(3, &texture[0]);  // number of texture names to be generated and an array of texture names

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

    glBindTexture(GL_TEXTURE_2D, texture[2]);
    glTexImage2D(GL_TEXTURE_2D, 0, 3, imagewidth2, imageheight2, 0, GL_BGR_EXT, GL_UNSIGNED_BYTE, p[2]);
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
    glutCreateWindow("Assessment2");
    glutDisplayFunc(display);
    glutKeyboardFunc(handleKeypress);

    init();
    initLighting();

    glutTimerFunc(0, update, 0); // Start the update function immediately

    glutMainLoop();

    return 0;
}
