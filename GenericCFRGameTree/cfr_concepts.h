#pragma once
#include "pch.h"
#include "framework.h"
#include <cstddef>
#include <concepts>
#include <functional>
#include <string>
#include <vector>
#include "nodes.h"

namespace CfrConcepts {

	
	//template<typename Action, typename PlayerNode, typename ChanceNode, typename GameClass>
	//concept StaticGameClassFuncs = requires( Action a, PlayerNode p, ChanceNode c, GameClass g ) {

	//	/*Game class must have terminal utility function that takens in
	//		a vector of history node pointers
	//	*/
	//	
	//	{ g.UtilityFunc() } -> std::convertible_to<std::vector< HistoryNode<Action, PlayerNode, ChanceNode>* >>;
	//};


}


