#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
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
    ipos[0] = 999;
    ipos[MAX_PIECES + 1] = 999;
    for (int i = 1; i <= MAX_PIECES; i++) {
        pos[i] = -1;
        ipos[i] = -1;
    }
    period = 0;
    goal_piece = 0;
    n_plies = 0;
    parentNode = nullptr;
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
        if (board[i] > 0) {// store the position of each piece
            pos[board[i]] = i;
            ipos[board[i]] = i;
        }
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
    for (int i = 0; i < ROW; i++) {
        for (int j = 0; j < COL; j++) {
            printf("%4d", board[i * COL + j]);
        }
        printf("\n");
    }
}

bool EWN::is_goal() { // reach goal
    if (goal_piece == 0) {
        if (board[ROW * COL - 1] > 0) return true;
    }
    else {
        if (board[ROW * COL - 1] == goal_piece) return true;
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

int EWN::move_gen_all(int *moves, int plies) {
    int count = 0;
    int dice = dice_seq[plies % period];
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

void EWN::setGoal() {
    int goal = 0, minCost = INT_MAX, minTemp = 0;
    for(int i = 1; i <= 6; i++) {
        if(pos[i] == -1) continue;
        goal_piece = i;
        minTemp = heuristic(0);
        if(minTemp < minCost) {
            goal = i;
            minCost = minTemp;
        }
    }
    goal_piece = goal;
}

int EWN::heuristic(int plies) {
    if(!pos[goal_piece]) return 1000;
    int freq[MAX_PIECES + 2] = {0};
    for(auto &i: dice_seq) {
        if(i < 1 || i > 6) break;
        if(pos[i] == -1) {
            int small = i - 1, large = i + 1;
            while(pos[small] == -1) small--;
            while(pos[large] == -1) large++;
            freq[small]++;
            freq[large]++;
        }
        else freq[i]++;
    }
    // 利用dice_seq一步一步縮短dice之間的距離(update distance)，然後在加上goal(nearest) piece與終點的距離。
    int piece = goal_piece;
    int cost1[MAX_PIECES + 2] = { 0 }, cost2[MAX_PIECES + 2] = { 0 }; // 幹掉i的cost、幹掉i後到達終點cost
    int dr = pos[piece] / COL + 1, dc = pos[piece] % COL + 1; // goal_piece's position
    int distance_goal = max(ROW - dr, COL - dc); // goal_piece與終點的距離
    // 幹掉小於goal_piece的dice。
    for(int i = piece - 1; i >= 1; i--) {
        // 如果是指定dice不能幹掉，在dice_seq沒出現過則不值得幹掉
        if(!pos[i]) {
            cost1[i] = cost1[i + 1];
            cost2[i] = period * ROW;
            continue;
        }
        int r = pos[i] / COL + 1, c = pos[i] % COL + 1;
        int dist[MAX_PIECES + 2] = {0}; // piece i 到各個piece的距離
        for(int p = 1; p <= 6; p++) {
            if(i == p || !pos[p]) dist[p] = -1;
            else {
                int tr = pos[p] / COL + 1, tc = pos[p] % COL + 1;
                dist[p] = max(abs(tr - r), abs(tc - c));
            }
        }
        int seq = plies % period;
        int distance = distance_goal;
        while(true) {
            int dice = dice_seq[seq];
            if(++seq == period) seq = 0;
            if(dice == piece) { // 若dice為goal_piece的話則可以往終點走一步
                distance--;
            }
            else if(dice == i) { // 若為i的話則可以任意往其他piece靠近一步
                for(int k = 1; k <= 6; k++) {
                    dist[k]--;
                }
            }
            else if(pos[dice] == -1) {
                int small = dice - 1, large = dice + 1;
                while(pos[small] == -1) small--;
                while(pos[large] == -1) large++;
                if(small || large == i) {
                    for(int k = 1; k <= 6; k++) {
                        dist[k]--;
                    }
                }
                else {
                    dist[small]--;
                    dist[large]--;
                }
            }
            else {
                dist[dice]--;
            }
            cost1[i]++;
            if(!dist[1] || !dist[2] || !dist[3] || !dist[4] || !dist[5] || !dist[6]) break;
        }

        int small = i, large = piece;
        while(pos[small] == -1) small--;
        while(pos[large] == -1) large++; 
        while(distance > 0 && cost2[i] < period * ROW) {
            if(dice_seq[seq] >= small && dice_seq[seq] <= large) {
                distance--;
            }
            cost2[i]++;
            if(++seq == period) seq = 0;
        }
    }
    // 幹掉大於goal_piece的dice。
    for(int i = piece + 1; i <= 6; i++) { // 先估計哪些子必須、值得幹掉。
        // 如果是指定dice不能幹掉，在dice_seq沒出現過則不值得幹掉
        if(!pos[i]) {
            cost1[i] = cost1[i - 1];
            cost2[i] = period * ROW;
            continue;
        }
        int r = pos[i] / COL + 1, c = pos[i] % COL + 1;
        int dist[MAX_PIECES + 2] = {0}; // piece i 到各個piece的距離
        for(int p = 1; p <= 6; p++) {
            if(i == p || !pos[p]) dist[p] = -1;
            else {
                int tr = pos[p] / COL + 1, tc = pos[p] % COL + 1;
                dist[p] = max(abs(tr - r), abs(tc - c));
            }
        }
        int seq = plies % period;
        int distance = distance_goal;
        while(true) {
            int dice = dice_seq[seq];
            if(++seq == period) seq = 0;
            if(dice == piece) {
                distance--;
            }
            else if(dice == i) {
                for(int k = 1; k <= 6; k++) {
                    dist[k]--;
                }
            }
            else if(pos[dice] == -1) {
                int small = dice - 1, large = dice + 1;
                while(pos[small] == -1) small--;
                while(pos[large] == -1) large++;
                if(small || large == i) {
                    for(int k = 1; k <= 6; k++) {
                        dist[k]--;
                    }
                }
                else {
                    dist[small]--;
                    dist[large]--;
                }
            }
            else {
                dist[dice]--;
            }
            cost1[i]++;
            if(!dist[1] || !dist[2] || !dist[3] || !dist[4] || !dist[5] || !dist[6]) break;
        }

        int small = piece, large = i;
        while(pos[small] == -1) small--;
        while(pos[large] == -1) large++; 
        while(distance > 0 && cost2[i] < period * ROW) {
            if(dice_seq[seq] >= small && dice_seq[seq] <= large) {
                distance--;
            }
            cost2[i]++;
            if(++seq == period) seq = 0;
        }
    }
    // 自己走的話
    int seq = plies % period;
    int distance = distance_goal, small = piece - 1, large = piece + 1;
    while(pos[small] == -1) small--;
    while(pos[large] == -1) large++;
    while(distance > 0 && cost2[piece] < period * ROW) {
        if(dice_seq[seq] > small && dice_seq[seq] < large) {
            distance--;
        }
        cost2[piece]++;
        if(++seq == period) seq = 0;
    }
    int left = piece - 1, right = piece + 1;
    while(left - 1 >= 1) { // ex: 要讓3骰到1也可以走必須同時幹掉1、2。
        cost1[left - 1] += cost1[left];
        left--;
    }
    while(right + 1 <= 6) {
        cost1[right + 1] += cost1[right];
        right++;
    }

    int minCost = INT_MAX;
    for(int i = 1; i <= 6; i++) {
        minCost = min(minCost, cost1[i] + cost2[i]);
    }
    return minCost;
}

void EWN::A_Star(Node *ancestor, bool &find) {
    if(!goal_piece) {
        setGoal();
    }
    int moves[MAX_MOVES];
    n_plies = 0;
    int n_move = move_gen_all(moves, ancestor->cost_g);
    copy(moves, moves + MAX_MOVES, history);
    for(int i = 0; i < n_move; i++) {
        int move = moves[i];
        do_move(move);
        int cost = heuristic(ancestor->cost_g + 1);
        Node *node = new Node();
        string currentBoard; // closed list's key
        for(int i = 1; i <= 6; i++) {
            node->position[i] = pos[i];
            currentBoard += to_string(pos[i]);
        }
     
        node->cost_g = ancestor->cost_g + 1, node->cost_h = cost;
        node->move = move, node->parent = ancestor;
        
        undo(); // check closed list
        if(!closed.count(currentBoard) || closed[currentBoard] > node->cost_g + cost) {
            opened.push(node); // haven't been visited  or  find less cost
        }
    }
    if(!opened.empty() && !find) {
        Node *current = opened.top();
        int move = current->move;
        opened.pop();

        int p[MAX_PIECES + 2] = { 999,-1,-1,-1,-1,-1,-1,999 }; // reserve current pos[i]
        memset(board, 0, MAX_ROW * MAX_COL * 4); // set all number of board to 0
        if(current->cost_g > 1) {
            Node *parent = current->parent;
            for(int i = 1; i <= 6; i++) { // to parent's board
                p[i] = pos[i];
                pos[i] = parent->position[i];
                if(pos[i] < 0) continue;
                board[pos[i]] = i;
            }
        }
        else {
            for(int i = 1; i <= 6; i++) { // to initial board
                pos[i] = ipos[i];
                p[i] = pos[i];
                if(pos[i] < 0) continue;
                board[pos[i]] = i;
            }
        }

        do_move(move); // do least cost move
        string currentBoard; // keep move into closed list
        for(int i = 1; i <= 6; i++) {
            currentBoard += to_string(current->position[i]);
        }
        closed[currentBoard] = current->cost_g + current->cost_h;

        int piece = move >> 4;
        int direction = move & 15;

        if(is_goal()) { // if reach destination
            find = true;
            parentNode = current->parent; // keep parentNode to find route
            pieces.push(piece);
            directions.push(direction);
        }
        else {
            A_Star(current, find);
        }
        // undo
        memset(board, 0, MAX_ROW * MAX_COL * 4);
        for(int i = 1; i <= 6; i++) {
            pos[i] = p[i];
            if(pos[i] < 0) continue;
            board[pos[i]] = i;
        }
        if(find && current == parentNode) {
            pieces.push(piece);
            directions.push(direction);
            if(current && current->parent) parentNode = current->parent;
        }
    }
}