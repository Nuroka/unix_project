#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "game_protocol.h"

void play_game(int sock) {
    GameState state;

    while (1) {
        // 서버로부터 상태 수신
        recv(sock, &state, sizeof(state), 0);

        // 현재 상태 출력
        printf("\nYour cards: %d, %d\n", state.player_card1, state.player_card2);
        printf("Your money: %d, Computer money: %d\n", state.player_money, state.computer_money);

        // 행동 선택
        printf("Enter choice (1 for CALL, 2 for DIE): ");
        scanf("%d", &state.player_choice);

        if (state.player_choice == CALL) {
            printf("Enter additional bet: ");
            scanf("%d", &state.player_bet);
        }

        // 서버로 상태 전송
        send(sock, &state, sizeof(state), 0);

        // 서버로부터 결과 수신
        recv(sock, &state, sizeof(state), 0);

        if (state.message_type == RESULT) {
            printf("Round result: Your money: %d, Computer money: %d\n",
                   state.player_money, state.computer_money);
        }

        if (state.player_money <= 0 || state.computer_money <= 0) {
            printf("Game over.\n");
            break;
        }
    }
}

int main() {
    int sock;
    struct sockaddr_in server_addr;

    // 소켓 생성
    sock = socket(AF_INET, SOCK_STREAM, 0);
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr));
    printf("Connected to server.\n");

    play_game(sock);
    close(sock);
    return 0;
}
