#include "pch.h"
#include "framework.h"
#include "cfr_tree_nodes.h"

using namespace Cfr::CfrTrees;

Search::SearchTreeNode::SearchTreeNode(byte* pos) {

	byte* initialPos = pos;
	mIdentifier = (char) *( pos++ );
	if (mIdentifier == 'p' || mIdentifier == 'c') {
		mNumChildren = ( uint8_t ) * ( pos++ );
		mpChildStartOffset = (byte*) *( pos += sizeof(byte*) );
	}
	if (mIdentifier == 'p') {
		mpInfoSetPtr = (byte*) *( pos += sizeof(byte*) );
	}
	if (mIdentifier == 'c') {
		mpChildProbs = (float*) *( pos += ( mNumChildren * sizeof(float) ) );
	}
	if (mIdentifier == 't') {
		mUtility = GetFloatFromBytePtr(pos);
		pos += sizeof(float);
	}
	mpNextNode = pos;
	mSizeInTree = (int) ( pos - initialPos );

}

std::vector<Search::SearchTreeNode> Search::SearchTreeNode::AllChildren() {
	std::vector<SearchTreeNode> children;
	byte* currPos = mpChildStartOffset;
	for (int iChild = 0; iChild < mNumChildren; iChild++) {
		SearchTreeNode child = SearchTreeNode(currPos);
		currPos = child.mpNextNode;
		children.push_back(child);
	}
	return children;
}