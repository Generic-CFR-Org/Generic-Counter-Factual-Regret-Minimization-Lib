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
	mNumChildren = (uint8_t) pGameTree[pGameNodePos++];
	long arrSize = mNumActions * sizeof(float);
	mpCurrStratArr = (float*) ( pGameTree + pGameNodePos );
	pGameNodePos += arrSize;
	mpCumStratArr = (float*) ( pGameTree + pGameNodePos );
	pGameNodePos += arrSize;
	mpCumRegretArr = (float*) ( pGameTree + pGameNodePos );
	pGameNodePos += arrSize;
	mpChildStartOffset = (long) pGameTree[pGameNodePos];
	pGameNodePos += sizeof(long);
	mpNextNodePos = pGameNodePos;
}

TreeChanceNode::TreeChanceNode(byte* pGameTree, long pChanceNodePos) {
	
	mIdentifier = (unsigned char) pGameTree[pChanceNodePos++];
	mNumChildren = (uint8_t) pGameTree[pChanceNodePos++];
	mpChildStartOffset = (long) pGameTree[pChanceNodePos];
	pChanceNodePos += sizeof(long);
	long arrSize = mNumChildren * sizeof(float);
	mpProbToChildArr = (float*) (pGameTree + pChanceNodePos);
	pChanceNodePos += arrSize;
	mpNextNodePos = pChanceNodePos;
	
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

void TreeGameNode::SetCurrStratProb(float prob, int index) {
	*( (this->mpCurrStratArr) + index ) = prob;
}

void TreeGameNode::AddCumStratProb(float prob, int index) {
	*( ( this->mpCumStratArr ) + index ) += prob;
}

void TreeGameNode::AddCumRegret(float regret, int index) {
	*( ( this->mpCumRegretArr ) + index ) += regret;
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
	os << " - " << "Number of Actions: " << (int) treeGameNode.mNumActions << "\n";
	os << " - " << "Number of Children: " << (int) treeGameNode.mNumChildren << "\n";
	os << " - " << "Offset for start of Children Nodes: " << (long) treeGameNode.mpChildStartOffset << "\n";
	os << " - " << "Current Strategy:  [";
	int numActions = treeGameNode.mNumActions;
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
	os << " " << treeGameNode.GetCumRegret(numActions - 1) << " ]\n\n";
	return os;
}


float TreeChanceNode::GetChildReachProb(int index) {
	return this->mpProbToChildArr[index];
}



