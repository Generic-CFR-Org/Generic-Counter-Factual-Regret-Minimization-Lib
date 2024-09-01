#pragma once
#include "pch.h"
#include "framework.h"
#include <string>
#include "cfr.h"

namespace Cfr {

	namespace CfrTrees {

		typedef unsigned char byte;
		const int NON_TERMINAL_BASE_SIZE = sizeof(char) + sizeof(uint8_t) + sizeof(byte*);
		const int TERMINAL_SIZE = sizeof(char) + sizeof(float);


		/*
		###########################################
		# BYTE ARRAY UTILITY FUNCTION DEFINITIONS #
		###########################################
		*/
		static void SetFloatAtBytePtr(unsigned char* pByte, float val) {
			float* pFloat = reinterpret_cast<float*>( pByte );
			*( pFloat ) = val;
		}

		static float GetFloatFromBytePtr(unsigned char* pByte) {
			float* pFloat = reinterpret_cast<float*>( pByte );
			return *( pFloat );
		}

		namespace Search {

			int PlayerNodeSizeInTree() { return NON_TERMINAL_BASE_SIZE + sizeof(byte*); }

			int ChanceNodeSizeInTree(int numChildren) {
				return NON_TERMINAL_BASE_SIZE + ( numChildren * sizeof(float) );
			}

			int TerminalNodeSizeInTree() { return TERMINAL_SIZE; }

			class SearchTreeNode {

			protected:
				char mIdentifier;

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

				byte* NextNodePos() { return mpNextNode; }
				SearchTreeNode NextNode() { return SearchTreeNode{ mpNextNode }; }

				std::vector<SearchTreeNode> AllChildren();

				int SizeInTree() { return mSizeInTree; }
			};
		}

		namespace InfoSets {

			int InfoSetSize(int numActions);

			class InfoSetData {

				uint8_t mNumActions;
				float* mpCurrStrategy;
				float* mpCumStrategy;
				float* mpCumRegret;


				InfoSetData(byte* pos);

				int size();

				float GetCurrentStrategy(int index);

				float GetCumulativeStrategy(int index);

				float GetCumulativeRegret(int index);

				void SetCurrentStrategy(float prob, int index);

				void AddToCumulativeStrategy(float prob, int index);

				void AddToCumulativeRegret(float prob, int index);

			};

		}
	}
}

