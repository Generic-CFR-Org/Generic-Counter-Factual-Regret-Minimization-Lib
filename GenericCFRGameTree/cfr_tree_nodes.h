#pragma once
#include "pch.h"
#include "framework.h"
#include <string>


/*
###########################################
# BYTE ARRAY UTILITY FUNCTION DEFINITIONS #
###########################################
*/

class TreeUtils {
public:
	typedef unsigned char byte;
	static const int NON_TERMINAL_BASE_SIZE = sizeof(char) + sizeof(uint8_t) + sizeof(byte*);
	static const int TERMINAL_SIZE = sizeof(char) + sizeof(float);
	static const int PLAYER_NODE_SIZE = NON_TERMINAL_BASE_SIZE + sizeof(byte*);
	
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

	static int PlayerNodeSizeInTree() { 
		return NON_TERMINAL_BASE_SIZE + sizeof(byte*); 
	}

	static int ChanceNodeSizeInTree(int numChildren) {
		return NON_TERMINAL_BASE_SIZE + ( numChildren * sizeof(float) );
	}

	static int TerminalNodeSizeInTree() { return TERMINAL_SIZE; }

	static byte* SetPlayerNode(byte* treePos, int numChildren, byte* childrenStart, byte* infoSetPointer);

	static byte* SetChanceNode(byte* treePos, byte* childrenStart, std::vector<float>& childProbs);

	static byte* SetTerminalNode(byte* treePos, float utility);

	static int InfoSetSize(int numActions);

	static byte* SetInfoSetNode(byte* pos, int numActions);

};


class SearchTreeNode {

protected:
	char mIdentifier;

	using byte = TreeUtils::byte;

	//Used to traverse to children for player and chance nodes.
	uint8_t mNumChildren = 0;
	byte* mpChildStartOffset = 0;

	//Used to access Information Set in Information Set table for player nodes.
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

	bool IsPlayerNode() { return mIdentifier == 'p'; }
	bool IsChanceNode() { return mIdentifier == 'c'; }
	bool IsTerminalNode() { return mIdentifier == 't'; }

	uint8_t NumChildren() { return mNumChildren; }
	byte* ChildrenStartOffset() { return mpChildStartOffset; }
	byte* InfoSetPosition() { return mpInfoSetPtr; }
	std::vector<float> ChildProbabilities() {
		std::vector<float> probs;
		float* temp = mpChildProbs;
		if (this->IsChanceNode()) {
			for (int iFloat = 0; iFloat < mNumChildren; iFloat++) {
				probs.push_back(*temp);
				temp++;
			}
		}
		return probs;
	}
	float Utility() {
		if (this->IsTerminalNode()) {
			return mUtility;
		}
		return 0.0;
	}

	byte* NextNodePos() { return mpNextNode; }
	SearchTreeNode NextNode() { return SearchTreeNode{ mpNextNode }; }

	std::vector<SearchTreeNode> AllChildren();

	int SizeInTree() { return mSizeInTree; }
};

std::ostream& operator<<(std::ostream& os, SearchTreeNode& searchNode);
		

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

			

		
	


