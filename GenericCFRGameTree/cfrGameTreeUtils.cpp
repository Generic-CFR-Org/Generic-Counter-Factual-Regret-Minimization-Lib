#pragma once
#include "pch.h"
#include "framework.h"
#include "cfrGameTreeUtils.h"

/*
###########################################
# BYTE ARRAY UTILITY FUNCTION DEFINITIONS #
###########################################
*/

void SetFloatAtBytePtr(unsigned char* pByte, float val) {
	float* pFloat = reinterpret_cast<float*>( pByte );
	*( pFloat ) = val;
}


float GetFloatFromBytePtr(unsigned char* pByte) {
	float* pFloat = reinterpret_cast<float*>( pByte );
	return *( pFloat );
}


/*
###########################################
# GAME TREE UTILITY FUNCTION DEFINITIONS  #
###########################################
*/
TreeGameNode::TreeGameNode(byte* pGameTree, long pGameNodePos) {

	mIdentifier = (unsigned char) pGameTree[pGameNodePos++];
	mPlayerToAct = (int8_t) pGameTree[pGameNodePos++];
	mNumActions = (uint8_t) pGameTree[pGameNodePos++];
	mNumNonTerminalChildren = (uint8_t) pGameTree[pGameNodePos++];
	mHasTerminalChildren = (bool) pGameTree[pGameNodePos++];
	mpChildStartOffset = (long) pGameTree[pGameNodePos];
	pGameNodePos += sizeof(long);

	long arrSize = mNumActions * sizeof(float);
	mpCurrStratArr = (float*) ( pGameTree + pGameNodePos );
	pGameNodePos += arrSize;

	mpCumStratArr = (float*) ( pGameTree + pGameNodePos );
	pGameNodePos += arrSize;

	mpCumRegretArr = (float*) ( pGameTree + pGameNodePos );
	pGameNodePos += arrSize;

	long tempPos = pGameNodePos;
	if (mNumNonTerminalChildren > 0) {
		long numChildArrSize = mNumNonTerminalChildren * sizeof(uint8_t);
		mpNumActionPerChild = (uint8_t*) ( pGameTree + pGameNodePos );
		

		//Calculate number of actionPerChild uint8_t's stores.
		int numActionElems = 0;
		for (tempPos; tempPos < pGameNodePos + numChildArrSize; tempPos += sizeof(uint8_t)) {
			uint8_t numActions = (uint8_t) pGameTree[tempPos];
			numActionElems += numActions;
		}
		pGameNodePos += numChildArrSize;
		mpActionIndexPerChild = (uint8_t*) ( pGameTree + pGameNodePos );
		pGameNodePos += numActionElems * sizeof(uint8_t);
	}
	
	long numActionArrSize = mNumActions * sizeof(uint8_t);
	mpNumChildPerAction = (uint8_t*) ( pGameTree + pGameNodePos );
	tempPos = pGameNodePos;

	//Calculate number of childPerAction uint8_t's stores.
	int numChildElems = 0;
	for (tempPos; tempPos < pGameNodePos + numActionArrSize; tempPos += sizeof(uint8_t)) {
		uint8_t numChildren = (uint8_t) pGameTree[tempPos];
		numChildElems += numChildren;
	}
	pGameNodePos += numActionArrSize;
	mpChildPosPerAction = (uint8_t*) ( pGameTree + pGameNodePos );
	pGameNodePos += numChildElems * sizeof(uint8_t);

	mpNextNodePos = pGameNodePos;
	mGameTreePtr = pGameTree;
}

TreeChanceNode::TreeChanceNode(byte* pGameTree, long pChanceNodePos) {

	mIdentifier = (unsigned char) pGameTree[pChanceNodePos++];
	mNumNonTerminalChildren = (uint8_t) pGameTree[pChanceNodePos++];
	mHasTerminalChildren = (bool) pGameTree[pChanceNodePos++];
	mpChildStartOffset = (long) pGameTree[pChanceNodePos];
	pChanceNodePos += sizeof(long);
	long arrSize = mNumNonTerminalChildren * sizeof(float);
	mpProbToChildArr = (float*) (pGameTree + pChanceNodePos);
	pChanceNodePos += arrSize;

	mpNextNodePos = pChanceNodePos;
	mGameTreePtr = pGameTree;
	
}

float TreeGameNode::GetCurrStratProb(int index) {
	return this->mpCurrStratArr[index];
}

float TreeGameNode::GetCumStratProb(int index) {
	return this->mpCumStratArr[index];
}

float TreeGameNode::GetCumRegret(int index) {
	return this->mpCumRegretArr[index];
}

std::vector<uint8_t> TreeGameNode::GetChildIndicesForAction(int actionIndex) {
	std::vector<uint8_t> childIndicesArr;
	int cumChildCount = 0;
	for (int iAction = 0; iAction < actionIndex; iAction+= sizeof(uint8_t)) {
		cumChildCount += mpNumChildPerAction[iAction];
	}
	int numChildren = mpNumChildPerAction[actionIndex];
	uint8_t* arrOffset = mpChildPosPerAction + cumChildCount;
	for (int iChild = 0; iChild < numChildren; iChild++) {
		uint8_t childIndex = arrOffset[iChild];
		childIndicesArr.push_back(childIndex);
	}
	return childIndicesArr;
}

std::vector<uint8_t> TreeGameNode::GetActionIndicesForChild(int childIndex) {
	std::vector<uint8_t> actionIndicesArr;
	if (mNumNonTerminalChildren == 0) {
		return actionIndicesArr;
	}
	int cumActionCount = 0;
	for (int iChild = 0; iChild < childIndex; iChild += sizeof(uint8_t)) {
		cumActionCount += mpNumActionPerChild[iChild];
	}
	int numActions = mpNumActionPerChild[childIndex];
	uint8_t* arrOffset = mpActionIndexPerChild + cumActionCount;
	for (int iAction = 0; iAction < numActions; iAction++) {
		uint8_t actionIndex = arrOffset[iAction];
		actionIndicesArr.push_back(actionIndex);
	}
	return actionIndicesArr;
}

void TreeGameNode::SetCurrStratProb(float prob, int index) {
	*( (this->mpCurrStratArr) + index ) = prob;
}

void TreeGameNode::AddCumStratProb(float prob, int index) {
	*( ( this->mpCumStratArr ) + index ) += prob;
}

void TreeGameNode::AddCumRegret(float regret, int index) {
	*( ( this->mpCumRegretArr ) + index ) += regret;
}

TreeNodeChildren TreeGameNode::GetChildren() {
	byte* pGameTree = mGameTreePtr;
	long childStartOffset = mpChildStartOffset;
	int numChildren = mNumNonTerminalChildren;
	return GetAllChildren(mGameTreePtr, numChildren, childStartOffset);
}


float TreeChanceNode::GetChildReachProb(int index) {
	return this->mpProbToChildArr[index];
}

TreeNodeChildren TreeChanceNode::GetChildren() {
	byte* pGameTree = mGameTreePtr;
	long childStartOffset = mpChildStartOffset;
	int numChildren = mNumNonTerminalChildren;
	return GetAllChildren(mGameTreePtr, numChildren, childStartOffset);
}



std::vector<TreeGameNode> TreeNodeChildren::GetChildrenGameNodes() {
	return treeGameNodes;
}

std::vector<TreeChanceNode> TreeNodeChildren::GetChildrenChanceNodes() {
	return treeChanceNodes;
}

void TreeNodeChildren::AddChildNode(TreeGameNode node) {
	this->treeGameNodes.push_back(node);
}

void TreeNodeChildren::AddChildNode(TreeChanceNode node) {
	this->treeChanceNodes.push_back(node);
}

static TreeNodeChildren GetAllChildren(byte* pGameTree, int numChildren, long childStartOffset) {

	TreeNodeChildren allChildren = TreeNodeChildren();
	char identifier;
	long childOffset = childStartOffset;
	for (int iChild = 0; iChild < numChildren; iChild++) {
		identifier = pGameTree[childOffset];
		if (identifier == 'g') {
			TreeGameNode childTreeNode = TreeGameNode(pGameTree, childOffset);
			childOffset = childTreeNode.mpNextNodePos;
			allChildren.AddChildNode(childTreeNode);
		}
		else if (identifier == 'c') {
			TreeChanceNode childTreeNode = TreeChanceNode(pGameTree, childOffset);
			childOffset = childTreeNode.mpNextNodePos;
			allChildren.AddChildNode(childTreeNode);
		}
		else {
			//TODO: Create error to throw.
		}
	}
	return allChildren;
}


std::ostream& operator<<(std::ostream& os, TreeGameNode& treeGameNode) {
	os << "Tree Game Node:\n";
	const char* playerOneToAct = "Player One";
	const char* playerTwoToAct = "Player Two";
	if (treeGameNode.mPlayerToAct == 1) {
		os << " - " << playerOneToAct << "'s Turn\n";
	} 
	else {
		os << " - " << playerTwoToAct << "'s Turn\n";
	}
	int numActions = (int) treeGameNode.mNumActions;
	int numNonTerminalChildren = (int) treeGameNode.mNumNonTerminalChildren;
	bool hasTerminalChildren = treeGameNode.mHasTerminalChildren;
	os << " - " << "Number of Actions: " << numActions << "\n";
	os << " - " << "Number of Non Terminal Children: " << numNonTerminalChildren << "\n";
	os << " - " << "Node has terminal children: " << hasTerminalChildren << "\n";
	os << " - " << "Offset for start of Children Nodes: " << (long) treeGameNode.mpChildStartOffset << "\n";
	os << " - " << "Current Strategy:  [";
	for (int iAction = 0; iAction < numActions - 1; iAction++) {
		os << " " << treeGameNode.GetCurrStratProb(iAction) << " ,";
	}
	os << " " << treeGameNode.GetCurrStratProb(numActions - 1) << " ]\n";

	os << " - " << "Cumulative Strategy Probabilities:  [";
	
	for (int iAction = 0; iAction < numActions - 1; iAction++) {
		os << " " << treeGameNode.GetCumStratProb(iAction) << " ,";
	}
	os << " " << treeGameNode.GetCumStratProb(numActions - 1) << " ]\n";

	os << " - " << "Cumulative Regrets:  [";
	
	for (int iAction = 0; iAction < numActions - 1; iAction++) {
		os << " " << treeGameNode.GetCumRegret(iAction) << " ,";
	}
	os << " " << treeGameNode.GetCumRegret(numActions - 1) << " ]\n";

	os << " - Children Indices for each Action:\n";
	for (int iAction = 0; iAction < numActions; iAction++) {

		os << "     * " << "Action Index " << iAction << ": [";
		for (int childIndex : treeGameNode.GetChildIndicesForAction(iAction)) {
			os << " " << childIndex << " ,";
		}
		os << "]\n";

	}
	os << "\n - Action Indices for each Child:\n";
	for (int iChild = 0; iChild < numNonTerminalChildren; iChild++) {

		os << "     * " << "Child Index " << iChild << ": [";
		for (int actionIndex : treeGameNode.GetActionIndicesForChild(iChild)) {
			os << " " << actionIndex << " ,";
		}
		os << "]\n";
	}
	os << "\n";
	
	return os;
}






