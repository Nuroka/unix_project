#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <stdbool.h>
#include "game_protocol.h"

/* 함수 프로토타입 */
void Connect();             // 서버 연결
void Intro();               // 게임 소개 출력
void SendBudget();          // 초기 예산 설정 및 전송
void Play();                // 게임 진행
void FirstCardPrint();      // 첫 번째 카드 출력
void SecondCardPrint();     // 두 번째 카드 출력
void ChoiceCallDIE();       // CALL/DIE 선택 처리
void GameResult(int temp);  // 게임 결과 출력
int Retry();                // 게임 재시작 여부 결정
void GameOver();            // 게임 종료 처리

/* 전역 변수 */
Card player[2];             // 플레이어 카드
Card com[2];                // 컴퓨터 카드
GameState state;            // 게임 상태
int sd;                     // 소켓 디스크립터

int main() {
    
    system("clear");    // 화면 초기화
    Connect();          // 서버 연결  
    Intro();            // 게임 소개
    SendBudget();       // 초기 예산 설정 및 전송
    Play();             // 게임 실행

    close(sd);          // 소켓 닫기
    return 0;
}

// 서버 연결
void Connect() {
    struct sockaddr_in server_addr;

    sd = socket(AF_INET, SOCK_STREAM, 0);   // 인터넷 소켓 생성
    ErrorCheck(sd, "Socket");               // 에러 처리

    memset((char *)&server_addr, '\0', sizeof(server_addr));    // 구조체 초기화
    server_addr.sin_family = AF_INET;                   // IPv4 인터넷 프로토콜
    server_addr.sin_port = htons(PORT);                 // NBO 변환, 포트번호 설정
    server_addr.sin_addr.s_addr = inet_addr(IPADDR);    // IP 주소 설정
    // 소켓 주소 구조체에 지정한 서버로 연결 요청
    ErrorCheck(connect(sd, (struct sockaddr*)&server_addr, sizeof(server_addr)), "Connect");
}

// 게임 소개
void Intro() {
    printf("****************************************\n");
    printf("*                                      *\n");
    printf("*             Seotda Game              *\n");
    printf("*                                      *\n");
    printf("*   Welcome to the traditional game!   *\n");
    printf("* Enjoy playing Seotda with computer.  *\n");
    printf("*                                      *\n");
    printf("****************************************\n");
}

// 예산 설정 및 전송
void SendBudget(){
    printf("\n");
    printf("Set budget: ");
    // 입력 예외 처리 / 양수만 입력
    while (scanf("%d", &state.budget) != 1 || state.budget <= 0) {
        printf("Invalid input. Please enter a positive number: ");
        while (getchar() != '\n'); // 입력 버퍼 비우기
    }
    // 예산을 서버에 전송
    ErrorCheck(send(sd, &state.budget, sizeof(state.budget), 0), "Send Budget");
}

void Play() {
    do {
        system("clear");

        int header;
        // 서버로부터 상태 수신
        // 헤더로 수신받는 데이터 구분
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

        // 플레이서 선택 서버로 전송
        ErrorCheck(send(sd, &state, sizeof(state), 0), "Send State");
        int temp = state.player_money;  // 게임 결과 전 플레이어 예산 저장

        // 서버로부터 게임 결과 수신
        ErrorCheck(recv(sd, &state, sizeof(state), 0), "Receive State");

        GameResult(temp); // 게임 결과 출력

    } while (Retry());  // 재시작 여부 확인

    // 예산이 0이거나 재시작 안할 때
    system("clear");
    printf("* Your remaining budget: %d, Computer remaining budget: %d *\n", state.player_money, state.computer_money);
    GameOver(); // 게임 종료
}

// 첫 번째 카드, 현재 예산 출력
void FirstCardPrint() {
    printf("========================================\n");
    printf("Your budget: %d, Computer budget: %d\n", state.player_money, state.computer_money);
    printf("\nYour First Card: %s\n", player[0].name);
}

// 전체 카드 출력
void SecondCardPrint() {
    printf("\nYour Second Card: %s\n", player[1].name);
    Ascending(player);  // 카드 오름차순 정열
    printf("Your Cards: %s, %s\n", player[0].name, player[1].name);
}

// CALL/DIE 선택 처리
void ChoiceCallDIE() {
    // 첫 번째 행동 선택 (1: CALL/ 2: DIE) 매핑
    printf("Enter choice (1 for CALL, Not 1 for DIE): ");
    if (scanf("%d", &state.player_choice1) != 1) {
        while (getchar() != '\n'); // 입력 버퍼 비우기
        state.player_choice1 = 2; // 숫자가 아니면 2로 설정
    } else if (state.player_choice1 != 1) {
        state.player_choice1 = 2; // 1이 아닌 숫자는 2로 설정
    }

    // 첫 번째 CALL 처리
    if (state.player_choice1 == CALL) {
        printf("Enter additional first bet: ");
        // 배팅금액 예외처리 / 배팅금액이 남은 돈을 초과할 때
        do {
            scanf("%d", &state.player_bet1);
            if ((state.player_bet1 > state.computer_money) || (state.player_bet1 > state.player_money)) {
                printf("Bet exceeds available money. Enter again: ");
            }
        } while ((state.player_bet1 > state.computer_money) || (state.player_bet1 > state.player_money));

        SecondCardPrint();  // 손패 전체 출력

        // 두 번째 CALL/DIE 선택
        printf("Enter choice (1 for CALL, Not 1 DIE): ");
        if (scanf("%d", &state.player_choice2) != 1) {
            while (getchar() != '\n'); // 입력 버퍼 비우기
            state.player_choice2 = 2; // 숫자가 아니면 2로 설정
        } else if (state.player_choice2 != 1) {
            state.player_choice2 = 2; // 1이 아닌 숫자는 2로 설정
        }

        // 두 번째 CALL 처리
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

// 게임 결과 출력
void GameResult(int temp) {
    // CALL & CALL일 경우 컴퓨터 카드 출력
    if ((state.player_choice1 == CALL) && (state.player_choice2 == CALL)) {
        Ascending(com); // 컴퓨터 카드 정렬
        printf("\nCom Cards: %s %s\n", com[0].name, com[1].name);
    }
    // 결과 출력
    if (temp < state.player_money) {            // 플레이어 승리
        printf("Player Win!\n");    
    } else if (temp == state.player_money) {    // 무승부
        printf("Draw!\n");
    } else {                                    //컴퓨터 승리
        printf("Computer Win!\n");  
    }
    // 라운드 결과 출력
    printf("Round result: Your budget: %d, Computer budget: %d\n", state.player_money, state.computer_money);
    printf("========================================\n");
}

// 게임 재시작 여부 확인 (0: end, 1: retry)
int Retry() {
    int retry;

    // 플레이어 또는 컴퓨터 예산이 0일 경우 게임 진행 불가
    if (state.player_money == 0 || state.computer_money == 0) {
        
        retry = 0;
        ErrorCheck(send(sd, &retry, sizeof(retry), 0), "Send Retry");
        sleep(3);   // 화면 초기화 전 3초 화면 노출
        return retry;
    }

    printf("\n");
    printf("Your remaining budget is %d\n", state.player_money);
    printf("If you want to play continue playing press 1 (1 : Continue, 2 : Quit): ");
    // retry 0 또는 1로 매핑
    if (scanf("%d", &retry) != 1) {
        while (getchar() != '\n'); // 입력 버퍼 비우기
        retry = 0; // 숫자가 아니면 0으로 설정
    } else if (retry != 1) {
        retry = 0; // 1이 아닌 숫자는 0으로 설정
    }
    ErrorCheck(send(sd, &retry, sizeof(retry), 0), "Send Retry");   // 재시작 여부 서버에 전송

    return retry;
}

// 게임 종료 처리
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