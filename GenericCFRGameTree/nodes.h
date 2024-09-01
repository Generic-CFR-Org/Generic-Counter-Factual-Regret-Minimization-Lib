#pragma once
#include "pch.h"
#include "framework.h"
#include <cstddef>
#include <concepts>
#include <functional>
#include <string>
#include "cfr_concepts.h"

namespace Cfr {

	namespace TreeNodes {

		template<typename Action, typename PlayerNode, typename ChanceNode>
		concept Hashable = requires( Action a, PlayerNode p, ChanceNode c ) {
			/*All Game classes must have ToHash() func that returns a string.*/
			{ a.ToHash() } -> std::convertible_to<std::string>;
			{ p.ToHash() } -> std::convertible_to<std::string>;
			{ c.ToHash() } -> std::convertible_to<std::string>;

			//PlayerNode must have ToInfoSetHash() func that returns a string for the player view.
			{ p.ToInfoSetHash() } -> std::convertible_to<std::string>;
		};

		template<typename Action, typename PlayerNode, typename ChanceNode>
		concept PlayerNodeFuncs = requires( Action a, PlayerNode p, ChanceNode c ) {


			/*PlayerNode must have function IsPlayerOne() that returns a bool for the currently
				acting player*/
			{ p.IsPlayerOne() } -> std::convertible_to<std::vector<bool>>;
		};

		template<typename Action, typename PlayerNode, typename ChanceNode>
		class TreeNode {

		protected:
			Action mAction;
			PlayerNode mPlayerNode;

			ChanceNode mChanceNode;
			float mProbability;

			bool mIsPlayerNode;
			bool mIsChanceNode;
			bool mIsTerminalNode;

		public:

			TreeNode() :
				mAction{}, mPlayerNode{}, mChanceNode{}, mProbability{ 1.0 },
				mIsPlayerNode{ false }, mIsChanceNode{ false }, mIsTerminalNode{ true } {}

			TreeNode(PlayerNode p) :
				mAction{}, mPlayerNode{ p }, mChanceNode{}, mProbability{ 1.0 },
				mIsPlayerNode{ true }, mIsChanceNode{ false }, mIsTerminalNode{ false } {}

			TreeNode(ChanceNode c) :
				mAction{}, mPlayerNode{}, mChanceNode{ c }, mProbability{ 1.0 },
				mIsPlayerNode{ false }, mIsChanceNode{ true }, mIsTerminalNode{ false } {}

			TreeNode(Action a) :
				mAction{ a }, mPlayerNode{}, mChanceNode{}, mProbability{ 1.0 },
				mIsPlayerNode{ false }, mIsChanceNode{ false }, mIsTerminalNode{ true } {}

			TreeNode(PlayerNode p, Action a) :
				mAction{ a }, mPlayerNode{ p }, mChanceNode{}, mProbability{ 1.0 },
				mIsPlayerNode{ true }, mIsChanceNode{ false }, mIsTerminalNode{ false } {}

			TreeNode(ChanceNode c, Action a) :
				mAction{ a }, mPlayerNode{}, mChanceNode{ c }, mProbability{ 1.0 },
				mIsPlayerNode{ false }, mIsChanceNode{ true }, mIsTerminalNode{ false } {}

			TreeNode(PlayerNode p, float prob) :
				mAction{}, mPlayerNode{ p }, mChanceNode{}, mProbability{ prob },
				mIsPlayerNode{ true }, mIsChanceNode{ false }, mIsTerminalNode{ false } {}

			TreeNode(ChanceNode c, float prob) :
				mAction{}, mPlayerNode{}, mChanceNode{ c }, mProbability{ prob },
				mIsPlayerNode{ false }, mIsChanceNode{ true }, mIsTerminalNode{ false } {}

			TreeNode(float prob) :
				mAction{}, mPlayerNode{}, mChanceNode{}, mProbability{ prob },
				mIsPlayerNode{ false }, mIsChanceNode{ false }, mIsTerminalNode{ true } {}

			bool IsPlayerNode() { return mIsPlayerNode; }
			bool IsChanceNode() { return mIsChanceNode; }
			bool IsTerminalNode() { return mIsTerminalNode; }

			Action GetAction() { return mAction; }
			PlayerNode GetPlayerNode() { return mPlayerNode; }
			ChanceNode GetChanceNode() { return mChanceNode; }
			float GetProbability() { return mProbability; }

		};

		/**
			* @brief Templated class for child node of a player node.
			* @tparam Action - Action taken by the parent player Node.
			* @tparam PlayerNode - The child player node.
			* @tparam ChanceNode - The child chance node.
			*/
		template<typename Action, typename PlayerNode, typename ChanceNode>
		class PlayerNodeChild : protected TreeNode<Action, PlayerNode, ChanceNode> {

		public:

			PlayerNodeChild(PlayerNode p, Action a) :
				TreeNode<Action, PlayerNode, ChanceNode>{ p, a } {}

			PlayerNodeChild(ChanceNode c, Action a) :
				TreeNode<Action, PlayerNode, ChanceNode>{ c, a } {}

			PlayerNodeChild(Action a) :
				TreeNode<Action, PlayerNode, ChanceNode>{ a } {}

			decltype( Action ) ActionToChild() {
				return TreeNode<Action, PlayerNode, ChanceNode>::GetAction();
			}

			decltype( PlayerNode ) ChildPlayerNode() {
				return TreeNode<Action, PlayerNode, ChanceNode>::GetPlayerNode();
			}

			decltype( ChanceNode ) ChildChanceNode() {
				return TreeNode<Action, PlayerNode, ChanceNode>::GetChanceNode();
			}

		};

		/**
			* @brief Templated class for child node of a chance node node.
			* @tparam Action - Probability from parent chance node to the child node.
			* @tparam PlayerNode - The child player node.
			* @tparam ChanceNode - The child chance node.
			*/
		template<typename Action, typename PlayerNode, typename ChanceNode>
		class ChanceNodeChild : protected TreeNode<Action, PlayerNode, ChanceNode> {

		public:

			ChanceNodeChild(PlayerNode p, float prob) :
				TreeNode<Action, PlayerNode, ChanceNode>{ p, prob } {}

			ChanceNodeChild(ChanceNode c, float prob) :
				TreeNode<Action, PlayerNode, ChanceNode>{ c , prob } {}

			ChanceNodeChild(float prob) :
				TreeNode<Action, PlayerNode, ChanceNode>{ prob } {}

			float ProbabilityToChild() { return TreeNode<Action, PlayerNode, ChanceNode>::GetProbability(); }
			decltype( PlayerNode ) ChildPlayerNode() { return TreeNode<Action, PlayerNode, ChanceNode>::GetPlayerNode(); }
			decltype( ChanceNode ) ChildChanceNode() { return TreeNode<Action, PlayerNode, ChanceNode>::GetChanceNode(); }

		};


		template<typename Action, typename PlayerNode, typename ChanceNode>
			requires Hashable<Action, PlayerNode, ChanceNode>&&
		PlayerNodeFuncs<Action, PlayerNode, ChanceNode>
			class HistoryNode : protected TreeNode<Action, PlayerNode, ChanceNode> {
			HistoryNode* mParent;


			public:
				HistoryNode(PlayerNode p, Action a, HistoryNode* parent) :
					TreeNode<Action, PlayerNode, ChanceNode>{ p, a },
					mParent{ parent } {}

				HistoryNode(ChanceNode c, HistoryNode* parent) :
					TreeNode<Action, PlayerNode, ChanceNode>{ c },
					mParent{ parent } {}

				HistoryNode(HistoryNode* parent) :
					TreeNode<Action, PlayerNode, ChanceNode>{},
					mParent{ parent } {}

				decltype( PlayerNode ) HistoryPlayerNode() { return TreeNode<Action, PlayerNode, ChanceNode>::GetPlayerNode(); }
				decltype( Action ) PlayerNodeAction() { return TreeNode<Action, PlayerNode, ChanceNode>::GetAction(); }
				decltype( ChanceNode ) HistoryChanceNode() { return TreeNode<Action, PlayerNode, ChanceNode>::GetChanceNode(); }

				std::string HistoryHash() {
					if (!this->IsPlayerNode()) {
						return "";
					}
					bool isPlayerOne = this->HistoryPlayerNode().IsPlayerOne();
					return HistoryHashRecursive(isPlayerOne);
				}

				std::string HistoryHashRecursive(bool isPlayerOne) {
					if (mParent == nullptr) {
						return "";
					}
					else if (this->IsChanceNode()) {
						return HistoryChanceNode().ToHash() + mParent->HistoryHash();
					}
					else {
						if (this->HistoryPlayerNode().IsPlayerOne() == isPlayerOne) {
							std::string currentHash = HistoryPlayerNode().ToInfoSetHash();
							currentHash += PlayerNodeAction().ToHash();
							return currentHash + mParent->HistoryHashRecursive(isPlayerOne);
						}
						else {
							return PlayerNodeAction().ToHash() + mParent->HistoryHashRecursive(isPlayerOne);
						}
					}
				}
		};
	}
}



















