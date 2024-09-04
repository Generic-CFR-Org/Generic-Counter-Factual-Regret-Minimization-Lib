#include "pch.h"
#include "framework.h"
#include "cfr_tree_nodes.h"


TreeUtils::byte* TreeUtils::SetPlayerNode
(
	byte* treePos, int numChildren, byte* childrenStart,
	bool isPlayerOne, byte* infoSetPointer
) 
{
	byte* temp = treePos;
	*(temp++) = (char) 'p';
	*(temp++) = (uint8_t) numChildren;
	TreeUtils::SetBytePtrAtBytePtr(temp, childrenStart);
	temp += sizeof(byte*);
	*(temp++) = (bool) isPlayerOne;
	TreeUtils::SetBytePtrAtBytePtr(temp, infoSetPointer);
	temp += sizeof(byte*);
	return temp;
}

TreeUtils::byte* TreeUtils::SetChanceNode(
	byte* treePos, byte* childrenStart,
	std::vector<float>& childProbs
) {
	byte* temp = treePos;
	*( temp++ ) = (char) 'c';
	*( temp++ ) = (uint8_t) childProbs.size();
	TreeUtils::SetBytePtrAtBytePtr(temp, childrenStart);
	temp += sizeof(byte*);
	for (float prob : childProbs) {
		TreeUtils::SetFloatAtBytePtr(temp, prob);
		temp += sizeof(float);
	}
	return temp;
}

TreeUtils::byte* TreeUtils::SetTerminalNode(byte* treePos, float utility) {
	byte* temp = treePos;
	*( temp++ ) = (char) 't';
	TreeUtils::SetFloatAtBytePtr(temp, utility);
	temp += sizeof(float);
	return temp;
}

using byte = unsigned char;

SearchTreeNode::SearchTreeNode(byte* pos) {

	TreeUtils::byte* initialPos = pos;
	mIdentifier = (char) *( pos++ );
	if (mIdentifier == 'p' || mIdentifier == 'c') {
		mNumChildren = (uint8_t) *( pos++ );
		mpChildStartOffset = TreeUtils::GetBytePtrAtBytePtr(pos);
		pos += sizeof(byte*);
	}
	if (mIdentifier == 'p') {
		mIsPlayerOne = (bool) *(pos++);
		mpInfoSetPtr = TreeUtils::GetBytePtrAtBytePtr(pos);
		pos += sizeof(byte*);
	}
	if (mIdentifier == 'c') {
		mpChildProbs = reinterpret_cast<float*>(pos);
		pos += (mNumChildren * sizeof(float));
	}
	if (mIdentifier == 't') {
		mUtility = TreeUtils::GetFloatFromBytePtr(pos);
		pos += sizeof(float);
	}
	mpNextNode = pos;
	mSizeInTree = (int) ( pos - initialPos );

}

bool SearchTreeNode::IsPlayerNode() const
{ return mIdentifier == 'p'; }

bool SearchTreeNode::IsChanceNode() const
{ return mIdentifier == 'c'; }

bool SearchTreeNode::IsTerminalNode() const
{ return mIdentifier == 't'; }

uint8_t SearchTreeNode::NumChildren() const
{ return mNumChildren; }

byte* SearchTreeNode::ChildrenStartOffset() const
{ return mpChildStartOffset; }

bool SearchTreeNode::IsPlayerOne() const
{ return mIsPlayerOne; }

byte* SearchTreeNode::InfoSetPosition() const
{ return mpInfoSetPtr; }

std::vector<float> SearchTreeNode::ChildProbabilities() const
{
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

std::vector<float> SearchTreeNode::CumulativeChildProbs() const
{
	std::vector<float> probs;
	float runningCnt = 0;
	float* temp = mpChildProbs;
	if (this->IsChanceNode()) {
		for (int iFloat = 0; iFloat < mNumChildren; iFloat++) {
			probs.push_back(runningCnt);
			runningCnt += *temp;
			temp++;
		}
		probs.push_back(runningCnt);
	}
	return probs;
}

float SearchTreeNode::Utility() const
{
	if (this->IsTerminalNode()) {
		return mUtility;
	}
	return 0.0;
}

byte* SearchTreeNode::NextNodePos() const
{ return mpNextNode; }

SearchTreeNode SearchTreeNode::NextNode() const
{ return SearchTreeNode{ mpNextNode }; }

std::vector<SearchTreeNode> SearchTreeNode::AllChildren() const
{
	std::vector<SearchTreeNode> children;
	byte* currPos = mpChildStartOffset;
	for (int iChild = 0; iChild < mNumChildren; iChild++) {
		SearchTreeNode child = SearchTreeNode(currPos);
		currPos = child.mpNextNode;
		children.push_back(child);
	}
	return children;
}

int SearchTreeNode::SizeInTree() const
{ return mSizeInTree; }

std::ostream& operator<<(std::ostream& os, SearchTreeNode& searchNode) {
	if (searchNode.IsPlayerNode()) {
		os << "Tree Player node:\n";
		os << "Num Children: " << (int) searchNode.NumChildren() << "\n";
		byte* pInfoSet = searchNode.InfoSetPosition();
		InfoSetData infoSetData = InfoSetData(pInfoSet);
		os << infoSetData << "\n";
	}
	else if (searchNode.IsChanceNode()) {
		os << "Tree Chance node:\n";
		os << "Num Children: " << (int) searchNode.NumChildren() << "\n";
		os << "Child Probabilities: [";
		std::vector<float> probs = searchNode.ChildProbabilities();
		for (float prob : probs) {
			os << " " << prob << ",";
		}
		os << "]\n\n";
	}
	else {
		os << "Terminal Node:\n";
		os << "Utility: " << searchNode.Utility() << "\n\n";
	}
	return os;
}

