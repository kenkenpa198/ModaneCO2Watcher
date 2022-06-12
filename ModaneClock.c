#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <unistd.h>

typedef char String[1024];

int main(void) {

    // もだねちゃんの顔を各変数へ格納
    String modaneTop =     "                          \n"
                           "                ____      \n"
                           "              /      \\   \n"
                           "             ( ____   ﾚ-、\n"
                           "           ／       -ノ_ﾗ \n"
                           "          \"  人      ヽ  \n"
                           "        /  ／_ `-_     \\ \n"
                           "       |  /        `-_  | ";

    String modaneEye01 =   "       (||   |    |   ||) ";
    String modaneEye02 =   "       (|| -==    ==- ||) ";

    String modaneMouth01 = "        \\) ,,  ー  ,, (ﾉ  ";
    String modaneMouth02 = "        \\) ,,  ワ  ,, (ﾉ  ";

    String modaneJaw =     "         )ヽ________ノ(   ";
    String modaneBottom =  "             \\_父_/\n    ";


    // 時計の表示処理
    srand((unsigned)time(NULL));
    char date[64], dow[64], time_[64];
    time_t prev, now;

    // 無限に繰り返し
    while(true) {

        // 現在日時と繰り返し前の値を比較して同じなら処理をコンティニューする
        now = time(NULL);
        if (prev == now) {
            continue;
        }
        prev = now; // 現在日時を次の比較用に格納

        // 日付と曜日の取得
        strftime(date, sizeof(date), "%Y-%m-%d %a.", localtime(&now));
        strftime(dow, sizeof(dow), "%A", localtime(&now));

        // 時間を取得。偶奇でコロンの有無を場合分け
        if (now % 2 == 0) {
            strftime(time_, sizeof(time_), "%H:%M", localtime(&now));
        } else {
            strftime(time_, sizeof(time_), "%H %M", localtime(&now));
        }

        // シェルの表示を消去する
        // TODO: Windows でも動かせるようにする？
        system("clear");

        // もだねちゃんと日時を表示
        // TODO: 関数化して一気に表示するようにする

        // 乱数生成
        int eyeNum = rand()%2;
        int mouthNum = rand()%2;

        // 頭の表示
        printf("%s\n", modaneTop);

        // 目と日付の表示
        switch (eyeNum) {
            case 0:
                printf("%s   %s\n",              modaneEye01, date);
                break;
            case 1:
                printf("%s   %s\n",              modaneEye02, date);
                break;
        }

        // 口と区切り線の表示
        switch (mouthNum) {
            case 0:
                printf("%s   ---------------\n", modaneMouth01);
                break;
            case 1:
                printf("%s   ---------------\n", modaneMouth02);
                break;
        }

        // アゴと時間の表示
        printf("%s        %s\n",         modaneJaw, time_);

        // 首下の表示
        printf("%s\n",                   modaneBottom);
    }

    return 0;
}
