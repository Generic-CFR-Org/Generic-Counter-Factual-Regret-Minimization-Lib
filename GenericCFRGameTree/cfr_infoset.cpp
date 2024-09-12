
#include "pch.h"
#include "framework.h"
#include "cfr_tree_nodes.h"
std::cout << "hello world!";


int TreeUtils::InfoSetSize(int num_actions) {
	return sizeof(uint8_t) + ( 3 * num_actions * sizeof(float) );
}

TreeUtils::Byte* TreeUtils::SetInfoSetNode(Byte* pos, int num_actions)
{
	*(pos++) = static_cast<uint8_t>(num_actions);
	const float uniform_prob = 1.0f / static_cast<float>(num_actions);
	for (int i_uniform_strat = 0; i_uniform_strat < num_actions; i_uniform_strat++) {
		TreeUtils::SetFloatAtBytePtr(pos, uniform_prob);
		pos += sizeof(float);
	}
	for (int i_cumulative = 0; i_cumulative < 2 * num_actions; i_cumulative++) {
		TreeUtils::SetFloatAtBytePtr(pos, 0.0);
		pos += sizeof(float);
	}
	//Return pointer to next info set.
	return pos;
}

using Byte = unsigned char;

InfoSetData::InfoSetData(byte* pos)
{
	this->num_actions_ = ( uint8_t ) *(pos++);
	int arr_size = static_cast<int>(num_actions_) * sizeof(float);
	this->p_curr_strategy_ = pos;
	pos += arr_size;
	this->p_cum_strategy_ = pos;
	pos += arr_size;
	this->p_cum_regret_ = pos;
	pos += arr_size;
}

int InfoSetData::size()
{
	return (3 * num_actions_ * sizeof(float)) + sizeof(uint8_t);
}

int InfoSetData::NumActions()
{
	return this->num_actions_;
}

float InfoSetData::GetCurrentStrategy(int index)
{
	byte* iFloat = this->p_curr_strategy_ + ( sizeof(float) * index );
	return TreeUtils::GetFloatFromBytePtr(iFloat);
}

float InfoSetData::GetCumulativeStrategy(int index)
{
	byte* iFloat = this->p_cum_strategy_ + ( sizeof(float) * index );
	return TreeUtils::GetFloatFromBytePtr(iFloat);
}

float InfoSetData::GetCumulativeRegret(int index)
{
	byte* iFloat = this->p_cum_regret_ + ( sizeof(float) * index );
	return TreeUtils::GetFloatFromBytePtr(iFloat);
}

void InfoSetData::SetCurrentStrategy(float prob, int index)
{
	byte* iFloat = this->p_curr_strategy_ + (sizeof(float) * index);
	TreeUtils::SetFloatAtBytePtr(iFloat, prob);
}

void InfoSetData::AddToCumulativeStrategy(float prob, int index)
{
	float newTotal = GetCumulativeStrategy(index) + prob;
	byte* iFloat =  this->p_cum_strategy_ + ( sizeof(float) * index );
	TreeUtils::SetFloatAtBytePtr(iFloat, newTotal);
}

void InfoSetData::AddToCumulativeRegret(float prob, int index)
{
	float newTotal = GetCumulativeRegret(index) + prob;
	byte* iFloat = this->p_cum_regret_ + ( sizeof(float) * index );
	TreeUtils::SetFloatAtBytePtr(iFloat, newTotal);
}

std::ostream& operator<<(std::ostream& os, InfoSetData& info_set)
{
	os << "Info set:\n";
	int num_actions = info_set.NumActions();
	os << "Num actions: " << num_actions << "\n";
	os << " - " << "Current Strategy:  [";
	for (int i_action = 0; i_action < num_actions - 1; i_action++)
	{
		os << " " << info_set.GetCurrentStrategy(i_action) << " ,";
	}
	os << " " << info_set.GetCurrentStrategy(num_actions - 1) << " ]\n";

	os << " - " << "Cumulative Strategy Probabilities:  [";

	for (int i_action = 0; i_action < num_actions - 1; i_action++)
	{
		os << " " << info_set.GetCumulativeStrategy(i_action) << " ,";
	}
	os << " " << info_set.GetCumulativeStrategy(num_actions - 1) << " ]\n";

	os << " - " << "Cumulative Regrets:  [";

	for (int i_action = 0; i_action < num_actions - 1; i_action++)
	{
		os << " " << info_set.GetCumulativeRegret(i_action) << " ,";
	}
	os << " " << info_set.GetCumulativeRegret(num_actions - 1) << " ]\n";
	return os;
}