#ifndef GAME_PROTOCOL_H
#define GAME_PROTOCOL_H

#include <stdbool.h>
#define PORT 9000
#define IPADDR "127.0.0.1"

// 메시지 타입 정의
#define CALL 1
#define DIE 2

// 데이터 구조 정의
typedef struct {
    int budget;             // 설정 예산
    int player_bet1;        // 첫 번째 배팅 금액
    int player_bet2;        // 두 번째 배팅 금액
    int player_choice1;     // 첫 번째 CALL/DIE
    int player_choice2;     // 두 번째 CALL/DIE
    int computer_choice;    // Always CALL
    int player_money;       // 플레이어 남은 예산
    int computer_money;     // 컴퓨터 남은 예산
} GameState;    // 게임 상태 구조체

typedef struct {
    int num;                // 카드 번호(월)
    bool special;           // 특수 카드 판별(eg. 1월 광, 7월 동물 등..)
    char name[15];          // 카드 이름
} Card;         // 카드 구조체

// 사용자 함수 정의
void Ascending(Card *cards);            // 카드 오름차순 정렬
void ErrorCheck(int num, char* str);    // 에러 처리 함수

#endif // GAME_PROTOCOL_H