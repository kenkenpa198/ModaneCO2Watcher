#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include <ncurses.h> // curses ライブラリ
#include <locale.h>  // curses で2バイト文字を扱えるようにする

int main(void) {
    ///// タイトル表示 /////
    printf("------------------------------------------\n");
    printf("                Moda Clock                \n");
    printf("------------------------------------------\n");


    ///// 時計の描画処理準備 /////
    // 時計の表示用変数を宣言
    char date[64], dow[64], time_[64];
    time_t prev, now;


    ///// 天気の描画処理準備 /////
    // TODO: この辺の処理を関数化する
    // 環境変数の読み込み
    const char* WTTR_LOCALE;
    WTTR_LOCALE = getenv("WTTR_LOCALE");

    // 環境変数が読み込めなかったらエラー文を表示して終了
    if (! WTTR_LOCALE) {
        printf("The environment variable for displaying weather is not set.\nPlease set \"WTTR_LOCALE\" to \".bashrc\".\n\n");
        printf("example:\n  $ export WTTR_LOCALE=\"Tokyo\"\n");
        return 1;
    }

    const char* WTTR_OPTION = "0MQT";                // wttr のオプション
    const char* WTTR_FORMAT = "&format=%C\\n%t+%p'"; // フォーマット指定

    // 天気取得コマンドを wttrCmd 配列へ格納
    char wttrCmd[512];
    sprintf(wttrCmd, "curl -fs 'wttr.in/%s?%s%s", WTTR_LOCALE, WTTR_OPTION, WTTR_FORMAT);

    // 確認用
    printf("WTTR_LOCALE : %s\n", WTTR_LOCALE);
    printf("wttrCmd     : %s\n", wttrCmd);

    // 天気の格納用配列を宣言
    FILE * fp = NULL;
    char wttrLines[5][256];

    // 天気取得コマンドの初回実行
    fp = popen(wttrCmd, "r"); // コマンド実行結果を fp へ格納
    for (int i = 0; i < 5; i++) {
        fgets(wttrLines[i], sizeof(wttrLines[i]), fp) != NULL; // 1行ごとに配列へ格納
    }


    ///// もだねちゃんの描画処理準備 /////
    // もだねちゃんの AA を2次元配列へ格納
    // NOTE: エスケープ文字列を一部使っているのでズレに注意
    char modaneTops[7][64] = {
        "         ____      ",
        "       /      \\    ",
        "      ( ____   ﾚ-、",
        "    ／       -ノ_ﾗ ",
        "   \"  人      ヽ   ",
        " /  ／_ `-_     \\  ",
        "|  /        `-_  | "
    };

    char modaneEyes[3][64] = {
        "(||   |    |   ||) ",  // Open
        "(|| -==    ==- ||) ",  // Close
        "(||  ★     ★   ||) " // Star ★を半角で表示するフォントの場合。全角の場合は調整必要
    };

    char modaneMouths[2][64] = {
        " \\) ,,  ワ  ,, (ﾉ  ", // Open
        " \\) ,,  ー  ,, (ﾉ  "  // Close
    };

    char modaneBottoms[2][64] = {
        "  )ヽ________ノ(   ",
        "      \\_父_/      "
    };


    ///// ppm graph /////
    char ppmGraphs[7][64] = {
        "(ppm)                            ",
        " 2000 |                          ",
        "      |        -''-.             ",
        " 1000 |   ..--'     '--..        ",
        "      |.-'               '-.  220",
        "    0 +---------------------     ",
        "      -20m      -10m      Cur.   "
    };


    ///// ループ処理の準備 /////
    // 秒数カウント・乱数用変数の初期化
    srand((unsigned)time(NULL)); // 乱数用のシードを設定
    int eyeNum       = 1;        // 目の再描画用乱数
    int mouthNum     = 1;        // 口の再描画用乱数
    int secCountWttr = 1;        // 天気を再取得する秒数カウント
    int secCountModa = 1;        // もだねちゃんを再描画する秒数カウント


    ///// 描画処理の準備 /////
    int w, h, x, y;              // mvaddstr() などで指定する座標用変数の宣言
    setlocale(LC_CTYPE, "");     // 2バイト文字を扱えるようにする


    ///// 端末制御の開始 /////
    initscr();               // cursesを初期化。端末制御の開始
    noecho();                // キー入力した文字を非表示
    curs_set(0);             // カーソルを消す

    while(1) {

        ///// ループ確認処理 /////
        // 前回の時間を確認して1秒経っていたら処理を開始する
        now = time(NULL);
        if (prev == now) {
            usleep(100000); // 同じ場合は指定マイクロ秒スリープしてループをコンティニューする
            continue;
        }
        prev = now; // 現在日時を次のループの比較用に格納


        ///// 画面の高さと幅の取得・描画準備 /////
        // 繰り返し毎に高さ幅を取得してレスポンシブにする
        getmaxyx(stdscr, h, w); // 画面の高さと幅の取得。stdscr は端末画面を表す定数。
        erase();                // 画面を消去


        ///// 時計の描画処理 /////
        // now 変数から日付と曜日を取得する
        strftime(date, sizeof(date), "%Y-%m-%d %A", localtime(&now));
        // strftime(dow, sizeof(dow), "%A", localtime(&now));

        // now 変数から時間を取得する
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
        mvaddstr(y++, x - strlen(date), date);
        mvaddstr(y++, x - 5, time_);


        ///// 天気の描画処理 /////
        // 基準となる座標の値を格納（シェルの右下）
        y++;
        x = w + 1 - 2; // + 1 しているのは改行文字分を含めるため

        // 天気の描画
        for (int i = 0; i < 2; i++) {
            mvaddstr(y++, x - strlen(wttrLines[i]), wttrLines[i]);
        }

        // 乱数と秒カウントの描画（確認用）
        // y++;
        // mvprintw(y++, x, "secCountWttr : %d", secCountWttr);

        // 指定時間秒経ったら天気取得コマンドを再実行して fp を更新する
        if (secCountWttr == 60 * 30) {
            // TODO: 関数化する
            // TODO: スレッド処理にして更新時のラグを無くす

            fp = popen(wttrCmd, "r");
            for (int i = 0; i < 5; i++) {
                fgets(wttrLines[i], sizeof(wttrLines[i]), fp) != NULL;
            }

            secCountWttr = 1; // カウントのリセット
        } else {
            secCountWttr++;
        }


        ///// もだねちゃんの描画処理 /////
        // 基準となる座標の値を格納（シェルの中央左側）
        y = h / 2 - 5;
        x = 2;

        // 頭の描画
        for (int i = 0; i < 7; i++) {
            mvaddstr(y++, x, modaneTops[i]);
        }

        // 目の描画
        if (eyeNum <= 6) {
            mvaddstr(y++, x, modaneEyes[0]); // Open
        } else if (eyeNum <= 9) {
            mvaddstr(y++, x, modaneEyes[1]); // Close
        } else {
            mvaddstr(y++, x, modaneEyes[2]); // Star
        }

        // 口の描画
        if (mouthNum == 1) {
            mvaddstr(y++, x, modaneMouths[0]); // Open
        } else {
            mvaddstr(y++, x, modaneMouths[1]); // Close
        }

        // 首元の描画
        for (int i = 0; i < 2; i++) {
            mvaddstr(y++, x, modaneBottoms[i]);
        }

        // 乱数と秒カウントの描画（確認用）
        // y++;
        // mvprintw(y++, x, "secCountModa : %d", secCountModa);
        // mvprintw(y++, x, "eyeNum       : %d", eyeNum);
        // mvprintw(y++, x, "mouthNum     : %d", mouthNum);

        // 4秒経ったら次のループ用の乱数を生成
        if (secCountModa == 4) {
            eyeNum   = rand()%10 + 1;
            mouthNum = rand()%2 + 1;
            secCountModa = 1;
        } else {
            secCountModa++;
        }


        ///// ppm グラフの描画処理 /////
        // 基準となる座標の値を格納（シェルの右下）
        y = h - 8;
        x = w - strlen(ppmGraphs[0]) - 2;

        for (int i = 0; i < 7; i++) {
            mvaddstr(y++, x, ppmGraphs[i]);
        }


        refresh(); // 画面を再描画
    }

    return 0;
}
