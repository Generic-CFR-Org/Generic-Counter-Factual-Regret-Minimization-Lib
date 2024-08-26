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
	Action* mpActionTaken;
	ChanceNode mChanceNode;


public:

	HistoryNode() {
		mIsGameNode, mIsChanceNode = false;
		mGameState = GameState{};
		mpActionTaken = nullptr;
		mChanceNode = ChanceNode{};
	}

	HistoryNode(GameState gameState, Action* actionTaken) {
		mIsGameNode = true;
		mIsChanceNode = false;
		mGameState = GameState{gameState};
		mpActionTaken = actionTaken;
		mChanceNode = ChanceNode{};
	}

	HistoryNode(ChanceNode chanceNode) {
		mIsGameNode = false;
		mIsChanceNode = true;
		mGameState = GameState{};
		mpActionTaken = nullptr;
		mChanceNode = ChanceNode{ chanceNode };

	}

	bool IsGameNode() { return mIsGameNode; }
	bool IsChanceNode() { return mIsChanceNode; }

	GameState GetGameState() {
		if (!mIsGameNode) {
			historyHasNoGameNode e = historyHasNoGameNode();
			std::cout << e.what();
			throw e;
		}
		return mGameState;
	}
	Action* GetAction() {
		if (!mIsGameNode) {
			historyHasNoGameNode e = historyHasNoGameNode();
			std::cout << e.what();
			throw e;
		}
		return mpActionTaken;
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


//template<typename GameState, typename ChanceNode, typename Action>
//class History {
//
//public:
//	std::vector<HistoryNode<GameState, ChanceNode, Action>> *mHistoryList;
//	History() { mHistoryList = new std::vector<HistoryNode<GameState, ChanceNode, Action>>; }
//	History(std::vector<HistoryNode<GameState, ChanceNode, Action>> oldHistory) {
//		mHistoryList = new std::vector<HistoryNode<GameState, ChanceNode, Action>>(oldHistory);
//	}
//	~History() {
//		/*auto iHistory = IterBegin();
//		auto iEnd = IterEnd();
//		for (iHistory; iHistory != iEnd; iHistory++) {
//			delete *iHistory;
//		}*/
//		try {
//			delete mHistoryList;
//
//		}
//		catch (...) {
//			std::cout << "couldn't delete history"; 
//		}
//	}
//	typename std::vector<HistoryNode<GameState, ChanceNode, Action>>::iterator  IterBegin() {
//		return mHistoryList->begin();
//	}
//
//	typename std::vector<HistoryNode<GameState, ChanceNode, Action>>::iterator  IterEnd() {
//		return mHistoryList->end();
//	}
//
//	History* AddToNewHistory(GameState gameState, Action* actionTaken) {
//		History<GameState, ChanceNode, Action> *pNewHistory = new History<GameState, ChanceNode, Action>(*mHistoryList);
//		HistoryNode<GameState, ChanceNode, Action> historyNode = HistoryNode<GameState, ChanceNode, Action>(gameState, actionTaken);
//		pNewHistory->mHistoryList->push_back(historyNode);
//		return pNewHistory;
//	}
//
//	History* AddToNewHistory(ChanceNode chanceNode) {
//		History<GameState, ChanceNode, Action>* pNewHistory = new History<GameState, ChanceNode, Action>(*mHistoryList);
//		HistoryNode<GameState, ChanceNode, Action> historyNode = HistoryNode<GameState, ChanceNode, Action>(chanceNode);
//		pNewHistory->mHistoryList->push_back(historyNode);
//		return pNewHistory;
//	}
//
//};



template<typename GameState, typename ChanceNode, typename Action>
class HistoryTreeNode {

	typedef std::vector<Action*> ActionList;

	GameState mGameState;
	ActionList mParentActions;
	ChanceNode mChanceNode;
	bool mIsGameNode;
	bool mIsChanceNode;

	std::vector<HistoryTreeNode*> mChildren;
	HistoryTreeNode* mpParent;
public:
	HistoryTreeNode() {
		mIsGameNode, mIsChanceNode = false;
		mGameState = GameState{};
		mChanceNode = ChanceNode{};
		mParentActions = ActionList{};
		mChildren = std::vector<HistoryTreeNode*>{};
		mpParent = nullptr;
	}
	HistoryTreeNode(HistoryTreeNode* pParentPtr) {
		mIsGameNode, mIsChanceNode = false;
		mGameState = GameState{};
		mChanceNode = ChanceNode{};
		mParentActions = ActionList{};
		mChildren = std::vector<HistoryTreeNode*>{};
		mpParent = pParentPtr;
	}
	HistoryTreeNode(GameState gameState, ActionList parentActions, HistoryTreeNode* pParentPtr) {
		mIsChanceNode = false;
		mIsGameNode = true;
		mChanceNode = ChanceNode{};
		mGameState = GameState{ gameState };
		mParentActions = ActionList{ parentActions };
		mChildren = std::vector<HistoryTreeNode*>{};
		mpParent = pParentPtr;
	}
	HistoryTreeNode(GameState gameState, HistoryTreeNode* pParentPtr) {
		mIsChanceNode = false;
		mIsGameNode = true;
		mChanceNode = ChanceNode{};
		mGameState = GameState{ gameState };
		mParentActions = ActionList{};
		mChildren = std::vector<HistoryTreeNode*>{};
		mpParent = pParentPtr;
	}
	HistoryTreeNode(ChanceNode chanceNode, HistoryTreeNode* pParentPtr) {
		mIsGameNode = false;
		mIsChanceNode = true;
		mGameState = GameState{};
		mParentActions = ActionList{};
		mChanceNode = ChanceNode{ chanceNode };
		mChildren = std::vector<HistoryTreeNode*>{};
		mpParent = pParentPtr;
	}
	HistoryTreeNode(ChanceNode chanceNode, ActionList parentActions, HistoryTreeNode* pParentPtr) {
		mIsGameNode = false;
		mIsChanceNode = true;
		mGameState = GameState{};
		mParentActions = ActionList{ parentActions };
		mChanceNode = ChanceNode{ chanceNode };
		mChildren = std::vector<HistoryTreeNode*>{};
		mpParent = pParentPtr;
	}
	HistoryTreeNode(ActionList parentActions, HistoryTreeNode* pParentPtr) {
		mIsGameNode = false;
		mIsChanceNode = false;
		mGameState = GameState{};
		mParentActions = ActionList{ parentActions };
		mChanceNode = ChanceNode{};
		mChildren = std::vector<HistoryTreeNode*>{};
		mpParent = pParentPtr;
	}

	HistoryTreeNode* AddChild(GameState gameState, ActionList parentActions) {
		HistoryTreeNode* childNode = new HistoryTreeNode(gameState, parentActions, this);
		this->mChildren.push_back(childNode);
		return childNode;
	}
	HistoryTreeNode* AddChild(GameState gameState) {
		HistoryTreeNode* childNode = new HistoryTreeNode(gameState, this);
		this->mChildren.push_back(childNode);
		return childNode;
	}
	HistoryTreeNode* AddChild(ChanceNode chanceNode, ActionList parentActions) {
		HistoryTreeNode* childNode = new HistoryTreeNode(chanceNode, parentActions, this);
		mChildren.push_back(childNode);
		return childNode;
	}
	HistoryTreeNode* AddChild(ChanceNode chanceNode) {
		HistoryTreeNode* childNode = new HistoryTreeNode(chanceNode, this);
		mChildren.push_back(childNode);
		return childNode;
	}
	HistoryTreeNode* AddChild(ActionList parentActions) {
		HistoryTreeNode* childNode = new HistoryTreeNode(parentActions, this);
		mChildren.push_back(childNode);
		return childNode;
	}
	HistoryTreeNode* AddChild() {
		HistoryTreeNode* childNode = new HistoryTreeNode(this);
		mChildren.push_back(childNode);
		return childNode;
	}

	bool IsTerminalNode() { return mChildren.size() == 0; }
	bool IsRoot() { return mpParent == nullptr; }
	std::vector<HistoryTreeNode*> GetChildren() { return mChildren; }
	HistoryTreeNode* GetParent() { return mpParent; }

	bool IsGameNode() { return mIsGameNode; }
	bool IsChanceNode() { return mIsChanceNode; }
	bool IsParentGameNode() { return mParentActions.size() > 0; }
	GameState GetGameState() {
		if (!mIsGameNode) {
			historyHasNoGameNode e = historyHasNoGameNode();
			std::cout << e.what();
			throw e;
		}
		return mGameState;
	}
	ActionList GetParentActions() {
		
		return mParentActions;
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
struct HistoryArray {

	using PathHistoryNode = HistoryNode<GameState, ChanceNode, Action>;

	std::vector<std::vector<PathHistoryNode>> mAllHistoryArr;
	HistoryArray(int numPaths) {
		for (int iPath = 0; iPath < numPaths; iPath++) {
			std::vector<PathHistoryNode> empty;
			mAllHistoryArr.push_back(empty);
		}
	}
	void AddToPath(PathHistoryNode historyNode, int pathIndex) {
		mAllHistoryArr.at(pathIndex).push_back(historyNode);
		//pathArr.push_back(historyNode);
	}
	/*void AddToFrontOfPath(PathHistoryNode historyNode, int pathIndex) {
		std::vector<PathHistoryNode> pathArr = mAllHistoryArr[pathIndex].insert(, historyNode);
		pathArr.insert(pathArr.begin(), historyNode);
	}*/
	//void AddToAllPaths(PathHistoryNode historyNode) {
	//	/*void (*adderFunc) ( PathHistoryNode, int );
	//	adderFunc = addToFront ? AddToFrontOfPath : AddToBackOfPath;*/
	//	for (int iPath = 0; iPath < mAllHistoryArr.size(); iPath++) {
	//		AddToPath(historyNode, iPath);
	//	}
	//}
	std::vector<PathHistoryNode> GetPath(int pathIndex) {
		return mAllHistoryArr.at(pathIndex);
	}
	int NumHistories() {
		return mAllHistoryArr.size();
	}
};

template<typename GameState, typename ChanceNode, typename Action>
class HistoryTree {
	
public:

	typedef std::vector<Action*> ActionList;
	using PathHistoryNode = HistoryNode<GameState, ChanceNode, Action>;
	using TreeNode = HistoryTreeNode<GameState, ChanceNode, Action>;
	using AllHistoryArray = HistoryArray<GameState, ChanceNode, Action>;
	
	TreeNode* root;
	
	HistoryTree(ChanceNode chanceNode) {
		root = new TreeNode(chanceNode, nullptr);
	}

	static AllHistoryArray* GetAllHistoriesToNode(TreeNode* terminalNode) {
		std::vector<TreeNode*> *pRootToNode = new std::vector<TreeNode*>();
		TreeNode* currNode = terminalNode;
		int numPaths = 1;
		while (currNode != nullptr) {
			pRootToNode->insert(pRootToNode->begin(), currNode);
			if (currNode->IsParentGameNode()) {
				numPaths *= currNode->GetParentActions().size();
			}
			currNode = currNode->GetParent();
		}
		AllHistoryArray* pAllHistories = new AllHistoryArray(numPaths);
		HistoryArrayHelper(pAllHistories, pRootToNode, numPaths, 0, 0);
		delete pRootToNode;
		return pAllHistories;
	}

private:

	static void HistoryArrayHelper(AllHistoryArray* allHistoryArr, std::vector<TreeNode*> *rootToNodeArr,
							int uniquePathsAtDepth, int iRootToNode, int startPathIndex)
	{

		TreeNode* currNode = rootToNodeArr->at(iRootToNode);
		//Base case for terminal node -> Do nothing.
		if (iRootToNode == rootToNodeArr->size() - 1) {return; }
		//If current node is a game node.
		else if (currNode->IsGameNode()) {
	
			GameState currGameState = currNode->GetGameState();
			

			TreeNode* childNode = rootToNodeArr->at(iRootToNode + 1);
			ActionList possibleActions = childNode->GetParentActions();
			int numPathsPerAction = uniquePathsAtDepth / possibleActions.size();
				
			int actionIndex = 0;
			for (Action* actionToNode : possibleActions) {
				int newPathIndex = startPathIndex + (numPathsPerAction * actionIndex);
				PathHistoryNode currHistoryNode = PathHistoryNode(currGameState, actionToNode);
				for (int iSubPath = 0; iSubPath < numPathsPerAction; iSubPath++) {
					allHistoryArr->AddToPath(currHistoryNode, newPathIndex + iSubPath);
				}
				actionIndex++;
				HistoryArrayHelper(allHistoryArr, rootToNodeArr, numPathsPerAction, iRootToNode + 1, newPathIndex);
			}
		}
		//Current node is a chance node.
		else {
			PathHistoryNode currHistoryNode = PathHistoryNode(currNode->GetChanceNode());
			allHistoryArr->AddToPath(currHistoryNode, startPathIndex);
			HistoryArrayHelper(allHistoryArr, rootToNodeArr, uniquePathsAtDepth, iRootToNode + 1, startPathIndex);
		}
	}

};