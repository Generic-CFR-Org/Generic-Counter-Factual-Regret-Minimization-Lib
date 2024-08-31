#pragma once
#include "pch.h"
#include "framework.h"


/*
###############################
### CHILD NODE DEFINITIONS  ###
###############################
*/

template<typename GameState, typename Action>
class GameNodeChildGameNode {

	typedef std::vector<Action*> Strategy;
	typedef std::vector<Action*> ActionsToChild;
private:
	GameState mChildGameState;
	Strategy mChildStrategy;
	ActionsToChild mParentActions;
public:
	GameNodeChildGameNode(GameState gameState, Strategy strategy, ActionsToChild parentActions) {
		mChildGameState = GameState{ gameState };
		mChildStrategy = Strategy{ strategy };
		mParentActions = ActionsToChild{ parentActions };
	}
	GameState GetGameState() { return mChildGameState; }
	Strategy GetStrategy() { return mChildStrategy; }
	ActionsToChild GetActionsToChild() { return mParentActions; }
};


template<typename ChanceNode, typename Action>
class GameNodeChildChanceNode {

	typedef std::vector<Action*> ActionsToChild;
private:
	ChanceNode mChildChanceNode;
	ActionsToChild mParentActions;
public:
	GameNodeChildChanceNode(ChanceNode chanceNode, ActionsToChild parentActions) {
		mChildChanceNode = ChanceNode{ chanceNode };
		mParentActions = ActionsToChild{ parentActions };
	}
	ChanceNode GetChanceNode() { return mChildChanceNode; }
	ActionsToChild GetActionsToChild() { return mParentActions; }
};


template<typename Action>
class GameNodeChildTerminalNode {

	typedef std::vector<Action*> ActionsToChild;
private:
	ActionsToChild mParentActions;
public:
	GameNodeChildTerminalNode(ActionsToChild parentActions) { mParentActions = ActionsToChild{ parentActions }; }
	ActionsToChild GetActionsToChild() { return mParentActions; }
};


template<typename GameState, typename Action>
class ChanceNodeChildGameNode {

	typedef std::vector<Action*> Strategy;
private:
	GameState mChildGameState;
	Strategy mChildStrategy;
	float mProbToChild;
public:
	ChanceNodeChildGameNode(GameState gameState, Strategy strategy, float probToChild) {
		mChildGameState = GameState{ gameState };
		mChildStrategy = Strategy{ strategy };
		mProbToChild = probToChild;
	}
	GameState GetGameState() { return mChildGameState; }
	Strategy GetStrategy() { return mChildStrategy; }
	float GetProbToChild() { return mProbToChild; }
};


class ChanceNodeChildTerminalNode {
private:
	float mProbToChild;
public:
	ChanceNodeChildTerminalNode() { mProbToChild = 1.0; }
	ChanceNodeChildTerminalNode(float probToChild) { mProbToChild = probToChild; }
	float GetProbToChild() { return mProbToChild; }
};

/*
##################################
### CHILDREN LIST DEFINITIONS  ###
##################################
*/

template<typename ChildNode>
class NodeChildren {
	
public:

	std::vector<ChildNode*>* mChildNodes;

	typename std::vector<ChildNode*>::iterator  IterBegin() {
		return mChildNodes->begin();
	}

	typename std::vector<ChildNode*>::iterator IterEnd() {
		return mChildNodes->end();
	}

	NodeChildren() { mChildNodes = new std::vector<ChildNode*>; }
	~NodeChildren() {
		auto iChild = this->IterBegin();
		auto iEnd = this->IterEnd();
		for (iChild; iChild < iEnd; iChild++) {
			delete* iChild;
		}
		delete mChildNodes;
	}
	int size() {return mChildNodes->size(); }
};


template<typename GameState, typename Action>
class GameNodeChildrenGamesNodes : public NodeChildren<GameNodeChildGameNode<GameState, Action>> {
public:
	typedef std::vector<Action*> ActionList;
	typedef GameNodeChildGameNode<GameState, Action> ChildNode;

	void AddChildNode(GameState gameState, ActionList strategy, ActionList actionsToChild) {
		ChildNode* childNode = new ChildNode(gameState, strategy, actionsToChild);
		this->mChildNodes->push_back(childNode);
	}
};

template<typename ChanceNode, typename Action>
class GameNodeChildrenChanceNodes : public NodeChildren<GameNodeChildChanceNode<ChanceNode, Action>> {
public:
	typedef std::vector<Action*> ActionList;
	typedef GameNodeChildChanceNode<ChanceNode, Action> ChildNode;

	void AddChildNode(ChanceNode chanceNode, ActionList actionsToChild) {
		ChildNode* childNode = new ChildNode(chanceNode, actionsToChild);
		this->mChildNodes->push_back(childNode);
	}
};

template<typename Action>
class GameNodeChildrenTerminalNodes : public NodeChildren<GameNodeChildTerminalNode<Action>> {
public:
	typedef std::vector<Action*> ActionList;
	typedef GameNodeChildTerminalNode<Action> ChildNode;

	void AddChildNode(ActionList actionsToChild) {
		ChildNode* terminalNode = new ChildNode(actionsToChild);
		this->mChildNodes->push_back(terminalNode);
	}
};

template<typename GameState, typename Action>
class ChanceNodeChildrenGamesNodes : public NodeChildren<ChanceNodeChildGameNode<GameState, Action>> {
public:
	typedef std::vector<Action*> ActionList;
	typedef ChanceNodeChildGameNode<GameState, Action> ChildNode;

	void AddChildNode(GameState gameState, ActionList strategy, float probToChild) {
		ChildNode* childNode = new ChildNode(gameState, strategy, probToChild);
		this->mChildNodes->push_back(childNode);
	}
};

class ChanceNodeChildrenTerminalNodes : public NodeChildren<ChanceNodeChildTerminalNode> {
public:

	typedef ChanceNodeChildTerminalNode ChildNode;

	void AddChildNode(float probToChild) {
		ChildNode* terminalNode = new ChildNode(probToChild);
		this->mChildNodes->push_back(terminalNode);
	}
};

/*
###############################################
### ALL CHILDREN FROM GAME NODE DEFINITION  ###
###############################################
*/

template<typename GameState, typename ChanceNode, typename Action>
class ChildrenFromGameNode {

public:
	typedef std::vector<Action*> ActionList;

	typedef GameNodeChildrenGamesNodes<GameState, Action> ChildGameNodes;
	typedef GameNodeChildrenChanceNodes<ChanceNode, Action> ChildChanceNodes;
	typedef GameNodeChildrenTerminalNodes<Action> ChildTerminalNodes;

	ChildGameNodes *mpChildGameNodes;
	ChildChanceNodes *mpChildChanceNodes;
	ChildTerminalNodes *mpChildTerminalNodes;

	ChildrenFromGameNode() {
		mpChildGameNodes = new ChildGameNodes();
		mpChildChanceNodes = new ChildChanceNodes();
		mpChildTerminalNodes = new ChildTerminalNodes();
	}
	~ChildrenFromGameNode() {
		delete mpChildGameNodes;
		delete mpChildChanceNodes;
		delete mpChildTerminalNodes;
	}

	ChildGameNodes* GetChildGameNodes() { return mpChildGameNodes; }
	ChildChanceNodes* GetChildChanceNodes() { return mpChildChanceNodes; }
	ChildTerminalNodes* GetChildTerminalNodes() { return mpChildTerminalNodes; }

	void AddChildGameNode(GameState gameState, ActionList strategy, ActionList actionsToChild) {
		mpChildGameNodes->AddChildNode(gameState, strategy, actionsToChild);
	}

	void AddChildChanceNode(ChanceNode chanceNode, ActionList actionsToChild) {
		mpChildChanceNodes->AddChildNode(chanceNode, actionsToChild);
	}

	void AddChildTerminalNode(ActionList actionsToChild) {
		mpChildTerminalNodes->AddChildNode(actionsToChild);
	}

	int GameSize() {
		return mpChildGameNodes->size();;
	}
	int ChanceSize() {
		return mpChildChanceNodes->size();
	}
	int TerminalSize() {
		return mpChildTerminalNodes->size();
	}
};

/*
#################################################
### ALL CHILDREN FROM CHANCE NODE DEFINITION  ###
#################################################
*/

template<typename GameState, typename Action>
class ChildrenFromChanceNode {

public:
	typedef std::vector<Action*> ActionList;

	typedef ChanceNodeChildrenGamesNodes<GameState, Action> ChildGameNodes;
	typedef ChanceNodeChildrenTerminalNodes ChildTerminalNodes;

	ChildGameNodes* mpChildGameNodes;
	ChildTerminalNodes* mpChildTerminalNodes;


	ChildrenFromChanceNode() {
		mpChildGameNodes = new ChildGameNodes();
		mpChildTerminalNodes = new ChildTerminalNodes();

	}
	~ChildrenFromChanceNode() {
		delete mpChildGameNodes;
		delete mpChildTerminalNodes;
	}

	ChildGameNodes* GetChildGameNodes() { return mpChildGameNodes; }
	ChildTerminalNodes* GetChildTerminalNodes() { return mpChildTerminalNodes; }

	void AddChildGameNode(GameState gameState, ActionList strategy, float probToChild) {
		mpChildGameNodes->AddChildNode(gameState, strategy, probToChild);

	}

	void AddChildTerminalNode(float probToChild) {
		mpChildTerminalNodes->AddChildNode(probToChild);

	}

	int GameSize() {
		return mpChildGameNodes->size();
	}

	int TerminalSize() {
		return mpChildTerminalNodes->size();
	}
};

