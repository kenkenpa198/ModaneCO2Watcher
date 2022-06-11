#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <unistd.h>

typedef char String[1024];

int main(void) {

    srand((unsigned)time(NULL));

    String modaneTop =    "                          \n"
                          "                ____      \n"
                          "              /      \\   \n"
                          "             ( ____   ﾄ-、\n"
                          "           ／       -ノ_ﾗ \n"
                          "          \"  人      ヽ  \n"
                          "        /  ／_ `-_     \\ \n"
                          "       |  /        `-_  | ";
                                  //   目と口は
                                  // 乱数で出し分け
    String modaneJaw =    "         )ヽ________ノ(   ";
    String modaneBottom = "             \\_父_/\n    ";

    while(true) {

        char date[64], dow[64], time_[64];
        time_t prev, now;

        now = time(NULL);

        if (prev == now) {
            continue;
        }
        prev = now;

        strftime(date, sizeof(date), "%Y-%m-%d %a.", localtime(&now));
        strftime(dow, sizeof(dow), "%A", localtime(&now));

        if (now % 2 == 0) {
            strftime(time_, sizeof(time_), "%H:%M", localtime(&now));
        } else {
            strftime(time_, sizeof(time_), "%H %M", localtime(&now));
        }

        system("clear");
        printf("%s\n", modaneTop);

        int eyeNum = rand()%11;
        if (eyeNum >= 5) {
            String modaneEye   =    "       (||   |    |   ||) ";
            printf("%s   %s\n", modaneEye, date);
        } else {
            String modaneEye   =    "       (|| -==    ==- ||) ";
            printf("%s   %s\n", modaneEye, date);
        }

        int mouthNum = rand()%11;
        if (mouthNum >= 5) {
            String modaneMouth =    "        \\) ,,  ー  ,, (ﾉ  ";
            printf("%s   ---------------\n", modaneMouth);
        } else {
            String modaneMouth =    "        \\) ,,  ワ  ,, (ﾉ  ";
            printf("%s   ---------------\n", modaneMouth);
        }

        printf("%s        %s\n", modaneJaw, time_);
        printf("%s\n", modaneBottom);        sleep(1);
    }
    return 0;
}
