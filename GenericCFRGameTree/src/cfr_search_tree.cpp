#include "pch.h"
#include "framework.h"
#include "cfr_tree_nodes.h"



TreeUtils::Byte* TreeUtils::SetPlayerNode
(
	Byte* tree_pos, const int num_children, Byte* children_start,
	bool is_player_one, Byte* info_set_pointer
) 
{
	Byte* temp = tree_pos;
	*(temp++) = (char) 'p';
	*(temp++) = static_cast<uint8_t>(num_children);
	TreeUtils::SetBytePtrAtBytePtr(temp, children_start);
	temp += sizeof(Byte*);
	*(temp++) = (bool) is_player_one;
	TreeUtils::SetBytePtrAtBytePtr(temp, info_set_pointer);
	temp += sizeof(Byte*);
	return temp;
}

TreeUtils::Byte* TreeUtils::SetChanceNode(
	Byte* tree_pos, Byte* children_start,
	const std::vector<float>& child_probs
) {
	Byte* temp = tree_pos;
	*( temp++ ) = (char) 'c';
	*( temp++ ) = static_cast<uint8_t>(child_probs.size());
	TreeUtils::SetBytePtrAtBytePtr(temp, children_start);
	temp += sizeof(Byte*);
	for (float prob : child_probs) {
		TreeUtils::SetFloatAtBytePtr(temp, prob);
		temp += sizeof(float);
	}
	return temp;
}

TreeUtils::Byte* TreeUtils::SetTerminalNode(Byte* tree_pos, const float utility) {
	Byte* temp = tree_pos;
	*( temp++ ) = (char) 't';
	TreeUtils::SetFloatAtBytePtr(temp, utility);
	temp += sizeof(float);
	return temp;
}

using Byte = unsigned char;

SearchTreeNode::SearchTreeNode(Byte* pos) {
	const TreeUtils::Byte* initialPos = pos;
	this->identifier_ = static_cast<char>(*(pos++));
	if (this->identifier_ == 'p' || this->identifier_ == 'c') {
		this->num_children_ = (uint8_t) *( pos++ );
		this->p_child_start_offset_ = TreeUtils::GetBytePtrAtBytePtr(pos);
		pos += sizeof(Byte*);
	}
	if (this->identifier_ == 'p') {
		this->is_player_one_ = static_cast<bool>(*pos++);
		this->p_info_set_ptr_ = TreeUtils::GetBytePtrAtBytePtr(pos);
		pos += sizeof(Byte*);
	}
	if (this->identifier_ == 'c') {
		this->p_child_probs_ = pos;
		pos += (this->num_children_ * sizeof(float));
	}
	if (this->identifier_ == 't') {
		this->utility_ = TreeUtils::GetFloatFromBytePtr(pos);
		pos += sizeof(float);
	}
	p_next_node_ = pos;
	size_in_tree_ = static_cast<int>(pos - initialPos);

}

bool SearchTreeNode::IsPlayerNode() const
{ return this->identifier_ == 'p'; }

bool SearchTreeNode::IsChanceNode() const
{ return this->identifier_ == 'c'; }

bool SearchTreeNode::IsTerminalNode() const
{ return this->identifier_ == 't'; }

uint8_t SearchTreeNode::NumChildren() const
{ return this->num_children_; }

Byte* SearchTreeNode::ChildrenStartOffset() const
{ return this->p_child_start_offset_; }

bool SearchTreeNode::IsPlayerOne() const
{ return this->is_player_one_; }

Byte* SearchTreeNode::InfoSetPosition() const
{ return this->p_info_set_ptr_; }

std::vector<float> SearchTreeNode::ChildProbabilities() const
{
	std::vector<float> probabilities;
	Byte* temp = this->p_child_probs_;
	if (this->IsChanceNode()) {
		for (int iFloat = 0; iFloat < this->num_children_; iFloat++) {
			probabilities.push_back(TreeUtils::GetFloatFromBytePtr(temp));
			temp += sizeof(float);
		}
	}
	return probabilities;
}

std::vector<float> SearchTreeNode::CumulativeChildProbs() const
{
	std::vector<float> probs;
	Byte* temp = this->p_child_probs_;
	if (this->IsChanceNode()) 
	{
		float running_cnt = 0;
		for (int iFloat = 0; iFloat < this->num_children_; iFloat++) {
			probs.push_back(running_cnt);
			running_cnt += TreeUtils::GetFloatFromBytePtr(temp);
			temp += sizeof(float);
		}
		probs.push_back(running_cnt);
	}
	return probs;
}

SearchTreeNode SearchTreeNode::SampleChild() const
{
	if (this->num_children_ == 1)
	{
		return SearchTreeNode{this->p_child_start_offset_};
	}
	float rand_float = static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX);
	float cumulative_prob = 0;
	int child_index = 0;
	Byte* childProbPos = this->p_child_probs_;
	for (child_index; child_index < this->num_children_; child_index++)
	{
		if (rand_float > cumulative_prob) {break;}
		cumulative_prob += TreeUtils::GetFloatFromBytePtr(childProbPos);
		childProbPos += sizeof(float);

	}
	std::vector<SearchTreeNode> children = this->AllChildren();
	return children.at(child_index);
}

float SearchTreeNode::Utility() const
{
	if (this->IsTerminalNode()) {
		return this->utility_;
	}
	return 0.0;
}

Byte* SearchTreeNode::NextNodePos() const
{ return this->p_next_node_; }

SearchTreeNode SearchTreeNode::NextNode() const
{ return SearchTreeNode{ this->p_next_node_ }; }

std::vector<SearchTreeNode> SearchTreeNode::AllChildren() const
{
	std::vector<SearchTreeNode> children;
	Byte* curr_pos = this->p_child_start_offset_;
	for (int iChild = 0; iChild < this->num_children_; iChild++) {
		SearchTreeNode child = SearchTreeNode(curr_pos);
		curr_pos = child.p_next_node_;
		children.push_back(child);
	}
	return children;
}

int SearchTreeNode::SizeInTree() const
{ return this->size_in_tree_; }

std::ostream& operator<<(std::ostream& os, const SearchTreeNode& search_node) {
	if (search_node.IsPlayerNode()) {
		os << "Tree Player node:\n";
		os << "Num Children: " << static_cast<int>(search_node.NumChildren()) << "\n";
		Byte* info_set = search_node.InfoSetPosition();
		InfoSetData infoSetData = InfoSetData(info_set);
		os << infoSetData << "\n";
	}
	else if (search_node.IsChanceNode()) {
		os << "Tree Chance node:\n";
		os << "Num Children: " << static_cast<int>(search_node.NumChildren()) << "\n";
		os << "Child Probabilities: [";
		std::vector<float> probabilities = search_node.ChildProbabilities();
		for (float prob : probabilities) {
			os << " " << prob << ",";
		}
		os << "]\n\n";
	}
	else {
		os << "Terminal Node:\n";
		os << "Utility: " << search_node.Utility() << "\n\n";
	}
	return os;
}

