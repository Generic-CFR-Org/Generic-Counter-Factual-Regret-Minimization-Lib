#pragma once
#include "pch.h"
#include "framework.h"
#include <cstddef>
#include <concepts>
#include <functional>
#include <string>
#include "cfr.h"


namespace CfrConcepts {
	
	template<typename Action, typename PlayerNode, typename ChanceNode>
	concept Hashable = requires( Action a, PlayerNode p, ChanceNode c ) {
		/*All Game classes must have ToHash() func that returns a string.*/
		{ a.ToHash() } -> std::convertible_to<std::string>;
		{ p.ToHash() } -> std::convertible_to<std::string>;
		{ c.ToHash() } -> std::convertible_to<std::string>;

		//PlayerNode must have ToInfoSetHash() func that returns a string for the player view.
		{ p.ToInfoSetHash() } -> std::convertible_to<std::string>;
	};

	template<typename PlayerNode>
	concept PlayerNodePlayerOneFunc = requires( PlayerNode p ) {

		/*PlayerNode must have function IsPlayerOne() that returns a bool for the currently
			acting player*/
		{ p.IsPlayerOne() } -> std::convertible_to<bool>;

	};
}

template<typename Action, typename PlayerNode, typename ChanceNode>
class ClientNode {
public:
	Action mAction;
	PlayerNode mPlayerNode;
	ChanceNode mChanceNode;

	float mProbability;

	bool mIsPlayerNode;
	bool mIsChanceNode;
	bool mIsTerminalNode;

	ClientNode(PlayerNode p) :
		mAction{}, mPlayerNode{ p }, mChanceNode{}, mProbability{ 1.0 },
		mIsPlayerNode{ true }, mIsChanceNode{ false },
		mIsTerminalNode{ false } {}

	ClientNode(ChanceNode c) :
		mAction{}, mPlayerNode{}, mChanceNode{ c }, mProbability{ 1.0 },
		mIsPlayerNode{ false }, mIsChanceNode{ true },
		mIsTerminalNode{ false } {}

	ClientNode() :
		mAction{}, mPlayerNode{}, mChanceNode{}, mProbability{ 1.0 },
		mIsPlayerNode{ false }, mIsChanceNode{ false },
		mIsTerminalNode{ true } {}

	ClientNode(PlayerNode p, float prob) :
		ClientNode{ p } { mProbability = prob; }

	ClientNode(ChanceNode c, float prob) :
		ClientNode{ c } {mProbability = prob; }

	ClientNode(float prob) :
		ClientNode{} {mProbability = prob; } 

	ClientNode(PlayerNode p, Action a) :
		ClientNode{ p } { mAction = a; }

	ClientNode(ChanceNode c, Action a) :
		ClientNode{ c } { mAction = a;}

	ClientNode(Action a) :
		ClientNode{} {
		mAction = a;
	}

	bool IsPlayerNode() { return mIsPlayerNode; }
	bool IsChanceNode() { return mIsChanceNode; }
	bool IsTerminalNode() { return mIsTerminalNode; }

	Action GetAction() { return mAction; }
	PlayerNode GetPlayerNode() { return mPlayerNode; }
	ChanceNode GetChanceNode() { return mChanceNode; }
	float GetProbability() { return mProbability; }
};


template<typename Action, typename PlayerNode, typename ChanceNode>
requires CfrConcepts::Hashable<Action, PlayerNode, ChanceNode>&&
		 CfrConcepts::PlayerNodePlayerOneFunc<PlayerNode>
class TreeNode {

	using byte = unsigned char;
	using TreeNodeList = std::vector<TreeNode>;
protected:
	Action mAction;
	PlayerNode mPlayerNode;
	ChanceNode mChanceNode;

	float mProbability;

	bool mIsPlayerNode;
	bool mIsChanceNode;
	bool mIsTerminalNode;
	TreeNode* mpParent;
	byte* mpChildStartPos;
	bool mIsChildStartSet;

public:

	TreeNode(const TreeNode<Action, PlayerNode, ChanceNode>& other) {
		mAction = other.mAction;
		mPlayerNode = other.mPlayerNode;
		mChanceNode = other.mChanceNode;
		mProbability = other.mProbability;
		mIsPlayerNode = other.mIsPlayerNode;
		mIsChanceNode = other.mIsChanceNode;
		mIsTerminalNode = other.mIsTerminalNode;
		mpParent = other.mpParent;
		mpChildStartPos = nullptr;
		mIsChildStartSet = false;
	}

	TreeNode (
		const TreeNode<Action, PlayerNode, ChanceNode>& other,
		TreeNode<Action, PlayerNode, ChanceNode>* parent
	) {
		mAction = other.mAction;
		mPlayerNode = other.mPlayerNode;
		mChanceNode = other.mChanceNode;
		mProbability = other.mProbability;
		mIsPlayerNode = other.mIsPlayerNode;
		mIsChanceNode = other.mIsChanceNode;
		mIsTerminalNode = other.mIsTerminalNode;
		mpParent = parent;
		mpChildStartPos = nullptr;
		mIsChildStartSet = false;
	}

	TreeNode(
		const ClientNode<Action, PlayerNode, ChanceNode>& other
	) {
		mAction = other.mAction;
		mPlayerNode = other.mPlayerNode;
		mChanceNode = other.mChanceNode;
		mProbability = other.mProbability;
		mIsPlayerNode = other.mIsPlayerNode;
		mIsChanceNode = other.mIsChanceNode;
		mIsTerminalNode = other.mIsTerminalNode;
		mpParent = nullptr;
		mpChildStartPos = nullptr;
		mIsChildStartSet = false;
	}

	TreeNode(
		const ClientNode<Action, PlayerNode, ChanceNode>& other,
		TreeNode<Action, PlayerNode, ChanceNode>* parent
	) {
		mAction = other.mAction;
		mPlayerNode = other.mPlayerNode;
		mChanceNode = other.mChanceNode;
		mProbability = other.mProbability;
		mIsPlayerNode = other.mIsPlayerNode;
		mIsChanceNode = other.mIsChanceNode;
		mIsTerminalNode = other.mIsTerminalNode;
		mpParent = parent;
		mpChildStartPos = nullptr;
		mIsChildStartSet = false;
	}



	/**
		* @brief Base constructors for each node type. 
		*/
	TreeNode(PlayerNode p) :
		mAction{}, mPlayerNode{ p }, mChanceNode{}, mProbability{ 1.0 },
		mIsPlayerNode{ true }, mIsChanceNode{ false },
		mIsTerminalNode{ false }, mpParent{ nullptr },
		mpChildStartPos{ nullptr }, mIsChildStartSet{ false } {}

	TreeNode(ChanceNode c) :
		mAction{}, mPlayerNode{}, mChanceNode{ c }, mProbability{ 1.0 },
		mIsPlayerNode{ false }, mIsChanceNode{ true },
		mIsTerminalNode{ false }, mpParent{ nullptr },
		mpChildStartPos{nullptr}, mIsChildStartSet{ false } {}

	TreeNode() :
		mAction{}, mPlayerNode{}, mChanceNode{}, mProbability{ 1.0 },
		mIsPlayerNode{ false }, mIsChanceNode{ false },
		mIsTerminalNode{ true }, mpParent{ nullptr },
		mpChildStartPos{nullptr}, mIsChildStartSet{ false } {}

	/**
		* @brief Constructors that give node a parent tree node.
		* @param parent Pointer to parent Tree Node.
		*/
	TreeNode(PlayerNode p, TreeNode* parent) : 
		TreeNode{ p }, mpParent{ parent } {}

	TreeNode(ChanceNode c, TreeNode* parent) :
		TreeNode{ c }, mpParent{ parent } {}

	TreeNode(TreeNode* parent) :
		TreeNode{}, mpParent{ parent } {}


	/**
		* @brief Constructors that set action taken to get to the node.
		* @param a Action taken by parent node.
		*/
	TreeNode(PlayerNode p, Action a) :
		TreeNode{ p }, mAction{ a } {}

	TreeNode(ChanceNode c, Action a) :
		TreeNode{ c }, mAction{ a } {}

	TreeNode(Action a) : TreeNode{}, mAction{ a } {}

	/**
		* @brief Constructors that set probability to get to the node.
		* @param prob Probability of reach node.
		*/
	TreeNode(PlayerNode p, float prob) :
		TreeNode{ p }, mProbability{ prob } {}

	TreeNode(ChanceNode c, float prob) :
		TreeNode{ c }, mProbability{ prob } {}

	TreeNode(float prob) :
		TreeNode{}, mProbability{ prob } {}


	bool IsPlayerNode() { return mIsPlayerNode; }
	bool IsChanceNode() { return mIsChanceNode; }
	bool IsTerminalNode() { return mIsTerminalNode; }
	bool IsChildOffsetSet() { return mIsChildStartSet; }

	Action GetAction() { return mAction; }
	PlayerNode GetPlayerNode() { return mPlayerNode; }
	ChanceNode GetChanceNode() { return mChanceNode; }
	float GetProbability() { return mProbability; }
	byte* GetChildOffset() { return mpChildStartPos; }

	std::string HistoryHash() {
		if (!this->IsPlayerNode()) {
			return "";
		}
		bool isPlayerOne = this->GetPlayerNode().IsPlayerOne();
		return HistoryHashRecursive(isPlayerOne);
	}

	TreeNodeList HistoryList() {
		TreeNodeList historyList;
		HistoryListRecursive(historyList, nullptr);
		return historyList;
	}

	void UpdateParentOffset(byte* childOffset) {
		if (this->mpParent == nullptr) {
			return;
		}
		if (this->mpParent->IsChildOffsetSet()) {
			return;
		}
		this->mpParent->mpChildStartPos = childOffset;
		this->mpParent->mIsChildStartSet = true;
	}


private:

	std::string HistoryHashRecursive(bool isPlayerOne) {
		if (mpParent == nullptr) {
			return "";
		} else if (this->IsChanceNode()) {
			return this->GetChanceNode().ToHash() + mpParent->HistoryHash();
		} else if (this->GetPlayerNode().IsPlayerOne() == isPlayerOne) {
			std::string currentHash = this->GetPlayerNode().ToInfoSetHash();
			currentHash += this->GetAction().ToHash();
			return currentHash + mpParent->HistoryHashRecursive(isPlayerOne);
		} else {
			return this->GetAction().ToHash() + mpParent->HistoryHashRecursive(isPlayerOne);
		}	
	}

	void HistoryListRecursive(TreeNodeList& historyList, TreeNode* lastChild) {
		if (this == nullptr) {
			return;
		}
		else if (this->mpParent == nullptr) {
			historyList.push_back(*this);
		}
		else {
			this->mpParent->HistoryListRecursive(historyList, this);
			TreeNode toAdd(*this);
			if (lastChild != nullptr) {
				toAdd.mAction = lastChild->GetAction();
			}
			else {
				toAdd.mAction = Action();
			}
			historyList.push_back(toAdd);
		}
	}
};

template<typename Action, typename PlayerNode, typename ChanceNode>
static std::vector<float> ToFloatList
(
	std::vector<ClientNode<Action, PlayerNode, ChanceNode>>& nodeList
) {
	std::vector<float> floatList;
	for (ClientNode<Action, PlayerNode, ChanceNode> node : nodeList) {
		floatList.push_back(node.GetProbability());
	}
	return floatList;
}

//template<typename Action, typename PlayerNode, typename ChanceNode>
//class HistoryList {
//public:
//	std::vector<TreeNode<Action, PlayerNode, ChanceNode>*> mList;
//
//	HistoryList() {}
//	void push_back(TreeNode<Action, PlayerNode, ChanceNode>* node) {
//		mList.push_back(node);
//	}
//	typename std::vector<TreeNode<Action, PlayerNode, ChanceNode>*>::iterator begin() { return mList.begin(); }
//	typename std::vector<TreeNode<Action, PlayerNode, ChanceNode>*>::iterator end() { return mList.end(); }
//	int size() { return mList.size(); }
//};

namespace CfrConcepts {

	


	template<typename Action, typename PlayerNode, typename ChanceNode, typename GameState>
	concept PlayerNodeChildFunc = requires( Action a, PlayerNode p, GameState g ) {

		/*Player Node must have function that returns child node of an action.*/
		{ p.Child(a, &g) } -> std::convertible_to<ClientNode<Action, PlayerNode, ChanceNode>>;

	};

	template<typename Action, typename PlayerNode, typename GameState>
	concept PlayerNodeActionListFunc = requires ( PlayerNode p, GameState g ) {
		/*Player node must have function that returns a list of actions it can take*/
		{ p.ActionList(&g) } -> std::convertible_to<std::vector<Action>>;
	};



	template<typename Action, typename PlayerNode, typename ChanceNode, typename GameState>
	concept ChanceNodeChildrenFunc = requires( Action a, PlayerNode p, ChanceNode c, GameState g ) {

		/*Chance Node must have function that returns vector of child nodes*/
		{ c.Children(&g) } -> std::convertible_to<std::vector<ClientNode<Action, PlayerNode, ChanceNode>>>;

	};

	template<typename Action, typename PlayerNode, typename ChanceNode, typename GameClass>
	concept NeedsUtilityFunc = requires( Action a, PlayerNode p, ChanceNode c, GameClass g ) {
		
		{ g.UtilityFunc(std::vector<TreeNode<Action, PlayerNode, ChanceNode>>()) } ->
			std::convertible_to<float>;
	};

}


















