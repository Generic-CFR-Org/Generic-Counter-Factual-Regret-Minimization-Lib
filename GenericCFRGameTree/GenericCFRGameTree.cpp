// GenericCFRGameTree.cpp : Defines the functions for the static library.
//

#include "pch.h"
#include "framework.h"
//#include "genericCFRGameTree.h"
//
//
///**
// * @brief A helper function to the type of a child node.
// * @param child_node 
// * @return true if child node is a Game Node, false otherwise
// */
//template<
//	typename Action, typename CommonGameState, typename GameState,
//	typename TerminalNode, typename ChanceNode
//>
//inline bool CFRGameTree<Action, CommonGameState, GameState, TerminalNode, ChanceNode>
//::IsChildGameNode(ChildNode* child_node) {
//	return std::get<0>(*child_node) != nullptr;
//}
//
///**
// * @brief A helper function to the type of a child node.
// * @param child_node
// * @return true if child node is a Terminal Node, false otherwise
// */
//template<
//	typename Action, typename CommonGameState, typename GameState,
//	typename TerminalNode, typename ChanceNode
//>
//inline bool CFRGameTree<Action, CommonGameState, GameState, TerminalNode, ChanceNode>
//::IsChildTerminal(ChildNode *child_node) {
//	return std::get<1>(*child_node) != nullptr;
//}
//
///**
// * @brief A helper function to the type of a child node.
// * @param child_node
// * @return true if child node is a Chance Node, false otherwise
// */
//template<
//	typename Action, typename CommonGameState, typename GameState,
//	typename TerminalNode, typename ChanceNode
//>
//inline bool CFRGameTree<Action, CommonGameState, GameState, TerminalNode, ChanceNode>
//::IsChildChance(ChildNode* child_node) {
//	return std::get<2>(*child_node) != nullptr;
//}
//
//
//
///**
// * @brief A helper function for pre - processing the game tree.
// * @param curr_node 
// * @return The size of the current GameNode and all of its descendants in bytes.
// */
//template<
//	typename Action, typename CommonGameState, typename GameState,
//	typename TerminalNode, typename ChanceNode
//>
//inline int CFRGameTree<Action, CommonGameState, GameState, TerminalNode, ChanceNode>
//::PreProcessorHelper(GameNode *curr_node) {
//	
//	GameState *curr_gamestate = std::get<GameState*>(*curr_node);
//	StrategyProfile *curr_strategy = std::get<StrategyProfile*>(*curr_node);
//	int gamestate_size = sizeof(*curr_gamestate);
//	int strategy_size = 2 * sizeof(float) * curr_strategy->size();
//	int node_size = gamestate_size + strategy_size;
//
//	int children_total_size = 0;
//	for (Action* action : curr_strategy) {
//		ChildNode child_node = childNodeFunc(curr_gamestate, action, commonState);
//		if (IsChildGameNode(child_node)) {
//			children_total_size += PreProcessorHelper(std::get<0>(child_node));
//		}
//		if (IsChildTerminal(child_node)) {
//			children_total_size += PreProcessorHelper(std::get<1>(child_node));
//		}
//		if (IsChildChance(child_node)) {
//			children_total_size += PreProcessorHelper(std::get<2>(child_node));
//		}
//	}
//	return node_size + children_total_size;
//}
//
///**
// * @brief  A helper function for pre - processing the game tree.
// * @param curr_node 
// * @return The size of the current Terminal Node, which has no descendants.
// */
//template<
//	typename Action, typename CommonGameState, typename GameState,
//	typename TerminalNode, typename ChanceNode
//>
//inline int CFRGameTree<Action, CommonGameState, GameState, TerminalNode, ChanceNode>
//::PreProcessorHelper(TerminalNode *curr_node) {
//	return sizeof(*curr_node);
//}
//
///**
// * @brief  A helper function for pre - processing the game tree.
// * @param curr_node 
// * @return The size of current Chance Node and all of its descendants in bytes.
// */
//template<
//	typename Action, typename CommonGameState, typename GameState,
//	typename TerminalNode, typename ChanceNode
//>
//inline int CFRGameTree<Action, CommonGameState, GameState, TerminalNode, ChanceNode>
//::PreProcessorHelper(ChanceNode *curr_node) {
//	
//	int node_size = sizeof(*curr_node);
//
//	ChildrenFromChance children_nodes = childrenFromChanceFunc(curr_node);
//	GameNodeListFromChance* children_game_nodes = std::get<0>(children_nodes);
//	TerminalNodeListFromChance* children_terminal_nodes = std::get<0>(children_nodes);
//
//	int children_total_size = 0;
//	for (GameNodeWithProb game_node_p : children_game_nodes) {
//		children_total_size += PreProcessorHelper(game_node_p[0]);
//	}
//	for (TerminalNodeWithProb terminal_node_p : children_terminal_nodes) {
//		children_total_size += PreProcessorHelper(terminal_node_p[0]);
//	}
//	
//	return node_size + children_total_size;
//}
//
//
///**
// * @brief Preprocess GameTree before construction
// * @return Size of entire game tree in bytes
// */
//template<typename Action, typename CommonGameState, typename GameState, typename TerminalNode, typename ChanceNode>
//int CFRGameTree<Action, CommonGameState, GameState, TerminalNode, ChanceNode>::PreProcessor() {
//	return PreProcessorHelper(startingChanceNode);
//}