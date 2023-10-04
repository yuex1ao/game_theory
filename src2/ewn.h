#ifndef EWN_H
#define EWN_H
#include <stack>
#include <queue>
#include <unordered_map>
#include <string>
using namespace std;

#define MAX_ROW 9
#define MAX_COL 9
#define MAX_PIECES 6
#define MAX_PERIOD 18
#define MAX_PLIES 100
#define MAX_MOVES 16
#define INT_MAX 2147483647

extern int ROW;
extern int COL;

class EWN {
    int row, col; // board's row and column
    int board[MAX_ROW * MAX_COL]; // board size
    int pos[MAX_PIECES + 2];  // pos[0] and pos[MAX_PIECES + 1] are not used, position of each piece
    int dice_seq[MAX_PERIOD]; // the cyclic dice sequence
    int period; // each cycle's number quantity
    int goal_piece; // 0 -> any piece , 1~6 -> only spicified piece can go to destination

    int history[MAX_PLIES]; // previous move
    int n_plies; // number of moves
    stack<int> pieces;
    stack<int> directions;
    priority_queue<pair<int,int>, vector<pair<int, int>>, greater<pair<int, int>>> opened; // cost, move
    unordered_map<string, int> closed; // pos[1~6], estimated cost

public:
    EWN();
    void scan_board();
    void print_board();
    bool is_goal();

    int move_gen_all(int *moves);
    void do_move(int move);
    void undo();

    void dfid(int, int, bool&);
    int heuristic(int);
    int heuristic2();
    void A_Star(int, bool&);
    int getSize();
    int getPiece();
    int getDir();
    void sort_move(int *moves, int n_move);
};

#endif
