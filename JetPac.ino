/* ToDo List
   Clear the laser array after each life
   Reset the monster positions after each life
   Make it more difficult as time goes on
 */

#define MAX_LEVELS 2
#define RANDOMXL 0-random(64)
#define RANDOMXR 128+random(64)
#define RANDOMY (float)random(48)+8
#define RANDOMXRATE 0.25+((float)random(30)/100)
#define RANDOMYRATE ((float)random(30)/100)-0.15

#include <Arduboy2.h>
#include "bitmaps.h"
#include <ArduboyTones.h>
#include <avr/pgmspace.h>

Arduboy2 arduboy;
ArduboyTones sound(arduboy.audio.enabled);

byte level = 0;
byte gameDifficulty = 2;
byte monsterType = 6;
byte gameState = 0;
int score = 0;
int highScore = 0;
int8_t lives = 4;


// Jet man data
float jetManX = 60;
float jetManY= 47;
float jetManXRate = 0;
float jetManYRate = 0;
boolean jetManDirection = 1;
byte jetManState=0;

byte cloudIndex = 0;
boolean jetPacFired = 0;
boolean buttonBPressed = false;
byte animToggle;
unsigned long frameRate;
unsigned long lastPressed;

const byte platforms[] PROGMEM = {
        2,4,3, 7,5,2, 11,3,3,
        4,2,2, 2,5,4, 9,3,3
};

struct movingObjects
{
        float x;
        float y;
        float xRate;
        float yRate;
        boolean direction;
};

movingObjects monsters[10];

struct staticObjects
{
        float x;
        float y;
        float xRate;
        float yRate;
        boolean state;
};

staticObjects rocketParts[] = {
        {75,55,0,0,1},
        {60,32,0,0,1},
        {24,24,0,0,1},
};

struct cloudObjects
{
        byte state;
        byte x;
        byte y;
        unsigned long time;
};

cloudObjects clouds[10];

struct laserObjects
{
        byte x;
        byte y;
        byte length;
        boolean direction;
        unsigned long time;
};

laserObjects laserbeams[20];
byte laserIndex = 0;


// #=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=
void setup()
{
        Serial.begin(115200);
        arduboy.boot(); // raw hardware
        arduboy.blank(); // blank the display
        arduboy.flashlight(); // light the RGB LED and screen if UP button is being held.
        arduboy.systemButtons();
        //arduboy.bootLogo();
        arduboy.audio.on();
        arduboy.clear();
        //arduboy.begin();
        arduboy.initRandomSeed();
        arduboy.setFrameRate(60);
        frameRate = millis();
        lastPressed = millis();
}

// #=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=
void createMonsters()
{
        for (int index=0; index < 9; index++)
        {
                boolean DIRECTION = (boolean)random(2);
                monsters[index] = {(DIRECTION ? RANDOMXR : RANDOMXL), RANDOMY, RANDOMXRATE, RANDOMYRATE, DIRECTION};
        }
}

// #=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=
void drawPlatforms()
{
        for (byte index = 0; index<9; index=index+3)
        {
                byte length = pgm_read_byte(&platforms[ (level*9) + (index+2) ]);
                for (byte row = 0; row < length; row++)
                {
                        byte platformX = pgm_read_byte(&platforms[(level*9)+index]);
                        platformX = (platformX*8)+(row*8);
                        byte platformY = pgm_read_byte(&platforms[(level*9)+(index+1)]);
                        platformY = platformY*8;
                        arduboy.drawBitmap(platformX, platformY, PLATFORM, 8, 8);
                }
        }
}

// #=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=
void drawJetMan()
{
        if (jetManState==0) // standing
        {

                if (jetManDirection)
                {
                        arduboy.drawBitmap(jetManX, jetManY, JETMANRIGHTHEAD, 8, 8);
                        if (jetManXRate == 0)
                                arduboy.drawBitmap(jetManX, jetManY+8, JETMANRIGHT1, 8, 8);
                        else{
                                switch(animToggle)
                                {
                                case 0:
                                        arduboy.drawBitmap(jetManX, jetManY+8, JETMANRIGHT1, 8, 8);
                                        break;
                                case 1:
                                        arduboy.drawBitmap(jetManX, jetManY+8, JETMANRIGHT2, 8, 8);
                                        break;
                                case 2:
                                        arduboy.drawBitmap(jetManX, jetManY+8, JETMANRIGHT3, 8, 8);
                                        break;
                                case 3:
                                        arduboy.drawBitmap(jetManX, jetManY+8, JETMANRIGHT2, 8, 8);
                                        break;
                                }
                        }
                }
                else
                {
                        arduboy.drawBitmap(jetManX, jetManY, JETMANLEFTHEAD, 8, 8);
                        if (jetManXRate == 0)
                                arduboy.drawBitmap(jetManX, jetManY+8, JETMANLEFT1, 8, 8);
                        else{
                                switch(animToggle)
                                {
                                case 0:
                                        arduboy.drawBitmap(jetManX, jetManY+8, JETMANLEFT1, 8, 8);
                                        break;
                                case 1:
                                        arduboy.drawBitmap(jetManX, jetManY+8, JETMANLEFT2, 8, 8);
                                        break;
                                case 2:
                                        arduboy.drawBitmap(jetManX, jetManY+8, JETMANLEFT3, 8, 8);
                                        break;
                                case 3:
                                        arduboy.drawBitmap(jetManX, jetManY+8, JETMANLEFT2, 8, 8);
                                        break;
                                }
                        }
                }
        }

        if (jetManState==1 || jetManState==2) // Rising or falling
        {
                if (jetManDirection)
                {
                        arduboy.drawBitmap(jetManX, jetManY, JETMANRIGHTHEAD, 8, 8);
                        switch(animToggle)
                        {
                        case 0:
                                arduboy.drawBitmap(jetManX, jetManY+8, JETMANRIGHTFLY1, 8, 8);
                                break;
                        case 1:
                                arduboy.drawBitmap(jetManX, jetManY+8, JETMANRIGHTFLY2, 8, 8);
                                break;
                        case 2:
                                arduboy.drawBitmap(jetManX, jetManY+8, JETMANRIGHTFLY3, 8, 8);
                                break;
                        case 3:
                                arduboy.drawBitmap(jetManX, jetManY+8, JETMANRIGHTFLY2, 8, 8);
                                break;
                        }
                }
                else
                {
                        arduboy.drawBitmap(jetManX, jetManY, JETMANLEFTHEAD, 8, 8);
                        switch(animToggle)
                        {
                        case 0:
                                arduboy.drawBitmap(jetManX, jetManY+8, JETMANLEFTFLY1, 8, 8);
                                break;
                        case 1:
                                arduboy.drawBitmap(jetManX, jetManY+8, JETMANLEFTFLY2, 8, 8);
                                break;
                        case 2:
                                arduboy.drawBitmap(jetManX, jetManY+8, JETMANLEFTFLY3, 8, 8);
                                break;
                        case 3:
                                arduboy.drawBitmap(jetManX, jetManY+8, JETMANLEFTFLY2, 8, 8);
                                break;
                        }
                }
        }
}

// #=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=
void drawThings()
{
        for (byte index = 0; index < gameDifficulty; index++)
        {
                float x = monsters[index].x;
                float y = monsters[index].y;
                boolean direction = monsters[index].direction;

                if (monsterType==0)
                {
                        if (direction==1) arduboy.drawBitmap(x, y, (animToggle%2)==0 ? ASTEROIDRIGHT1 : ASTEROIDRIGHT2, 8, 8);
                        else arduboy.drawBitmap(x, y, (animToggle%2)==0 ? ASTEROIDLEFT1 : ASTEROIDLEFT2, 8, 8);
                }

                if (monsterType==1)
                {
                        arduboy.drawBitmap(x, y, (animToggle%2)==0 ? FUZZY1 : FUZZY2, 8, 8);
                }

                if (monsterType==2)
                {
                        arduboy.drawBitmap(x, y, (animToggle%2)==0 ? BALL1 : BALL2, 8, 8);
                }

                if (monsterType==3)
                {
                        arduboy.drawBitmap(x, y, (animToggle%2)==0 ? PLANE : PLANE, 8, 8);
                }

                if (monsterType==4)
                {
                        arduboy.drawBitmap(x, y, (animToggle%2)==0 ? CROSS : CROSS, 8, 8);
                }

                if (monsterType==5)
                {
                        arduboy.drawBitmap(x, y, (animToggle%2)==0 ? FALCON : FALCON, 8, 8);
                }

                if (monsterType==6)
                {
                        arduboy.drawBitmap(x, y, (animToggle%2)==0 ? UFO1 : UFO2, 8, 8);
                }

                if (monsterType==7)
                {
                        arduboy.drawBitmap(x, y, (animToggle%2)==0 ? JELLY : JELLY, 8, 8);
                }

                drawRocket();
        }
}

// #=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=
void drawRocket()
{
        for (byte index=0; index<3; index++)
        {
                byte x = rocketParts[index].x;
                byte y = rocketParts[index].y;
                if (index==2) arduboy.drawBitmap(x, y, TOPROCKET, 8, 8);
                if (index==1) arduboy.drawBitmap(x, y, MIDDLEROCKET, 8, 8);
                if (index==0) arduboy.drawBitmap(x, y, BOTTOMROCKET, 8, 8);
        }
}

// #=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=
void moveThings()
{
        for (byte index = 0; index < gameDifficulty; index++)
        {
                boolean direction = monsters[index].direction;
                float xRate = monsters[index].xRate;
                float yRate = monsters[index].yRate;

                monsters[index].x = monsters[index].x + (direction ? +xRate : -xRate);
                monsters[index].y = monsters[index].y + yRate;

                if ((monsters[index].x > 127) || (monsters[index].x < -7.0))
                {
                        monsters[index].x = direction ? -7 : 127;
                        monsters[index].xRate = 0.25+((float)random(30)/100);
                }
                if ((monsters[index].y > 57) || (monsters[index].y < 7))
                {
                        monsters[index].yRate = -(monsters[index].yRate);
                }


        }
}

// #=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=
void moveJetMan()
{
        jetManX = jetManX + (jetManDirection ? +jetManXRate : -jetManXRate);
        jetManY = jetManY + jetManYRate;

        if (jetManX > 119)
        {
                jetManXRate = 0;
                jetManX = 119;

        }
        if (jetManX < 0)
        {
                jetManXRate = 0;
                jetManX = 0;
        }

        if (jetManY <0)
        {
                jetManYRate = 0;
                jetManY = 0;
        }

        if (jetManY >47)
        {
                jetManYRate = 0;
                jetManY = 47;
                if (jetManState == 2)
                {
                        jetManState = 0;   // Standing
                }
        }


        for (byte index = 0; index<9; index=index+3)
        {
                byte length = pgm_read_byte(&platforms[ (level*9) + (index+2) ]);
                byte platformX = pgm_read_byte(&platforms[ (level*9) + index] );
                platformX = (platformX*8);
                byte platformY = pgm_read_byte(&platforms[ (level*9) + (index+1)] );
                platformY = platformY*8;
                byte platformEnd = platformX+(8*length);

                if (((jetManState == 0) && (jetManY<47)) && ((jetManX+6<platformX) || (jetManX>platformEnd)) )
                {
                        jetManState = 2;
                        jetManYRate = 0.5;
                }

                if ((((jetManX+6)>=platformX) && ((jetManX+4)<=platformEnd)) && (jetManY==platformY-16))
                {
                        jetManState = 0;
                        jetManY = platformY-16;
                        jetManXRate = 0;
                        jetManYRate = 0;
                        break;
                }

        }

}

// #=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=
void jetPacFire()
{

        jetManState = 1; // rising;
        jetManYRate = -1;

        if (jetPacFired == false)
        {
                clouds[cloudIndex].state = 1;
                clouds[cloudIndex].x = jetManX;
                clouds[cloudIndex].y = jetManY+8;
                clouds[cloudIndex].time = millis();
                cloudIndex++;
                if (cloudIndex>9) cloudIndex=0;
        }
        jetPacFired = true;
}

// #=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=
void jetPacRelease()
{
        if (jetManState == 1)
        {
                jetManState = 2; // falling;
                jetManYRate = 0.5;
        }
        jetPacFired = false;
}

// #=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=
void checkButtons()
{
        if( arduboy.pressed(A_BUTTON) == true || arduboy.pressed(UP_BUTTON) == true) {
                jetPacFire();
        }

        if( arduboy.pressed(A_BUTTON) == false && arduboy.pressed(UP_BUTTON) == false) {
                jetPacRelease();
        }

        if( arduboy.notPressed(RIGHT_BUTTON) == true ) {
                jetManXRate = 0;
        }

        if( arduboy.notPressed(LEFT_BUTTON) == true ) {
                jetManXRate = 0;
        }

        if( arduboy.pressed(RIGHT_BUTTON) == true ) {
                jetManXRate = 1;
                jetManDirection = 1;
        }

        if( arduboy.pressed(LEFT_BUTTON) == true ) {
                jetManXRate = 1;
                jetManDirection = 0;
        }

        if((arduboy.pressed(B_BUTTON) == true || arduboy.pressed(DOWN_BUTTON) == true) && ((millis()-lastPressed)>20) && buttonBPressed==false)
        {
                laserbeams[laserIndex].x = jetManX + (jetManDirection ? 10 : -10);
                laserbeams[laserIndex].length = 1;
                laserbeams[laserIndex].y = jetManY+9;
                laserbeams[laserIndex].direction = jetManDirection;
                laserbeams[laserIndex].time = millis();
                laserIndex++;
                if (laserIndex>19) laserIndex=0;
                buttonBPressed = true;
                for (int start = 50; start < 175; start = start + 1) {
                        digitalWrite(2, HIGH); //positive square wave
                        digitalWrite(5, LOW); //positive square wavedelayMicroseconds(start);      //192uS
                        delayMicroseconds(start);
                        digitalWrite(2, LOW); //neutral square wave
                        digitalWrite(5, HIGH); //positive square wave
                        delayMicroseconds(start); //192uS
                }

        }

        if(arduboy.notPressed(B_BUTTON) == true && arduboy.notPressed(DOWN_BUTTON) == true)
        {
                buttonBPressed = false;
        }

}

// #=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=
void drawClouds()
{
        for (byte index=0; index<9; index++)
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

// #=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=
void drawlasers()
{
        for (byte index = 0; index < 20; index++)
        {
                if(laserbeams[index].length != 0)
                {
                        byte beamLength = laserbeams[index].length;
                        if (beamLength>8) beamLength = 8;
                        for (byte multiplier=0; multiplier<beamLength; multiplier++)
                        {
                                if ((laserbeams[index].length>4) && multiplier<4) arduboy.drawBitmap(laserbeams[index].x+((8*multiplier)*(laserbeams[index].direction ? 1 : -1)),laserbeams[index].y, LASER2, 8, 8);
                                else arduboy.drawBitmap(laserbeams[index].x+((8*multiplier)*(laserbeams[index].direction ? 1 : -1)),laserbeams[index].y, LASER1, 8, 8);
                                checkForHit(laserbeams[index].x+((8*multiplier)*(laserbeams[index].direction ? 1 : -1)), laserbeams[index].y);
                        }

                        if ((millis()-laserbeams[index].time)>20)
                        {
                                laserbeams[index].length +=1;
                                if (laserbeams[index].length>8) laserbeams[index].x +=(8*(laserbeams[index].direction ? 1 : -1));
                                if (laserbeams[index].length>16) laserbeams[index].length = 0;
                                laserbeams[index].time = millis();
                        }
                }
        }
}

// #=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=
void checkForHit(byte laserX, byte laserY)
{
        for (byte index=0; index<gameDifficulty; index++)
        {
                boolean hit=false;
                byte x = monsters[index].x;
                byte y = monsters[index].y;
                boolean direction = monsters[index].direction;

                if ((laserY >= y && laserY <=y+7) && (laserX>=x && laserX<=x+7))
                {
                        hit = true;
                }

                if (hit==true)
                {

                        clouds[cloudIndex].state = 1;

                        explosionFX();

                        clouds[cloudIndex].x = x;
                        clouds[cloudIndex].y = y;
                        clouds[cloudIndex].time = millis();
                        cloudIndex++;
                        if (cloudIndex>9) cloudIndex=0;
                        score=score+25;

                        boolean DIRECTION = (boolean)random(2);
                        monsters[index] = {DIRECTION ? RANDOMXR : RANDOMXL, RANDOMY,0.25+((float)random(30)/100),((float)random(30)/100)-0.15,DIRECTION};
                }
        }
}



// #=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=
void checkJetManHit()
{
        for (byte index=0; index<gameDifficulty; index++)
        {
                boolean hit=false;
                byte x = monsters[index].x;
                byte y = monsters[index].y;
                byte manX = jetManX;
                byte manY = jetManY;
                boolean direction = monsters[index].direction;

                if (((x>=manX+1 && x<=manX+6) && (y>=manY+1 && y<=manY+14)) || ((x+7>=manX+1 && x+7<=manX+6) && (y+7>=manY+1 && y+7<=manY+14)))
                {
                        hit = true;
                }

                if (hit==true)
                {

                        explosionFX();
                        explosionFX();
                        explosionFX();
                        explosionFX();

                        clouds[cloudIndex].state = 1;
                        clouds[cloudIndex].x = manX;
                        clouds[cloudIndex].y = manY+4;
                        clouds[cloudIndex].time = millis();
                        cloudIndex++;
                        if (cloudIndex>9) cloudIndex=0;

                        clouds[cloudIndex].state = 1;
                        clouds[cloudIndex].x = manX-4;
                        clouds[cloudIndex].y = manY;
                        clouds[cloudIndex].time = millis();
                        cloudIndex++;
                        if (cloudIndex>9) cloudIndex=0;

                        clouds[cloudIndex].state = 1;
                        clouds[cloudIndex].x = manX+4;
                        clouds[cloudIndex].y = manY;
                        clouds[cloudIndex].time = millis();
                        cloudIndex++;
                        if (cloudIndex>9) cloudIndex=0;

                        gameState=0;
                        lives=lives-1;
                        if (lives<0) gameState = 2;
                        sound.tone(800,200, 600, 200, 200, 400);
                        for (int x=0; x<300; x++)
                        {
                                arduboy.clear();
                                arduboy.drawFastHLine(0, 63, 128); // ground
                                drawPlatforms();
                                drawThings();
                                drawClouds();
                                drawlasers();
                                Serial.write(arduboy.getBuffer(), 128 * 64 / 8);
                                arduboy.display();
                        }
                }
        }
}

// #=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=
void explosionFX()
{
        // Explosion sound
        for (int start = 0; start < 50; start = start + 1) {
                digitalWrite(2, HIGH);
                digitalWrite(5, LOW);
                delayMicroseconds(random(100));
                digitalWrite(2, LOW);
                digitalWrite(5, HIGH);
                delayMicroseconds(random(100));
        }
}

// #=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=
void gameOn()
{
        arduboy.drawFastHLine(0, 63, 128); // ground

        drawPlatforms();
        drawThings();
        drawJetMan();
        drawClouds();
        drawlasers();
        checkJetManHit();

        checkButtons();
        moveJetMan();
        moveThings();

        if ((millis()-frameRate)>50)
        {
                animToggle++;
                if (animToggle>3) animToggle=0;
                frameRate=millis();
        }
}

// #=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=
void gameOver()
{
        arduboy.clear();
        arduboy.setTextSize(2);
        arduboy.setCursor(8, 24);
        arduboy.print("GAME OVER ");
        arduboy.setTextSize(1);
        arduboy.setCursor(30, 44);
        arduboy.print("Score: ");
        arduboy.print(score);
        arduboy.display();
        Serial.write(arduboy.getBuffer(), 128 * 64 / 8);
        delay(10000);
        lives = 4;
        score = 0;
        gameState = 0;
}

// #=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=
void countdown()
{
        for (int x=3; x>0; x=x-1)
        {
                arduboy.clear();
                arduboy.setCursor(30,4);
                arduboy.setTextSize(1);
                arduboy.print("Score: ");
                arduboy.print(score);

                arduboy.drawBitmap(60,24, LITTLEMAN, 8, 8);
                arduboy.setCursor(70, 24);
                arduboy.print(lives);

                arduboy.setTextSize(2);
                arduboy.setCursor(22, 50);
                arduboy.print("READY ");
                arduboy.print(x);
                sound.tone(2000, 100);
                arduboy.display();
                Serial.write(arduboy.getBuffer(), 128 * 64 / 8);
                delay(1000);
        }
        initialiseGameState();
        createMonsters();
}

// #=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=
void initialiseGameState()
{
        gameState = 1;
        jetManX=60;
        jetManY=47;
        jetManDirection=1;
        jetManXRate = 0;
        jetManYRate = 0;

        for (int index=0; index<9; index++)
        {
                clouds[index] = {0,0,0,0};
        }
        for (int index=0; index<19; index++)
        {
                laserbeams[index] = {0,0,0,0,0};
        }

}

// #=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=
void loop() {
        if (!arduboy.nextFrame()) return;  // Keep frame rate at 60fps
        arduboy.clear();

        switch(gameState)
        {
        case 0:
                countdown();
                break;
        case 1:
                gameOn();
                break;
        case 2:
                gameOver();
                break;
        }

        Serial.write(arduboy.getBuffer(), 128 * 64 / 8);
        arduboy.display();
}
