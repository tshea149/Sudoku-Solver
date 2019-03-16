#include <queue>
#include <iostream>
#include <chrono>
#include <cstdint>

/* for loading puzzles from file */
#include <string>
#include <fstream>
#include <ctype.h>
/**/

struct BoardSpace
{
	// board coordinates range from 0 - 8
	// possible values from a space range from 1 - 9
	// smallest addressable value in C++ : 1 byte (uint8_t / char)
	uint8_t x, y;
	std::queue<uint8_t> possible_values;

	BoardSpace() = default;
	BoardSpace(uint8_t _x, uint8_t _y) : x(_x), y(_y) {};
};

struct Move
{
	uint8_t x;
	uint8_t y;
	uint8_t value;

	Move(uint8_t _x, uint8_t _y, uint8_t _value) : x(_x), y(_y), value(_value) {};
};

bool checkRowForValue(uint8_t value, uint8_t row, const uint8_t(&board)[9][9])
{
	for (uint8_t col = 0; col < 9; col++)
	{
		if (board[row][col] == value)
			return true;
	}

	return false; // value not found
}

bool checkColumnForValue(uint8_t value, uint8_t col, const uint8_t(&board)[9][9])
{
	for (uint8_t row = 0; row < 9; ++row)
	{
		if (board[row][col] == value)
			return true;
	}

	return false; // value not found
}

bool checkSectionForValue(uint8_t value, uint8_t row, uint8_t col, const uint8_t(&board)[9][9])
{
	// determine which 3x3 section to check
	uint8_t search_row = row - (row % 3);
	uint8_t search_col = col - (col % 3);

	for (uint8_t i = search_row; i < search_row + 3; ++i)
	{
		for (uint8_t j = search_col; j < search_col + 3; ++j)
		{
			if (value == board[i][j])
				return true;
		}
	}

	return false; // value not found
}

// Parameter 1/2 : x/y coordinate of board space of which to check moves for 
// Parameter 3 : const reference to board
// no return. After call, board_space.possible_values will contain all valid moves for the given position
BoardSpace getMoveValues(const uint8_t x, const uint8_t y, const uint8_t(&board)[9][9])
{
	// space.x/space.y represent which board row/column to look at respectively

	// a value for the space is possible if it is not found in the approprate row, column, and section
	//	if a value is found within row, column, or section, the value is not possible and checking other spaces is a waste of time

	BoardSpace board_space(x, y);

	for (uint8_t value = 1; value <= 9; ++value)
	{
		if (false == checkRowForValue(value, x, board))
		{
			if (false == checkColumnForValue(value, y, board))
			{
				if (false == checkSectionForValue(value, x, y, board))
				{
					board_space.possible_values.push(value); // value is valid for the space, add it to the queue
				}
			}
		}
	}

	return board_space;
}

// returns a BoardSpace containing a queue of all possible values for the best move on the given board state
// returned BoardSpace.possible_values will be empty if no possible moves exist
bool getBestBoardSpace(BoardSpace &best_move, const uint8_t(&board)[9][9])
{
	for (uint8_t i = 0; i < 10; i++)
		best_move.possible_values.push(i);	// initialize best possible move to bad (and impossible) case for move comparisons

	BoardSpace move;
	for (uint8_t x = 0; x < 9; ++x)
	{
		for (uint8_t y = 0; y < 9; ++y)
		{
			if (board[x][y] == 0)	// check for available space
			{
				move = getMoveValues(x, y, board);	// get possible values for the space

				if (false == move.possible_values.empty())	// check if a valid move exists for the space
				{
					if (move.possible_values.size() == 1)
					{
						// no better move can be found
						best_move = move;
						return false;
					}

					if (move.possible_values.size() < best_move.possible_values.size())
						best_move = move;
				}
				else
				{
					// a BoardSpace was found with no possible moves, no moves will be valid
					best_move.possible_values = {};
					return false;
				}
			}
		}
	}

	if (10 != best_move.possible_values.size()) // if a best move was found
		return false;							// puzzle is not solved

	return true;	// no best move or space with 0 moves were found. puzzle must be complete.
}

// solveBoard will hold the last move coordinates, so that the board space can be set to empty if backtracking is necessary
// returns false if the board is in an unsolvable state
bool solveBoard(uint8_t(&board)[9][9])
{
	// get best space based on current board state and check if puzzle is complete
	BoardSpace move;
	if (true == getBestBoardSpace(move, board))
	{
		// puzzle is solved
		return true;
	}

	else
	{
		// puzzle is not solved
		if (move.possible_values.empty())
			return false;	// unsolvable board state

		while (false == move.possible_values.empty())	// run through all possible moves
		{
			board[move.x][move.y] = move.possible_values.front();	// make move
			move.possible_values.pop();								// remove move

			if (true == solveBoard(board))							// advance board state
				return true;
		}

		// board is in an unsolvable state
		board[move.x][move.y] = 0;	// clear board space
		return false;				// revert to previous state
	}
}

void printBoard(uint8_t(&board)[9][9])
{
	for (int i = 0; i < 9; i++)
	{
		if (i % 3 == 0 && i != 0)
			std::cout << "------+-------+------\n";
		for (int j = 0; j < 9; j++)
		{
			if (j % 3 == 0 && j != 0)
				std::cout << "| ";
			std::cout << (int)board[i][j] << " ";
		}
		std::cout << std::endl;
	}
}

bool loadPuzzle(std::string filename, uint8_t(&puzzle)[9][9])
{
	std::ifstream in;
	in.open(filename);

	if (false == in.is_open())
	{
		std::cout << "File " << filename << " not found." << std::endl;
		return false;
	}

	std::string row_str;
	for (uint8_t x = 0; x < 9; ++x)
	{
		getline(in, row_str);
		if (9 != row_str.size())
		{
			std::cout << "Invalid puzzle file format." << std::endl;
			return false;
		}

		for (uint8_t y = 0; y < 9; ++y)
		{
			if (isdigit(row_str[y]))
				puzzle[x][y] = (uint8_t)row_str[y] - 48;	// numbers begin at 48 in ASCII, subtract 48 from value of int as char to proper value as int

			else
			{
				std::cout << "Invalid puzzle file format." << std::endl;
				return false;
			}
		}
	}

	if (false == in.eof())
	{
		std::cout << "Invalid puzzle file format. (Unexpected end of file.)" << std::endl;
		return false;
	}

	return true;
}

int main(int argc, char* argv[])
{
	if (argc > 2)
		return -1;

	uint8_t puzzle[9][9];

	if (argc == 2)
	{
		if (false == loadPuzzle(argv[1], puzzle))
			return -2;
	}
	else // load default board
	{
		if (false == loadPuzzle("puzzle0.dat", puzzle))
			return -3;
	}

	std::cout << "   Unsolved Puzzle\n---------------------" << std::endl;
	printBoard(puzzle);

	// solve board
	auto start = std::chrono::high_resolution_clock::now();
	solveBoard(puzzle);
	auto end = std::chrono::high_resolution_clock::now();
	auto time_taken = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();

	std::cout << "\n\n   Solved Puzzle\n---------------------" << std::endl;
	printBoard(puzzle);
	std::cout << "\nTime taken: " << time_taken << "  microseconds." << std::endl;

	return 0;
}