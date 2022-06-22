#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <time.h>
#include <unistd.h>

#include <ncurses.h> // curses ライブラリ
#include <locale.h>  // curses で2バイト文字を扱えるようにする

typedef char String[1024];

int main(void) {
    ///// 時計の表示用変数を宣言 /////
    char date[64], dow[64], time_[64];
    time_t prev, now;


    ///// もだねちゃんの顔を各変数へ格納 /////
    // エスケープ文字列を一部使っているのでズレに注意
    String modaneTop01 =      "         ____      ";
    String modaneTop02 =      "       /      \\    ";
    String modaneTop03 =      "      ( ____   ﾚ-、";
    String modaneTop04 =      "    ／       -ノ_ﾗ ";
    String modaneTop05 =      "   \"  人      ヽ   ";
    String modaneTop06 =      " /  ／_ `-_     \\  ";
    String modaneTop07 =      "|  /        `-_  | ";

    String modaneEyeOpen =    "(||   |    |   ||) ";
    String modaneEyeClose =   "(|| -==    ==- ||) ";
    String modaneEyeStar =    "(||  ★     ★   ||) "; // ★を半角で表示するフォントの場合。全角の場合は★の右側を1つずつ減らす

    String modaneMouthClose = " \\) ,,  ー  ,, (ﾉ  ";
    String modaneMouthOpen =  " \\) ,,  ワ  ,, (ﾉ  ";

    String modaneBottom01 =   "  )ヽ________ノ(   ";
    String modaneBottom02 =   "      \\_父_/      ";

    // もだねちゃんの目と口を表示する乱数用のシードを設定
    srand((unsigned)time(NULL));

    // 乱数用変数の初期化
    int secCount = 1; // 秒数カウント用
    int eyeNum   = 0;
    int mouthNum = 0;


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



    ///// curses で描画処理を行う準備 /////
    int w, h, x, y;          // move() などで指定する座標用変数の宣言

    setlocale(LC_CTYPE, ""); // 2バイト文字を扱えるようにする
    initscr();               // cursesを初期化。端末制御の開始
    noecho();                // キー入力した文字を非表示
    curs_set(0);             // カーソルを消す


    ///// 描画ループ処理 /////
    while(1) {

        ///// ループ確認処理 /////
        // 前回の時間を確認して1秒経っていたら処理を開始する
        now = time(NULL);
        if (prev == now) {
            usleep(2000); // 同じ場合は指定マイクロ秒スリープしてループをコンティニューする
            continue;
        }
        prev = now; // 現在日時を次のループの比較用に格納


        ///// 画面の高さと幅の取得・描画準備 /////
        // stdscr は端末画面を表す定数
        // 繰り返し毎に高さ幅を取得してレスポンシブにする
        getmaxyx(stdscr, h, w);
        erase();  // 画面を消去


        ///// 時計の描画処理 /////
        // now 変数から日付と曜日を取得する
        strftime(date, sizeof(date), "%Y-%m-%d", localtime(&now));
        strftime(dow, sizeof(dow), "%A", localtime(&now));

        // now 変数から日付と曜日を取得する
        if (now % 2 == 0) {
            // 偶数の場合はコロンあり
            strftime(time_, sizeof(time_), "%H:%M", localtime(&now));
        } else {
            // 奇数の場合はコロンなし
            strftime(time_, sizeof(time_), "%H %M", localtime(&now));
        }

        // 基準となる座標の値を格納（シェルの右上）
        y = 1;
        x = w - 2;

        // 時計を描画
        mvaddstr(y++, x - 10, date);
        mvaddstr(y++, x - strlen(dow), dow);
        mvaddstr(y++, x - 5, time_);


        ///// もだねちゃんの描画処理 /////
        // 基準となる座標の値を格納（シェルの中央左側）
        y = h / 2 - 6;
        x = 2;

        // 頭の描画
        mvaddstr(y++, x, modaneTop01);
        mvaddstr(y++, x, modaneTop02);
        mvaddstr(y++, x, modaneTop03);
        mvaddstr(y++, x, modaneTop04);
        mvaddstr(y++, x, modaneTop05);
        mvaddstr(y++, x, modaneTop06);
        mvaddstr(y++, x, modaneTop07);

        // 目の描画
        if (eyeNum <= 6) {
            mvaddstr(y++, x, modaneEyeOpen);
        } else if (eyeNum <= 9) {
            mvaddstr(y++, x, modaneEyeClose);
        } else {
            mvaddstr(y++, x, modaneEyeStar);
        }

        // 口の描画
        if (eyeNum <= 2) {
            mvaddstr(y++, x, modaneMouthClose);
        } else {
            mvaddstr(y++, x, modaneMouthOpen);
        }

        // 口より下の描画
        mvaddstr(y++, x, modaneBottom01);
        mvaddstr(y++, x, modaneBottom02);

        // 乱数と秒カウントの描画（確認用）
        // y++;
        // mvprintw(y++, x, "secCount : %d", secCount);
        // mvprintw(y++, x, "eyeNum   : %d", eyeNum);
        // mvprintw(y++, x, "mouthNum : %d", mouthNum);

        // 4秒経ったら次のループ用の乱数を生成
        if (secCount == 4) {
            secCount = 1;
            eyeNum   = rand()%9 + 1;
            mouthNum = rand()%4 + 1;
        } else {
            secCount++;
        }


        ///// ラッキーアイテムの描画処理 /////
        // 基準となる座標の値を格納
        y = 5;
        x = w - 2;

        // 描画
        mvaddstr(y, x - strlen(luckyItem), luckyItem);


        ///// 天気の描画処理 /////
        // 基準となる座標の値を格納（シェルの右下）
        y = h - 6;
        x = w - strlen(wttr01) - 2;

        // 描画
        mvaddstr(y++, x, wttr01);
        mvaddstr(y++, x, wttr02);
        mvaddstr(y++, x, wttr03);
        mvaddstr(y++, x, wttr04);
        mvaddstr(y++, x, wttr05);


        refresh(); // 画面を再描画
    }




    return 0;
}
