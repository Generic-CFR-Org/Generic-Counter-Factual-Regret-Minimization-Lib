#pragma once

//Forward Declaration
struct TreeNodeChildren;

/*
###########################################
### GAME TREE UTILITY TYPES AND STRUCTS ###
###########################################
*/

/*Representation of byte type for gameTree byte array*/
typedef unsigned char byte;

/*Representation of Game node in constructed Game Tree*/
class TreeGameNode {
	
public:
	int8_t mPlayerToAct;
	uint8_t mNumActions;
	uint8_t mNumGameChildren;
	uint8_t mNumChanceChildren;
	uint8_t mNumTerminalChildren;
	long mpChildStartOffset;
	float* mpCurrStratArr;
	float* mpCumStratArr;
	float* mpCumRegretArr;
	uint8_t* mpNumActionPerChild;
	uint8_t* mpActionIndexPerChild;
	long mpNextNodePos;
	byte* mGameTreePtr;

	/**
	 * @brief Constructs TreeGameNode from pointer to the game Tree and the node's offset.
	 * @param pGameTree Byte pointer to the constructed Game Tree
	 * @param pGameNodePos Position of Chance Node in Game Tree Byte Array
	 * @return TreeGameNode struct
	 */
	TreeGameNode(byte* pGameTree, long pGameNodePos);

	/**
	 * @brief Gets (float) probability of Game Node Action indexed by 'index'
	 * @param index Index of Action's probabilty to retrieve within the Strategy Probability Array
	 * @return (float) value of probability
	 */
	float GetCurrStratProb(int index);

	/**
	 * @brief Gets (float) cumulative probability of Game Node Action indexed by 'index'
	 * @param index Index of Action's cumulative probabilty to retrieve within
					the Strategy Cumulative Probability Array
	 * @return (float) value of cumulative probability
	 */
	float GetCumStratProb(int index);

	/**
	 * @brief Gets (float) cumulative regret of Game Node Action indexed by 'index'
	 * @param index Index of regret to retrieve within the Cumulative Regret Array.
	 * @return (float) value of cumulative regret
	 */
	float GetCumRegret(int index);

	std::vector<uint8_t> GetActionIndicesForChild(int childIndex);

	/**
	 * @brief Updates (float) probability of Game Node Action indexed by 'index'
	 * @param index Index of Action's probabilty to update within Current Strategy Probability Array
	 */
	void SetCurrStratProb(float prob, int index);

	/**
	 * @brief Adds probability to (float) cumulative probability of Game Node Action indexed by 'index'
	 * @param index Index of Action's cumulative probabilty to update within
					the Strategy Cumulative Probability Array
	 */
	void AddCumStratProb(float prob, int index);

	/**
	 * @brief Adds regret to (float) cumulative regret of Game Node Action indexed by 'index'
	 * @param index Index of regret to update within the Cumulative Regret Array.
	 */
	void AddCumRegret(float regret, int index);

	/**
	 * @brief Get all Game node and Chance node children
	 * @return Struct containing all non terminal children.
	 */
	TreeNodeChildren GetChildren();
};

std::ostream& operator<<(std::ostream& os, TreeGameNode& treeGameNode);

/*Representation of Terminal Node in constructed Game Tree*/
class TreeTerminalNode {
public:
	float mUtilityVal;
	long mpNextNodePos;

	TreeTerminalNode(byte* pGameTree, long pTerminalNodePos);

	static int NodeSize();
};

/*Representation of Chance Node in constructed Game Tree*/
class TreeChanceNode {
public:
	uint8_t mNumGameChildren;
	uint8_t mNumTerminalChildren;
	long mpChildStartOffset;
	float* mpProbToChildArr;
	long mpNextNodePos;
	byte* mGameTreePtr;

	/**
	 * @brief Constructs TreeChanceNode from pointer to the game Tree and the node's offset.
	 * @param pGameTree Byte pointer to the constructed Game Tree
	 * @param pChaneNodePos Position of Chance Node in Game Tree Byte Array
	 * @return TreeChanceNode struct
	 */
	TreeChanceNode(byte* pGameTree, long pChanceNodePos);

	/**
	 * @brief Gets (float) probability of reaching Child Node indexed by 'index'
	 * @param treeGameNode Representation of Chance Node in the Game Tree.
	 * @param index Index of probabilty of reaching Child Node in Probability to Child Array.
	 * @return (float) value of probability of reaching Child Node.
	 */
	float GetChildReachProb(int index);

	/**
	 * @brief Get all Game node  children
	 * @return Struct containing all non terminal children.
	 */
	TreeNodeChildren GetChildren();
};

/*
##########################################
###     BYTE ARRAY UTILITY FUNCTIONS   ###
##########################################
*/
/**
* @brief Sets value at byte pointer to given float value.
* @param pByte Address to set to val.
* @param val Value of float to set.
*/
void SetFloatAtBytePtr(unsigned char* pByte, float val);

/**
* @brief Retrieves float value from given byte pointer.
* @param pByte Address to retrieve float value.
* @return
*/
float GetFloatFromBytePtr(unsigned char* pByte);


/*
##########################################
###     CHILD GETTER HELPER FUNCTION   ###
##########################################
*/

struct TreeNodeChildren {

	std::vector<TreeGameNode> treeGameNodes;
	std::vector<TreeChanceNode> treeChanceNodes;

	std::vector<TreeGameNode> GetChildrenGameNodes();
	std::vector<TreeChanceNode> GetChildrenChanceNodes();
	void AddChildNode(TreeGameNode);
	void AddChildNode(TreeChanceNode);
};

static TreeNodeChildren GetAllChildren(byte*, int, int, long);










