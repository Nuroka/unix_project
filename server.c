#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <time.h>
#include "game_protocol.h"

void handle_client(int client_sock) {
    GameState state;
    srand(time(NULL)); // 랜덤 시드 설정

    // 초기화
    state.player_money = 10000;
    state.computer_money = 10000;

    while (state.player_money > 0 && state.computer_money > 0) {
        // 카드 분배
        state.computer_card1 = rand() % 10 + 1;
        state.computer_card2 = rand() % 10 + 1;
        state.player_card1 = rand() % 10 + 1;
        state.player_card2 = rand() % 10 + 1;

        // 초기 배팅금액 설정
        state.player_bet = 300;
        state.computer_choice = CALL;

        // 상태 전송
        send(client_sock, &state, sizeof(state), 0);

        // 클라이언트 행동 수신
        recv(client_sock, &state, sizeof(state), 0);

        if (state.player_choice == DIE) {
            // 플레이어 die, 컴퓨터 승리
            state.computer_money += state.player_bet;
            state.player_money -= state.player_bet;
        } else {
            // 결과 계산
            int player_score = (state.player_card1 + state.player_card2) % 10;
            int computer_score = (state.computer_card1 + state.computer_card2) % 10;

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
