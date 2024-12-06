#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <time.h>
#include <stdbool.h>
#include "game_protocol.h"

/* 함수 프로토타입 */
void SetServer();           // 서버 설정
void ReceiveBudget();       // 예산 수신
int ReceiveRetry();         // 재도전 여부 확인
void Result();              // 게임 결과 계산
void PrintResult(int num);  // 결과 출력
void Shuffle(Card *all);    // 카드 섞기
Card *Init(void);           // 카드 초기화
void Divider(Card *all);    // 카드 분배
int GetScore(Card *hand);   // 점수 계산
int CompareScore(int player_score, int com_score);  // 점수 비교
void PlayGame();            // 게임 실행


/* 전역 변수 */
Card player[2];             // 플레이어 카드
Card com[2];                // 컴퓨터 카드
GameState state;            // 게임 상태
int server_sock, client_sock;   // 서버, 클라이언트 소켓 디스크립터

int main() {

    system("clear");
    struct sockaddr_in client_addr;
    socklen_t addr_len = sizeof(client_addr);
    SetServer();    // 서버 초기화

    // 반복 실행 서버
    while(1) {
        // system("clear");
        printf("\n========================================\n");
        printf("Ready to connect...\n");
        // 서버 - 클라이언트 연결
        client_sock = accept(server_sock, (struct sockaddr*)&client_addr, &addr_len);
        ErrorCheck(client_sock, "Accept");  // 에러 처리
        printf("* Connection Success *\n");
        // 클라이언트 IP, PORT 출력
        printf("Client IP: %s, PORT: %d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
        
        PlayGame();         // 게임 실행
        close(client_sock); // 클라이언트 소켓 닫기
        printf("\n* Disconnected *\n");
        printf("\n========================================\n");
    }

    close(server_sock);     // 서버 소켓 닫기
    return 0;
}

// 서버 설정
void SetServer() {
    struct sockaddr_in server_addr;

    server_sock = socket(AF_INET, SOCK_STREAM, 0);  // 소켓 생성
    ErrorCheck(server_sock, "Socket");  // 에러 처리

    memset((char *)&server_addr, '\0', sizeof(server_addr));    // 구조체 초기화
    server_addr.sin_family = AF_INET;   // IPv4
    server_addr.sin_port = htons(PORT); // 9000
    server_addr.sin_addr.s_addr = inet_addr(IPADDR);    // 127.0.0.1

    ErrorCheck(bind(server_sock, (struct sockaddr*)&server_addr, sizeof(server_addr)), "Bind"); // 소켓 바인딩
    ErrorCheck(listen(server_sock, 1), "Listen");   // 연결 대기상태로 전환
}

// 예산 수신
void ReceiveBudget() {
    ErrorCheck(recv(client_sock, &state.budget, sizeof(state.budget), 0), "Receive Budget");
    printf("\n");
    printf("****************************************\n");
    printf("        Player's budget is %d\n", state.budget);
    printf("****************************************\n");
}

// 재도전 여부 수신
int ReceiveRetry() {
    int retry;
    ErrorCheck(recv(client_sock, &retry, sizeof(retry), 0), "Receive Retry");

    return retry;
}

// 게임 결과 계산
void Result() {
    if (state.player_choice1 == DIE) {
        // 첫 번째 배팅에서 플레이어가 죽은 경우
        PrintResult(2); // 컴퓨터 승리 출력
        if (state.player_money < 300) { 
            // 플레이어 보유금액 300 미만인 경우
            state.computer_money += state.player_money;
            state.player_money = 0;
        } else {
            state.computer_money += state.player_bet1;
            state.player_money -= state.player_bet1;
        }
    } else {
        // 두 번째 배팅에서 플레이어가 죽은 경우
        if (state.player_choice2 == DIE) {
            state.computer_money += state.player_bet1;
            state.player_money -= state.player_bet1;
            PrintResult(2);
        } else {
            Ascending(player); // 카드 오름차순 정렬
            Ascending(com);

            // 결과 계산
            int player_score = GetScore(player);   // 플레이저 점수 계산
            int com_score = GetScore(com);         // 컴퓨터 점수 계산

            int player_bet = state.player_bet1 + state.player_bet2; // 배팅 총액

            switch (CompareScore(player_score, com_score)) {
                case 1:     // 플레이서 승리
                    state.player_money += player_bet;
                    state.computer_money -= player_bet;
                    PrintResult(1);
                    break;
                case 2:     // 플레이어 패배
                    state.computer_money += player_bet;
                    state.player_money -= player_bet;
                    PrintResult(2);
                    break;
                default:    // 무승부, 49파토, 멍텅구리구사
                    PrintResult(3);
                    break;
            }
        }
    }
}

// 결과 출력
void PrintResult(int num) {
    printf("\n");
    printf("========================================\n");
    switch (num) {
        case 1:
            printf("||            Player Win!             ||\n");
            break;
        case 2:
            printf("||              Com Win!              ||\n");
            break;
        case 3:
            printf("||               Draw!                ||\n");
            break;
        default:
            break;
    }
    printf("========================================\n");
}

// 카드 섞기
void Shuffle(Card *all) {
    for (int i = 0; i < 20; i++) {
        int rnd = rand() % 20;
        Card temp = all[i];
        all[i] = all[rnd];
        all[rnd] = temp;
    }
}

// 카드 초기화
Card *Init(void){
    static Card all[20];
    int card_num[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10,
                      1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    bool power[] = {false, false, false, false, false, false, false, false, false, false,
                    true, false, true, true, false, false, true, true, true, false};
    char* name[] = {"1", "2", "3", "4", "5", "6", "7", "8", "9", "10",
                    "1+", "2+", "3+", "4+", "5+", "6+", "7+", "8+", "9+", "10+"};

    for (int i = 0; i < 20; i++){
        all[i].num = card_num[i];
        all[i].special = power[i];
        strncpy(all[i].name, name[i], sizeof(all[i].name) - 1); // 문자열 복사
        all[i].name[sizeof(all[i].name) - 1] = '\0'; // NULL 문자 추가
    }
    return all;
}

// 카드 분배
void Divider(Card *all) {
    player[0] = all[0];
    player[1] = all[1];
    com[0] = all[2];
    com[1] = all[3];
}

// 손패 계산
int GetScore(Card *hand) {
    // 38광땡
    if (hand[0].num == 3 && hand[1].num == 8 && hand[0].special == true && hand[1].special == true) return 3800; // 가장 높은 점수
    // 13광땡, 18광땡
    if ((hand[0].num == 1 && hand[1].num == 3 && hand[0].special == true && hand[1].special == true) || (hand[0].num == 1 && hand[1].num == 8 && hand[0].special == true && hand[1].special == true)) 
        return 3000;
    // 땡
    if (hand[0].num == hand[1].num) return 1000 + hand[0].num; // 땡 점수 (1000 + 숫자)
    
    // 특수 족보 판별
    if (hand[0].num == 1 && hand[1].num == 2) return 806; // 알리
    if (hand[0].num == 1 && hand[1].num == 4) return 805; // 독사
    if (hand[0].num == 1 && hand[1].num == 9) return 804; // 구삥
    if (hand[0].num == 1 && hand[1].num == 10) return 803; // 장삥
    if (hand[0].num == 4 && hand[1].num == 10) return 802; // 장사
    if (hand[0].num == 4 && hand[1].num == 6) return 801; // 세륙

    // 특수 패
    if (hand[0].num == 3 && hand[1].num == 7 && hand[0].special == true && hand[1].special == true) return 10; // 땡잡이
    if (hand[0].num == 4 && hand[1].num == 7 && hand[0].special == true && hand[1].special == true) return 11; // 암행어사
    if (hand[0].num == 4 && hand[1].num == 9 && hand[0].special == true && hand[1].special == true) return 12; // 멍텅구리구사
    if (hand[0].num == 4 && hand[1].num == 9) return 13; // 49파토
    
    int sum = (hand[0].num + hand[1].num) % 10; // 끗 판별
    return sum; // 끗 점수 (0~9)
}

// 승패 계산
int CompareScore(int player_score, int com_score) {
    // 땡잡이
    if ((player_score == 10 && com_score > 1000 && com_score < 1010) || (com_score == 10 && player_score > 1000 && player_score < 1010)){
        if (player_score == 10) return 1;   // player win
        else return 2;   // com win
    } else {
        if (player_score == 10) player_score = 0;
        else if (com_score == 10) com_score = 0;
    }
    // 암행어사
    if ((player_score == 11 && com_score == 3000) || (com_score == 11 && player_score == 3000)){
        if (player_score == 11) return 1;
        else return 2;
    } else {
        if (player_score == 11) player_score = 1;
        else if (com_score == 11) com_score = 1;
    }
    // 49파토
    if ((player_score == 13 && com_score < 1000) || (com_score == 13 && player_score < 1000)) return 3;   // regame
    // 멍텅구리구사
    if ((player_score == 12 && com_score < 1010) || (com_score == 12 && player_score < 1010)) return 3;
    
    if (player_score > com_score) return 1;
    else if(player_score == com_score) return 3;
    else return 2;
}

// 게임 실행
void PlayGame() {
    Card *all;
    all = Init(); // 카드 초기화
    
    ReceiveBudget(); // 예산 수신
    state.player_money = state.budget;
    state.computer_money = state.budget;
    state.computer_choice = CALL;

    do {
        srand(time(NULL)); // 랜덤 시드 설정
        
        Shuffle(all); // 카드 섞기
        Divider(all); // 카드 분배
        state.player_bet1 = 300;    // 초기 배팅금액 설정

        // 데이터 구분해서 전송
        int header;
        header = sizeof(state);
        ErrorCheck(send(client_sock, &header, sizeof(header), 0), "Send Header");       // 헤더 전송
        ErrorCheck(send(client_sock, &state, sizeof(state), 0), "Send State");          // 상태 전송

        header = sizeof(player);
        ErrorCheck(send(client_sock, &header, sizeof(header), 0), "Send Header");       // 헤더 전송
        ErrorCheck(send(client_sock, &player, sizeof(player), 0), "Send Player Card");  // player 카드 전송

        header = sizeof(com);
        ErrorCheck(send(client_sock, &header, sizeof(header), 0), "Send Header");       // 헤더 전송
        ErrorCheck(send(client_sock, &com, sizeof(com), 0), "Send Computer Card");      // com 카드 전송

        // 클라이언트 행동 수신
        ErrorCheck(recv(client_sock, &state, sizeof(state), 0), "Receive State");
        
        Result(); // 결과 계산 및 출력
        ErrorCheck(send(client_sock, &state, sizeof(state), 0), "Send Result");         // 결과 전송
    } while (ReceiveRetry());   // 재도전 여부 수신
}