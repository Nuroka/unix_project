#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>

struct card{
    int num;
    bool special;
    char name[15];
};

struct card player[2];
struct card com[2];

// 카드 섞기
void shuffle(struct card *all) {
    for (int i = 0; i < 20; i++) {
        int rnd = rand() % 20;
        struct card temp = all[i];
        all[i] = all[rnd];
        all[rnd] = temp;
    }
}

// 카드 초기화
struct card *init(void){
    static struct card all[20];
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
        all[i].name[sizeof(all[i].name) - 1] = '\0';
    }
    shuffle(all);
    return all;
}

// 카드 분배
void divider(struct card *all, struct card *player, struct card *com) {
    player[0] = all[0];
    player[1] = all[1];
    com[0] = all[2];
    com[1] = all[3];
    
    // 카드 내림차순 재구성
    if (player[0].num > player[1].num) {
        struct card temp;
        temp = player[0];
        player[0] = player[1];
        player[1] = temp;
    }
    if (com[0].num > com[1].num) {
        struct card temp;
        temp = com[0];
        com[0] = com[1];
        com[1] = temp;
    }
    
}

// 손패 계산
int get_score(struct card *hand) {
    
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

int main(void) {
    srand(time(NULL));  // 난수 생성
    struct card *all;
    all = init();
    divider(all, player, com);
    printf("Player's Cards: %d (Special: %d), %d (Special: %d)\n",
           player[0].num, player[0].special, player[1].num, player[1].special);
    printf("Com's Cards: %d (Special: %d), %d (Special: %d)\n",
           com[0].num, com[0].special, com[1].num, com[1].special);
    printf("%d, %d\n", get_score(player), get_score(com));
    printf("%s\n", player[0].name);

    return 0;
}