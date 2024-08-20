#pragma once


template <
	typename Action,
	typename CommonGameState,
	typename GameState,
	typename TerminalNode,
	typename ChanceNode
>
class CFRGameTree {

public:


	typedef std::vector<Action*> StrategyProfile;
	typedef std::vector<float> Probabilities;

	typedef std::tuple<StrategyProfile*, Probabilities*> Strategy;
	typedef std::tuple<GameState*, StrategyProfile*> GameNode;
	typedef std::tuple<GameNode*, float> GameNodeWithProb;
	typedef std::tuple<TerminalNode*, float> TerminalNodeWithProb;

	typedef std::vector<StrategyProfile*> StrategyProfileList;

	typedef std::vector<GameNode*> GameNodeList;
	typedef std::vector<TerminalNode*> TerminalNodeList;
	typedef std::vector<ChanceNode*> ChanceNodeList;

	typedef std::vector<GameNodeWithProb*> GameNodeListFromChance;
	typedef std::vector<TerminalNodeWithProb*> TerminalNodeListFromChance;

	typedef std::tuple<GameNode*, TerminalNode*, ChanceNode*> ChildNode;
	typedef std::tuple<GameNodeListFromChance*, TerminalNodeListFromChance*> ChildrenFromChance;

	CFRGameTree() {
		commonState = nullptr;
		childNodeFunc = nullptr;
		childrenFromChanceFunc = nullptr;
		strategyProfileList = nullptr;
		startingChanceNode = nullptr;
		
	}

	CFRGameTree(
		CommonGameState* common_state,
		std::tuple<GameNode*, TerminalNode*, ChanceNode*>* ( *child_node_from_gamenode )( GameState*, Action*, CommonGameState*, StrategyProfileList* ),
		ChildrenFromChance* ( *child_node_from_chancenode ) ( ChanceNode*, CommonGameState*, StrategyProfileList*),
		ChanceNode* starting_chance_node,
		StrategyProfileList* all_strategies
	) {
		commonState = common_state;
		childNodeFunc = child_node_from_gamenode;
		childrenFromChanceFunc = child_node_from_chancenode;
		strategyProfileList = all_strategies;
		startingChanceNode = starting_chance_node;
	}

	int PreProcessor();

private:

	char *gameTree = nullptr;
	CommonGameState *commonState;
	ChanceNode *startingChanceNode;
	StrategyProfileList *strategyProfileList;

	ChildNode* (*childNodeFunc)( GameState*, Action*, CommonGameState*, StrategyProfileList* );
	ChildrenFromChance* (*childrenFromChanceFunc) ( ChanceNode*, CommonGameState*, StrategyProfileList*);

	static bool IsChildGameNode(ChildNode*);
	static bool IsChildTerminal(ChildNode*);
	static bool IsChildChance(ChildNode*);

	int PreProcessorHelper(GameNode*);
	int PreProcessorHelper(TerminalNode*);
	int PreProcessorHelper(ChanceNode*);
};

/**
* @brief A helper function to the type of a child node.
* @param child_node
* @return true if child node is a Game Node, false otherwise
*/
template<
	typename Action, typename CommonGameState, typename GameState,
	typename TerminalNode, typename ChanceNode
>
inline bool CFRGameTree<Action, CommonGameState, GameState, TerminalNode, ChanceNode>
	::IsChildGameNode(CFRGameTree::ChildNode* child_node) {
	return std::get<0>(*child_node) != nullptr;
}

/**
* @brief A helper function to the type of a child node.
* @param child_node
* @return true if child node is a Terminal Node, false otherwise
*/
template<
	typename Action, typename CommonGameState, typename GameState,
	typename TerminalNode, typename ChanceNode
>
inline bool CFRGameTree<Action, CommonGameState, GameState, TerminalNode, ChanceNode>
	::IsChildTerminal(CFRGameTree::ChildNode* child_node) {
	return std::get<1>(*child_node) != nullptr;
}

/**
* @brief A helper function to the type of a child node.
* @param child_node
* @return true if child node is a Chance Node, false otherwise
*/
template<
	typename Action, typename CommonGameState, typename GameState,
	typename TerminalNode, typename ChanceNode
>
inline bool CFRGameTree<Action, CommonGameState, GameState, TerminalNode, ChanceNode>
	::IsChildChance(CFRGameTree::ChildNode* child_node) {
	return std::get<2>(*child_node) != nullptr;
}



/**
* @brief A helper function for pre - processing the game tree.
* @param curr_node
* @return The size of the current GameNode and all of its descendants in bytes.
*/
template<
	typename Action, typename CommonGameState, typename GameState,
	typename TerminalNode, typename ChanceNode
>
inline int CFRGameTree<Action, CommonGameState, GameState, TerminalNode, ChanceNode>
	::PreProcessorHelper(CFRGameTree::GameNode* curr_node) {

	GameState* curr_gamestate = std::get<GameState*>(*curr_node);
	CFRGameTree::StrategyProfile* curr_strategy = std::get<CFRGameTree::StrategyProfile*>(*curr_node);
	int gamestate_size = sizeof(*curr_gamestate);
	int strategy_size = 2 * sizeof(float) * curr_strategy->size();
	int node_size = gamestate_size + strategy_size;

	int children_total_size = 0;


	for (auto action = curr_strategy->begin(); action != curr_strategy->end(); action++) {
		CFRGameTree::ChildNode* child_node = childNodeFunc(curr_gamestate, *action, commonState, strategyProfileList);
		if (IsChildGameNode(child_node)) {
			children_total_size += PreProcessorHelper(std::get<0>(*child_node));
		}
		if (IsChildTerminal(child_node)) {
			children_total_size += PreProcessorHelper(std::get<1>(*child_node));
		}
		if (IsChildChance(child_node)) {
			children_total_size += PreProcessorHelper(std::get<2>(*child_node));
		}
	}

	/*for (Action* action : curr_strategy) {
		CFRGameTree::ChildNode child_node = childNodeFunc(curr_gamestate, action, commonState, strategyProfileList);
		if (IsChildGameNode(child_node)) {
			children_total_size += PreProcessorHelper(std::get<0>(child_node));
		}
		if (IsChildTerminal(child_node)) {
			children_total_size += PreProcessorHelper(std::get<1>(child_node));
		}
		if (IsChildChance(child_node)) {
			children_total_size += PreProcessorHelper(std::get<2>(child_node));
		}
	}*/
	return node_size + children_total_size;
}

/**
* @brief  A helper function for pre - processing the game tree.
* @param curr_node
* @return The size of the current Terminal Node, which has no descendants.
*/
template<
	typename Action, typename CommonGameState, typename GameState,
	typename TerminalNode, typename ChanceNode
>
inline int CFRGameTree<Action, CommonGameState, GameState, TerminalNode, ChanceNode>
	::PreProcessorHelper(TerminalNode* curr_node){
	return sizeof(*curr_node);
}

/**
* @brief  A helper function for pre - processing the game tree.
* @param curr_node
* @return The size of current Chance Node and all of its descendants in bytes.
*/
template<
	typename Action, typename CommonGameState, typename GameState,
	typename TerminalNode, typename ChanceNode
>
inline int CFRGameTree<Action, CommonGameState, GameState, TerminalNode, ChanceNode>
	::PreProcessorHelper(ChanceNode* curr_node) {

	int node_size = sizeof(*curr_node);

	CFRGameTree::ChildrenFromChance* children_nodes = childrenFromChanceFunc(curr_node, commonState, strategyProfileList);
	CFRGameTree::GameNodeListFromChance* children_game_nodes = std::get<CFRGameTree::GameNodeListFromChance*>(*children_nodes);
	CFRGameTree::TerminalNodeListFromChance* children_terminal_nodes = std::get<CFRGameTree::TerminalNodeListFromChance*>(*children_nodes);

	int children_total_size = 0;

	


	/*CFRGameTree::GameNodeWithProb** game_node_p;*/
	for (auto game_node_p = (*children_game_nodes).begin(); game_node_p != (*children_game_nodes).end(); game_node_p++) {
		GameNode* new_game_node = std::get<GameNode*>(**game_node_p);
		children_total_size += PreProcessorHelper(new_game_node);
	}
	/*CFRGameTree::TerminalNodeWithProb** terminal_node_p;*/
	for (auto terminal_node_p = (*children_terminal_nodes).begin(); terminal_node_p != (*children_terminal_nodes).end(); terminal_node_p++) {
		TerminalNode* new_terminal_node = std::get<TerminalNode*>(**terminal_node_p);
		children_total_size += PreProcessorHelper(new_terminal_node);
	}
	return node_size + children_total_size;
}


/**
* @brief Preprocess GameTree before construction
* @return Size of entire game tree in bytes
*/
template<typename Action, typename CommonGameState, typename GameState, typename TerminalNode, typename ChanceNode>
int CFRGameTree<Action, CommonGameState, GameState, TerminalNode, ChanceNode>::PreProcessor() {
	return PreProcessorHelper(startingChanceNode);
}


