#pragma once

template
<typename CommonState,
typename IsTerminal,
typename NextStateFunc,
typename NextTerminalStateFunc>
class NextState {

	CommonState commonState;
	IsTerminal *isTerminalBoolFunc;
	NextStateFunc *nextStateFunc;
	NextTerminalStateFunc *nextTerminalStateFunc;

public:

	NextState(
		CommonState common_state,
		IsTerminal *is_terminal_f,
		NextStateFunc *next_state_f,
		NextTerminalStateFunc *next_terminal_state_f
	);
	
	template<typename Action, typename GenericState, typename TerminalState>
	static std::pair<std::vector<GenericState>, std::vector<TerminalState>>
	GetNextState(Action action, GenericState current_state);
};


