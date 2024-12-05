#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <time.h>
#include <stdbool.h>
#include "game_protocol.h"

void shuffle(Card *all);
Card *init(void);
void divider(Card *all, Card *player, Card *com);
int get_score(Card *hand);
int cmp(int player_score, int com_score);
void handle_client(int client_sock);

Card player[2];
Card com[2];

// 카드 섞기
void shuffle(Card *all) {
    for (int i = 0; i < 20; i++) {
        int rnd = rand() % 20;
        Card temp = all[i];
        all[i] = all[rnd];
        all[rnd] = temp;
    }
}

// 카드 초기화
Card *init(void){
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
        all[i].name[sizeof(all[i].name) - 1] = '\0'; // NULL 문자 보장
    }
    shuffle(all);
    return all;
}

// 카드 분배
void divider(Card *all, Card *player, Card *com) {
    player[0] = all[0];
    player[1] = all[1];
    com[0] = all[2];
    com[1] = all[3];
}

// 손패 계산
int get_score(Card *hand) {
    
    // 38광땡
    if (hand[0].num == 3 && hand[1].num == 8 && hand[0].special == true && hand[1].special == true) {
        return 3800; // 가장 높은 점수
    }

    // 13광땡, 18광땡
    if ((hand[0].num == 1 && hand[1].num == 3 && hand[0].special == true && hand[1].special == true) || 
        (hand[0].num == 1 && hand[1].num == 8 && hand[0].special == true && hand[1].special == true)) {
        return 3000;
    }

    // 땡
    if (hand[0].num == hand[1].num) {
        return 1000 + hand[0].num; // 땡 점수 (1000 + 숫자)
    }

    // 특수 족보 판별
    if (hand[0].num == 1 && hand[1].num == 2) return 806; // 알리
    if (hand[0].num == 1 && hand[1].num == 4) return 805; // 독사
    if (hand[0].num == 1 && hand[1].num == 9) return 804; // 구삥
    if (hand[0].num == 1 && hand[1].num == 10) return 803; // 장삥
    if (hand[0].num == 4 && hand[1].num == 10) return 802; // 장사
    if (hand[0].num == 4 && hand[1].num == 6) return 801; // 세륙

    // 특수 패
    if (hand[0].num == 3 && hand[1].num == 7 && hand[0].special == true && hand[1].special == true) {
        return 10; // 땡잡이
    }
    if (hand[0].num == 4 && hand[1].num == 7 && hand[0].special == true && hand[1].special == true) {
        return 11; // 암행어사
    }
    if (hand[0].num == 4 && hand[1].num == 9 && hand[0].special == true && hand[1].special == true) {
        return 12; // 멍텅구리구사
    }
    if (hand[0].num == 4 && hand[1].num == 9) {
        return 13; // 49파토
    }
    
    // 끗 판별
    int sum = (hand[0].num + hand[1].num) % 10;
    return sum; // 끗 점수 (0~9)
}

// 승패 계산
int cmp(int player_score, int com_score) {
    // 땡잡이
    if ((player_score == 10 && com_score > 1000 && com_score < 1010) || (com_score == 10 && player_score > 1000 && player_score < 1010)){
        if (player_score == 10) 
            return 1;   // player win
        else 
            return 2;   // com win
    }
    else {
        if (player_score == 10) 
            player_score = 0;
        else if (com_score == 10)
            com_score = 0;
    }
    // 암행어사
    if ((player_score == 11 && com_score == 3000) || (com_score == 11 && player_score == 3000)){
        if (player_score == 11) 
            return 1;
        else 
            return 2;
    }
    else {
        if (player_score == 11) 
            player_score = 1;
        else if (com_score == 11)
            com_score = 1;
    }
    // 49파토
    if ((player_score == 13 && com_score < 1000) || (com_score == 13 && player_score < 1000))
        return 3;   // regame
    // 멍텅구리구사
    if ((player_score == 12 && com_score < 1010) || (com_score == 12 && player_score < 1010)){
        return 3;
    }
    if (player_score > com_score) 
        return 1;
    else if(player_score == com_score)
        return 3;
    else
        return 2;
}

void handle_client(int client_sock) {
    GameState state;
    Card *all;

    // 초기화
    state.player_money = 10000;
    state.computer_money = 10000;

    while (state.player_money > 0 && state.computer_money > 0) {
        srand(time(NULL)); // 랜덤 시드 설정
        // 카드 분배
        all = init();
        divider(all, player, com);

        /* test
        player[0].num = 1;
        player[0].special = false;
        strncpy(player[0].name, "1", sizeof(player[0].name) - 1);
        player[0].name[sizeof(player[0].name) - 1] = '\0';

        player[1].num = 1;
        player[1].special = true;
        strncpy(player[1].name, "1+", sizeof(player[1].name) - 1);
        player[1].name[sizeof(player[1].name) - 1] = '\0';

        com[0].num = 3;
        com[0].special = true;
        strncpy(com[0].name, "3+", sizeof(com[0].name) - 1);
        com[0].name[sizeof(com[0].name) - 1] = '\0';

        com[1].num = 7;
        com[1].special = true;
        strncpy(com[1].name, "7+", sizeof(com[1].name) - 1);
        com[1].name[sizeof(com[1].name) - 1] = '\0';
        */

        // 초기 배팅금액 설정
        state.player_bet1 = 300;
        state.computer_choice = CALL;

        // 상태 전송
        int header;
        header = sizeof(state);
        send(client_sock, &header, sizeof(header), 0); // 헤더 전송
        send(client_sock, &state, sizeof(state), 0);   // 상태 전송

        header = sizeof(player);
        send(client_sock, &header, sizeof(header), 0); // 헤더 전송
        send(client_sock, &player, sizeof(player), 0); // player 카드 전송

        header = sizeof(com);
        send(client_sock, &header, sizeof(header), 0); // 헤더 전송
        send(client_sock, &com, sizeof(com), 0);       // com 카드 전송

        // 클라이언트 행동 수신
        recv(client_sock, &state, sizeof(state), 0);

        if (state.player_choice1 == DIE) {
            // 첫 번째 배팅 / 플레이어 die, 컴퓨터 승리
            state.computer_money += state.player_bet1;
            state.player_money -= state.player_bet1;
        } else {
            // 두 번째 배팅 / 플레이어 die, 컴퓨터 승리
            if (state.player_choice2 == DIE) {
                state.computer_money += state.player_bet1;
                state.player_money -= state.player_bet1;
            } else {
                ascending(player); // 카드 정렬
                ascending(com);

                // 결과 계산
                int player_score = get_score(player);   // 플레이저 점수
                int com_score = get_score(com);         // 컴퓨터 점수

                int player_bet = state.player_bet1 + state.player_bet2; // 배팅 총 액수

                switch (cmp(player_score, com_score)) {
                    case 1:     // 플레이서 승리
                        state.player_money += player_bet;
                        state.computer_money -= player_bet;
                        break;
                    case 2:     // 플레이어 패배
                        state.computer_money += player_bet;
                        state.player_money -= player_bet;
                        break;
                    default:    // 무승부, 49파토, 멍텅구리구사
                        break;
                }
            }
        }

        // 상태 전송
        state.message_type = RESULT;
        send(client_sock, &state, sizeof(state), 0);
    }

    close(client_sock);
}

int main() {
    int server_sock, client_sock;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_len = sizeof(client_addr);

    // 소켓 생성
    server_sock = socket(AF_INET, SOCK_STREAM, 0);
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    bind(server_sock, (struct sockaddr*)&server_addr, sizeof(server_addr));
    listen(server_sock, 1);

    printf("Waiting for client...\n");
    client_sock = accept(server_sock, (struct sockaddr*)&client_addr, &addr_len);
    printf("Client connected.\n");

    handle_client(client_sock);
    close(server_sock);
    return 0;
}