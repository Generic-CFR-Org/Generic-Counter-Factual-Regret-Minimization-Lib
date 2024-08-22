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
	bool mIsCheck;
	bool mIsFold;
	


};

class CommonPokerState {

	//Size = 3 cards * 2 bytes (rank & suit) + 1 byte for null terminator
	const char mFlop[7];
	float mStartingPot;
	float mEffectiveStack;

};

class GameState {

	int mPlayerWithAction;
	//Size = 2 cards * 2 bytes (rank & suit) + 1 byte for null terminator
	Street mCurrentStreet;
	const char mHoleCards[5];
	float mPot;
};

class ChanceNode {

	Street mPreviousStreet;

};

class TerminalNode {

	bool mPlayerOneFolded;
	bool mPlayerTwoFolded;
	float mPot;
	//Size = 5 cards * 2 bytes (rank & suit) + 1 byte for null terminator
	const char mBoard[11];
	//Size = 2 cards * 2 bytes (rank & suit) + 1 byte for null terminator
	const char mPlayerOneCards[5];
	const char mPlayerTwoCards[5];

};
