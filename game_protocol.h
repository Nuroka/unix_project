#ifndef GAME_PROTOCOL_H
#define GAME_PROTOCOL_H

#define PORT 8080
#define BUFFER_SIZE 1024

// 메시지 타입 정의
#define CALL 1
#define DIE 2
#define RESULT 3

// 데이터 구조 정의
typedef struct {
    int message_type;
    int player_bet;
    int player_choice;
    int computer_choice;
    int player_money;
    int computer_money;
    int player_card1;
    int player_card2;
    int computer_card1;
    int computer_card2;
} GameState;

#endif // GAME_PROTOCOL_H
