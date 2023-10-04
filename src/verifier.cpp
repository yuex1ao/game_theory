#include <cstdio>
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <cstring>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/wait.h>
#include "ewn.cpp"

void Redir_Stdin(const char *testcase) { // 將指定的檔案設定為程式的標準輸入
    int fd;
    if ((fd = open(testcase, O_RDONLY)) == -1) { // open file (read only)
        fprintf(stderr, "open failed: %s: %s\n", strerror(errno), testcase);
        exit(1);
    }
    // 將 fd 指向的檔案描述符複製到標準輸入描述符（0），所有的標準輸入操作將從 Redir_Stdin 函數中打開的檔案中讀取。
    if (dup2(fd, 0) == -1) { // 0 -> stdin, 1 -> stdout, 2 -> stderr;
        perror("dup2 failed");
        exit(1);
    }
    close(fd);
}

int Exec_EWN_Program(const char *program) {
    int pipe_fd[2];
    if (pipe(pipe_fd) == -1) {
        perror("pipe failed");
        exit(1);
    }

    int pid = fork();
    if (pid == -1) {
        perror("fork failed");
        exit(1);
    } 
    else if (pid == 0) {
        lseek(0, 0, SEEK_SET);
        if (dup2(pipe_fd[1], 1) == -1) {
            perror("dup2 failed");
            exit(1);
        }
        close(pipe_fd[0]);
        close(pipe_fd[1]);
        execl(program, program, NULL);
        fprintf(stderr, "exec failed: %s: %s\n", strerror(errno), program);
        exit(1);
    }
    else {
        if (dup2(pipe_fd[0], 0) == -1) {
            perror("dup2 failed");
            exit(1);
        }
        close(pipe_fd[0]);
        close(pipe_fd[1]);
    }

    return pid;
}

int main(int argc, char *argv[]) {
    if (argc != 3) { // XXX.cpp, XXX.exe, XX.in -> count = 3
        fprintf(stderr, "usage: ./verifier program testcase\n");
        exit(1);
    }
    EWN game;
    int moves[MAX_MOVES]; // 16 (a piece has 8 legal moves, if piece die -> front and next can move -> 16 moves)
    int n_move; // legal moves count
    int n_ply; // total round

    Redir_Stdin(argv[2]); // testcase
    game.scan_board();
    Exec_EWN_Program(argv[1]); // exe
    wait(NULL);

    printf("ply: 0\n");
    game.print_board();
    printf("\n");

    // DO
    
    bool find = false;
    for(int i = 1; i < MAX_PLIES; i++) {
        game.dfid(0, i, find); // index of dice_seq, 第幾步
        if(find) break;
    }
    n_ply = game.getSize();

    ofstream fout("Output.out");
    if(!fout) {
        cout << "Output.out open failed." << endl;
        exit(1);
    }
    fout << n_ply << endl;

    //scanf(" %d", &n_ply); // min number of moves
    for (int i = 0; i < n_ply; i++) {
        int piece, direction; // piece and direction of each move
        piece = game.getPiece();
        direction = game.getDir();
        //scanf(" %d %d", &piece, &direction);
        int move = piece << 4 | direction; // 7~4: store the piece number, 3~0: store the direction
        bool legal = false;

        printf("ply: %d\n", i + 1);
        printf("piece: %d, dir: %d\n", piece, direction);

        n_move = game.move_gen_all(moves); // the quantity of legal moves
        for (int j = 0; j < n_move; j++) {
            if (moves[j] == move) {
                legal = true;
                break;
            }
        }
        if (!legal) {
            printf("ILLEGAL!!\n");
            exit(1);
        }

        fout << piece << " " << direction << endl;
        game.do_move(move);
        game.print_board();
        printf("\n");
    }

    fout.close();
    printf("Legal\n");
    return 0;
}
