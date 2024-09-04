Generic CFR Library

The Generic CFR Library allows you to find the game theory optimal strategies for any 2-player, Zero-sum, Imperfect information game.

Your game must be built on 4 separate user-defined classes:
- The Player Node: Represents a node in the game tree that records information that is only known to the player, and states which player gets to act.
- The Chance Node: A node representing an event in the game out of either player's control, that leads to other nodes in the tree.
- The Action Class: A class used by the PlayerNode to advance to the next node in the game tree.
- The Game Class: A class that stores information that is static throughout the game tree, and employs the game's rules to advance player nodes and Chance nodes, and to evaluate the utility of a terminal node.
  
Your game must implement a few functions for the library to use, to traverse nodes for tree construction and navigation.
- The Player Node, The Chance Node, and the Action node must implement a ToHash() function that returns a string representation of the object. Hashes of 2 objects with the same information must be equal.
- The Player Node must implement a ToInfoSetHash() function, that returns a string representation of the object in the current player's view. (Ex: In poker, a player does not know their opponent's cards, so can only include his cards in the InfoSet Hash)
- The Player Node must implement a Child(Action) function, that takes an Action, and returns a Child Node of any of the types included in {Player Node, Chance Node, Action}
- The Chance Node must implement a Children() function, that returns all children nodes alongside the probabilities of reaching them. These probabilities must add up to one.
- The Game Class must implement a UtilityFunc() function, that takes a history of a path of the game tree, from the root node to any given terminal node, and returns the utility of the terminal node for player one.
  
