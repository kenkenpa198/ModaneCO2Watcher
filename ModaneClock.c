#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <unistd.h>

typedef char String[1024];

int main(void) {

    // もだねちゃんの顔を各変数へ格納
    // エスケープ文字列を一部使っているのでズレに注意
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
    String modaneEye03 =   "       (||  ★     ★   ||) "; // ★を半角で表示するフォントの場合。全角の場合は調整必要

    String modaneMouth01 = "        \\) ,,  ー  ,, (ﾉ  ";
    String modaneMouth02 = "        \\) ,,  ワ  ,, (ﾉ  ";

    String modaneJaw =     "         )ヽ________ノ(   ";
    String modaneBottom =  "             \\_父_/\n    ";


    // 時計の表示処理
    srand((unsigned)time(NULL));
    char date[64], dow[64], time_[64];
    time_t prev, now;

    // 乱数用変数の初期化
    int secCount = 0; // 秒数カウント用
    int eyeNum   = 0;
    int mouthNum = 0;

    // メインのループ処理
    while(1) {

        // 現在日時と繰り返し前の値を比較して同じなら処理をコンティニューする
        now = time(NULL);
        if (prev == now) {
            usleep(2000); // 同じ場合は指定マイクロ秒スリープして次の処理へ
            continue;
        }
        prev = now; // 現在日時を次のループの比較用に格納

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
        // TODO: Windows でも動かせるようにできる？
        system("clear");

        // 確認用
        printf("secCount : %d\n", secCount);
        printf("eyeNum   : %d\n", eyeNum);
        printf("mouthNum : %d\n", mouthNum);

        // もだねちゃんと日時を表示
        // TODO: 関数化して一気に表示するようにする

        // 頭の表示
        printf("%s\n", modaneTop);

        // 目と日付の表示
        switch (eyeNum) {
            case 0:
            case 1:
            case 2:
            case 3:
            case 4:
                printf("%s   %s\n",              modaneEye01, date);
                break;
            case 5:
            case 6:
            case 7:
            case 8:
                printf("%s   %s\n",              modaneEye02, date);
                break;
            case 9:
                printf("%s   %s\n",              modaneEye03, date);
                break;
        }

        // 口と区切り線の表示
        switch (mouthNum) {
            case 0:
            case 1:
            case 2:
                printf("%s   ---------------\n", modaneMouth01);
                break;
            case 3:
            case 4:
                printf("%s   ---------------\n", modaneMouth02);
                break;
        }

        // アゴと時間の表示
        printf("%s        %s\n",         modaneJaw, time_);

        // 首下の表示
        printf("%s\n",                   modaneBottom);

        // 4秒経ったら次のループ用の乱数を生成
        if (secCount == 3) {
            secCount = 0;
            eyeNum   = rand()%10;
            mouthNum = rand()%5;
        } else {
            secCount++;
        }
    }

    return 0;
}
