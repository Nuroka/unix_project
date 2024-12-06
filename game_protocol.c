// game_protocol.c
#include "game_protocol.h"
#include <stdio.h>
#include <stdlib.h>

// 카드 오름차순 정렬
void Ascending(Card *cards) {
    if (cards[0].num > cards[1].num) {
        Card temp = cards[0];
        cards[0] = cards[1];
        cards[1] = temp;
    }
}

// 에러 체크
void ErrorCheck(int num, char* str) {
    if (num == -1) {
        perror(str);
        exit(1);
    }
}