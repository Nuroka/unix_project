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
    int player_bet1;
    int player_bet2;
    int player_choice1;
    int player_choice2;
    int computer_choice;
    int player_money;
    int computer_money;
} GameState;

typedef struct {
    int num;
    bool special;
    char name[15];
} Card;

#endif // GAME_PROTOCOL_H
