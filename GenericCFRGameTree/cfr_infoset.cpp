
#include "pch.h"
#include "framework.h"
#include "cfr_tree_nodes.h"
using namespace Cfr::CfrTrees;


int InfoSets::InfoSetSize(int numActions) {
	return sizeof(uint8_t) + ( 3 * numActions * sizeof(float) );
}

InfoSets::InfoSetData::InfoSetData(byte* pos) {
	mNumActions = ( uint8_t ) * ( pos++ );
	int arrSize = mNumActions * sizeof(float);
	mpCurrStrategy = (float*) *( pos += arrSize );
	mpCumStrategy = (float*) *( pos += arrSize );
	mpCumRegret = (float*) *( pos += arrSize );
}

int InfoSets::InfoSetData::size() {
	return sizeof(uint8_t) + ( 3 * mNumActions * sizeof(float) );
}

float InfoSets::InfoSetData::GetCurrentStrategy(int index) {
	byte* iFloat = (byte*) ( mpCurrStrategy + index );
	return GetFloatFromBytePtr(iFloat);
}

float InfoSets::InfoSetData::GetCumulativeStrategy(int index) {
	byte* iFloat = (byte*) ( mpCumStrategy + index );
	return GetFloatFromBytePtr(iFloat);
}

float InfoSets::InfoSetData::GetCumulativeRegret(int index) {
	byte* iFloat = (byte*) ( mpCumRegret + index );
	return GetFloatFromBytePtr(iFloat);
}

void InfoSets::InfoSetData::SetCurrentStrategy(float prob, int index) {
	byte* iFloat = (byte*) ( mpCurrStrategy + index );
	SetFloatAtBytePtr(iFloat, prob);
}

void InfoSets::InfoSetData::AddToCumulativeStrategy(float prob, int index) {
	float newTotal = GetCumulativeStrategy(index) + prob;
	byte* iFloat = (byte*) ( mpCumStrategy + index );
	SetFloatAtBytePtr(iFloat, newTotal);
}

void InfoSets::InfoSetData::AddToCumulativeRegret(float prob, int index) {
	float newTotal = GetCumulativeRegret(index) + prob;
	byte* iFloat = (byte*) ( mpCumRegret + index );
	SetFloatAtBytePtr(iFloat, newTotal);
}