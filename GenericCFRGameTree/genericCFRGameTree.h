#pragma once
#if _WIN32 || _WIN64
	#if _WIN64
		#define ENVIRONMENT64
	#else
		#define ENVIRONMENT32
#endif
#endif

#if __GNUC__
	#if __x86_64__ || __ppc64__
		#define ENVIRONMENT64
	#else
		#define ENVIRONMENT32
	#endif
#endif

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

	typedef unsigned char byte;

	CFRGameTree() {

		commonState = nullptr;
		childNodeFunc = nullptr;
		childrenFromChanceFunc = nullptr;
		strategyProfileList = nullptr;
		startingChanceNode = nullptr;

		serializeGameState = nullptr;
		serializeTerminalNode = nullptr;
		serializeChanceNode = nullptr;
		deserializeGameState = nullptr;
		deserializeTerminalNode = nullptr;
		deserializeChanceNode = nullptr;

		gameTree = nullptr;
		sizeAtDepth = nullptr;
		cumulativeSizeAtDepth = nullptr;
		regretFunc = nullptr;	
	}

	CFRGameTree(
		CommonGameState* common_state,
		std::tuple<GameNode*, TerminalNode*, ChanceNode*>* ( *child_node_from_gamenode )( GameState*, Action*, CommonGameState*, StrategyProfileList* ),
		ChildrenFromChance* ( *child_node_from_chancenode ) ( ChanceNode*, CommonGameState*, StrategyProfileList*),
		ChanceNode* starting_chance_node,
		StrategyProfileList* all_strategies,
		int ( *serialize_gamestate )(GameState*, byte*),
		int ( *serialize_terminal_node )( TerminalNode*, byte* ),
		int ( *serialize_chance_node )( ChanceNode*, byte* ),
		GameState* ( *deserialize_gamestate )( byte*, int ),
		TerminalNode* ( *deserialize_terminal_node )( byte*, int ),
		ChanceNode* ( *deserialize_chance_node )( byte*, int ),
		int ( *regret_from_terminal )( TerminalNode* )
	) {
		commonState = common_state;
		childNodeFunc = child_node_from_gamenode;
		childrenFromChanceFunc = child_node_from_chancenode;
		strategyProfileList = all_strategies;
		startingChanceNode = starting_chance_node;

		serializeGameState = serialize_gamestate;
		serializeTerminalNode = serialize_terminal_node;
		serializeChanceNode = serialize_chance_node;
		deserializeGameState = deserialize_gamestate;
		deserializeTerminalNode = deserialize_terminal_node;
		deserializeChanceNode = deserialize_chance_node;
		
		regretFunc = regret_from_terminal;
		sizeAtDepth = nullptr;
		cumulativeSizeAtDepth = nullptr;
		cumulativeOffsetAtDepth = nullptr;
		gameTree = nullptr;
	}

	int PreProcessor();
	void ConstructTree();
	void CFR(int, float);
	
private:


	/*INSTANCE MEMBERS*/
	/*byte *gameTree;*/
	byte* gameTree;
	CommonGameState *commonState;
	ChanceNode *startingChanceNode;
	StrategyProfileList *strategyProfileList;
	std::vector<long> *sizeAtDepth;
	std::vector<long> *cumulativeSizeAtDepth;
	std::vector<long> *cumulativeOffsetAtDepth;

	/*Functions that retrieve child node in Game Tree*/
	ChildNode* (*childNodeFunc)( GameState*, Action*, CommonGameState*, StrategyProfileList* );
	ChildrenFromChance* (*childrenFromChanceFunc) ( ChanceNode*, CommonGameState*, StrategyProfileList*);

	/*Functions that serialize / deserialize nodes in Game Tree*/
	int ( *serializeGameState) (GameState*, byte*);
	int ( *serializeTerminalNode ) ( TerminalNode*, byte* );
	int ( *serializeChanceNode ) ( ChanceNode*, byte* );
	GameState* ( *deserializeGameState ) ( byte*, int );
	TerminalNode* ( *deserializeTerminalNode ) ( byte*, int );
	ChanceNode* ( *deserializeChanceNode ) ( byte*, int );

	
	int ( *regretFunc ) ( TerminalNode* );


	/**
	 * @brief Helper function to find the type of a child node.
	 * @param  ChildNode*
	 * @return Returns true if the ChildNode matches the Node type specified by the function name.
	 *		   Returns false otherwise.
	 */
	static bool IsChildGameNode(ChildNode*);
	static bool IsChildTerminal(ChildNode*);
	static bool IsChildChance(ChildNode*);

	/**
	 * @brief Helper functions for pre - processing the game tree.
	 * @param Node*
	 * @return Returns the size of a serialized node and all of its descendants.
	 */
	int PreProcessorHelper(GameNode*, int depth);
	int PreProcessorHelper(TerminalNode*, int depth);
	int PreProcessorHelper(ChanceNode*, int depth);

	void TreeConstructorHelper(GameNode*, int);
	void TreeConstructorHelper(TerminalNode*, int);
	void TreeConstructorHelper(ChanceNode*, int);

	float CFRHelper(GameNode*, float);
	float CFRHelper(TerminalNode*, float);
	float CFRHelper(ChanceNode*, float);
};

/**
* @brief A helper function to find the type of a child node.
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
* @brief A helper function to find the type of a child node.
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
* @brief A helper function to find the type of a child node.
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
* @return The size of the a serialized current GameNode and all of its descendants in bytes.
*/
template<
	typename Action, typename CommonGameState, typename GameState,
	typename TerminalNode, typename ChanceNode
>
inline int CFRGameTree<Action, CommonGameState, GameState, TerminalNode, ChanceNode>
	::PreProcessorHelper(CFRGameTree::GameNode* curr_node, int depth) {
	if (sizeAtDepth->size() <= depth) {
		sizeAtDepth->push_back(0);
	}
	GameState* curr_gamestate = std::get<GameState*>(*curr_node);
	CFRGameTree::StrategyProfile* curr_strategy = std::get<CFRGameTree::StrategyProfile*>(*curr_node);

	byte* temp_buf = new byte[sizeof(*curr_gamestate)];
	int gamestate_size = serializeGameState(curr_gamestate, temp_buf);
	delete temp_buf;
	int strategy_size = 3 * sizeof(float) * curr_strategy->size();

	//sizeof(uint8_t)* 2:	required for storage of node identifier "c", size of gamestate "uint_8"
	//sizeof(uint16_t):		required for storage of size of all child nodes.
	//sizeof(byte*)	:		required for storage of starting offset for children.
	int node_size = gamestate_size + strategy_size + (2 * sizeof(uint8_t)) + sizeof(byte*);

	int children_total_size = 0;
	for (auto action = curr_strategy->begin(); action != curr_strategy->end(); action++) {
		CFRGameTree::ChildNode* child_node = childNodeFunc(curr_gamestate, *action, commonState, strategyProfileList);
		if (IsChildGameNode(child_node)) {
			children_total_size += PreProcessorHelper(std::get<0>(*child_node), depth + 1);
		}
		if (IsChildTerminal(child_node)) {
			children_total_size += PreProcessorHelper(std::get<1>(*child_node), depth + 1);
		}
		if (IsChildChance(child_node)) {
			children_total_size += PreProcessorHelper(std::get<2>(*child_node), depth + 1);
		}
	}
	sizeAtDepth->at(depth) += node_size;
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
	::PreProcessorHelper(TerminalNode* curr_node, int depth){
	byte* temp_buf = new byte[sizeof(*curr_node)];
	int terminal_node_size = serializeTerminalNode(curr_node, temp_buf) + (2 * sizeof(uint8_t));
	delete temp_buf;
	if (sizeAtDepth->size() <= depth) {
		sizeAtDepth->push_back(terminal_node_size);
	}
	else {
		sizeAtDepth->at(depth) += terminal_node_size;
	}
	return terminal_node_size;
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
	::PreProcessorHelper(ChanceNode* curr_node, int depth) {
	if (sizeAtDepth->size() <= depth) {
		sizeAtDepth->push_back(0);
	}
	byte* temp_buf = new byte[sizeof(*curr_node)];
	int chance_node_size = serializeChanceNode(curr_node, temp_buf);
	delete temp_buf;
	int node_size = chance_node_size + ( 2 * sizeof(uint8_t) ) + sizeof(uint16_t) + sizeof(byte*);;

	CFRGameTree::ChildrenFromChance* children_nodes = childrenFromChanceFunc(curr_node, commonState, strategyProfileList);
	CFRGameTree::GameNodeListFromChance* children_game_nodes = std::get<CFRGameTree::GameNodeListFromChance*>(*children_nodes);
	CFRGameTree::TerminalNodeListFromChance* children_terminal_nodes = std::get<CFRGameTree::TerminalNodeListFromChance*>(*children_nodes);

	int children_total_size = 0;
	for (auto game_node_p = (*children_game_nodes).begin(); game_node_p != (*children_game_nodes).end(); game_node_p++) {
		GameNode* new_game_node = std::get<GameNode*>(**game_node_p);
		children_total_size += PreProcessorHelper(new_game_node, depth + 1);
	}
	for (auto terminal_node_p = (*children_terminal_nodes).begin(); terminal_node_p != (*children_terminal_nodes).end(); terminal_node_p++) {
		TerminalNode* new_terminal_node = std::get<TerminalNode*>(**terminal_node_p);
		children_total_size += PreProcessorHelper(new_terminal_node, depth + 1);
	}
	sizeAtDepth->at(depth) += node_size;
	return node_size + children_total_size;
}




/**
* @brief Preprocess GameTree before construction
* @return Size of entire game tree in bytes
*/
template<typename Action, typename CommonGameState, typename GameState, typename TerminalNode, typename ChanceNode>
int CFRGameTree<Action, CommonGameState, GameState, TerminalNode, ChanceNode>::PreProcessor() {
	sizeAtDepth = new std::vector<long>();
	int tree_size = PreProcessorHelper(startingChanceNode, 0);
	int cumulative_depth_size = 0;
	cumulativeSizeAtDepth = new std::vector<long>(sizeAtDepth->size(), 0);
	for (int i = 0; i < sizeAtDepth->size(); i++) {
		long depth_size = sizeAtDepth->at(i);
		cumulative_depth_size += depth_size;
		cumulativeSizeAtDepth->push_back(cumulative_depth_size);
	}
	return tree_size;
}



/*GAME TREE CONSTRUCTOR DEFINITIONS*/
typedef unsigned char byte;

/**
 * @brief Sets current GameNode and metadata in gameTree and recursively sets its children.
 *		  Uses Depth First Search to set nodes on left most part of tree, using a 
 *		  Cumulative Offset counter at depth i to create an easily traversable tree structure.
 * @param curr_node 
 * @param depth 
 */
template<typename Action, typename CommonGameState, typename GameState, typename TerminalNode, typename ChanceNode>
inline void CFRGameTree<Action, CommonGameState, GameState, TerminalNode, ChanceNode>::TreeConstructorHelper(GameNode* curr_node, int depth) {
	
	GameState* curr_gamestate = std::get<GameState*>(*curr_node);
	CFRGameTree::StrategyProfile* curr_strategy = std::get<CFRGameTree::StrategyProfile*>(*curr_node);

	//Obtain offset using cumulativeOffsetAtDepth as part of Depth First Search tree construction.
	long offset = cumulativeOffsetAtDepth->at(depth);
	long inital_offset = offset;

	//Set classifier "g" at offset to indicate that this node is a gamenode
	gameTree[offset++] = (byte) "g";

	//Stores size of serialized gamestate
	long offset_size_ptr = offset;
	offset++;
	//Stores serialized gamestate
	uint8_t gamestate_size = serializeGameState(curr_gamestate, gameTree + offset);
	gameTree[offset_size_ptr] = (byte) gamestate_size;
	offset += gamestate_size;


	//Stores number of children / number of acitons
	int num_actions = curr_strategy->size();
	gameTree[offset++] = (uint8_t) num_actions;

	//Stores floating point values for cumulative regret, strategy probabilities, and current regret.
	float uniform_prob = 1.0 / ((float) num_actions);
	int total_actions_size = 3 * num_actions * sizeof(float);
	for (long offset_temp = offset; offset_temp < offset + ( total_actions_size ); offset_temp += sizeof(float)) {
		gameTree[offset_temp] = uniform_prob;
	}
	offset += total_actions_size;

	//Stores starting offset of children.
	gameTree[offset] = cumulativeOffsetAtDepth->at(depth + 1);
	offset += sizeof(long);

	//Update cumulativeOffsetAtDepth for Depth First Search tree construction.
	long offset_diff = offset - inital_offset;
	cumulativeOffsetAtDepth->at(depth) += offset_diff;

	//Recursively call Tree Constructor Helper on each child at next depth.
	int children_total_size = 0;
	for (auto action = curr_strategy->begin(); action != curr_strategy->end(); action++) {
		CFRGameTree::ChildNode* child_node = childNodeFunc(curr_gamestate, *action, commonState, strategyProfileList);
		if (IsChildGameNode(child_node)) {
			GameNode* child_game_node = std::get<0>(*child_node);
			TreeConstructorHelper(child_game_node, depth + 1);
		}
		if (IsChildTerminal(child_node)) {
			TerminalNode* child_terminal_node = std::get<1>(*child_node);
			TreeConstructorHelper(child_terminal_node, depth + 1);
		}
		if (IsChildChance(child_node)) {
			ChanceNode* child_chance_node = std::get<2>(*child_node);
			TreeConstructorHelper(child_chance_node, depth + 1);
		}
	}
	return ;
}

/**
 * @brief Sets current TerminalNode and metadata in gameTree.
 *		  Uses Depth First Search to set nodes on left most part of tree, using a
 *		  Cumulative Offset counter at depth i to create an easily traversable tree structure.
 * @param curr_node
 * @param depth
 */
template<typename Action, typename CommonGameState, typename GameState, typename TerminalNode, typename ChanceNode>
inline void CFRGameTree<Action, CommonGameState, GameState, TerminalNode, ChanceNode>::TreeConstructorHelper(TerminalNode* curr_node, int depth) {
	long offset = cumulativeOffsetAtDepth->at(depth);
	int inital_offset = offset;

	//Set classifier to "t" to identify that the node is a Terminal Node
	gameTree[offset] = (byte) "t";
	offset++;

	//Stores size of serialized terminal node
	int offset_size_ptr = offset;
	offset++;

	//Stores serialized terminal node
	uint8_t terminal_node_size = serializeTerminalNode(curr_node, gameTree + offset);
	gameTree[offset_size_ptr] = (byte) terminal_node_size;
	offset += terminal_node_size;

	//Update Cumulative Offset At Depth for Depth First Search Tree Construction.
	long offset_diff = offset - inital_offset;
	cumulativeOffsetAtDepth->at(depth) += offset_diff;
	return;
}

/**
 * @brief Sets current ChanceNode and metadata in gameTree and recursively sets its children.
 *		  Uses Depth First Search to set nodes on left most part of tree, using a
 *		  Cumulative Offset counter at depth i to create an easily traversable tree structure.
 * @param curr_node
 * @param depth
 */
template<typename Action, typename CommonGameState, typename GameState, typename TerminalNode, typename ChanceNode>
inline void CFRGameTree<Action, CommonGameState, GameState, TerminalNode, ChanceNode>::TreeConstructorHelper(ChanceNode* curr_node, int depth) {
	
	long offset = cumulativeOffsetAtDepth->at(depth);
	int initial_offset = offset;

	//Set classifier to "c" to identify that the node is a Chance Node
	gameTree[offset++] = (byte) "c";

	//Store size of Chance Node
	int offset_size_ptr = offset++;

	//Store serialized Chance Node
	uint8_t chance_node_size = serializeChanceNode(curr_node, gameTree + offset);
	gameTree[offset_size_ptr] = (byte) chance_node_size;
	offset += chance_node_size;

	CFRGameTree::ChildrenFromChance* children_nodes = childrenFromChanceFunc(curr_node, commonState, strategyProfileList);
	CFRGameTree::GameNodeListFromChance* children_game_nodes = std::get<CFRGameTree::GameNodeListFromChance*>(*children_nodes);
	CFRGameTree::TerminalNodeListFromChance* children_terminal_nodes = std::get<CFRGameTree::TerminalNodeListFromChance*>(*children_nodes);

	//Stores number of children 
	int num_children = children_game_nodes->size() + children_terminal_nodes->size();
	gameTree[offset++] = (uint8_t) num_children;

	//Stores starting offset for children
	gameTree[offset] = cumulativeOffsetAtDepth->at(depth + 1);
	offset += sizeof(long);

	//Update Cumulative Offset At Depth for Depth First Search Tree Construction
	long offset_diff = offset - initial_offset;
	cumulativeOffsetAtDepth->at(depth) += offset_diff;

	//Recursively call Tree Constructor Helper on each child at next depth.
	for (auto game_node_p = children_game_nodes->begin(); game_node_p != children_game_nodes->end(); game_node_p++) {
		GameNode* new_game_node = std::get<GameNode*>(**game_node_p);
		TreeConstructorHelper(new_game_node, depth + 1);
	}
	for (auto terminal_node_p = children_terminal_nodes->begin(); terminal_node_p != children_terminal_nodes->end(); terminal_node_p++) {
		TerminalNode* new_terminal_node = std::get<TerminalNode*>(**terminal_node_p);
		TreeConstructorHelper(new_terminal_node, depth + 1);
	}
	return;
}


/**
 * @brief Calls Tree Constructor Helper on starting Chance node (the root of the tree).
 *		  Sets values for CumulativeOffsetAtDepth that continously updates throughout
 *		  the traversal of the tree.
		  This creates a tree structure that is organized by the depth of nodes.
 */
template<typename Action, typename CommonGameState, typename GameState, typename TerminalNode, typename ChanceNode>
inline void CFRGameTree<Action, CommonGameState, GameState, TerminalNode, ChanceNode>::ConstructTree() {
	int tree_size = PreProcessor();
	gameTree = new byte[tree_size];
	cumulativeOffsetAtDepth = new std::vector<long>(cumulativeSizeAtDepth->size());
	cumulativeOffsetAtDepth->at(0) = 0;
	for (int i = 0; i < cumulativeSizeAtDepth->size() - 1; i++) {
		cumulativeOffsetAtDepth->at(i + 1) = cumulativeSizeAtDepth->at(i);
	}
	TreeConstructorHelper(startingChanceNode, 0);
}


template<typename Action, typename CommonGameState, typename GameState, typename TerminalNode, typename ChanceNode>
inline float CFRGameTree<Action, CommonGameState, GameState, TerminalNode, ChanceNode>
::CFRHelper(GameNode* curr_node, float reach_prob) {

}

template<typename Action, typename CommonGameState, typename GameState, typename TerminalNode, typename ChanceNode>
inline float CFRGameTree<Action, CommonGameState, GameState, TerminalNode, ChanceNode>
::CFRHelper(TerminalNode* curr_node, float reach_prob) {
	return regretFunc(curr_node);
}

template<typename Action, typename CommonGameState, typename GameState, typename TerminalNode, typename ChanceNode>
inline float CFRGameTree<Action, CommonGameState, GameState, TerminalNode, ChanceNode>
::CFRHelper(ChanceNode* curr_node, float reach_prob) {
	
	return 0.0f;
}

template<typename Action, typename CommonGameState, typename GameState, typename TerminalNode, typename ChanceNode>
inline void CFRGameTree<Action, CommonGameState, GameState, TerminalNode, ChanceNode>
::CFR(int iterations, float accuracy) {
	
	return;
}

