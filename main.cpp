//
//  main.cpp
//  Marroco
//
//  Created by عزة on 25/11/1447 AH.
//
//////////
//Libraries//
 //////////
#ifdef APPLE
#define GL_SILENCE_DEPRECATION
#include <OpenGL/gl.h>
#include <GLUT/glut.h>

#include <GLUT/glut.h>
#else
#endif
#include <stdlib.h>
#include <iostream>
#include <string>
#include <vector>
#include <math.h>
#pragma comment(lib, "winmm.lib")


// Source: Microsoft PlaySound documentation: https://learn.microsoft.com/en-us/windows/win32/api/mmsystem/nf-mmsystem-playsound

using namespace std;


//////////
//Variables//
 //////////
int windowWidth = 1024; int windowHeight = 600;

int gameState = 0;  double PI = 3.14159265359;

int triangleAmount = 200; float twicePi = 2.0f * PI;
// NEW: Animation timer for butterfly movement.
float butterflyTime = 0.0f;
// NEW: Zoom value for 2D viewing/zooming using keyboard (+ / -).
float zoom = 1.0f; float sunAngle = 0.0f; const char* feedbackMessage = "";
float charX = 0.0f; // Horizontal movement
float charY = 0.0f; // Vertical movement
float trophyRotation = 0.0f; // Variable to control trophy rotation
float transValueX = 3.5f;
float transValueY = 2.0f;
float transValueZ = 2.0f;
float nearValue = 0.01f;
float farValue = 50.0f;
float windmillAngle = 0.0f; // Variable to control windmill blades rotation

/////////////
//Screen cases//
 /////////////
#define Start_Screen 0
#define Winter 1
#define Spring 2
#define Summer 3
#define Autumn 4
#define End_Screen 5


 /////////
//Texture//
 ////////
 uint winterTexture, spring1Texture, spring2Texture, spring3Texture, autumnTexture;
char winterPath[] = "winter.bmp";
char spring1Path[] = "FloralTexture.bmp";
char spring2Path[] = "PinkTexture.bmp";
char spring3Path[] = "RiverTexture.bmp";
char autumnPath[] = "autumn.bmp";

 uint LoadTexture(const char* filename, int width, int height)
{
    uint texture;
    unsigned char* data;
    FILE* file;

    // تم استبدال fopen_s بدالة fopen القياسية لتجنب خطأ التوافق في الماك
    file = fopen(filename, "rb");

    if (file == NULL)
    {
        cout << "Warning: Texture not found -> " << filename << endl;
        return 0;
    }

    data = (unsigned char*)malloc(width * height * 3);
    fread(data, width * height * 3, 1, file);
    fclose(file);

    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
    
    free(data);
    return texture;
}

    data = (unsigned char*)malloc(width * height * 3);
    fseek(file, 54, SEEK_SET); // Skip BMP header (54 bytes)
    fread(data, width * height * 3, 1, file);
    fclose(file);

    // reorder the image colors to RGB not BGR
    for (int i = 0; i < width * height; ++i)
    {
        int index = i * 3;
        unsigned char B, R;
        B = data[index];
        R = data[index + 2];

        data[index] = R;
        data[index + 2] = B;

    }

    glGenTextures(1, &texture);            //generate the texture with the loaded data
    glBindTexture(GL_TEXTURE_2D, texture); //bind the texture to it's array

    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);  //set texture environment parameters

    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);

    free(data); //free the texture array

    if (glGetError() != GL_NO_ERROR)
        printf("GLError in genTexture()\n");

    return texture; //return whether it was successfull
}

///////////////
//Basic functions//
 ////////////////


// NEW: Helper functions to reduce type-conversion warnings in OpenGL calls.
// Source: OpenGL fixed-function transformation reference: https://www.khronos.org/registry/OpenGL-Refpages/gl2.1/

// ----------------------------- Safe OpenGL Helpers -----------------------------
void V2(double x, double y) { glVertex2f(static_cast<GLfloat>(x), static_cast<GLfloat>(y)); }
void C3(double r, double g, double b) { glColor3f(static_cast<GLfloat>(r), static_cast<GLfloat>(g), static_cast<GLfloat>(b)); }
void C4(double r, double g, double b, double a) { glColor4f(static_cast<GLfloat>(r), static_cast<GLfloat>(g), static_cast<GLfloat>(b), static_cast<GLfloat>(a)); }
void T2(double u, double v) { glTexCoord2f(static_cast<GLfloat>(u), static_cast<GLfloat>(v)); }
void Translate(double x, double y, double z) { glTranslatef(static_cast<GLfloat>(x), static_cast<GLfloat>(y), static_cast<GLfloat>(z)); }
void Scale(double x, double y, double z) { glScalef(static_cast<GLfloat>(x), static_cast<GLfloat>(y), static_cast<GLfloat>(z)); }
void Rotate(double angle, double x, double y, double z) { glRotatef(static_cast<GLfloat>(angle), static_cast<GLfloat>(x), static_cast<GLfloat>(y), static_cast<GLfloat>(z)); }

// Function to write text
void drawText(float x, float y, const char* text) {
    glRasterPos2f(x, y);
    for (int i = 0; text[i] != '\0'; i++) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, text[i]);
    }
}

// Circle
void drawCircle(GLfloat x, GLfloat y, GLfloat radius) {
    int i;

    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(x, y);

    for (i = 0; i <= triangleAmount; i++) {
        glVertex2f(
            x + (radius * cos(i * twicePi / triangleAmount)),
            y + (radius * sin(i * twicePi / triangleAmount))
        );
    }

    glEnd();
}

// NEW: Ellipse helper used for clouds, flowers, and butterfly wings.
void drawEllipse(double x, double y, double rx, double ry) {
    glBegin(GL_TRIANGLE_FAN);
    V2(x, y);
    for (int i = 0; i <= triangleAmount; i++) {
        double angle = static_cast<double>(i) * twicePi / static_cast<double>(triangleAmount);
        V2(x + rx * cos(angle), y + ry * sin(angle));
    }
    glEnd();
}



//////////
//character//
 //////////
//Face
void DrawFace()
{
    glColor3f(1.0, 0.9, 0.8);
    drawCircle(0.0, 0.0, 0.5);
}

//Back hair
void DrawBackHair()
{
    glColor3f(0.1f, 0.1f, 0.1f);
    glBegin(GL_POLYGON);
    glVertex2f(-0.75f, -0.25f);

    glVertex2f(-0.73f, 0.05f); glVertex2f(-0.70f, 0.25f); glVertex2f(-0.65f, 0.35f); glVertex2f(-0.60f, 0.45f); glVertex2f(-0.50f, 0.55f); glVertex2f(-0.45f, 0.60f); glVertex2f(-0.35f, 0.65f); glVertex2f(-0.25f, 0.70f);
    glVertex2f(0.0f, 0.75f);     // Middle of the head
    glVertex2f(0.25f, 0.70f); glVertex2f(0.35f, 0.65f); glVertex2f(0.45f, 0.60f); glVertex2f(0.50f, 0.55f); glVertex2f(0.60f, 0.45f); glVertex2f(0.65f, 0.35f); glVertex2f(0.70f, 0.25f); glVertex2f(0.73f, 0.05f);

    glVertex2f(0.75f, -0.25f);
    glEnd();
}

// Top hair
void DrawTopHairSimple()
{
    glColor3f(0.1f, 0.1f, 0.1f);
    // Middle circle
    drawCircle(0.0f, 0.47f, 0.22f); // x, y, radius
    // Left circle
    drawCircle(-0.28f, 0.47f, 0.20f);
    // Right circle
    drawCircle(0.28f, 0.47f, 0.20f);
}

// Bangs
void DrawBangs()
{
    glColor3f(0.1f, 0.1f, 0.1f);
    // Triangles for bangs
    glBegin(GL_TRIANGLES);
    // left
    glVertex2f(-0.50f, 0.32f); glVertex2f(-0.30f, 0.32f); glVertex2f(-0.40f, 0.12f);
    // mid left
    glVertex2f(-0.30f, 0.32f); glVertex2f(-0.08f, 0.32f); glVertex2f(-0.19f, 0.08f);
    // middle
    glVertex2f(-0.12f, 0.34f); glVertex2f(0.12f, 0.34f); glVertex2f(0.0f, 0.05f);
    // mid right
    glVertex2f(0.08f, 0.32f); glVertex2f(0.30f, 0.32f); glVertex2f(0.19f, 0.08f);
    // right
    glVertex2f(0.30f, 0.32f); glVertex2f(0.50f, 0.32f); glVertex2f(0.40f, 0.12f);

    glEnd();
}

//Ear
void DrawEarLeft()
{
    glColor3f(1.0f, 0.9f, 0.8f);
    drawCircle(-0.52f, -0.07f, 0.09f);
}

//Ear
void DrawEarRight()
{
    glColor3f(1.0f, 0.9f, 0.8f);
    drawCircle(0.52f, -0.07f, 0.09f);
}
//Eye
void DrawEyes()
{
    glColor3f(0.0, 0.0, 0.0);
    drawCircle(-0.15f, -0.04f, 0.07f);// Left eye
    drawCircle(0.15f, -0.04f, 0.07f);// Right eye
    // Eye shine
    glColor3f(1.0f, 1.0f, 1.0f);
    drawCircle(-0.13f, -0.01f, 0.025f);// left
    drawCircle(0.17f, -0.01f, 0.025f);// right
}

// Eyebrow
void DrawEyebrows()
{
    glColor3f(0.0, 0.0, 0.0);
    glLineWidth(0.1f);
    // Left eyebrow
    glBegin(GL_LINE_STRIP);
    glVertex2f(-0.30f, 0.05f); glVertex2f(-0.27f, 0.07f); glVertex2f(-0.24f, 0.08f); glVertex2f(-0.20f, 0.09f); glVertex2f(-0.16f, 0.08f); glVertex2f(-0.12f, 0.07f); glVertex2f(-0.10f, 0.05f);
    glEnd();
    // Right eyebrow
    glBegin(GL_LINE_STRIP);
    glVertex2f(0.10f, 0.05f); glVertex2f(0.12f, 0.07f); glVertex2f(0.16f, 0.08f); glVertex2f(0.20f, 0.09f); glVertex2f(0.24f, 0.08f); glVertex2f(0.27f, 0.07f); glVertex2f(0.30f, 0.05f);
    glEnd();
}

// Cheeks
void DrawCheeks()
{
    glColor3f(1.0f, 0.6f, 0.6f);
    // Left cheek
    drawCircle(-0.30f, -0.31f, 0.07f);
    // Right cheek
    drawCircle(0.30f, -0.31f, 0.07f);
}

// Smile
void DrawSmile()
{
    glColor3f(0.0, 0.0, 0.0);
    glLineWidth(0.1f);
    glBegin(GL_LINE_STRIP);
    glVertex2f(-0.08f, -0.28f); glVertex2f(-0.05f, -0.30f); glVertex2f(0.0f, -0.31f); glVertex2f(0.05f, -0.30f); glVertex2f(0.08f, -0.28f);
    glEnd();
}

// Neck
void drawNeck()
{
    glColor3f(1.0, 0.9, 0.8);
    glBegin(GL_POLYGON);
    glVertex2f(-0.05f, -0.50f); glVertex2f(0.05f, -0.50f); glVertex2f(0.05f, -0.60f); glVertex2f(-0.05f, -0.60f);
    glEnd();
}

// Torso (Shirt)
void drawTorso() {
    // This function is for start/end screens and winter only
    if (gameState != Start_Screen && gameState != End_Screen && gameState != Winter)
        return;
    // For Winter
    if (gameState == Winter) {
        glColor3f(1.0f, 1.0f, 0.6f);
    }
    else {
        glColor3f(1.0f, 1.0f, 1.0f); // Start and End screens
    }
    glBegin(GL_POLYGON);
    glVertex2f(-0.35f, -0.60f); glVertex2f(0.35f, -0.60f); glVertex2f(0.30f, -1.25f); glVertex2f(-0.30f, -1.25f);
    glEnd();

    // Suspender straps
    glColor3f(1.0f, 0.0f, 0.0f);
    // left
    glBegin(GL_POLYGON);
    glVertex2f(-0.20f, -0.60f); glVertex2f(-0.12f, -0.60f); glVertex2f(-0.12f, -1.25f); glVertex2f(-0.20f, -1.25f);
    glEnd();
    // right
    glBegin(GL_POLYGON);
    glVertex2f(0.12f, -0.60f); glVertex2f(0.20f, -0.60f); glVertex2f(0.20f, -1.25f); glVertex2f(0.12f, -1.25f);
    glEnd();
}

// Torso (Autumn shirt)
void drawAutumnTorso()
{
    if (gameState == Autumn) {
        // Base brown shirt
        glColor3f(0.72f, 0.45f, 0.20f);
        glBegin(GL_POLYGON);
        glVertex2f(-0.38f, -0.60f); glVertex2f(0.38f, -0.60f); glVertex2f(0.32f, -1.28f); glVertex2f(-0.32f, -1.28f);
        glEnd();
        // Collar
        glColor3f(0.90f, 0.55f, 0.08f);
        glBegin(GL_POLYGON);
        glVertex2f(-0.18f, -0.60f); glVertex2f(0.18f, -0.60f); glVertex2f(0.08f, -0.78f); glVertex2f(-0.08f, -0.78f);
        glEnd();
        // Left pocket
        glColor3f(0.80f, 0.30f, 0.02f);
        glBegin(GL_POLYGON);
        glVertex2f(-0.14f, -0.78f); glVertex2f(-0.04f, -0.78f); glVertex2f(-0.05f, -1.32f); glVertex2f(-0.15f, -1.32f);
        glEnd();
        // Right pocket
        glBegin(GL_POLYGON);
        glVertex2f(0.04f, -0.78f); glVertex2f(0.14f, -0.78f); glVertex2f(0.15f, -1.32f); glVertex2f(0.05f, -1.32f);
        glEnd();
        // Line details on the shirt
        glColor3f(0.55f, 0.30f, 0.12f);
        glLineWidth(1.3f);
        glBegin(GL_LINES);
        glVertex2f(-0.20f, -0.65f); glVertex2f(-0.22f, -1.25f); glVertex2f(0.0f, -0.62f);   glVertex2f(0.0f, -1.25f); glVertex2f(0.20f, -0.65f);  glVertex2f(0.22f, -1.25f);
        glEnd();
    }
}

// Torso (Summer shirt)
void drawSummerTorso()
{
    if (gameState == Summer) {
        //Pink Top
                glColor3f(1.0f, 0.7f, 0.8f);
                glBegin(GL_POLYGON);
                glVertex2f(-0.35f, -0.60f); glVertex2f(0.35f, -0.60f); glVertex2f(0.30f, -1.25f); glVertex2f(-0.30f, -1.25f);
                glEnd();
                //Yellow Dots
                glColor3f(1.0f, 1.0f, 0.0f);
                drawCircle(-0.15f, -0.80f, 0.04f); drawCircle(0.15f, -0.95f, 0.04f); drawCircle(-0.05f, -1.10f, 0.04f); drawCircle(0.10f, -0.75f, 0.04f);
            }
        }

        // Scarf
        void drawScarf()
        {
            if (gameState == Winter) {
                glColor3f(0.9f, 0.0f, 0.0f);
                // Scarf wrapped around neck
                glBegin(GL_POLYGON);
                glVertex2f(-0.15f, -0.50f); glVertex2f(0.15f, -0.50f); glVertex2f(0.20f, -0.65f); glVertex2f(-0.20f, -0.65f);
                glEnd();
                // Hanging part
                glBegin(GL_POLYGON);
                glVertex2f(0.10f, -0.65f); glVertex2f(0.18f, -0.65f); glVertex2f(0.18f, -0.90f); glVertex2f(0.10f, -0.90f);
                glEnd();
            }
        }

        // Legs
        void drawLegs() {
            // Like long socks
            if (gameState == Winter) {
                glColor3f(0.5, 0.5, 0.5);
            }
            else {
                glColor3f(1.0, 0.9, 0.8);
            }
            // left
            glBegin(GL_POLYGON);
            glVertex2f(-0.20f, -1.75f); glVertex2f(-0.05f, -1.75f); glVertex2f(-0.05f, -2.45f); glVertex2f(-0.20f, -2.45f);
            glEnd();
            // right
            glBegin(GL_POLYGON);
            glVertex2f(0.05f, -1.75f); glVertex2f(0.20f, -1.75f); glVertex2f(0.20f, -2.45f); glVertex2f(0.05f, -2.45f);
            glEnd();
        }

        // Skirt
        void drawSkirt() {
            if (gameState == Autumn) {
                // Autumn = textured skirt
                glEnable(GL_TEXTURE_2D);
                glBindTexture(GL_TEXTURE_2D, autumnTexture);
                glColor3f(1.0f, 1.0f, 1.0f);
                glBegin(GL_POLYGON);
                glTexCoord2f(0.0, 0.0); glVertex2f(-0.35f, -1.25f); glTexCoord2f(1.0, 0.0); glVertex2f(0.35f, -1.25f); glTexCoord2f(1.0, 1.0); glVertex2f(0.55f, -1.95f); glTexCoord2f(0.0, 1.0); glVertex2f(-0.55f, -1.95f);
                glEnd();
                glDisable(GL_TEXTURE_2D);
            }
            else {
                glColor3f(0.9f, 0.0f, 0.0f);
                glBegin(GL_POLYGON);
                glVertex2f(-0.35f, -1.25f); glVertex2f(0.35f, -1.25f); glVertex2f(0.55f, -1.95f); glVertex2f(-0.55f, -1.95f);
                glEnd();
                // Draw pleats on the skirt
                glColor3f(0.05f, 0.05f, 0.05f);
                glLineWidth(1.2);
                glBegin(GL_LINES);
                glVertex2f(-0.15f, -1.25f); glVertex2f(-0.25f, -1.95f); glVertex2f(0.0f, -1.25f); glVertex2f(0.0f, -1.95f); glVertex2f(0.15f, -1.25f);  glVertex2f(0.25f, -1.95f);
                glEnd();
            }
        }

        // Summer skirt
        void drawSummerSkirt()
        {
            if (gameState == Summer) {
                glColor3f(1.0f, 1.0f, 0.0f);
                glBegin(GL_POLYGON);
                glVertex2f(-0.35f, -1.25f); glVertex2f(0.35f, -1.25f); glVertex2f(0.55f, -1.95f); glVertex2f(-0.55f, -1.95f);
                glEnd();
            }
        }

        // Socks and shoes
        void drawFeet() {
            glColor3f(1.0f, 1.0f, 1.0f);
            // Left sock (small rectangle above shoe)
            glBegin(GL_POLYGON);
            glVertex2f(-0.20f, -2.35f); glVertex2f(-0.05f, -2.35f); glVertex2f(-0.05f, -2.50f); glVertex2f(-0.20f, -2.50f);
            glEnd();
            // Right sock
            glBegin(GL_POLYGON);
            glVertex2f(0.05f, -2.35f); glVertex2f(0.20f, -2.35f); glVertex2f(0.20f, -2.50f); glVertex2f(0.05f, -2.50f);
            glEnd();

            // Left shoe
            if (gameState == Autumn) {
                glColor3f(0.72f, 0.43f, 0.16f);
            }
            else if (gameState == Summer) {
                glColor3f(1.0f, 0.7f, 0.8f);
            }
            // Spring case
            else if (gameState == Spring) {
                glColor3f(1.0f, 0.45f, 0.65f);
            }
            else {
                glColor3f(0.8f, 0.0f, 0.0f);
            }
            glBegin(GL_POLYGON);
            glVertex2f(-0.28f, -2.50f); glVertex2f(-0.07f, -2.50f); glVertex2f(-0.05f, -2.70f); glVertex2f(-0.30f, -2.70f);
            glEnd();

            // Right shoe
            if (gameState == Autumn) {
                glColor3f(0.72f, 0.43f, 0.16f);
            }
            else if (gameState == Summer) {
                glColor3f(1.0f, 0.7f, 0.8f);
                    }
                    // Spring case
                    else if (gameState == Spring) {
                        glColor3f(1.0f, 0.45f, 0.65f);
                    }
                    else {
                        glColor3f(0.8f, 0.0f, 0.0f);
                    }
                    glBegin(GL_POLYGON);
                    glVertex2f(0.07f, -2.50f); glVertex2f(0.28f, -2.50f); glVertex2f(0.30f, -2.70f); glVertex2f(0.05f, -2.70f);
                    glEnd();
                }

                // Sleeves and hands
                void drawArms() {
                    glColor3f(1.0f, 1.0f, 1.0f);
                    // Left sleeve (covers shoulder area)
                     // Winter case
                    if (gameState == Winter) {
                        glColor3f(1.0f, 1.0f, 0.6f);
                    }
                    // Other seasons
                    else {
                        glColor3f(1.0f, 1.0f, 1.0f);
                    }
                    glBegin(GL_POLYGON);
                    glVertex2f(-0.35f, -0.60f); glVertex2f(-0.52f, -0.70f); glVertex2f(-0.52f, -1.05f); glVertex2f(-0.30f, -1.05f);
                    glEnd();
                    // Right sleeve
                    // Winter case
                    if (gameState == Winter) {
                        glColor3f(1.0f, 1.0f, 0.6f);
                    }
                    // Other seasons
                    else {
                        glColor3f(1.0f, 1.0f, 1.0f);
                    }
                    glBegin(GL_POLYGON);
                    glVertex2f(0.35f, -0.60f); glVertex2f(0.52f, -0.70f); glVertex2f(0.52f, -1.05f); glVertex2f(0.30f, -1.05f);
                    glEnd();

                    // Hands
                    glColor3f(1.0, 0.9, 0.8);
                    // Left hand
                    glBegin(GL_POLYGON);
                    glVertex2f(-0.35f, -1.05f); glVertex2f(-0.48f, -1.05f); glVertex2f(-0.48f, -1.50f); glVertex2f(-0.35f, -1.50f);
                    glEnd();
                    // Right hand
                    glBegin(GL_POLYGON);
                    glVertex2f(0.35f, -1.05f); glVertex2f(0.48f, -1.05f); glVertex2f(0.48f, -1.50f); glVertex2f(0.35f, -1.50f);
                    glEnd();
                }

                // Summer sunglasses
                void drawSunglasses() {
                    if (gameState == Summer) {
                        glColor3f(0.0f, 0.0f, 0.0f);
                        drawCircle(-0.16f, -0.04f, 0.11f); // Left Lens
                        drawCircle(0.16f, -0.04f, 0.11f);  // Right Lens
                        glLineWidth(2.5f);
                        glBegin(GL_LINES);
                        glVertex2f(-0.05f, -0.04f); glVertex2f(0.05f, -0.04f); // Bridge
                        glVertex2f(-0.27f, -0.04f); glVertex2f(-0.52f, -0.08f); // Left temple
                        glVertex2f(0.27f, -0.04f); glVertex2f(0.52f, -0.08f);  // Right temple
                        glEnd();
                    }
                }

                // Ice Cream
                void drawIceCream() {
                    // Summer case
                    if (gameState == Summer) {
                        // Cone
                        glColor3f(0.8f, 0.5f, 0.2f);
                        glBegin(GL_TRIANGLES);
                        glVertex2f(0.38f, -0.90f); glVertex2f(0.50f, -0.90f); glVertex2f(0.44f, -1.35f);
                        glEnd();
                        // Scoop
                        glColor3f(1.0f, 0.5f, 0.7f);
                        drawCircle(0.44f, -0.85f, 0.13f);
                    }
                }

                // Function to assemble all character parts
                void MyMainObj()
                {
                    glPushMatrix();
                    glTranslatef(300.0f + charX, 350.0f + charY, 0.0f); // Character position
                    glScalef(80, 80, 1);       // Character size

                    DrawBackHair(); DrawEarLeft(); DrawEarRight(); DrawFace(); DrawTopHairSimple(); DrawBangs(); DrawEyes(); DrawEyebrows(); DrawCheeks(); DrawSmile(); drawSunglasses();
                    drawNeck(); drawScarf(); drawTorso(); drawAutumnTorso(); drawSummerTorso(); drawArms(); drawLegs(); drawSkirt(); drawSummerSkirt(); drawFeet(); drawIceCream();
                    glPopMatrix();
                }



                ////////////
                //environment//
                 ////////////

                // Sun
                void drawAttractiveSun(float cx, float cy, float r) {
                    glPushMatrix();
                    glTranslatef(cx, cy, 0);       // 1. Move to sun center
                    glRotatef(sunAngle, 0, 0, 1);   // 2. Rotate
                    glTranslatef(-cx, -cy, 0);     // 3. Move back
                    // Back glow
                    glBegin(GL_TRIANGLE_FAN);
                    // Autumn case
                    if (gameState == Autumn) {
                        glColor4f(0.95f, 0.70f, 0.20f, 0.18f);
                    }
                    // Other seasons
                    else {
                        glColor4f(1.0f, 0.9f, 0.0f, 0.3f);
                    }

                    glVertex2f(cx, cy);
                    // Autumn case
                    if (gameState == Autumn) {
                        glColor4f(0.95f, 0.70f, 0.20f, 0.0f);
                    }
                    // Other seasons
                    else {
                        glColor4f(1.0f, 0.9f, 0.0f, 0.0f);
                    }

                    // Drawing the glow circle using a triangle fan
                    for (int i = 0; i <= triangleAmount; i++) {
                        glVertex2f(
                            cx + (r * 1.8f * cos(i * twicePi / triangleAmount)),
                            cy + (r * 1.8f * sin(i * twicePi / triangleAmount)));
                    }
                    glEnd();
