#pragma once
#include "pch.h"
#include "framework.h"

/*LIBRARY DEFINED TYPES*/

/*Representation of byte type for gameTree byte array*/
typedef unsigned char byte;

/*Representation of Game node in constructed Game Tree*/
struct TreeGameNode {
	unsigned char _identifier;
	int8_t _playerToAct;
	uint8_t _numChildrenOrActions;
	float* _pCurrStratArr;
	float* _pCumStratArr;
	float* _pCumRegretArr;
	long _pChildStartOffset;
};

struct TreeTerminalNode {
	unsigned char _identifier;
	float utilityVal;
};

/*Representation of Chance Node in constructed Game Tree*/
struct TreeChanceNode {
	unsigned char _identifier;
	uint8_t _numChildren;
	long _pChildStartOffset;
	float* _pProbToChildArr;
};
