#pragma once
#include "Deck.h"
#include "Card.h"
#include "vector"
#include <genericCFRGameTree.h>
#include "EvaluatorLibrary.h"


//Define enum for street.
enum Street {
	FLOP,
	TURN,
	RIVER
};

typedef std::vector<Card*> PlayerCards;
typedef std::vector<Card*> CardList;
typedef std::vector<Bet*> Strategy;
typedef std::vector<PlayerCards*> Range;



class NLHPoker {
public:
	Strategy mAllPossibleBets;
	Bet* mRaiseA;
	Bet* mRaiseB;
	Bet* mCheckOrCall;
	Bet* mFold;
	std::vector<Strategy> mAllStrategies;
	Range mPlayerOneRange;
	Range mPlayerTwoRange;
	CardList mFlop;
	float mEffectiveStack;
	float mStartingPot;
	int mMaxBetDepth;
	
	NLHPoker(CardList board, float effStack, float startingPot, int maxDepth);
	CardList PossibleCards(PlayerCards playerCards, CardList drawnCards);
	Range PlayerRange(bool isPlayerOne);
};
 
class PokerState {
public:
	bool mPlayerOneHasAction;
	//Size = 2 cards * 2 bytes (rank & suit) + 1 byte for null terminator
	PlayerCards* mpPlayerCards;
	float mPot;
	int mBetDepth;

	std::vector<Card*> mTurnAndRiver;

	PokerState(int player, PlayerCards *pPlayerCards, float pot, int betDepth, std::vector<Card*> turnAndRiver);
	/*PokerState(const PokerState& other);*/

};

class PokerChance {
public:

	//Common State for both players.
	CardList mPotentialCards;
	float mPot;

	PokerChance(CardList potentialCards, float pot);
};


class Bet {
public:
	float betAmt;

	bool mIsPct;
	bool mIsBB;
	bool mIsCheckOrCall;

	bool mIsFold;

	Bet(float betPct, bool isBetPct);
	Bet(float betPct, bool isBetPct);
	Bet(bool isCheckorCall);
};

static float newPotSize(Bet* bet, float oldPot, float effectiveStack) {
	
	//Bet Cannot surpass effective stack.

	if (bet->mIsFold) {
		return oldPot;
	}
	if (bet->mIsPct) {
		return std::min(oldPot *= (1.0 + bet->betAmt), effectiveStack);
	}
	if (bet->mIsBB) {
		return std::min(oldPot += bet->betAmt, effectiveStack);
	}
	if (bet->mIsCheckOrCall) {
		return std::min(oldPot += bet->betAmt, effectiveStack);
	}
}


using GameHistoryNode = HistoryNode<PokerState, PokerChance, Bet>;
using History = std::vector<GameHistoryNode>;

using GameNodeChildren = ChildrenFromGameNode<PokerState, PokerChance, Bet>;
using ChanceNodeChildren = ChildrenFromChanceNode<PokerState, Bet>;

using BetList = std::vector<Bet*>;

Range PossibleRange(Range playerRange, CardList possibleCards);

static bool PlayerOneHasAction(PokerState state, NLHPoker* pokerGame) {
	return state.mPlayerOneHasAction;
}

static float ShowdownUtility(History history, NLHPoker* pokerGame) {

	int lastNodeIndex = history.size() - 1;
	int secondLastNodeIndex = history.size() - 2;

	GameHistoryNode lastNode = history[lastNodeIndex];
	GameHistoryNode secondLastNode = history[secondLastNodeIndex];

	PokerState lastState = lastNode.GetGameState();
	PokerState secondLastState = secondLastNode.GetGameState();

	float preActionPot = lastState.mPot;
	Bet* finalBet = lastNode.GetAction();
	float finalPotSize = newPotSize(finalBet, preActionPot, pokerGame->mEffectiveStack);
	
	std::string flopStr = CardArrToString(pokerGame->mFlop);
	std::string restOfBoardStr = CardArrToString(lastState.mTurnAndRiver);
	std::string boardStr = flopStr + restOfBoardStr;

	PlayerCards *playerOneCards;
	PlayerCards *playerTwoCards;
	if (PlayerOneHasAction(lastState)) {
		playerOneCards = lastState.mpPlayerCards;
		playerTwoCards = secondLastState.mpPlayerCards;
	}
	else {
		playerOneCards = secondLastState.mpPlayerCards;
		playerTwoCards = lastState.mpPlayerCards;
	}
	std::string playerOneCardStr = CardArrToString(*playerOneCards);
	std::string playerTwoCardStr = CardArrToString(*playerTwoCards);
	int winnerMultiplier = Get_Winner(playerOneCardStr, playerTwoCardStr, boardStr);
	return winnerMultiplier * finalPotSize;
}

static GameNodeChildren* childrenFromGame(PokerState state, BetList betStrat, NLHPoker* pokerGame) {

	GameNodeChildren* allChildren;
	bool playerOneHasAction = PlayerOneHasAction(state, pokerGame);

	//Create a chance node for next street.
	if (state.mBetDepth == pokerGame->mMaxBetDepth) {
		CardList nextPossibleCards = pokerGame->PossibleCards(*state.mpPlayerCards, state.mTurnAndRiver);
		float currPot = state.mPot;
		PokerChance nextStreetNode = PokerChance(nextPossibleCards, currPot);
		allChildren->AddChildChanceNode(nextStreetNode, betStrat);
	}
	else {

		Street currStreet;
		if (state.mTurnAndRiver[0] == nullptr) {
			currStreet = FLOP;
		}
		else if (state.mTurnAndRiver[1] == nullptr) {
			currStreet = TURN;
		}
		else {
			currStreet = RIVER;
		}
		for (Bet* bet : betStrat) {
			BetList betAsList = BetList(1, bet);

			/*Moving to next street.
				- If player calls a bet, move on to the next street or showdown.
				- If In position player has action and checks, move on to the next street or showdown.
			*/
			bool isBetCall = bet->mIsCheckOrCall && bet->betAmt > 0;
			bool isBetCheck = bet->mIsCheckOrCall && bet->betAmt == 0;
			bool moveToNextStreet = isBetCall || (!playerOneHasAction && isBetCheck);
			if (moveToNextStreet) {
				
				if (currStreet == RIVER) {
					//If Street is River, create terminal node.
					allChildren->AddChildTerminalNode(betAsList);
				}
				else {
					//Create a chance node for next street.
					CardList nextPossibleCards = pokerGame->PossibleCards(*state.mpPlayerCards, state.mTurnAndRiver);
					float nextPot = newPotSize(bet, state.mPot, pokerGame->mEffectiveStack);
					PokerChance nextStreetNode = PokerChance(nextPossibleCards, nextPot);
					allChildren->AddChildChanceNode(nextStreetNode, betAsList);
				}
			}
			/*At maximum bet depth.
				- Action now on other player who can only call or fold.
			*/
			else if (state.mBetDepth == pokerGame->mMaxBetDepth) {
				BetList nextPlayerStrat;
				nextPlayerStrat.push_back(pokerGame->mCheckOrCall);
				nextPlayerStrat.push_back(pokerGame->mFold);
				float nextPot = newPotSize(bet, state.mPot, pokerGame->mEffectiveStack);
				CardList nextPossibleCards = pokerGame->PossibleCards(*state.mpPlayerCards, state.mTurnAndRiver);
				Range nextPlayerRange = PossibleRange(pokerGame->PlayerRange(!playerOneHasAction), nextPossibleCards);
				for (PlayerCards* hand : nextPlayerRange) {
					PokerState newState(!playerOneHasAction, hand, nextPot, state.mBetDepth, state.mTurnAndRiver);
					allChildren->AddChildGameNode(newState, nextPlayerStrat, betAsList);
				}
			}
			
			


			//If player is at maximum bet depth, opponent can only call / fold.


		}

	}

}


