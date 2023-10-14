#ifndef EWN_H
#define EWN_H
#include <stack>
#include <queue>
#include <vector>
#include <unordered_map>
#include <string>
using namespace std;

#define MAX_ROW 9
#define MAX_COL 9
#define MAX_PIECES 6
#define MAX_PERIOD 18
#define MAX_MOVES 16
#define INT_MAX 2147483647

extern int ROW;
extern int COL;

class Node {
public:
    int cost_g;
    int cost_h;
    int move;
    int position[MAX_PIECES + 2];
    Node *parent;
    Node() : move(0), cost_g(0), cost_h(0), parent(nullptr) {
        memset(position, -1, 32);
    }
    bool operator==(Node const& obj) {
        if(cost_g != obj.cost_g || cost_h != obj.cost_h || move != obj.move) return false;
        for(int i = 1; i <= 6; i++) {
            if(position[i] != obj.position[i]) return false;
        }
        return true;
    }
};

struct cmp {
    bool operator()(Node* a, Node* b) {
        return a->cost_g + a->cost_h > b->cost_g + b->cost_h;
    }
};

class EWN {
    int row, col; // board's row and column
    int board[MAX_ROW * MAX_COL]; // board size
    int pos[MAX_PIECES + 2];  // pos[0] and pos[MAX_PIECES + 1] are not used, position of each piece
    int dice_seq[MAX_PERIOD]; // the cyclic dice sequence
    int period; // each cycle's number quantity
    int goal_piece; // 0 -> any piece , 1~6 -> only spicified piece can go to destination

    int history[MAX_MOVES]; // previous move
    int n_plies; // number of moves

    int ipos[MAX_PIECES + 2]; // initial position
    Node *parentNode;
    stack<int> pieces;
    stack<int> directions;
    priority_queue<Node*, vector<Node*>, cmp> opened; // cost, move
    unordered_map<string, int> closed; // pos[1~6], estimated cost

public:
    EWN();
    void scan_board();
    void print_board();
    bool is_goal();

    int move_gen_all(int *moves, int);
    void do_move(int move);
    void undo();

    int getSize();
    int getPiece();
    int getDir();

    int heuristic(int, int);
    int heuristic2(int);
    void A_Star(Node*, bool&);
    void sort_move(int *moves, int n_move);
};

#endif
