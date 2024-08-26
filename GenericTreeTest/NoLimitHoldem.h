#pragma once

//Define enum for street.
enum Street {
	FLOP,
	TURN,
	RIVER
};

class NLHPoker {

	



};

class Bet {

	float mBetPct;
	float mBetBB;
	bool isPct;
	bool isBB;
	bool mIsCheckorCall;
	bool mIsFold;
	
};

class PokerState {

	int mPlayerWithAction;
	//Size = 2 cards * 2 bytes (rank & suit) + 1 byte for null terminator
	Street mCurrentStreet;
	const char mHoleCards[5];
	float mPot;

	//Common state for both players.
	const char mBoard[11];
	float mEffectiveStack;
};

class PokerChance {

	Street mPreviousStreet;

	//Common State for both players.
	const char mBoard[11];
	float mPot;
};

