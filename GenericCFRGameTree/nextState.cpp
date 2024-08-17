
#include "pch.h"
#include "framework.h"
#include "nextState.h"

template<
typename CommonState,
typename IsTerminal,
typename NextStateFunc,
typename NextTerminalStateFunc>
inline NextState<CommonState, IsTerminal, NextStateFunc, NextTerminalStateFunc>
::NextState(
	CommonState common_state,
	IsTerminal* is_terminal_f,
	NextStateFunc* next_state_f,
	NextTerminalStateFunc* next_terminal_state_f) {

	commonState = common_state;
	isTerminalBoolFunc = is_terminal_f;
	nextStateFunc = next_state_f;
	nextTerminalStateFunc = next_terminal_state_f;
}

template<typename CommonState, typename IsTerminal, typename NextStateFunc, typename NextTerminalStateFunc>
template<typename Action, typename GenericState, typename TerminalState>
inline std::pair<std::vector<GenericState>, std::vector<TerminalState>>
NextState<CommonState, IsTerminal, NextStateFunc, NextTerminalStateFunc>
::GetNextState(Action action, GenericState current_state) {
	
	if (isTerminalBoolFunc(action)) {
		std::pair<std::vector<GenericState>, std::vector<TerminalState>> terminal_state;
		terminal_state.first = std::vector<GenericState>();

		TerminalState t_state = nextTerminalStateFunc(action, current_state, commonState);
		std::vector<TerminalState> t_state_v{t_state};
		terminal_state.second = t_state_v;

		return terminal_state;
	}
	std::vector<GenericState> generic_next_states = nextStateFunc(action, current_state, commonState);
	std::vector<GenericStates> non_terminal_next_states;
	std::vector<TerminalState> terminal_next_states;

	for (GenericState new_state : generic_next_states) {
		if (isTerminalBoolFunc(new_state)) {
			 terminal_next_states.push_back(nextTerminalStateFunc(action, current_state, commonState))
		}
		else {
			non_terminal_next_states.push_back(new_state);
		}
	}
	return std::pair<std::vector<GenericState>, std::vector<TerminalState>>(non_terminal_next_states, terminal_next_states);
}