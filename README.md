# 2803ICT Assignment1 Milestone2
## Socket with multiple player game ("Numbers")

Taiyo Ishiyama (s5287456)
This program is written using Mac OS


## Command line arguments to initiate the program
**Game server: <Port Number> <Game Type> <Number of Players>
*eg: ./server.exe 8080 numbers 3

**Game client: <Game Type> <Server Name> <Port Number>
*eg: ./player.exe numbers mypc 8080


## Rules
Each player can enter a number between 1 - 9 and send it to the server in their turn.
The game server calculates the sum of numbers all players send in their previous turn and send back to players before each turn.
The first player reaching the sum of 30 or more is a winner.
Each player receive a message at the end of the game.

** *NOTES
** Players can leave the game by entering "quit" in their turn.
** If players do not respond for 30 seconds in their turn, they automatically get disconnected from the server.
** When players send invalid input, they receive error messages. If they send invalid input in a row, they automatically get disconnected from the server.
** If there is only one player left in the game, the player automaticallly becomes a winner.
