
#include "pch.h"
#include "framework.h"
#include "cfr_tree_nodes.h"


int TreeUtils::InfoSetSize(int numActions) {
	return sizeof(uint8_t) + ( 3 * numActions * sizeof(float) );
}

TreeUtils::byte* TreeUtils::SetInfoSetNode(byte* pos, int numActions) {
	*(pos++) = ( uint8_t ) numActions;
	float uniformProb = 1.0 / (float) numActions;
	for (int iUniformStrat = 0; iUniformStrat < numActions; iUniformStrat++) {
		TreeUtils::SetFloatAtBytePtr(pos, uniformProb);
		pos += sizeof(float);
	}
	for (int iCumulative = 0; iCumulative < 2 * numActions; iCumulative++) {
		TreeUtils::SetFloatAtBytePtr(pos, 0.0);
		pos += sizeof(float);
	}
	//Return pointer to next info set.
	return pos;
}

using byte = unsigned char;

InfoSetData::InfoSetData(byte* pos) {
	mNumActions = ( uint8_t ) * ( pos++ );
	int arrSize = mNumActions * sizeof(float);
	mpCurrStrategy = reinterpret_cast<float*>(pos);
	pos += arrSize;
	mpCumStrategy = reinterpret_cast<float*>( pos );
	pos += arrSize;
	mpCumRegret = reinterpret_cast<float*>( pos );
	pos += arrSize;
}

int InfoSetData::size() {
	return sizeof(uint8_t) + ( 3 * mNumActions * sizeof(float) );
}

int InfoSetData::numActions() {
	return mNumActions;
}

float InfoSetData::GetCurrentStrategy(int index) {
	byte* iFloat = (byte*) ( mpCurrStrategy + index );
	return TreeUtils::GetFloatFromBytePtr(iFloat);
}

float InfoSetData::GetCumulativeStrategy(int index) {
	byte* iFloat = (byte*) ( mpCumStrategy + index );
	return TreeUtils::GetFloatFromBytePtr(iFloat);
}

float InfoSetData::GetCumulativeRegret(int index) {
	byte* iFloat = (byte*) ( mpCumRegret + index );
	return TreeUtils::GetFloatFromBytePtr(iFloat);
}

void InfoSetData::SetCurrentStrategy(float prob, int index) {
	byte* iFloat = (byte*) ( mpCurrStrategy + index );
	TreeUtils::SetFloatAtBytePtr(iFloat, prob);
}

void InfoSetData::AddToCumulativeStrategy(float prob, int index) {
	float newTotal = GetCumulativeStrategy(index) + prob;
	byte* iFloat = (byte*) ( mpCumStrategy + index );
	TreeUtils::SetFloatAtBytePtr(iFloat, newTotal);
}

void InfoSetData::AddToCumulativeRegret(float prob, int index) {
	float newTotal = GetCumulativeRegret(index) + prob;
	byte* iFloat = (byte*) ( mpCumRegret + index );
	TreeUtils::SetFloatAtBytePtr(iFloat, newTotal);
}

std::ostream& operator<<(std::ostream& os, InfoSetData& infoSet) {
	os << "Info set:\n";
	int numActions = infoSet.numActions();
	os << "Num actions: " << numActions << "\n";
	os << " - " << "Current Strategy:  [";
	for (int iAction = 0; iAction < numActions - 1; iAction++) {
		os << " " << infoSet.GetCurrentStrategy(iAction) << " ,";
	}
	os << " " << infoSet.GetCurrentStrategy(numActions - 1) << " ]\n";

	os << " - " << "Cumulative Strategy Probabilities:  [";

	for (int iAction = 0; iAction < numActions - 1; iAction++) {
		os << " " << infoSet.GetCumulativeStrategy(iAction) << " ,";
	}
	os << " " << infoSet.GetCumulativeStrategy(numActions - 1) << " ]\n";

	os << " - " << "Cumulative Regrets:  [";

	for (int iAction = 0; iAction < numActions - 1; iAction++) {
		os << " " << infoSet.GetCumulativeRegret(iAction) << " ,";
	}
	os << " " << infoSet.GetCumulativeRegret(numActions - 1) << " ]\n";
	return os;
}