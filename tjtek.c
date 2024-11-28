#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main() {
    int pae1[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    int pae2[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10}; // 1, 3, 8 광
    int P_bet = 300, C_bet = 300;                  // 초기 배팅금액
    int P_money = 10000, C_money = 10000;          // 보유 금액
    int choice = 0, ai = 0;                        // player computer 선택
    int ddang = 0, ggut = 0;                       // 컴퓨터 끗과 땡
    int dang = 0, gut = 0;                         // 플레이어 끗과 땡
    int P_bnew = 0, C_bnew = 0;                    // 배팅 금액
    int i = 0, a = 0, c = 0;
    int j = 0, b = 0;
    int prob = 0;                                  // 확률 변수

    while (1) {
        // 매 판 시작할 때 소모되는 기본 배팅금액
        C_bet = 300;
        P_bet = 300;

        // 각 변수 정의
        srand(time(NULL));
        i = rand() % 10;
        j = rand() % 10;
        a = rand() % 10;
        b = rand() % 10;
        prob = rand() % 100 + 1;

        printf("\n-------------------------------------\n");
        printf("기본 배팅금액 >> 300\n");
        printf("Player card 1 : %d\n", pae1[i]);
        printf("Player card 2 : %d\n", pae2[j]);

        // player card
        if (pae1[i] == pae2[j]) {
            dang = pae1[i] + 10;
            printf("%d땡\n", dang - 10);
            gut = dang;
        } else {
            gut = (pae1[i] + pae2[j]) % 10;
            printf("%d끗\n", gut);
        }

        // computer card
        if (pae1[a] == pae2[b]) {
            ddang = pae1[a] + 10;
            ggut = ddang;
        } else {
            ggut = (pae1[a] + pae2[b]) % 10;
        }

        // call die
        for (c = 0; c < 1; ++c) {
        INPUT:
            printf("call[1] die[2] : ");
            scanf("%d", &choice);

            // my turn
            if (choice == 1) {
                printf("bet: ");
                scanf("%d", &P_bnew);
                P_bet += P_bnew;
            } else if (choice == 2) {
                break;
            } else {
                printf("1 or 2 중 선택\n");
                goto INPUT;
            }

            // computer turn
            if (ddang > 0) {
                if (prob < 95) {
                    ai = 1;
                    C_bnew = 700;
                    C_bet += C_bnew;
                } else {
                    ai = 2;
                    break;
                }
            } else if (ggut && ddang == 0) {
                if (ggut == 0) {
                    ai = 2;
                    break;
                } else if (ggut > 8) {
                    if (prob < 80) {
                        ai = 1;
                        C_bnew = 500;
                        C_bet += C_bnew;
                    } else {
                        ai = 2;
                        break;
                    }
                } else {
                    if (prob < 40) {
                        ai = 1;
                        C_bnew = 300;
                        C_bet += C_bnew;
                    } else {
                        ai = 2;
                        break;
                    }
                }
            }
        }

        // result
        if (choice == 2 && ai == 1) { // computer win
            C_money += P_bet;
            P_money -= P_bet;
            printf("Player Die\n");
            printf("Computer Win\n");
            printf("Computer card 1 : %d\n", pae1[a]);
            printf("Computer card 2 : %d\n", pae2[b]);
            printf("P_money : %d\n", P_money);
            printf("C_money : %d\n", C_money);
        } else if (choice == 1 && ai == 2) { // player win
            C_money -= C_bet;
            P_money += C_bet;
            printf("Computer Die\n");
            printf("Player Win\n");
            printf("Computer card 1 : %d\n", pae1[a]);
            printf("Computer card 2 : %d\n", pae2[b]);
            printf("P_money : %d\n", P_money);
            printf("C_money : %d\n", C_money);
        } else if (choice == 2 && ai == 2) { // draw
            printf("Player Die\n");
            printf("Computer Die\n");
            printf("Draw\n");
        } else { // compare (둘 다 call 한 경우)
            if (dang > ddang || gut > ggut || dang > ggut) { // player win
                C_money -= C_bet;
                P_money += C_bet;
                printf("Computer Call\n");
                printf("Computer Bet: %d\n", C_bnew);
                printf("Player Win\n");
                printf("Computer card 1 : %d\n", pae1[a]);
                printf("Computer card 2 : %d\n", pae2[b]);
                printf("P_money : %d\n", P_money);
                printf("C_money : %d\n", C_money);
            } else if (ddang > dang || ggut > gut || ddang > gut) { // computer win
                C_money += P_bet;
                P_money -= P_bet;
                printf("Computer Call\n");
                printf("Computer Bet: %d\n", C_bnew);
                printf("Computer Win\n");
                printf("Computer card 1 : %d\n", pae1[a]);
                printf("Computer card 2 : %d\n", pae2[b]);
                printf("P_money : %d\n", P_money);
                printf("C_money : %d\n", C_money);
            }
        }

        printf("\n1을 입력하여 다시 시작(종료하길 원하면 0을 입력): ");
        int x;
        scanf("%d", &x);
        if (x == 0)
            break;
    }

    return 0;
}
