#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <stdbool.h>
#include "game_protocol.h"

Card player[2];
Card com[2];

void play_game(int sock) {
    GameState state;

    while (1) {
        // 서버로부터 상태 수신
        /*recv(sock, &state, sizeof(state), 0);
        recv(sock, &player, sizeof(player), 0);*/
        int header;
        recv(sock, &header, sizeof(header), 0);       // 헤더 수신
        recv(sock, &state, header, 0);                // 상태 수신

        recv(sock, &header, sizeof(header), 0);       // 헤더 수신
        recv(sock, &player, header, 0);               // 카드 수신

        // 첫 번째 카드, 남은 돈 출력
        printf("\nYour first cards: %s\n", player[0].name);
        printf("Your money: %d, Computer money: %d\n", state.player_money, state.computer_money);

        // 첫 번째 행동 선택
        printf("Enter choice (1 for CALL, 2 for DIE): ");
        scanf("%d", &state.player_choice1);

        // CALL
        if (state.player_choice1 == CALL) {
            printf("Enter additional bet: ");
            // 배팅금액 예외처리 / 배팅금액이 남은 돈을 초과할 때
            do {
                scanf("%d", &state.player_bet1);
                if ((state.player_bet1 > state.computer_money) || 
                    (state.player_bet1 > state.player_money)) {
                    printf("Bet exceeds available money. Enter again: ");
                }
            } while ((state.player_bet1 > state.computer_money) ||
                    (state.player_bet1 > state.player_money));

            // 손패 전체 출력
            printf("Your second card: %s\n", player[1].name);
            printf("Cards: %s %s\n", player[0].name, player[1].name);

            // 두 번째 행동 선택
            printf("Enter choice (1 for CALL, 2 for DIE): ");
            scanf("%d", &state.player_choice2);

            // CALL
            if (state.player_choice2 == CALL) {
                printf("Enter additional bet: ");
                // 배팅금액 예외처리 / 배팅금액이 남은 돈을 초과할 때
                do {
                    scanf("%d", &state.player_bet2);
                    if ((state.player_bet1 + state.player_bet2 > state.computer_money) || 
                        (state.player_bet1 + state.player_bet2 > state.player_money)) {
                    printf("Total bet exceeds available money. Enter again: ");
                    }
                } while ((state.player_bet1 + state.player_bet2 > state.computer_money) || 
                        (state.player_bet1 + state.player_bet2 > state.player_money));
            }
        }

        // 서버로 상태 전송
        send(sock, &state, sizeof(state), 0);

        // 서버로부터 결과 수신
        recv(sock, &state, sizeof(state), 0);

        if (state.message_type == RESULT) {
            printf("Round result: Your money: %d, Computer money: %d\n",
                   state.player_money, state.computer_money);
            printf("===========================================================\n");
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