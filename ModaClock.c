#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <time.h>
#include <unistd.h>

#include <ncurses.h> // curses ライブラリ
#include <locale.h>  // curses で2バイト文字を扱えるようにする

typedef char String[1024];

int main(void) {

    ///// もだねちゃんの顔を各変数へ格納 /////

    // エスケープ文字列を一部使っているのでズレに注意
    String modaneTop01 =      "         ____      ";
    String modaneTop02 =      "       /      \\   ";
    String modaneTop03 =      "      ( ____   ﾚ-、";
    String modaneTop04 =      "    ／       -ノ_ﾗ ";
    String modaneTop05 =      "   \"  人      ヽ  ";
    String modaneTop06 =      " /  ／_ `-_     \\ ";
    String modaneTop07 =      "|  /        `-_  | ";

    String modaneEyeOpen =    "(||   |    |   ||) ";
    String modaneEyeClose =   "(|| -==    ==- ||) ";
    String modaneEyeStar =    "(||  ★     ★   ||) "; // ★を半角で表示するフォントの場合。全角の場合は調整必要

    String modaneMouthClose = " \\) ,,  ー  ,, (ﾉ  ";
    String modaneMouthOpen =  " \\) ,,  ワ  ,, (ﾉ  ";

    String modaneBottom01 =   "  )ヽ________ノ(   ";
    String modaneBottom02 =   "      \\_父_/      ";

    // 乱数シード生成
    srand((unsigned)time(NULL));


    ///// 日時の処理 /////

    // 時計の表示処理
    char date[64], dow[64], time_[64];
    time_t prev, now;

    now = time(NULL);
    if (prev == now) {
        usleep(2000); // 同じ場合は指定マイクロ秒スリープして次の処理へ
        // continue;
    }
    prev = now; // 現在日時を次のループの比較用に格納

    // 日付と曜日の取得
    strftime(date, sizeof(date), "%Y-%m-%d", localtime(&now));
    strftime(dow, sizeof(dow), "%A", localtime(&now));

    // 時間を取得。偶奇でコロンの有無を場合分け
    if (now % 2 == 0) {
        strftime(time_, sizeof(time_), "%H:%M", localtime(&now));
    } else {
        strftime(time_, sizeof(time_), "%H %M", localtime(&now));
    }

    ///// ラッキーアイテム /////
    // TODO: ランダム出し分け
    String luckyItem = "Lucky item : Cat";


    ///// wttr /////
    // TODO: curl で取得
    String wttr01 = "     \\  /       Partly cloudy";
    String wttr02 = "   _ /\"\".-.     +25(27) °C     ";
    String wttr03 = "     \\_(   ).   ↓ 9 km/h       ";
    String wttr04 = "     /(___(__)  10 km          ";
    String wttr05 = "                0.0 mm         ";



    ///// curses で表示操作 : 準備 /////

    int x, y, w, h;         // move() などで指定する座標用変数の宣言
    setlocale(LC_ALL, "");  // 2バイト文字を扱えるようにする
    initscr();              // cursesを初期化。端末制御の開始
    getmaxyx(stdscr, h, w); // 画面の高さ（行数）と幅（桁数）を取得して h, w に格納する。stdscr は端末画面を表す定数。


    ///// curses で表示操作 : もだねちゃん /////

    y = h/2;
    x = 2;

    // 頭の表示
    mvaddstr(y - 5, x, modaneTop01);
    mvaddstr(y - 4, x, modaneTop02);
    mvaddstr(y - 3, x, modaneTop03);
    mvaddstr(y - 2, x, modaneTop04);
    mvaddstr(y - 1, x, modaneTop05);
    mvaddstr(y    , x, modaneTop06);
    mvaddstr(y + 1, x, modaneTop07);

    // 目の表示
    int eyeNum   = rand()%9 + 1;
    if (eyeNum <= 6) {
        mvaddstr(y + 2, x, modaneEyeOpen);
    } else if (eyeNum <= 9) {
        mvaddstr(y + 2, x, modaneEyeClose);
    } else {
        mvaddstr(y + 2, x, modaneEyeStar);
    }

    // 口の表示
    int mouthNum = rand()%3 + 1;
    if (eyeNum <= 2) {
        mvaddstr(y + 3, x, modaneMouthClose);
    } else {
        mvaddstr(y + 3, x, modaneMouthOpen);
    }

    // 口より下の表示
    mvaddstr(y + 4, x, modaneBottom01);
    mvaddstr(y + 5, x, modaneBottom02);

    printf("%d\n", eyeNum);
    printf("%d\n", mouthNum);


    ///// curses で表示操作 : 時計 /////

    y = 1;
    x = w;

    mvaddstr(y    , x - 12               , date);
    mvaddstr(y + 1, x - (strlen(dow) + 2), dow);
    mvaddstr(y + 2, x - 7                , time_);

    ///// curses で表示操作 : ラッキーアイテム /////

    y = 1;
    x = w;

    mvaddstr(y + 4, x - (strlen(luckyItem) + 2), luckyItem);


    ///// curses で表示操作 : wttr /////

    y = h - 1;
    x = w - strlen(wttr01) - 2;

    mvaddstr(y - 5, x, wttr01);
    mvaddstr(y - 4, x, wttr02);
    mvaddstr(y - 3, x, wttr03);
    mvaddstr(y - 2, x, wttr04);
    mvaddstr(y - 1, x, wttr05);



    // キー入力を検知したら終了
    getch();
    endwin();

    return 0;
}
