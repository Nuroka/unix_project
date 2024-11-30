#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <time.h>
#include <stdbool.h>
#include "game_protocol.h"

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
    
    // 카드 내림차순 재구성
    if (player[0].num > player[1].num) {
        Card temp;
        temp = player[0];
        player[0] = player[1];
        player[1] = temp;
    }
    if (com[0].num > com[1].num) {
        Card temp;
        temp = com[0];
        com[0] = com[1];
        com[1] = temp;
    }
    
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

    // 끗 판별
    int sum = (hand[0].num + hand[1].num) % 10;
    return sum; // 끗 점수 (0~9)
}

void handle_client(int client_sock) {
    GameState state;
    srand(time(NULL)); // 랜덤 시드 설정
    Card *all;

    // 초기화
    state.player_money = 10000;
    state.computer_money = 10000;

    while (state.player_money > 0 && state.computer_money > 0) {
        // 카드 분배
        /*state.computer_card1 = rand() % 10 + 1;
        state.computer_card2 = rand() % 10 + 1;
        state.player_card1 = rand() % 10 + 1;
        state.player_card2 = rand() % 10 + 1;*/

        all = init();
        divider(all, player, com);

        // 초기 배팅금액 설정
        state.player_bet = 300;
        state.computer_choice = CALL;

        // 상태 전송
        /*send(client_sock, &state, sizeof(state), 0);
        send(client_sock, &player, sizeof(player), 0);*/
        int header;
        header = sizeof(state);
        send(client_sock, &header, sizeof(header), 0); // 헤더 전송
        send(client_sock, &state, sizeof(state), 0);  // 상태 전송

        header = sizeof(player);
        send(client_sock, &header, sizeof(header), 0); // 헤더 전송
        send(client_sock, &player, sizeof(player), 0); // 카드 전송

        // 클라이언트 행동 수신
        recv(client_sock, &state, sizeof(state), 0);

        if (state.player_choice == DIE) {
            // 플레이어 die, 컴퓨터 승리
            state.computer_money += state.player_bet;
            state.player_money -= state.player_bet;
        } else {
            // 결과 계산
            int player_score = get_score(player);
            int computer_score = get_score(com);

            if (player_score > computer_score) {
                state.player_money += state.player_bet;
                state.computer_money -= state.player_bet;
            } else {
                state.computer_money += state.player_bet;
                state.player_money -= state.player_bet;
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