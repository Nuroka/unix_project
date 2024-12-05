// game_protocol.c
#include "game_protocol.h"
#include <stdio.h>
#include <stdlib.h>

void Ascending(Card *cards) {
    if (cards[0].num > cards[1].num) {
        Card temp = cards[0];
        cards[0] = cards[1];
        cards[1] = temp;
    }
}

void ErrorCheck(int num, char* str) {
    if (num == -1) {
        perror(str);
        exit(1);
    }
}