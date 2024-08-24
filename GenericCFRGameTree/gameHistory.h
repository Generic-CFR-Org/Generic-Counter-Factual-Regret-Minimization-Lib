#pragma once
#include "pch.h"
#include "framework.h"

class historyHasNoGameNode : public std::exception {
public:
	const char* what() { return "History Node has no Game Node"; }
};

class historyHasNoChanceNode : public std::exception {
public: 
	const char* what() { return "History Node has no Chance Node"; }
};

template<typename GameState, typename ChanceNode, typename Action>
class HistoryNode {

	bool mIsGameNode;
	bool mIsChanceNode;
	GameState mGameState;
	ChanceNode mChanceNode;
	Action *mpActionTaken;

public:

	typedef std::pair<GameState, Action*> GameHistoryNode;

	HistoryNode() {
		mIsGameState, mIsChanceNode, mIsAction = false;
		mGameState();
		mChanceNode();
		mpActionTaken = nullptr;
	}

	HistoryNode(GameState gameState, Action* actionTaken) {
		mIsChanceNode = false;
		mIsGameNode = true;
		mChanceNode();
		mGameState = gameState;
		mpActionTaken = actionTaken;
	}

	HistoryNode(ChanceNode chanceNode) {
		mIsGameNode = false;
		mIsChanceNode = true;
		mGameState();
		mpActionTaken = nullptr;
		mChanceNode = chanceNode;
	}


	bool IsGameNode() { return mIsGameNode; }
	bool IsChanecNode() { return mIsChanceNode; }

	GameHistoryNode GetGameNode() {
		if (!mIsGameNode) {
			historyHasNoGameNode e = historyHasNoGameNode();
			std::cout << e.what();
			throw e;
		}
		return gameHistoryNode(mGamestate, mpActionTaken); 
	}
	ChanceNode GetChanceNode() { 
		if (!mIsChanceNode) {
			historyHasNoChanceNode e = historyHasNoChanceNode();
			std::cout << e.what();
			throw e;
		}
		return mChanceNode; 
	}
};


template<typename GameState, typename ChanceNode, typename Action>
class History {

public:
	std::vector<*HistoryNode<GameState, ChanceNode, Action>> *mHistoryList;
	History() {historyList = new std::vector<historyNode<GameState, ChanceNode, Action>>}
	~History() {
		auto iHistory = IterBegin();
		auto iEnd = IterEnd();
		for (iHistory; iHistory != iEnd; iHistory++) {
			delete iHistory;
		}
		delete mHistoryList;
	}
	typename std::vector<*HistoryNode<GameState, ChanceNode, Action>>::iterator  IterBegin() {
		return mHistoryList->begin();
	}

	typename std::vector<*HistoryNode<GameState, ChanceNode, Action>>::iterator  IterEnd() {
		return mHistoryList->end();
	}

	History* AddToHistory(GameState gameState, Action* actionTaken) {
		HistoryNode *historyNode = new HistoryNode(gameState, actionTaken);
		mHistoryList->push_back(historyNode);
	}

	History* AddToHistory(ChanceNode chanceNode) {
		HistoryNode* historyNode = new HistoryNode(chanceNode);
		mHistoryList->push_back(historyNode);
	}

};