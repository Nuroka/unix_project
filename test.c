#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>

struct card{
    int num;
    bool special;
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
    for (int i = 0; i < 20; i++){
        all[i].num = card_num[i];
        all[i].special = power[i];
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
    /*
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
    */
}

int main(void) {
    srand(time(NULL));  // 난수 생성
    struct card *all = init();
    divider(all, player, com);
    printf("Player's Cards: %d (Special: %d), %d (Special: %d)\n",
           player[0].num, player[0].special, player[1].num, player[1].special);
    printf("Com's Cards: %d (Special: %d), %d (Special: %d)\n",
           com[0].num, com[0].special, com[1].num, com[1].special);
    return 0;
}