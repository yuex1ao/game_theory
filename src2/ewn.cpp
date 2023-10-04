#include <cstdio>
#include <cstdlib>
#include <cstring>
#include "ewn.h"


// these variables are available after calling EWN::scan_board()
int ROW;
int COL;
static int dir_value[8];

EWN::EWN() {
    row = 0;
    col = 0;
    pos[0] = 999;
    pos[MAX_PIECES + 1] = 999;
    for (int i = 1; i <= MAX_PIECES; i++) {
        pos[i] = -1;
    }
    period = 0;
    goal_piece = 0;
    n_plies = 0;
}

void set_dir_value() { // set direction (1D)
    dir_value[0] = -COL - 1;
    dir_value[1] = -COL;
    dir_value[2] = -COL + 1;
    dir_value[3] = -1;
    dir_value[4] = 1;
    dir_value[5] = COL - 1;
    dir_value[6] = COL;
    dir_value[7] = COL + 1;
}

void EWN::scan_board() { // scan the board in testcase file
    scanf(" %d %d", &row, &col);
    for (int i = 0; i < row * col; i++) {
        scanf(" %d", &board[i]);
        if (board[i] > 0) // store the position of each piece
            pos[board[i]] = i;
    }
    scanf(" %d", &period);
    for (int i = 0; i < period; i++) {
        scanf(" %d", &dice_seq[i]);
    }
    scanf(" %d", &goal_piece);

    // initialize global variables
    ROW = row;
    COL = col;
    set_dir_value();
}

void EWN::print_board() {
    for (int i = 0; i < row; i++) {
        for (int j = 0; j < col; j++) {
            printf("%4d", board[i * col + j]);
        }
        printf("\n");
    }
}

bool EWN::is_goal() { // reach goal
    if (goal_piece == 0) {
        if (board[row * col - 1] > 0) return true;
    }
    else {
        if (board[row * col - 1] == goal_piece) return true;
    }
    return false;
}

/*
move: an integer using only 12 bits
3~0: store the direction
7~4: store the piece number
11~8: store the eaten piece (used only in history)
*/

int move_gen2(int *moves, int piece, int location) {
    int count = 0;
    int row = location / COL; // 1-D array
    int col = location % COL;

    // the legality of up, left, right, down four dir
    bool left_ok = (col != 0);
    bool right_ok = (col != COL - 1);
    bool up_ok = (row != 0);
    bool down_ok = (row != ROW - 1);

    // 7~4: store the piece number, 3~0: store the direction
    if (up_ok) moves[count++] = piece << 4 | 1; // up: 1
    if (left_ok) moves[count++] = piece << 4 | 3; // left: 3
    if (right_ok) moves[count++] = piece << 4 | 4; // right: 4
    if (down_ok) moves[count++] = piece << 4 | 6; // down: 6

    if (up_ok && left_ok) moves[count++] = piece << 4 | 0; // up-left: 0
    if (up_ok && right_ok) moves[count++] = piece << 4 | 2; // up-right: 2
    if (down_ok && left_ok) moves[count++] = piece << 4 | 5; // down-left: 5
    if (down_ok && right_ok) moves[count++] = piece << 4 | 7; // down-right: 7

    return count;
}

int EWN::move_gen_all(int *moves) {
    int count = 0;
    int dice = dice_seq[n_plies % period];
    if (pos[dice] == -1) { // the dice is die
        int small = dice - 1;
        int large = dice + 1;

        while (pos[small] == -1) small--;
        while (pos[large] == -1) large++;

        if (small >= 1)
            count += move_gen2(moves, small, pos[small]);
        if (large <= MAX_PIECES)
            count += move_gen2(moves + count, large, pos[large]);
    }
    else { // the dice is still on the board
        count = move_gen2(moves, dice, pos[dice]);
    }
    return count;
}

void EWN::do_move(int move) {
    int piece = move >> 4; // 7~4: store the piece number
    int direction = move & 15; // 1111
    int dst = pos[piece] + dir_value[direction]; // current move destination

    if (n_plies == MAX_PLIES) {
        fprintf(stderr, "cannot do anymore moves\n");
        exit(1);
    }
    if (board[dst] > 0) {
        pos[board[dst]] = -1; // one dice is been eating
        move |= board[dst] << 8; // 11~8: store the eaten piece (used only in history)
    }
    board[pos[piece]] = 0;
    board[dst] = piece;
    pos[piece] = dst;
    history[n_plies++] = move;
}

void EWN::undo() { // return to previous move
    if (n_plies == 0) {
        fprintf(stderr, "no history\n");
        exit(1);
    }

    int move = history[--n_plies];
    int eaten_piece = move >> 8;
    int piece = (move & 255) >> 4;
    int direction = move & 15;
    int dst = pos[piece] - dir_value[direction];

    if (eaten_piece > 0) {
        board[pos[piece]] = eaten_piece;
        pos[eaten_piece] = pos[piece];
    }
    else board[pos[piece]] = 0;
    board[dst] = piece;
    pos[piece] = dst;
}

void EWN::dfid(int count, int depth, bool &find) {
    if(count == depth) return;
    int moves[MAX_MOVES];
    int n_move = move_gen_all(moves);
    for(int i = 0; i < n_move; i++) {
        int move = moves[i];
        do_move(move);
        int piece = move >> 4;
        int direction = move & 15;
        if(is_goal()) {
            pieces.push(piece);
            directions.push(direction);
            find = true;
            undo();
            return;
        }
        dfid(count + 1, depth, find);
        undo();
        if(find) {
            pieces.push(piece);
            directions.push(direction);
            return;
        }
    }
    return;
}

int EWN::heuristic(int move) { // cost function
    // pos[1~6]看誰還活著，有哪些dice值自己可以走、然後去看dice_seq結合pos，算一輪有多少步可以走 + 當前離終點距離。
    int piece = move >> 4, cost = 0;
    int r = 0, c = 0, distance = INT_MAX;
    if (goal_piece) { 
        r = pos[goal_piece] / col + 1, c = pos[goal_piece] % col + 1;
        distance = min(row - r, col - c);
    }
    else {
        for(int i = 1; i <= 6; i++) {
            if(pos[i] == -1) continue;
            r = pos[i] / col + 1, c = pos[i] % col + 1;
            int dist = min(row - r, col - c);
            distance = min(dist, distance);
        }
    }
    int seq = n_plies % period;
    while(distance > 0) {
        bool match = false;
        int dice = dice_seq[seq];
        if(dice == -1) {
            int small = dice - 1, large = dice + 1;
            while (pos[small] == -1) small--;
            while (pos[large] == -1) large++;
            if(small == piece || large == piece) match = true;
        }
        else if (dice == piece) match = true;
        if(match) distance--;
        if(++cost >= 100) break;
    }
    // add to closed list
    string close;
    for(int i = 1; i <= 6; i++) {
        close += to_string(pos[i]);
    }
    closed[close] = cost;
    return cost;
}

int EWN::heuristic2() {
    return 0;
}

void EWN::A_Star(int count, bool &find) {
    int moves[MAX_MOVES];
    int n_move = move_gen_all(moves);
    for(int i = 0; i < n_move; i++) {
        int move = moves[i];
        do_move(move);
        int cost = count + heuristic(move);
        undo();
        opened.push(make_pair(cost, move));
    }
    if(!opened.empty() && !find) { // DO：找到goal的話要怎麼trace之前走過的盤面
        int move = opened.top().second;
        opened.pop();
        do_move(move);
        int piece = move >> 4;
        int direction = move & 15;
        if(is_goal()) {
            pieces.push(piece);
            directions.push(direction);
            find = true;
            undo();
        }
    }
}

int EWN::getSize() {
    return pieces.size();
}

int EWN::getPiece() {
    int piece = pieces.top();
    pieces.pop();
    return piece;
}

int EWN::getDir() {
    int dir = directions.top();
    directions.pop();
    return dir;
}