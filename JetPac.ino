#define MAX_LEVELS 2
#define RANDOMXL 0-random(64)
#define RANDOMXR 128+random(64)
#define RANDOMY random(56)+8
#define NUMBEROFOBJECTS 8

#include <Arduboy2.h>
#include "bitmaps.h"

Arduboy2 arduboy;

int level = 0;
int cloudIndex=0;
boolean jetPacFired = 0;
byte animToggle;
unsigned long frameRate;
byte jetManState=0;

byte platforms[] = {
        2,4,3, 7,5,2, 11,3,3,
        4,2,2, 2,5,4, 9,3,3
};

struct movingObjects
{
        byte type;
        float x;
        float y;
        float xRate;
        float yRate;
        boolean direction;
};

movingObjects gameObjects[10];

void createObjects()
{
gameObjects[0] = {1,60,47,0,0,1};
gameObjects[1] = {12,75,55,0,0,1};
gameObjects[2] = {11,60,32,0,0,1};
gameObjects[3] = {10,24,24,0,0,1};
gameObjects[4] = {22,(float)RANDOMXR,(float)RANDOMY,0.3,0.05,0};
gameObjects[5] = {22,(float)RANDOMXR,(float)RANDOMY,0.3,0.05,0};
gameObjects[6] = {22,(float)RANDOMXL,(float)RANDOMY,0.3,0.05,1};
gameObjects[7] = {2,96,-8,0,0,0};
}

struct cloudObjects
{
        byte state;
        byte x;
        byte y;
        unsigned long time;
};

cloudObjects clouds[10];

void setup()
{
        Serial.begin(115200);
        arduboy.boot(); // raw hardware
        arduboy.blank(); // blank the display
        arduboy.flashlight(); // light the RGB LED and screen if UP button is being held.
        arduboy.systemButtons();
        //arduboy.bootLogo();
        arduboy.audio.begin();
        arduboy.clear();
        //arduboy.begin();
        arduboy.initRandomSeed();
        arduboy.setFrameRate(60);
        frameRate = millis();
        createObjects();
}

void drawPlatforms()
{
        for (byte index = 0; index<9; index=index+3)
        {
                byte length = platforms[ (level*9) + (index+2) ];
                for (byte row = 0; row < length; row++)
                {
                        arduboy.drawBitmap(
                                (platforms[(level*9)+index]*8)+(row*8),
                                platforms[(level*9)+(index+1)]*8,
                                PLATFORM, 8, 8);
                }
        }
}

void drawJetMan(byte x, byte y, boolean direction)
{
        if (jetManState==0) // standing
        {

                if (direction)
                {
                        arduboy.drawBitmap(x, y, JETMANRIGHTHEAD, 8, 8);
                        if (gameObjects[0].xRate==0)
                                arduboy.drawBitmap(x, y+8, JETMANRIGHT1, 8, 8);
                        else{
                                switch(animToggle)
                                {
                                case 0:
                                        arduboy.drawBitmap(x, y+8, JETMANRIGHT1, 8, 8);
                                        break;
                                case 1:
                                        arduboy.drawBitmap(x, y+8, JETMANRIGHT2, 8, 8);
                                        break;
                                case 2:
                                        arduboy.drawBitmap(x, y+8, JETMANRIGHT3, 8, 8);
                                        break;
                                case 3:
                                        arduboy.drawBitmap(x, y+8, JETMANRIGHT2, 8, 8);
                                        break;
                                }
                        }
                }
                else
                {
                        arduboy.drawBitmap(x, y, JETMANLEFTHEAD, 8, 8);
                        if (gameObjects[0].xRate==0)
                                arduboy.drawBitmap(x, y+8, JETMANLEFT1, 8, 8);
                        else{
                                switch(animToggle)
                                {
                                case 0:
                                        arduboy.drawBitmap(x, y+8, JETMANLEFT1, 8, 8);
                                        break;
                                case 1:
                                        arduboy.drawBitmap(x, y+8, JETMANLEFT2, 8, 8);
                                        break;
                                case 2:
                                        arduboy.drawBitmap(x, y+8, JETMANLEFT3, 8, 8);
                                        break;
                                case 3:
                                        arduboy.drawBitmap(x, y+8, JETMANLEFT2, 8, 8);
                                        break;
                                }
                        }
                }
        }

        if (jetManState==1 || jetManState==2) // Rising or falling
        {
                if (direction)
                {
                        arduboy.drawBitmap(x, y, JETMANRIGHTHEAD, 8, 8);
                        switch(animToggle)
                        {
                        case 0:
                                arduboy.drawBitmap(x, y+8, JETMANRIGHTFLY1, 8, 8);
                                break;
                        case 1:
                                arduboy.drawBitmap(x, y+8, JETMANRIGHTFLY2, 8, 8);
                                break;
                        case 2:
                                arduboy.drawBitmap(x, y+8, JETMANRIGHTFLY3, 8, 8);
                                break;
                        case 3:
                                arduboy.drawBitmap(x, y+8, JETMANRIGHTFLY2, 8, 8);
                                break;
                        }
                }
                else
                {
                        arduboy.drawBitmap(x, y, JETMANLEFTHEAD, 8, 8);
                        switch(animToggle)
                        {
                        case 0:
                                arduboy.drawBitmap(x, y+8, JETMANLEFTFLY1, 8, 8);
                                break;
                        case 1:
                                arduboy.drawBitmap(x, y+8, JETMANLEFTFLY2, 8, 8);
                                break;
                        case 2:
                                arduboy.drawBitmap(x, y+8, JETMANLEFTFLY3, 8, 8);
                                break;
                        case 3:
                                arduboy.drawBitmap(x, y+8, JETMANLEFTFLY2, 8, 8);
                                break;
                        }
                }
        }
}

void drawThings()
{
        for (int index=0; index<NUMBEROFOBJECTS; index++)
        {
                /*  0=type
                    1 = x
                    2 = y
                    3 = xrate
                    4 = yRate
                    5 = direction
                    6 = state
                 */
                byte type = gameObjects[index].type;
                float x = gameObjects[index].x;
                float y = gameObjects[index].y;
                boolean direction = gameObjects[index].direction;

                if (type==1) drawJetMan(x, y, direction);
                if (type==2) arduboy.drawBitmap(x, y, FUEL, 8, 8);
                if (type==10) arduboy.drawBitmap(x, y, TOPROCKET, 8, 8);
                if (type==11) arduboy.drawBitmap(x, y, MIDDLEROCKET, 8, 8);
                if (type==12) arduboy.drawBitmap(x, y, BOTTOMROCKET, 8, 8);
                if (type==22)
                {
                        if (direction==1) arduboy.drawBitmap(x, y, (animToggle%2)==0 ? ASTEROIDRIGHT1 : ASTEROIDRIGHT2, 8, 8);
                        else arduboy.drawBitmap(x, y, (animToggle%2)==0 ? ASTEROIDLEFT1 : ASTEROIDLEFT2, 8, 8);
                }
        }
}

void moveThings()
{
        for (int index=0; index<NUMBEROFOBJECTS; index++)
        {
                /*  0=type
                    1 = x
                    2 = y
                    3 = xrate
                    4 = yRate
                    5 = direction
                    6 = state
                 */

                boolean direction = gameObjects[index].direction;
                float xRate = gameObjects[index].xRate;
                float yRate = gameObjects[index].yRate;

                gameObjects[index].x = gameObjects[index].x + (direction ? +xRate : -xRate);
                gameObjects[index].y = gameObjects[index].y + yRate;

                if ((gameObjects[index].type >= 22) && (gameObjects[index].type<=29)) // Move the monsters
                {
                        if (((gameObjects[index].x > 127) || (gameObjects[index].x < -7.0)) || ((gameObjects[index].y > 63) || (gameObjects[index].y < -7.0)))
                        {
                                gameObjects[index].x = direction ? -7 : 127;
                                gameObjects[index].y = RANDOMY;
                                gameObjects[index].xRate = 0.25+((float)random(30)/100);
                                gameObjects[index].yRate = ((float)random(30)/100)-0.15;
                        }
                }

                if (gameObjects[index].type == 1)
                {
                        if (gameObjects[index].x > 119)
                        {
                                gameObjects[index].xRate = 0;
                                gameObjects[index].x = 119;

                        }
                        if (gameObjects[index].x < 0)
                        {
                                gameObjects[index].xRate = 0;
                                gameObjects[index].x = 0;
                        }

                        if (gameObjects[index].y <0)
                        {
                                gameObjects[index].yRate = 0;
                                gameObjects[index].y = 0;
                        }

                        if (gameObjects[index].y >47)
                        {
                                gameObjects[index].yRate = 0;
                                gameObjects[index].y = 47;
                                if (jetManState == 2)
                                {
                                        jetManState = 0; // Standing
                                }
                        }

                }
        }
}

void jetPacFire()
{

        jetManState = 1; // rising;
        gameObjects[0].yRate = -1;

        if (jetPacFired == false)
        {
                clouds[cloudIndex].state = 1;
                clouds[cloudIndex].x = gameObjects[0].x;
                clouds[cloudIndex].y = gameObjects[0].y+8;
                clouds[cloudIndex].time = millis();
                cloudIndex++;
                if (cloudIndex>9) cloudIndex=0;
        }
        jetPacFired = true;
}

void jetPacRelease()
{
        if (jetManState == 1) jetManState = 2; // falling;
        gameObjects[0].yRate = 0.5;
        jetPacFired = false;
}

void checkButtons()
{
        if( arduboy.pressed(A_BUTTON) == true ) {
                jetPacFire();
        }

        if( arduboy.pressed(A_BUTTON) == false ) {
                jetPacRelease();
        }

        if( arduboy.notPressed(RIGHT_BUTTON) == true ) {
                gameObjects[0].xRate = 0;
        }

        if( arduboy.notPressed(LEFT_BUTTON) == true ) {
                gameObjects[0].xRate = 0;
        }

        if( arduboy.pressed(RIGHT_BUTTON) == true ) {
                gameObjects[0].xRate = 1;
                gameObjects[0].direction = 1;
        }

        if( arduboy.pressed(LEFT_BUTTON) == true ) {
                gameObjects[0].xRate = 1;
                gameObjects[0].direction = 0;
        }
}

void drawClouds()
{
        for (int index=0; index<9; index++)
        {
                if (clouds[index].state >0)
                {
                        switch(clouds[index].state)
                        {
                        case 1:
                                arduboy.drawBitmap(clouds[index].x,clouds[index].y, CLOUD1, 8, 8);
                                break;
                        case 2:
                                arduboy.drawBitmap(clouds[index].x-4,clouds[index].y-4, CLOUD2, 16, 16);
                                break;
                        case 3:
                                arduboy.drawBitmap(clouds[index].x-4,clouds[index].y-4, CLOUD3, 16, 16);
                                break;
                        }
                        if ((millis()-clouds[index].time)>100)
                        {
                                clouds[index].state++;
                                if (clouds[index].state>3) clouds[index].state = 0;
                                clouds[index].time = millis();
                        }
                }
        }
}

void loop() {
        if (!arduboy.nextFrame()) return;  // Keep frame rate at 60fps
        arduboy.clear();
        arduboy.drawFastHLine(0, 63, 128); // ground
        drawPlatforms();
        drawThings();
        drawClouds();
        moveThings();
        checkButtons();

        if ((millis()-frameRate)>50)
        {
                animToggle++;
                if (animToggle>3) animToggle=0;
                frameRate=millis();
        }

        Serial.write(arduboy.getBuffer(), 128 * 64 / 8);
        arduboy.display();
}
