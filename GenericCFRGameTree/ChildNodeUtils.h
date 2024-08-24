#pragma once
#include "pch.h"
#include "framework.h"


/*
##################################################
### CHILDREN FROM GAME NODE CLASS DEFINITIONS  ###
##################################################
*/

template<typename GameState, typename Action>
class GameNodesFromGameNode {

public:

	typedef std::vector<Action*> Strategy;
	typedef std::vector<Action*> ActionsToChild;


	std::vector<ChildGameNode<GameState, Action>*> *mChildrenGameNodes;

	class ChildGameNode {
	private:
		GameState mChildGameState;
		Strategy* mpChildStrategy;
		ActionsToChild mParentActions;

	public:
		ChildGameNode(GameState gameState, Strategy* strategy, ActionsToChild parentActions) {
			mChildGameState = gameState;
			mpChildStrategy = strategy;
			mParentActions = parentActions;
		}
		GameState GetGameState() {return mChildGameState;}
		Strategy *GetStrategy() {return mpChildStrategy;}
		ActionsToChild GetActionsToChild() {return mParentActions;}
	};

	typename std::vector<ChildGameNode*>::iterator  IterBegin() {
		return mChildrenGameNodes->begin();
	}

	typename std::vector<ChildGameNode*>::iterator IterEnd() {
		return mChildrenGameNodes->end();
	}

	GameNodesFromGameNode() {mChildrenGameNodes = new vector<ChildGameNode>};
	~GameNodesFromGameNode() {
		auto iChild = this->IterBegin();
		auto iEnd = this->IterEnd();
		for (iChild, iChild < iEnd, iChild++) {
			delete iChild;
		}
		delete mChildrenGameNodes;
	}
protected:
	void AddChildGameNode(GameState* gameState, std::vector<Action>* pStrategy, std::vector<Action> actionsToChild) {
		ChildGameNode* childNode = new ChildGameNode(gameState, pStrategy, actionsToChild);
		mChildrenGameNodes->push_back(childNode);
	}

};

template< typename ChanceNode, typename Action>
class ChanceNodesFromGameNode {

public:

	typedef std::vector<Action*> ActionsToChild;

	std::vector<ChildChanceNode<ChanceNode, Action>*>* mChildrenChanceNodes;

	class ChildChanceNode {
	private:
		ChanceNode mChildChanceNode;
		ActionsToChild mParentActions;

	public:
		ChildGameNode(ChanceNode chanceNode,  ActionsToChild parentActions) {
			mChildChanceNode = chanceNode;
			mParentActions = parentActions;
		}
		ChanceNode GetChanceNode() { return mChildChanceNode; }
		ActionsToChild GetActionsToChild() { return mParentActions; }
	};

	typename std::vector<ChildChanceNode*>::iterator  IterBegin() {
		return mChildrenChanceNodes->begin();
	}

	typename std::vector<ChildChanceNode*>::iterator IterEnd() {
		return mChildrenChanceNodes->end();
	}

	ChanceNodesFromGameNode() { mChildrenGameNodes = new vector<ChildGameNode> };
	~ChanceNodesFromGameNode() {
		auto iChild = this->IterBegin();
		auto iEnd = this->IterEnd();
		for (iChild, iChild < iEnd, iChild++) {
			delete iChild;
		}
		delete mChildrenGameNodes;
	}

protected:
	void AddChanceNode(ChanceNode* chanceNode, std::vector<Action> actionsToChild) {
		ChildChanceNode chanceNode = new ChildChanceNode(chanceNode, actionsToChild);
		mChildrenChanceNodes->push_back(chanceNode);
	}

};

template<typename Action>
class TerminalNodesFromGameNode {

public:

	typedef std::vector<Action*> ActionsToChild;

	std::vector<ChildTerminalNode<Action>*>* mChildrenTerminalNodes;

	class ChildTerminalNode {
	private:
		ActionsToChild mParentActions;

	public:
		ChildGameNode(ActionsToChild parentActions) {mParentActions = parentActions;}
		ActionsToChild GetActionsToChild() { return mParentActions; }
	};

	typename std::vector<ChildChanceNode*>::iterator  IterBegin() {
		return mChildrenChanceNodes->begin();
	}

	typename std::vector<ChildChanceNode*>::iterator IterEnd() {
		return mChildrenChanceNodes->end();
	}

	ChanceNodesFromGameNode() { mChildrenGameNodes = new vector<ChildGameNode> };
	~ChanceNodesFromGameNode() {
		auto iChild = this->IterBegin();
		auto iEnd = this->IterEnd();
		for (iChild, iChild < iEnd, iChild++) {
			delete iChild;
		}
		delete mChildrenGameNodes;
	}
protected:
	void AddChildTerminalNode(std::vector<Action> actionsToChild) {
		ChildTerminalNode *terminalNode = new ChildTerminalNode(actionsToChild);
		mChildrenTerminalNodes->push_back(terminalNode);
	}

};

template<typename GameState, typename ChanceNode, typename Action>
class AllNodesFromGameNode {

public:

	GameNodesFromGameNode<GameState, Action>* mChildGameNodes;
	ChanceNodesFromGameNode<ChanceNode, Action>* mChildChanceNodes;
	TerminalNodesFromGameNode<Action>* mChildTerminalNodes;

	AllNodesFromGameNode() {
		mChildGameNodes = new GameNodesFromGameNode<GameState, Action>();
		mChildChanceNodes = new ChanceNodesFromGameNode<ChanceNode, Action>();
		mChildTerminalNodes = new TerminalNodesFromGameNode<Action>();
	}

	~AllNodesFromGameNode() {
		delete mChildGameNodes;
		delete mChildChanceNodes;
		delete mChildTerminalNodes;
	}
	GameNodesFromGameNode<GameState, Action>* GetChildGameNodes() { return mChildGameNodes; }
	ChanceNodesFromGameNode<ChanceNode, Action>* GetChildChanceNodes() { return mChildChanceNodes; }
	TerminalNodesFromGameNode<Action>* GetChildTerminalNodes() { return mChildTerminalNodes; }

	void AddChildGameNode(GameState gameState, std::vector<Action>* pStrategy, std::vector<Action> actionsToChild) {
		mChildGameNodes->AddGameNode(gameState, pStrategy, actionsToChild);
	}

	void AddChildGameNode(ChanceNode chanceNode, std::vector<Action> actionsToChild) {
		mChildChanceNodes->AddChanceNode(chanceNode, actionsToChild);
	}

	void AddChildTerminalNode(std::vector<Action> actionsToChild) {
		mChildTerminalNodes->AddChildTerminalNode(actionsToChild);
	}

};







/*
####################################################
### CHILDREN FROM CHANCE NODE CLASS DEFINITIONS  ###
####################################################
*/

template<typename GameState, typename Action>
class GameNodesFromChanceNode {

public:

	typedef std::vector<Action*> Strategy;

	std::vector<ChildGameNode<GameState, Action>*>* mChildrenGameNodes;

	class ChildGameNode {
	private:
		GameState mChildGameState;
		Strategy* mpChildStrategy;
		float mProbToChild;

	public:
		ChildGameNode(GameState gameState, Strategy* strategy, float probToChild) {
			mChildGameState = gameState;
			mpChildStrategy = strategy;
			mProbToChild = probToChild;
			
		GameState GetGameState() { return mChildGameState; }
		Strategy* GetStrategy() { return mpChildStrategy; }
		float GetProbToChild() { return mProbToChild; }
	};

	typename std::vector<ChildGameNode*>::iterator  IterBegin() {
		return mChildrenGameNodes->begin();
	}

	typename std::vector<ChildGameNode*>::iterator IterEnd() {
		return mChildrenGameNodes->end();
	}

	GameNodesFromGameNode() { mChildrenGameNodes = new vector<ChildGameNode*> };
	~GameNodesFromGameNode() {
		auto iChild = this->IterBegin();
		auto iEnd = this->IterEnd();
		for (iChild, iChild < iEnd, iChild++) {
			delete iChild;
		}
		delete mChildrenGameNodes;
	}


	void AddGameNode(GameState* gameState, std::vector<Action>* pStrategy, float probToChild) {
		ChildGameNode* childNode = new ChildGameNode(gameState, pStrategy, probToChild);
		mChildrenGameNodes->push_back(childNode);
	}

};


class TerminalNodesFromChanceNode {

public:

	std::vector<ChildTerminalNode*>* mChildrenTerminalNodes;

	class ChildTerminalNode {
	private:
		float mProbToChild;

	public:
		ChildGameNode(float probToChild) { mProbToChild = probToChild; }
		float GetProbToChild() { return mProbToChild; }
	};

	typename std::vector<ChildChanceNode*>::iterator  IterBegin() {
		return mChildrenChanceNodes->begin();
	}

	typename std::vector<ChildChanceNode*>::iterator IterEnd() {
		return mChildrenChanceNodes->end();
	}

	ChanceNodesFromGameNode() { mChildrenGameNodes = new vector<ChildTerminalNode*> };
	~ChanceNodesFromGameNode() {
		auto iChild = this->IterBegin();
		auto iEnd = this->IterEnd();
		for (iChild, iChild < iEnd, iChild++) {
			delete iChild;
		}
		delete mChildrenGameNodes;
	}

	void AddTerminalNode(float probToChild) {
		ChildTerminalNode* terminalNode = new ChildTerminalNode(probToChild);
		mChildrenTerminalNodes->push_back(terminalNode);
	}

};

template<typename GameState, typename Action>
class AllNodesFromChanceNode {

public:
	GameNodesFromChanceNode<GameState, Action>* mChildGameNodes;
	TerminalNodesFromChanceNode* mChildTerminalNodes;

	AllNodesFromChanceNode() {
		mChildGameNodes = new GameNodesFromChanceNode<GameState, Action>();
		mChildTerminalNodes = new TerminalNodesFromChanceNode();
	}

	~AllNodesFromChanceNode() {
		delete mChildGameNodes;
		delete mChildTerminalNodes;
	}
	GameNodesFromGameNode* GetChildGameNodes() { return mChildGameNodes; }
	TerminalNodesFromGameNode* GetChildTerminalNodes() { return mChildTerminalNodes; }
	
	void AddChildTerminalNode(float probToChild) {
		mChildTerminalNodes->AddChildTerminalNode(probToChild);
	}

	void AddChildGameNode(GameState* gameState, std::vector<Action>* pStrategy, float probToChild) {
		mChildGameNodes->AddGameNode(gameState, pStrategy, probToChild);
	}
};




