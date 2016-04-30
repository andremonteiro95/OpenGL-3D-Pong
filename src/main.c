// André Monteiro , número 31906
// cc tp1.c -o tp1 -lGL -lglut -lm -lGLU
#include <stdio.h>         // Header File For Standard Input / Output
#include <stdarg.h>        // Header File For Variable Argument Routines
#include <string.h>        // Header File For String Management
#include <stdlib.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>

// CONFIGURÁVEIS
float initialPlayerSpeed = 0.006;  // Velocidade inicial dos jogadores
float initialBallSpeed = 0.004;    // Velocidade inicial da bola
float maxPoints = 7;        // Limite de pontos para ganhar
float ballAccel   = 1.07;   // Aceleração da bola: velocidade * aceleraçao
float playerAccel = 1.03;   // Acel jogadores

// Estruturas
struct Image {
    unsigned long sizeX;
    unsigned long sizeY;
    char *data;
};

typedef struct Image Image;

// Variáveis
GLuint texture_id[3];  // 1 - crate, 2 - floor
GLfloat xOne[2] = { 5.0f , 5.5f } , xTwo[2] = { 14.5f , 15.0f };
GLfloat yOne[2] = { 4.0f , 6.0f } , yTwo[2] = { 4.0f , 6.0f };
GLfloat zHeight[2] = { 1.0f , 3.0f };
GLfloat fieldX[2] = { 0.0f , 20.0f };
GLfloat fieldYOne[2] = { 0.0f , 0.5f } , fieldYTwo[2] = { 9.5f , 10.0f };
GLfloat fieldZ[2] = { 1.0f , 2.0f };
GLfloat ball[3] = { 10.0f, 5.0f, 1.0f};
GLfloat ballRadius = 0.3;
float tamPong = 1.5;
int keystates[4] = {0 , 0 , 0, 0}; // { UP, DOWN, W, S };
int directionX = 0;
int gameRunning=0;
int npoints[2] = {0,0};
float ballVerticalSpeed = 0; 
float playerSpeed;
float ballSpeed;

//Cabeçalhos funções
unsigned int getint(FILE *fp);
unsigned int getshort(FILE* fp);
GLvoid LoadTexture(GLvoid);
int LoadBMP(char *filename, Image *image);
void drawFunc();
void keyboardArrows(int key, int x, int y);
void keyboardArrowsUp(int key, int x, int y);
void keyboardLetters(unsigned char key, int x, int y);
void keyboardLettersUp(unsigned char key, int x, int y);
void init();
void display();
void reshape (int w, int h);
void resetBoard(int mode);
void getRandomDirection();
void loop();
void changeDirectionY(int side);
void drawText(char* text, float x, float y, float z);

//Funções
int main(int argc, char** argv)
{
   glutInit(&argc, argv);
   glutInitDisplayMode (GLUT_SINGLE | GLUT_RGBA | GLUT_DEPTH);
   glutInitWindowSize (800, 500); 
   glutInitWindowPosition (100, 100);
   glutCreateWindow ("[CG] Pong - Andre Monteiro, 31906");
   init();
   glutDisplayFunc(display); 
   glutReshapeFunc(reshape);
   glutSpecialFunc(keyboardArrows);
   glutSpecialUpFunc(keyboardArrowsUp);
   glutKeyboardFunc(keyboardLetters);
   glutKeyboardUpFunc(keyboardLettersUp);
   resetBoard(1);
   glutIdleFunc(loop);

   glutMainLoop();
   return 0;
}

unsigned int getint(FILE *fp)
{
  int c, c1, c2, c3;

  /*  get 4 bytes */ 
  c = getc(fp);  
  c1 = getc(fp);  
  c2 = getc(fp);  
  c3 = getc(fp);
  
  return ((unsigned int) c) +   
    (((unsigned int) c1) << 8) + 
    (((unsigned int) c2) << 16) +
    (((unsigned int) c3) << 24);
}

unsigned int getshort(FILE* fp)
{
  int c, c1;
  
  /* get 2 bytes*/
  c = getc(fp);  
  c1 = getc(fp);

  return ((unsigned int) c) + (((unsigned int) c1) << 8);
}

GLvoid LoadTexture(GLvoid)
{  
   Image *TextureImage;
    
   TextureImage = (Image *) malloc(sizeof(Image));
   
   if (TextureImage == NULL) 
   {
      printf("Error allocating space for image");
      exit(1);
   }
   
   //Texture bumpers
   LoadBMP("./cratergb.bmp", TextureImage);

   glGenTextures(1, &texture_id[0]);
      
   glBindTexture(GL_TEXTURE_2D, texture_id[0]);
   
   glTexImage2D(GL_TEXTURE_2D, 0, 3, TextureImage->sizeX, TextureImage->sizeY, 0, GL_RGB, GL_UNSIGNED_BYTE, TextureImage->data);
   glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
   glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);      // Create The Texture

   free (TextureImage->data);
   free( TextureImage );   

   //Texture floor
   TextureImage = (Image *) malloc(sizeof(Image));

   LoadBMP("./floor.bmp", TextureImage);

   glGenTextures(1, &texture_id[1]);
      
   glBindTexture(GL_TEXTURE_2D, texture_id[1]);
   
   glTexImage2D(GL_TEXTURE_2D, 0, 3, TextureImage->sizeX, TextureImage->sizeY, 0, GL_RGB, GL_UNSIGNED_BYTE, TextureImage->data);
   glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
   glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);      // Create The Texture

   free (TextureImage->data);
   free( TextureImage );
}

int LoadBMP(char *filename, Image *image) {
    FILE *file;
    unsigned long size;                 /*  size of the image in bytes. */
    unsigned long i;                    /*  standard counter. */
    unsigned short int planes;          /*  number of planes in image (must be 1)  */
    unsigned short int bpp;             /*  number of bits per pixel (must be 24) */
    char temp;                          /*  used to convert bgr to rgb color. */

    /*  make sure the file is there. */
    if ((file = fopen(filename, "rb"))==NULL) {
      printf("File Not Found : %s\n",filename);
      return 0;
    }
    
    /*  seek through the bmp header, up to the width height: */
    fseek(file, 18, SEEK_CUR);

    /*  No 100% errorchecking anymore!!! */

    /*  read the width */    
    image->sizeX = getint (file);
    
    /*  read the height */ 
    image->sizeY = getint (file);
    
    /*  calculate the size (assuming 24 bits or 3 bytes per pixel). */
    size = image->sizeX * image->sizeY * 3;

    /*  read the planes */    
    planes = getshort(file);
    if (planes != 1) {
   printf("Planes from %s is not 1: %u\n", filename, planes);
   return 0;
    }

    /*  read the bpp */    
    bpp = getshort(file);
    if (bpp != 24) {
      printf("Bpp from %s is not 24: %u\n", filename, bpp);
      return 0;
    }
   
    /*  seek past the rest of the bitmap header. */
    fseek(file, 24, SEEK_CUR);

    /*  read the data.  */
    image->data = (char *) malloc(size);
    if (image->data == NULL) {
   printf("Error allocating memory for color-corrected image data");
   return 0;   
    }

    if ((i = fread(image->data, size, 1, file)) != 1) {
   printf("Error reading image data from %s.\n", filename);
   return 0;
    }

    for (i=0;i<size;i+=3) { /*  reverse all of the colors. (bgr -> rgb) */
      temp = image->data[i];
      image->data[i] = image->data[i+2];
      image->data[i+2] = temp;
    }

    fclose(file); /* Close the file and release the filedes */

    /*  we're done. */
    return 1;
}

void drawFunc()
{
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
   glEnable(GL_COLOR_MATERIAL);
   glEnable(GL_BLEND);
   glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
   glBindTexture(GL_TEXTURE_2D, 0);
   if (!gameRunning) {
      if (npoints[0] == maxPoints){
         glColor3f(1.0,0.0,0.0);
         drawText("Player 1 wins!", 8.9, 10, 4.8);
         resetBoard(1);
      }
      else if (npoints[1] == maxPoints){
         glColor3f(0.0,0.0,1.0);
         drawText("Player 2 wins!", 8.9, 10, 4.8);
         resetBoard(1);
      }
      else 
         drawText("Click any key!", 8.9, 10, 4.8);
   }
   glColor3f(1.0,1.0,1.0);
   drawText("SCORE", 9.4, 10, 4);
   drawText("-", 9.9, 10, 3);
   char score1[2],score2[2];
   sprintf(score1,"%d",npoints[0]);
   sprintf(score2,"%d",npoints[1]);
   drawText(score1, 9.38, 10, 3);
   drawText(score2, 10.5, 10, 3);


   // FIELD LIMITS - NEAR 
   glBegin(GL_QUADS); 
      // Front Face = + proxima do eixo x
      glColor3f(0.137255,0.556863,0.137255);
      glVertex3f( fieldX[0], fieldYOne[0], fieldZ[0]);  // Bottom Left Of The Texture and Quad
      glVertex3f( fieldX[1], fieldYOne[0], fieldZ[0]);  // Bottom Right Of The Texture and Quad
      glVertex3f( fieldX[1], fieldYOne[0], fieldZ[1]);  // Top Right Of The Texture and Quad
      glVertex3f( fieldX[0], fieldYOne[0], fieldZ[1]);  // Top Left Of The Texture and Quad

      // Top Face
      glColor3f(1.0f,1.0f,1.0f);
      glVertex3f( fieldX[0], fieldYOne[1], fieldZ[1]);  // Top Left Of The Texture and Quad
      glVertex3f( fieldX[0], fieldYOne[0], fieldZ[1]);  // Bottom Left Of The Texture and Quad
      glVertex3f( fieldX[1], fieldYOne[0], fieldZ[1]);  // Bottom Right Of The Texture and Quad
      glVertex3f( fieldX[1], fieldYOne[1], fieldZ[1]);  // Top Right Of The Texture and Quad
   glEnd();

   // FIELD LIMITS - FAR
   glBegin(GL_QUADS);  
      glColor3f(0.137255,0.556863,0.137255);
      // Front Face = + proxima do eixo x
      glVertex3f( fieldX[0], fieldYTwo[0], fieldZ[0]);  // Bottom Left Of The Texture and Quad
      glVertex3f( fieldX[1], fieldYTwo[0], fieldZ[0]);  // Bottom Right Of The Texture and Quad
      glVertex3f( fieldX[1], fieldYTwo[0], fieldZ[1]);  // Top Right Of The Texture and Quad
      glVertex3f( fieldX[0], fieldYTwo[0], fieldZ[1]);  // Top Left Of The Texture and Quad

      // Top Face
      glColor3f(1.0f,1.0f,1.0f);
      glVertex3f( fieldX[0], fieldYTwo[1], fieldZ[1]);  // Top Left Of The Texture and Quad
      glVertex3f( fieldX[0], fieldYTwo[0], fieldZ[1]);  // Bottom Left Of The Texture and Quad
      glVertex3f( fieldX[1], fieldYTwo[0], fieldZ[1]);  // Bottom Right Of The Texture and Quad
      glVertex3f( fieldX[1], fieldYTwo[1], fieldZ[1]);  // Top Right Of The Texture and Quad
   glEnd();



      // FIELD LIMITS - FLOOR
   glBindTexture(GL_TEXTURE_2D, texture_id[1]);
   glColor4f(1, 0.85882352941176,  0.98, 1.0f);
   glBegin(GL_QUADS); 
      glTexCoord2f(0.0f, 0.0f); glVertex3f( fieldX[0], fieldYOne[0], fieldZ[0]);  // Bottom Left Of The Texture and Quad
      glTexCoord2f(0.0f, 2.0f); glVertex3f( fieldX[1], fieldYOne[1], fieldZ[0]);  // Bottom Right Of The Texture and Quad
      glTexCoord2f(2.0f, 2.0f); glVertex3f( fieldX[1], fieldYTwo[1], fieldZ[0]);  // Top Right Of The Texture and Quad
      glTexCoord2f(2.0f, 0.0f); glVertex3f( fieldX[0], fieldYTwo[0], fieldZ[0]);  // Top Left Of The Texture and Quad
   glEnd();

   glBindTexture(GL_TEXTURE_2D, texture_id[0]);
   
   // PLAYER 1
   glBegin(GL_QUADS); 
      glColor3f(1.0, 0.184314, 0.184314);
      // Front Face = + proxima do eixo x
      glTexCoord2f(0.0f, 0.0f); glVertex3f( xOne[0], yOne[0], zHeight[0]);  // Bottom Left Of The Texture and Quad
      glTexCoord2f(0.0f, 1.0f); glVertex3f( xOne[1], yOne[0], zHeight[0]);  // Bottom Right Of The Texture and Quad
      glTexCoord2f(1.0f, 1.0f); glVertex3f( xOne[1], yOne[0], zHeight[1]);  // Top Right Of The Texture and Quad
      glTexCoord2f(1.0f, 0.0f); glVertex3f( xOne[0], yOne[0], zHeight[1]);  // Top Left Of The Texture and Quad

      // Back Face
      glTexCoord2f(0.0f, 0.0f); glVertex3f( xOne[0], yOne[1], zHeight[0]);  // Bottom Right Of The Texture and Quad
      glTexCoord2f(0.0f, 1.0f); glVertex3f( xOne[1], yOne[1], zHeight[0]);  // Top Right Of The Texture and Quad
      glTexCoord2f(1.0f, 1.0f); glVertex3f( xOne[1], yOne[1], zHeight[1]);  // Top Left Of The Texture and Quad
      glTexCoord2f(1.0f, 0.0f); glVertex3f( xOne[0], yOne[1], zHeight[1]);  // Bottom Left Of The Texture and Quad

      // Top Face
      glTexCoord2f(0.0f, 0.0f); glVertex3f( xOne[0], yOne[1], zHeight[1]);  // Top Left Of The Texture and Quad
      glTexCoord2f(0.0f, 1.0f); glVertex3f( xOne[0], yOne[0], zHeight[1]);  // Bottom Left Of The Texture and Quad
      glTexCoord2f(1.0f, 1.0f); glVertex3f( xOne[1], yOne[0], zHeight[1]);  // Bottom Right Of The Texture and Quad
      glTexCoord2f(1.0f, 0.0f); glVertex3f( xOne[1], yOne[1], zHeight[1]);  // Top Right Of The Texture and Quad

      // Bottom Face
      glTexCoord2f(0.0f, 0.0f); glVertex3f( xOne[1], yOne[1], zHeight[0]);  // Top Right Of The Texture and Quad
      glTexCoord2f(0.0f, 1.0f); glVertex3f( xOne[0], yOne[1], zHeight[0]);  // Top Left Of The Texture and Quad
      glTexCoord2f(1.0f, 1.0f); glVertex3f( xOne[0], yOne[0], zHeight[0]);  // Bottom Left Of The Texture and Quad
      glTexCoord2f(1.0f, 0.0f); glVertex3f( xOne[1], yOne[0], zHeight[0]);  // Bottom Right Of The Texture and Quad
      
      // Right face
      glTexCoord2f(0.0f, 0.0f); glVertex3f( xOne[1], yOne[1], zHeight[0]);  // Bottom Right Of The Texture and Quad
      glTexCoord2f(0.0f, 1.0f); glVertex3f( xOne[1], yOne[1], zHeight[1]);  // Top Right Of The Texture and Quad
      glTexCoord2f(1.0f, 1.0f); glVertex3f( xOne[1], yOne[0], zHeight[1]);  // Top Left Of The Texture and Quad
      glTexCoord2f(1.0f, 0.0f); glVertex3f( xOne[1], yOne[0], zHeight[0]);  // Bottom Left Of The Texture and Quad

      // Left Face
      glTexCoord2f(0.0f, 0.0f); glVertex3f( xOne[0], yOne[1], zHeight[0]);  // Bottom Left Of The Texture and Quad
      glTexCoord2f(0.0f, 1.0f); glVertex3f( xOne[0], yOne[0], zHeight[0]);  // Bottom Right Of The Texture and Quad
      glTexCoord2f(1.0f, 1.0f); glVertex3f( xOne[0], yOne[0], zHeight[1]);  // Top Right Of The Texture and Quad
      glTexCoord2f(1.0f, 0.0f); glVertex3f( xOne[0], yOne[1], zHeight[1]);  // Top Left Of The Texture and Quad
   glEnd();

   // PLAYER 2
   glBegin(GL_QUADS); 
   glColor3f(0.196078,0.3,1.0);
      // Front Face = + proxima do eixo x
      glTexCoord2f(0.0f, 0.0f); glVertex3f( xTwo[0], yTwo[0], zHeight[0]);  // Bottom Left Of The Texture and Quad
      glTexCoord2f(0.5f, 0.0f); glVertex3f( xTwo[1], yTwo[0], zHeight[0]);  // Bottom Right Of The Texture and Quad
      glTexCoord2f(0.5f, 0.5f); glVertex3f( xTwo[1], yTwo[0], zHeight[1]);  // Top Right Of The Texture and Quad
      glTexCoord2f(0.0f, 0.5f); glVertex3f( xTwo[0], yTwo[0], zHeight[1]);  // Top Left Of The Texture and Quad

      // Back Face
      glTexCoord2f(1.0f, 0.0f); glVertex3f( xTwo[0], yTwo[1], zHeight[0]);  // Bottom Right Of The Texture and Quad
      glTexCoord2f(1.0f, 1.0f); glVertex3f( xTwo[1], yTwo[1], zHeight[0]);  // Top Right Of The Texture and Quad
      glTexCoord2f(0.0f, 1.0f); glVertex3f( xTwo[1], yTwo[1], zHeight[1]);  // Top Left Of The Texture and Quad
      glTexCoord2f(0.0f, 0.0f); glVertex3f( xTwo[0], yTwo[1], zHeight[1]);  // Bottom Left Of The Texture and Quad

      // Top Face
      glTexCoord2f(0.5f, 1.0f); glVertex3f( xTwo[0], yTwo[1], zHeight[1]);  // Top Left Of The Texture and Quad
      glTexCoord2f(0.5f, 0.5f); glVertex3f( xTwo[0], yTwo[0], zHeight[1]);  // Bottom Left Of The Texture and Quad
      glTexCoord2f(1.0f, 0.5f); glVertex3f( xTwo[1], yTwo[0], zHeight[1]);  // Bottom Right Of The Texture and Quad
      glTexCoord2f(1.0f, 1.0f); glVertex3f( xTwo[1], yTwo[1], zHeight[1]);  // Top Right Of The Texture and Quad

      // Bottom Face
      glTexCoord2f(1.0f, 1.0f); glVertex3f( xTwo[1], yTwo[1], zHeight[0]);  // Top Right Of The Texture and Quad
      glTexCoord2f(0.0f, 1.0f); glVertex3f( xTwo[0], yTwo[1], zHeight[0]);  // Top Left Of The Texture and Quad
      glTexCoord2f(0.0f, 0.0f); glVertex3f( xTwo[0], yTwo[0], zHeight[0]);  // Bottom Left Of The Texture and Quad
      glTexCoord2f(1.0f, 0.0f); glVertex3f( xTwo[1], yTwo[0], zHeight[0]);  // Bottom Right Of The Texture and Quad
      
      // Right face
      glTexCoord2f(1.0f, 0.0f); glVertex3f( xTwo[1], yTwo[1], zHeight[0]);  // Bottom Right Of The Texture and Quad
      glTexCoord2f(1.0f, 1.0f); glVertex3f( xTwo[1], yTwo[1], zHeight[1]);  // Top Right Of The Texture and Quad
      glTexCoord2f(0.0f, 1.0f); glVertex3f( xTwo[1], yTwo[0], zHeight[1]);  // Top Left Of The Texture and Quad
      glTexCoord2f(0.0f, 0.0f); glVertex3f( xTwo[1], yTwo[0], zHeight[0]);  // Bottom Left Of The Texture and Quad

      // Left Face
      glTexCoord2f(0.0f, 0.0f); glVertex3f( xTwo[0], yTwo[1], zHeight[0]);  // Bottom Left Of The Texture and Quad
      glTexCoord2f(1.0f, 0.0f); glVertex3f( xTwo[0], yTwo[0], zHeight[0]);  // Bottom Right Of The Texture and Quad
      glTexCoord2f(1.0f, 1.0f); glVertex3f( xTwo[0], yTwo[0], zHeight[1]);  // Top Right Of The Texture and Quad
      glTexCoord2f(0.0f, 1.0f); glVertex3f( xTwo[0], yTwo[1], zHeight[1]);  // Top Left Of The Texture and Quad
   glEnd();


   // CYLINDER
   glPushMatrix();
      glTranslatef(ball[0],ball[1],ball[2]);
      glColor3f(0.1,0.1,0.1);
      GLUquadricObj *quadratic;
      quadratic = gluNewQuadric();
      gluCylinder(quadratic,0.3f,ballRadius,0.5f,32,32);
   glPopMatrix();

   // TOP DISK OF CYLINDER
   glPushMatrix();
      glTranslatef(ball[0],ball[1],ball[2]+0.5f);
      glColor3f(0.1, 0.1, 0.1);
      GLUquadricObj *quadratic2;
      quadratic2 = gluNewQuadric();
      gluDisk(quadratic,0,ballRadius,32,32);
   glPopMatrix();
}

void keyboardArrows(int key, int x, int y)
{
   gameRunning=1;
    switch(key){
      case GLUT_KEY_UP:{
         //printf("Pos: %f \nLimit %f\n",yOne[1] , fieldYTwo[0]);
         keystates[0] = 1;
         break;
      }
      case GLUT_KEY_DOWN:{
         //printf("Pos: %f \nLimit %f\n",yOne[0] , fieldYOne[1]);
         keystates[1] = 1;
         break;
      }
      default:return;
    }
}

void keyboardArrowsUp(int key, int x, int y)
{
    switch(key){
      case GLUT_KEY_UP:{
         //printf("Pos: %f \nLimit %f\n",yOne[1] , fieldYTwo[0]);
         keystates[0] = 0;
         break;
      }
      case GLUT_KEY_DOWN:{
         //printf("Pos: %f \nLimit %f\n",yOne[0] , fieldYOne[1]);
         keystates[1] = 0;
         break;
      }
      default:return;
    }
}

void keyboardLetters(unsigned char key, int x, int y)
{
   gameRunning=1;
    switch(key){
      case 'w':{
         keystates[2] = 1;
         break;
      }
      case 's':{
         keystates[3] = 1;
         break;
      }
      case 'r':{
         resetBoard(1);
         break;
      }
      default:return;
    }
}

void keyboardLettersUp(unsigned char key, int x, int y)
{
    switch(key){
      case 'w':{
         keystates[2] = 0;
         break;
      }
      case 's':{
         keystates[3] = 0;
         break;
      }
      default:return;
    }
}

void init()
{
   
   LoadTexture();          // Load The Texture(s) ( NEW )
   glEnable(GL_TEXTURE_2D);      // Enable Texture Mapping ( NEW )
   
   glClearColor(0.752941f, 0.752941f, 0.752941f, 0.0f);  // This Will Clear The Background Color To Black
   glClearDepth(1.0);         // Enables Clearing Of The Depth Buffer
   glDepthFunc(GL_LESS);         // The Type Of Depth Test To Do
   glEnable(GL_DEPTH_TEST);      // Enables Depth Testing 
   glShadeModel(GL_SMOOTH);      // Enables Smooth Color Shading

   //LIGHTING
   GLfloat mat_specular[] = {1.0,1.0,1.0,1.0};
    GLfloat mat_shininess[] = {50.0};
    GLfloat light_position[] = {10.0,-1.0,3.0,1.0};



   glMaterialfv(GL_FRONT,GL_SPECULAR,mat_specular);
   glMaterialfv(GL_FRONT,GL_SHININESS,mat_shininess);
   glLightfv(GL_LIGHT0,GL_POSITION,light_position);   
   //END LIGHTING

   glMatrixMode(GL_PROJECTION);
   glLoadIdentity();       // Reset The Projection Matrix

   gluPerspective(45.0f, (GLfloat) 1000 / (GLfloat) 600, 0.1f, 100.0f);
                        
   /*glEnable(GL_LIGHTING);
   glEnable(GL_LIGHT0);*/


   //Keyboard inits
   glutIgnoreKeyRepeat(1);

}

void display()
{
   glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
   glColor3f (1.0, 1.0, 1.0);
   glLoadIdentity ();             /* clear the matrix */
           /* viewing transformation  */
   gluLookAt (10.0, -3, 9, 10.0, 5.0, 0.0, 0.0, 0.0, 1.0);
   glScalef (1, 1, 1);      /* modeling transformation */ 

   drawFunc();

   glFlush ();
}

void reshape (int w, int h)
{
   glViewport (0, 0, (GLsizei) w, (GLsizei) h); 
   glMatrixMode (GL_PROJECTION);
   glLoadIdentity ();
   glFrustum (-1.0, 1.0, -1.0, 1.0, 1.5, 70.0);
   glMatrixMode (GL_MODELVIEW);
}

void getRandomDirection(){
   int random = rand() % 100;
   if (random < 50)
      directionX = -1;
   else
      directionX = 1;
}

void resetBoard(int mode){
   gameRunning=0;
   if (mode != 0) // Restart game
   {
      npoints[0] = 0;
      npoints[1] = 0;
   }
   ball[0] = 10.0f;
   ball[1] = 5.0f;
   getRandomDirection();
   yOne[0]=4.0f;
   yOne[1]=6.0f;
   yTwo[0]=4.0f;
   yTwo[1]=6.0f;
   ballSpeed = initialBallSpeed;
   playerSpeed = initialPlayerSpeed;
   ballVerticalSpeed=0;
}

void loop(){
   if(!gameRunning) return;
   if (keystates[0]){
      yTwo[1] += playerSpeed;
      yTwo[0] += playerSpeed;
      if(yTwo[1] > fieldYTwo[0]){
         yTwo[1] -= playerSpeed;
         yTwo[0] -= playerSpeed;
      }
   }
   if (keystates[1]){
      yTwo[1] -= playerSpeed;
      yTwo[0] -= playerSpeed;
      if(yTwo[0] < fieldYOne[1]){
         yTwo[1] += playerSpeed;
         yTwo[0] += playerSpeed;
      }
   }
   if (keystates[2]){
      yOne[1] += playerSpeed;
      yOne[0] += playerSpeed;
      if(yOne[1] > fieldYTwo[0]){
         yOne[1] -= playerSpeed;
         yOne[0] -= playerSpeed;
      }
   }
   if (keystates[3]){
      yOne[1] -= playerSpeed;
      yOne[0] -= playerSpeed;
      if(yOne[0] < fieldYOne[1]){
         yOne[1] += playerSpeed;
         yOne[0] += playerSpeed;
      }
   }
   // Direction + Colisions X + Change direction Y
   switch (directionX){
      case -1: {
         ball[0] -= ballSpeed;
         if (ball[0] - ballRadius <= xOne[1]){
            if (ball[1] > yOne[0] && ball[1] < yOne[1]){
              directionX = 1;
              ballSpeed*=ballAccel;
              playerSpeed*=playerAccel;
              changeDirectionY(-1);
            }
            else if(ball[0] - ballRadius <= xOne[0] - 1.0f){
              resetBoard(0);
              npoints[1]++;
            }
         }
         break;
      }
      case 1: {
         ball[0] += ballSpeed;
         if (ball[0] + ballRadius >= xTwo[0]){
            if (ball[1] > yTwo[0] && ball[1] < yTwo[1]){
              directionX = -1;
              ballSpeed*=ballAccel;
              playerSpeed*=playerAccel;
              changeDirectionY(1);
            }
            else if(ball[0] + ballRadius >= xTwo[1] + 1.0f){
              resetBoard(0);
              npoints[0]++;
            }
         }
      }
   }
   //End direction X + colisions X + direction Y
   //Colisions Y
   if (ball[1] + ballRadius >= fieldYTwo[0]){
      ballVerticalSpeed *=-1;
   }
   if (ball[1]-ballRadius <= fieldYOne[1]){
      ballVerticalSpeed *=-1;
   }

   //End Colisions Y
   ball[1] += ballVerticalSpeed;
   glutPostRedisplay();
}

void changeDirectionY(int side){
   //side = -1 Esquerda, side = 1 Direita
   switch (side){
      case -1: {
         if (keystates[2] == keystates[3])
            return;
         if (keystates[2])
            ballVerticalSpeed+=0.002;
         if (keystates[3])
            ballVerticalSpeed-=0.002;
      }
      case 1: {
         if (keystates[0] == keystates[1])
            return;
         if (keystates[0])
            ballVerticalSpeed+=0.002;
         if (keystates[1])
            ballVerticalSpeed-=0.002;
         break;
      }
   }
}

void drawText(char* text, float x, float y, float z){
   glRasterPos3f(x,y,z);
   int i;
   for (i=0; i<strlen(text); i++){
      glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, (int)text[i]);
   }
}
