#ifndef CONST_H
#define CONST_H

const int SWIDTH = 768;
const int SHEIGHT = 768;
const float SWIDTHF = 768.0f;
const float SHEIGHTF = 768.0f;

enum
{
    STATE_QUIT,
    STATE_PLAY,
    STATE_MENU,
	STATE_ENDSCREEN
};

enum
{
    EID_PLAYER = 1,
    EID_BALL,
    EID_BLOCK,
	EID_GOAL
};

enum
{
	DIFF_VERYEASY,
	DIFF_EASY,
	DIFF_AVERAGE,
	DIFF_HARD,
	DIFF_VERYHARD
};

#endif
