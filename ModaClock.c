#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include <ncurses.h> // curses ライブラリ
#include <locale.h>  // curses で2バイト文字を扱えるようにする


/***************************************
天気取得コマンドを作成する関数

引数   : コマンドを格納する既定の配列
戻り値 : なし（既定の配列を書き換えるため）

***************************************/
void makeWttrCmd (char wttrCmd[512]) {
    // 取得するロケールを初期化
    const char* WTTR_LOCALE = "Tokyo";

    // 環境変数に WTTR_LOCALE が設定してあればその値を読み込む
    if (getenv("WTTR_LOCALE")) {
        WTTR_LOCALE = getenv("WTTR_LOCALE");
        printf("天気表示用の環境変数を読み込みました。\n");
    } else {
        printf("天気表示用の環境変数が読み込めなかったため、デフォルト設定 \"%s\" を使用します。\n", WTTR_LOCALE);
        printf("好きな場所の天気予報を表示したい場合、WTTR_LOCALE 環境変数を設定してください。\n");
        printf("example : $ export WTTR_LOCALE=\"Tokyo\"\n\n");
    }
    printf("WTTR_LOCALE : %s\n", WTTR_LOCALE);

    const char* WTTR_OPTION = "0MQT";                // wttr のオプション
    const char* WTTR_FORMAT = "&format=%C\\n%t+%p'"; // フォーマット指定

    // 天気取得コマンドを wttrCmd 配列へ格納
    sprintf(wttrCmd, "curl -fs 'wttr.in/%s?%s%s", WTTR_LOCALE, WTTR_OPTION, WTTR_FORMAT);

    return;
}

/***************************************
天気を取得する関数

引数   : 天気取得コマンドの配列, 天気情報を格納する既定の二次元配列（512バイト × 2）
戻り値 : なし（既定の配列を書き換えるため）

***************************************/
void getWttrLines (char wttrCmd[512], char wttrLines[2][512]) {
    // 天気の格納用配列を宣言
    FILE * fp = NULL;

    // 天気取得コマンドの実行
    fp = popen(wttrCmd, "r"); // コマンド実行結果を fp へ格納
    for (int i = 0; i < 2; i++) {
        fgets(wttrLines[i], sizeof(wttrLines[i]), fp) != NULL; // 1行ごとに配列へ格納
    }

    pclose(fp);

    return;
}

/***************************************
描画用関数 : 現在時刻を毎秒描画する

引数   : Y 座標, X 座標, time_t 型の現在日時
戻り値 : なし

***************************************/
void printDatetime(int y, int x, time_t now) {
    char datetime[64];

    // now を日付時間と曜日へ変換
    if (now % 2 == 0) {
        // 偶数の場合はコロンなし
        strftime(datetime, sizeof(datetime), "%Y-%m-%d (%a) %H %M", localtime(&now));
    } else {
        // 奇数の場合はコロンあり
        strftime(datetime, sizeof(datetime), "%Y-%m-%d (%a) %H:%M", localtime(&now));
    }

    // 時計を描画
    mvaddstr(y, x - strlen(datetime), datetime);
    return;
}

/***************************************
描画用関数 : 天気を描画する

引数   : Y 座標, X 座標, 天気文字列の二次元配列
戻り値 : なし

***************************************/
void printWttr(int y, int x, char wttrLines[2][512]) {
    for (int i = 0; i < 2; i++) {
        mvaddstr(y++, x - strlen(wttrLines[i]), wttrLines[i]);
    }
    return;
}

/***************************************
描画用関数 : CO2グラフの軸を描画する

引数   : Y 座標, X 座標, time_t 型の現在日時
戻り値 : なし

***************************************/
void printCo2GraphBase(int y, int x) {
    char co2GraphBaseLines[7][64] = {
        //                            XXXX ←この列に現在の ppm 数値が入るのでその分スペースを確保している
        "(ppm)                            ",
        " 2000 |                          ",
        "      |                          ",
        " 1000 |                          ",
        "      |                          ",
        "    0 +---------------------     ",
        "      -20m      -10m      Cur.   "
    };

    for (int i = 0; i < 7; i++) {
        mvaddstr(y++, x, co2GraphBaseLines[i]);
    }
    return;
}

/***************************************
描画用関数 : CO2グラフの折れ線グラフを描画する

引数   : Y 座標, X 座標, CO2 濃度の配列
戻り値 : なし

***************************************/
void printCo2LineGraph(int y, int x, int co2Values[11]) {
    // 境界値配列の初期化
    // 下限値値（250）+ 2000 / 12 の値を切り捨てた値（167）* 繰り返し回数（i）で境界値を計算
    // NOTE: 切り捨て時の端数を考慮していないため、少しずつ境界値が本来の境界値よりズレていく
    int co2BoundaryValue[11];
    for (int i = 0; i < 11; i++) {
        co2BoundaryValue[i] = 250 + 167 * (i + 1);
    }

    // 折れ線グラフの描画
    for (int i = 0; i < 21; i++) {
        if (co2Values[i] < co2BoundaryValue[0]) {
            mvaddstr(y, x, "_");
        } else if (co2Values[i] < co2BoundaryValue[1]) {
            mvaddstr(y, x, "-");
        } else if (co2Values[i] < co2BoundaryValue[2]) {
            mvaddstr(y, x, "`");
        } else if (co2Values[i] < co2BoundaryValue[3]) {
            mvaddstr(y - 1, x, "_");
        } else if (co2Values[i] < co2BoundaryValue[4]) {
            mvaddstr(y - 1, x, "-");
        } else if (co2Values[i] < co2BoundaryValue[5]) {
            mvaddstr(y - 1, x, "`");
        } else if (co2Values[i] < co2BoundaryValue[6]) {
            mvaddstr(y - 2, x, "_");
        } else if (co2Values[i] < co2BoundaryValue[7]) {
            mvaddstr(y - 2, x, "-");
        } else if (co2Values[i] < co2BoundaryValue[8]) {
            mvaddstr(y - 2, x, "`");
        } else if (co2Values[i] < co2BoundaryValue[9]) {
            mvaddstr(y - 3, x, "_");
        } else if (co2Values[i] < co2BoundaryValue[10]) {
            mvaddstr(y - 3, x, "-");
        } else {
            mvaddstr(y - 3, x, "`");
        }
        x++;
        }
    return;
}

/***************************************
描画用関数 : CO2グラフの現在の値を描画

引数   : Y 座標, X 座標, 現在のCO2濃度
戻り値 : なし

***************************************/
void printCo2ValueNow(int y, int x, int co2ValueNow) {
        if (co2ValueNow < 750) {
            mvprintw(y    , x, "%4d", co2ValueNow);
        } else if (co2ValueNow < 1250) {
            mvprintw(y - 1, x, "%4d", co2ValueNow);
        } else if (co2ValueNow < 1750) {
            mvprintw(y - 2, x, "%4d", co2ValueNow);
        } else {
            mvprintw(y - 3, x, "%4d", co2ValueNow);
        }
    return;
}

/***************************************
描画用関数 : CO2グラフを1秒毎に点滅させる

引数   : Y 座標, X 座標, time_t 型の現在日時
戻り値 : なし

***************************************/
void doBlinkCo2Graph(int y, int x, time_t now) {
    if (now % 2 == 0) {
        mvaddstr(y    , x - 1, " ");
        mvaddstr(y - 1, x - 1, " ");
        mvaddstr(y - 2, x - 1, " ");
        mvaddstr(y - 3, x - 1, " ");
    }
    return;
}

/***************************************
描画用関数 : もだねちゃん AA を描画する

引数   : Y 座標, X 座標, 目の乱数, 口の乱数
戻り値 : なし

***************************************/
void printModane(int y, int x, int eyeNum, int mouthNum) {
    // AA 用配列を初期化
    char modaneTopLines[7][64] = {
        "         ____      ",
        "       /      \\    ",
        "      ( ____   ﾚ-、",
        "    ／       -ノ_ﾗ ",
        "   \"  人      ヽ   ",
        " /  ／_ `-_     \\  ",
        "|  /        `-_  | "
    };

    char modaneEyeLines[3][64] = {
        "(||   |    |   ||) ",  // Open
        "(|| -==    ==- ||) ",  // Close
        "(||  ＞    ＜  ||) "   // Gyu
    };

    char modaneMouthLines[2][64] = {
        " \\) ,,  ワ  ,, (ﾉ  ", // Open
        " \\) ,,  ー  ,, (ﾉ  "  // Close
    };

    char modaneBottomLines[2][64] = {
        "  )ヽ________ノ(   ",
        "      \\_父_/      "
    };

    // 頭の描画
    for (int i = 0; i < 7; i++) {
        mvaddstr(y++, x, modaneTopLines[i]);
    }

    // 目の描画
    if (eyeNum <= 11) {
        mvaddstr(y++, x, modaneEyeLines[0]); // Open
    } else if (eyeNum <= 19) {
        mvaddstr(y++, x, modaneEyeLines[1]); // Close
    } else {
        mvaddstr(y++, x, modaneEyeLines[2]); // Gyu
    }

    // 口の描画
    if (mouthNum == 1) {
        mvaddstr(y++, x, modaneMouthLines[0]); // Open
    } else {
        mvaddstr(y++, x, modaneMouthLines[1]); // Close
    }

    // 首元の描画
    for (int i = 0; i < 2; i++) {
        mvaddstr(y++, x, modaneBottomLines[i]);
    }

    return;
}


int main(void) {
    ////////// タイトル表示 //////////
    printf("------------------------------------------\n");
    printf("                Moda Clock                \n");
    printf("------------------------------------------\n");


    ////////// CO2 グラフの描画処理準備 //////////
    // TODO: 別ファイルから濃度の値を取得する関数をこの辺で実行。時系列の昇順で格納する（1番目が21分前、20番目が1分前、21番目が現在）
    // CO2 濃度配列の初期化
    // int co2Values[21] = {0};

    // 確認用（直接指定）
    // int co2Values[21] = {
    //     0,
    //     100,
    //     200,
    //     300,
    //     400,
    //     500,
    //     600,
    //     700,
    //     800,
    //     900,
    //     1000,
    //     1100,
    //     1200,
    //     1300,
    //     1400,
    //     1500,
    //     1600,
    //     1700,
    //     1800,
    //     1900,
    //     2000
    // };

    // 確認用（ランダム値）
    // ランダム値を生成後に昇順で並び替え（https://programminglang.com/ja/sort-array-in-descending-order-ja/）
    int co2Values[21];
    for (int i = 0; i < 22; i++) {
        int n = rand() % 2300 + 1;
        co2Values[i] = n;
    }
    int i, j, tmp;
    for (i=0; i<sizeof(co2Values)/sizeof(co2Values[0]); i++) {
        for(j=i+1; j<sizeof(co2Values)/sizeof(co2Values[0]); j++) {
            if(co2Values[j] < co2Values[i]) {
                tmp = co2Values[i];
                co2Values[i] = co2Values[j];
                co2Values[j] = tmp;
            }
        }
    }

    // 取得したCO2配列の確認
    for (int i = 0; i < 21; i++) {
        printf("co2Values %2d : %4d\n", i, co2Values[i]);
    }
    printf("\n");


    ////////// 天気の描画処理準備 //////////
    // wttr コマンドの作成
    char wttrCmd[512];
    makeWttrCmd(wttrCmd);
    printf("wttrCmd     : %s\n\n", wttrCmd);

    // 天気取得コマンドの初回実行
    char wttrLines[2][512];
    getWttrLines(wttrCmd, wttrLines);
    printf("wttrLines :\n%s%s\n\n", wttrLines[0], wttrLines[1]);


    ////////// もだねちゃんの描画処理準備 //////////
    // 目と口を出し分けするための乱数用変数を初期化
    int eyeNum       = 1;
    int mouthNum     = 1;


    ////////// 描画処理の準備 //////////
    time_t prev, now;            // 現在とループ前の時間
    int w, h;                    // 描画する画面の幅・高さ
    int x, y;                    // mvaddstr() などで指定する縦横軸の座標
    int	key;                     // 送信されたキー入力の番号を格納する変数。getch() の戻り値は int のため int 型で宣言
    srand((unsigned)time(NULL)); // 乱数用のシードを設定
    setlocale(LC_CTYPE, "");     // 2バイト文字を扱えるようにする


    ////////// 端末制御の開始 //////////
    printf("描画を開始します。\n終了するには Q キーを押してください。\n");
    sleep(3);                // 設定内容表示のため一旦待機
    initscr();               // cursesを初期化。端末制御の開始
    noecho();                // キー入力した文字を非表示
    timeout(0);              // キー入力を待たない
    curs_set(0);             // カーソルを消す


    // 以降はループ処理で毎秒画面を更新して描画を行う
    while(1) {
        ////////// ループ確認処理 //////////
        // 前回の時間を確認して1秒経っていたら処理を開始する
        // 同じ場合は指定マイクロ秒スリープして処理をスキップする
        now = time(NULL);
        if (prev == now) {
            usleep(100000);
            continue;
        }
        prev = now; // 現在日時を次のループの比較用に格納


        ////////// 画面の高さと幅の取得・描画準備 //////////
        // 繰り返し毎に高さ幅を取得してレスポンシブにする
        getmaxyx(stdscr, h, w); // 画面の高さと幅の取得。stdscr は端末画面を表す定数。
        erase();                // 画面を消去


        ////////// 時計の描画処理 //////////
        printDatetime(1, w - 2, now);


        ////////// 天気の描画処理 //////////
        // 天気の描画
        printWttr(2, w - 1, wttrLines); // w - 2 でないのは改行文字分を含めるため

        // 30分毎に天気取得コマンドを再実行して天気情報を更新する
        // TODO: スレッド処理にして更新時のラグを無くす
        if (now % (60 * 30) == 0) {
            getWttrLines(wttrCmd, wttrLines);
        }


        ////////// CO2 グラフの描画処理 //////////
        // TODO: 1分毎に濃度配列を更新する

        // グラフのベースを描画
        printCo2GraphBase(h - 8, w - 35);

        // CO2 濃度配列を折れ線グラフで描画
        printCo2LineGraph(h - 4, w - 28, co2Values);

        // CO2 濃度配列の現在の数値を描画
        printCo2ValueNow(h - 4, w - 6, co2Values[20]);

        // 1秒毎に現在のグラフ線部分にスペースを上書きして点滅させる
        doBlinkCo2Graph(h - 4, w - 7, now);


        ////////// もだねちゃんの描画処理 //////////
        printModane(h / 2 - 5, 2, eyeNum, mouthNum);

        // 4秒毎に次のループ用の乱数を生成
        if (now % 4 == 0) {
            eyeNum   = rand() % 20 + 1;
            mouthNum = rand() % 2 + 1;
        }


        ////////// 描画処理 //////////
        // 画面を再描画
        refresh();

        // Q キーを受け取ったらループを抜ける
        key = getch();
        if (key == 'q') {
            break;
        }
    }
    endwin(); // 端末制御の終了

    return 0;
}
