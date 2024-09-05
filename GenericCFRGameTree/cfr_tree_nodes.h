#pragma once
#include "pch.h"
#include "framework.h"
#include <string>
#include <cstdint>


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
	typedef unsigned char Byte;
	static const int kNonTerminalBaseSize = sizeof(char) + sizeof(uint8_t) + sizeof(Byte*);
	static const int kTerminalSize = sizeof(char) + sizeof(float);
	static const int kPlayerNodeSize = kNonTerminalBaseSize + sizeof(Byte*) + sizeof(char);
	
	/**
	 * @brief General setters and getters for float and byte* types.
	 */
	static void SetFloatAtBytePtr(unsigned char* p_byte, float val) {
		float* p_float = reinterpret_cast<float*>( p_byte );
		*( p_float ) = val;
	}

	static float GetFloatFromBytePtr(unsigned char* p_byte) {
		float* p_float = reinterpret_cast<float*>( p_byte );
		return *( p_float );
	}

	static void SetBytePtrAtBytePtr(unsigned char* p_byte, unsigned char* ptr) {
		Byte** p_byte_ptr = reinterpret_cast<Byte**>( p_byte );
		*( p_byte_ptr ) = ptr;
	}

	static Byte* GetBytePtrAtBytePtr(unsigned char* p_byte) {
		Byte** p_byte_ptr = reinterpret_cast<Byte**>( p_byte );
		return *( p_byte_ptr );
	}

	/**
	 * @return Number of bytes required to store a search tree node.
	 */
	static int PlayerNodeSizeInTree() { 
		return kNonTerminalBaseSize + sizeof(Byte*) + sizeof(bool);
	}

	static int ChanceNodeSizeInTree(int numChildren) {
		return kNonTerminalBaseSize + ( numChildren * sizeof(float) );
	}

	static int TerminalNodeSizeInTree() { return kTerminalSize; }

	
	/**
	 * @brief Sets each type of node in the search tree with relevant data required.
	 * @return Address of the next node to be set.
	 */
	static Byte* SetPlayerNode(Byte* tree_pos, int num_children, Byte* children_start, bool is_player_one, Byte* info_set_pointer);

	static Byte* SetChanceNode(Byte* tree_pos, Byte* children_start, const std::vector<float>& child_probs);

	static Byte* SetTerminalNode(Byte* tree_pos, float utility);

	/**
	 * @return Number of bytes required to store an info set with N actions.
	 */
	static int InfoSetSize(int num_actions);

	/**
	 * @brief Sets an Information Set in the regret table.
	 * @return Address of the next Information Set to be set.
	 */
	static Byte* SetInfoSetNode(Byte* pos, int num_actions);

};


/**
 * @brief Object used to cast bytes in the search tree array for use by CFR algorithm.
 */
class SearchTreeNode {

protected:
	char identifier_;

	using Byte = unsigned char;

	//Used to traverse to children for player and chance nodes.
	uint8_t num_children_ = 0;
	Byte* p_child_start_offset_ = nullptr;

	//Used to access Information Set in Information Set table for player nodes.
	bool is_player_one_ = (bool) true;
	Byte* p_info_set_ptr_ = (Byte*) nullptr;

	//Used to access probabilities for each child for chance nodes.
	Byte* p_child_probs_ = nullptr;

	//Used to get utility for each terminal node.
	float utility_ = 0;

	//Used to get to next sibling nodes.
	Byte* p_next_node_ = (Byte*) nullptr;

	//Used to store size the node takes in the tree.
	int size_in_tree_ = 0;

public:


	SearchTreeNode(Byte* pos);

	SearchTreeNode(Byte* p_tree, long offset) : SearchTreeNode(p_tree + offset) {}

	/**
	 * @brief Evaluates the type of search Tree node.
	 */
	bool IsPlayerNode() const;

	bool IsChanceNode() const;

	bool IsTerminalNode() const;


	/**
	 * @brief Functions used by both Player and Chance Search Tree Nodes. 
	 */
	uint8_t NumChildren() const;

	Byte* ChildrenStartOffset() const;

	std::vector<SearchTreeNode> AllChildren() const;


	/**
	 * @brief Functions used only by the Player Search Tree Node.
	 */
	bool IsPlayerOne() const;

	Byte* InfoSetPosition() const;


	/**
	 * @brief Functions used only by the Chance Search Tree Node.
	 */
	std::vector<float> ChildProbabilities() const;

	std::vector<float> CumulativeChildProbs() const;

	SearchTreeNode SampleChild() const;


	/**
	 * @brief Functions used only by the Terminal Search Tree Node.
	 */
	float Utility() const;


	/**
	 * Functions used by all search tree nodes.
	 */
	Byte* NextNodePos() const;

	SearchTreeNode NextNode() const;

	int SizeInTree() const;
};

std::ostream& operator<<(std::ostream& os, const SearchTreeNode& search_node);
		

/**
 * @brief Object used to cast bytes in Regret Table for use by CFR algorithm
 */
class InfoSetData {

	using byte = unsigned char;

	uint8_t num_actions_;
	byte* p_curr_strategy_;
	byte* p_cum_strategy_;
	byte* p_cum_regret_;

public:
	InfoSetData(TreeUtils::Byte* pos);

	int size();

	int NumActions();

	float GetCurrentStrategy(int index);

	float GetCumulativeStrategy(int index);

	float GetCumulativeRegret(int index);

	void SetCurrentStrategy(float prob, int index);

	void AddToCumulativeStrategy(float prob, int index);

	void AddToCumulativeRegret(float prob, int index);

};

std::ostream& operator<<(std::ostream& os, InfoSetData& info_set);

			

		
	


