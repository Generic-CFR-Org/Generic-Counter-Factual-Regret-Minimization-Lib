#pragma once
#include "pch.h"
#include "framework.h"
#include <string>


/*
###########################################
# BYTE ARRAY UTILITY FUNCTION DEFINITIONS #
###########################################
*/

/**
 * @brief Util constants and functions to assist in tree preprocessing and construction.
 */
class TreeUtils {
public:
	typedef unsigned char byte;
	static const int NON_TERMINAL_BASE_SIZE = sizeof(char) + sizeof(uint8_t) + sizeof(byte*);
	static const int TERMINAL_SIZE = sizeof(char) + sizeof(float);
	static const int PLAYER_NODE_SIZE = NON_TERMINAL_BASE_SIZE + sizeof(byte*) + sizeof(char);
	
	/**
	 * @brief General settors and gettors for float and byte* types.
	 */
	static void SetFloatAtBytePtr(unsigned char* pByte, float val) {
		float* pFloat = reinterpret_cast<float*>( pByte );
		*( pFloat ) = val;
	}

	static float GetFloatFromBytePtr(unsigned char* pByte) {
		float* pFloat = reinterpret_cast<float*>( pByte );
		return *( pFloat );
	}

	static void SetBytePtrAtBytePtr(unsigned char* pByte, unsigned char* ptr) {
		byte** pBytePtr = reinterpret_cast<byte**>( pByte );
		*( pBytePtr ) = ptr;
	}

	static byte* GetBytePtrAtBytePtr(unsigned char* pByte) {
		byte** pBytePtr = reinterpret_cast<byte**>( pByte );
		return *( pBytePtr );
	}

	/**
	 * @return Number of bytes required to store a search tree node.
	 */
	static int PlayerNodeSizeInTree() { 
		return NON_TERMINAL_BASE_SIZE + sizeof(byte*) + sizeof(bool); 
	}

	static int ChanceNodeSizeInTree(int numChildren) {
		return NON_TERMINAL_BASE_SIZE + ( numChildren * sizeof(float) );
	}

	static int TerminalNodeSizeInTree() { return TERMINAL_SIZE; }

	
	/**
	 * @brief Sets each type of node in the search tree with relevant data required.
	 * @return Address of the next node to be set.
	 */
	static byte* SetPlayerNode(byte* treePos, int numChildren, byte* childrenStart, bool isPlayerOne, byte* infoSetPointer);

	static byte* SetChanceNode(byte* treePos, byte* childrenStart, std::vector<float>& childProbs);

	static byte* SetTerminalNode(byte* treePos, float utility);

	/**
	 * @return Number of bytes required to store an info set with N actions.
	 */
	static int InfoSetSize(int numActions);

	/**
	 * @brief Sets an Information Set in the regret table.
	 * @return Address of the next Information Set to be set.
	 */
	static byte* SetInfoSetNode(byte* pos, int numActions);

};


/**
 * @brief Object used to cast bytes in the search tree array for use by CFR algorithm.
 */
class SearchTreeNode {

protected:
	char mIdentifier;

	using byte = unsigned char;

	//Used to traverse to children for player and chance nodes.
	uint8_t mNumChildren = 0;
	byte* mpChildStartOffset = 0;

	//Used to access Information Set in Information Set table for player nodes.
	bool mIsPlayerOne = (bool) true;
	byte* mpInfoSetPtr = (byte*) 0;

	//Used to access probabilities for each child for chance nodes.
	float* mpChildProbs = (float*) 0;

	//Used to get utility for each terminal node.
	float mUtility = 0;

	//Used to get to next sibling nodes.
	byte* mpNextNode = (byte*) 0;

	//Used to store size the node takes in the tree.
	int mSizeInTree = 0;

public:


	SearchTreeNode(byte* pos);

	SearchTreeNode(byte* pTree, long offset) : SearchTreeNode(pTree + offset) {}

	bool IsPlayerNode() const;
	bool IsChanceNode() const;
	bool IsTerminalNode() const;

	uint8_t NumChildren() const;
	byte* ChildrenStartOffset() const;
	bool IsPlayerOne() const;
	byte* InfoSetPosition() const;

	std::vector<float> ChildProbabilities() const;

	std::vector<float> CumulativeChildProbs() const;

	float Utility() const;

	byte* NextNodePos() const;
	SearchTreeNode NextNode() const;

	std::vector<SearchTreeNode> AllChildren() const;

	int SizeInTree() const;
};

std::ostream& operator<<(std::ostream& os, SearchTreeNode& searchNode);
		

/**
 * @brief Object used to cast bytes in Regret Table for use by CFR algorithm
 */
class InfoSetData {

	uint8_t mNumActions;
	float* mpCurrStrategy;
	float* mpCumStrategy;
	float* mpCumRegret;

public:
	InfoSetData(TreeUtils::byte* pos);

	int size();

	int numActions();

	float GetCurrentStrategy(int index);

	float GetCumulativeStrategy(int index);

	float GetCumulativeRegret(int index);

	void SetCurrentStrategy(float prob, int index);

	void AddToCumulativeStrategy(float prob, int index);

	void AddToCumulativeRegret(float prob, int index);

};

std::ostream& operator<<(std::ostream& os, InfoSetData& infoSet);

			

		
	


