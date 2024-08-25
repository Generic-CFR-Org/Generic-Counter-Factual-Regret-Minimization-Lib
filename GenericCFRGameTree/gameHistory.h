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

	typedef std::pair<GameState, Action*> GameNodeHistoryNode;

	HistoryNode() {
		mIsGameNode, mIsChanceNode = false;
		mGameState = GameState{};
		mChanceNode = ChanceNode{};
		mpActionTaken = nullptr;
	}

	HistoryNode(GameState gameState, Action* actionTaken) {
		mIsChanceNode = false;
		mIsGameNode = true;
		mChanceNode = ChanceNode{};
		mGameState = GameState{gameState};
		mpActionTaken = actionTaken;
	}

	HistoryNode(ChanceNode chanceNode) {
		mIsGameNode = false;
		mIsChanceNode = true;
		mGameState = GameState{};
		mpActionTaken = nullptr;
		mChanceNode = ChanceNode{ chanceNode };
	}



	bool IsGameNode() { return mIsGameNode; }
	bool IsChanecNode() { return mIsChanceNode; }

	GameNodeHistoryNode GetGameNode() {
		if (!mIsGameNode) {
			historyHasNoGameNode e = historyHasNoGameNode();
			std::cout << e.what();
			throw e;
		}
		return GameNodeHistoryNode(mGameState, mpActionTaken);
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
	std::vector<HistoryNode<GameState, ChanceNode, Action>> *mHistoryList;
	History() { mHistoryList = new std::vector<HistoryNode<GameState, ChanceNode, Action>>; }
	History(std::vector<HistoryNode<GameState, ChanceNode, Action>> oldHistory) {
		mHistoryList = new std::vector<HistoryNode<GameState, ChanceNode, Action>>(oldHistory);
	}
	~History() {
		/*auto iHistory = IterBegin();
		auto iEnd = IterEnd();
		for (iHistory; iHistory != iEnd; iHistory++) {
			delete *iHistory;
		}*/
		try {
			delete mHistoryList;

		}
		catch (...) {
			std::cout << "couldn't delete history"; 
		}
	}
	typename std::vector<HistoryNode<GameState, ChanceNode, Action>>::iterator  IterBegin() {
		return mHistoryList->begin();
	}

	typename std::vector<HistoryNode<GameState, ChanceNode, Action>>::iterator  IterEnd() {
		return mHistoryList->end();
	}

	History* AddToNewHistory(GameState gameState, Action* actionTaken) {
		History<GameState, ChanceNode, Action> *pNewHistory = new History<GameState, ChanceNode, Action>(*mHistoryList);
		HistoryNode<GameState, ChanceNode, Action> historyNode = HistoryNode<GameState, ChanceNode, Action>(gameState, actionTaken);
		pNewHistory->mHistoryList->push_back(historyNode);
		return pNewHistory;
	}

	History* AddToNewHistory(ChanceNode chanceNode) {
		History<GameState, ChanceNode, Action>* pNewHistory = new History<GameState, ChanceNode, Action>(*mHistoryList);
		HistoryNode<GameState, ChanceNode, Action> historyNode = HistoryNode<GameState, ChanceNode, Action>(chanceNode);
		pNewHistory->mHistoryList->push_back(historyNode);
		return pNewHistory;
	}



};