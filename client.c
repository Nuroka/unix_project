#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <stdbool.h>
#include "game_protocol.h"

void Connect();
void Intro();
void SendBudget();
void play_game();
void FirstCardPrint();
void SecondCardPrint();
void ChoiceCallDIE();
void GameResult(int temp);
void ErrorCheck(int num, char* str);
int Retry();
void GameOver();

Card player[2];
Card com[2];
GameState state;
int sd;
int budget;

int main() {
    
    system("clear");
    Connect();
    Intro();
    SendBudget();
    play_game(sd);

    close(sd);
    return 0;
}

void Connect() {
    struct sockaddr_in server_addr;

    sd = socket(AF_INET, SOCK_STREAM, 0);
    ErrorCheck(sd, "socket");

    memset((char *)&server_addr, '\0', sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = inet_addr(IPADDR);
    ErrorCheck(connect(sd, (struct sockaddr*)&server_addr, sizeof(server_addr)), "connect");
}

void Intro() {
    printf("****************************************\n");
    printf("*                                      *\n");
    printf("*             Seotda Game              *\n");
    printf("*                                      *\n");
    printf("*   Welcome to the traditional game!   *\n");
    printf("* Enjoy playing Seotda with computer.  *\n");
    printf("*                                      *\n");
    printf("****************************************\n");
    sleep(5);
}

void SendBudget(){
    int budget;

    system("clear");
    printf("Set budget (10000 or more): ");
    scanf("%d", &budget);

    ErrorCheck(send(sd, &budget, sizeof(budget), 0), "Send Budget");
}

void play_game() {
    do {
        system("clear");

        int header;
        // 서버로부터 상태 수신
        ErrorCheck(recv(sd, &header, sizeof(header), 0), "Receive Header");   // 헤더 수신
        ErrorCheck(recv(sd, &state, header, 0), "Receive State");             // 상태 수신
        // 플레이어 카드 수신
        ErrorCheck(recv(sd, &header, sizeof(header), 0), "Receive Header");   // 헤더 수신
        ErrorCheck(recv(sd, &player, header, 0), "Receive Player Card");      // 카드 수신
        // 컴 카드 수신
        ErrorCheck(recv(sd, &header, sizeof(header), 0), "Receive Header");   // 헤더 수신
        ErrorCheck(recv(sd, &com, header, 0), "Receive Computer Card");       // 카드 수신

        
        FirstCardPrint(player[0]);  // 첫 번째 카드, 남은 돈 출력
        ChoiceCallDIE();            // Call/Die 선택

        // 서버로 상태 전송
        ErrorCheck(send(sd, &state, sizeof(state), 0), "Send State");
        int temp = state.player_money;
        // 서버로부터 결과 수신
        ErrorCheck(recv(sd, &state, sizeof(state), 0), "Receive State");

        if (state.message_type == RESULT) {
            GameResult(temp);
        }

        // 돈 다 떨어졌을 때
        if (state.player_money == 0 || state.computer_money == 0) {
            GameOver();
            close(sd);
            exit(0);
        }
    } while (Retry());

    printf("\n");
    printf("Your remaining budget: %d, Computer remaining budget: %d\n", state.player_money, state.computer_money);
    GameOver();
}

void FirstCardPrint() {
    printf("========================================\n");
    printf("Your budget: %d, Computer budget: %d\n", state.player_money, state.computer_money);
    printf("\nYour First Card: %s\n", player[0].name);
}

void SecondCardPrint() {
    printf("\nYour Second Card: %s\n", player[1].name);
    Ascending(player);
    printf("Your Card: %s, %s\n", player[0].name, player[1].name);
}

void ChoiceCallDIE() {
    // 첫 번째 행동 선택 / 1또는 2값 매핑
    printf("Enter choice (1 for CALL, Not 1 for DIE): ");
    if (scanf("%d", &state.player_choice1) != 1) {
        while (getchar() != '\n'); // 입력 버퍼 비우기
        state.player_choice1 = 2; // 숫자가 아니면 2로 설정
    } else if (state.player_choice1 != 1) {
        state.player_choice1 = 2; // 1이 아닌 숫자는 2로 설정
    }

    // First CALL
    if (state.player_choice1 == CALL) {
        printf("Enter additional first bet: ");
        // 배팅금액 예외처리 / 배팅금액이 남은 돈을 초과할 때
        do {
            scanf("%d", &state.player_bet1);
            if ((state.player_bet1 > state.computer_money) || (state.player_bet1 > state.player_money)) {
                printf("Bet exceeds available money. Enter again: ");
            }
        } while ((state.player_bet1 > state.computer_money) || (state.player_bet1 > state.player_money));

        // 손패 전체 출력
        SecondCardPrint();

        // 두 번째 행동 선택
        printf("Enter choice (1 for CALL, Not 1 DIE): ");
        if (scanf("%d", &state.player_choice2) != 1) {
            while (getchar() != '\n'); // 입력 버퍼 비우기
            state.player_choice2 = 2; // 숫자가 아니면 2로 설정
        } else if (state.player_choice2 != 1) {
            state.player_choice2 = 2; // 1이 아닌 숫자는 2로 설정
        }

        // Second CALL
        if (state.player_choice2 == CALL) {
            printf("Enter additional second bet: ");
            // 배팅금액 예외처리 / 배팅금액이 남은 돈을 초과할 때
            do {
                scanf("%d", &state.player_bet2);
                if ((state.player_bet1 + state.player_bet2 > state.computer_money) || (state.player_bet1 + state.player_bet2 > state.player_money)) {
                    printf("Total bet exceeds available money. Enter again: ");
                }
            } while ((state.player_bet1 + state.player_bet2 > state.computer_money) || (state.player_bet1 + state.player_bet2 > state.player_money));
        }
    }
}

void GameResult(int temp) {
    // 손 패로 승패를 정하고 컴퓨터 카드 노출
    if ((state.player_choice1 == CALL) && (state.player_choice2 == CALL)) {
        Ascending(com);
        printf("\nCom Cards: %s %s\n", com[0].name, com[1].name);
    }
    // 결과 출력
    if (temp < state.player_money) {
        printf("Player Win!\n");
    } else if (temp == state.player_money) {
        printf("Draw!\n");
    } else {
        printf("Computer Win!\n");
    }
    printf("Round result: Your budget: %d, Computer budget: %d\n", state.player_money, state.computer_money);
    printf("========================================\n");
}

void ErrorCheck(int num, char* str) {
    if (num == -1) {
        perror(str);
        exit(1);
    }
}

int Retry() {
    int retry;
    printf("\n");
    printf("Your remaining budget is %d\n", state.player_money);
    printf("If you want to play continue playing press 1 (1 : Continue, 2 : Quit): ");
    if (scanf("%d", &retry) != 1) {
        while (getchar() != '\n'); // 입력 버퍼 비우기
        retry = 0; // 숫자가 아니면 0으로 설정
    } else if (retry != 1) {
        retry = 0; // 1이 아닌 숫자는 0으로 설정
    }
    
    return retry;
}

void GameOver() {
    printf("\n");
    printf("========================================\n");
    printf("||                                    ||\n");
    if (state.computer_money < state.player_money)
        printf("||            Player Win!             ||\n");
    else if (state.computer_money == state.player_money)
        printf("||               Draw!                ||\n");
    else
        printf("||              Com Win!              ||\n");
    printf("||                                    ||\n");
    printf("========================================\n");
}