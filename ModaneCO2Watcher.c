#include <stdio.h>

#include <locale.h>   // setlocale
#include <stdlib.h>   // system, getenv, srand, rand, maclloc, exit
#include <string.h>   // strlen
#include <sys/stat.h> // stat, mkdir
#include <time.h>     // time, localtime, strftime
#include <unistd.h>   // sleep, usleep

#include <ncurses.h>  // curses


/***************************************
 * ディレクトリが存在するか確認する関数
 *
 * ▼引数
 * char dir[] : 確認するファイルパスのポインタ
 *
 * ▼戻り値
 * 0          : 存在する
 * 1          : 存在しない
 *
***************************************/
int checkExistDir(char dir[]) {
    struct stat st;
    if (stat(dir, &st) != 0) {
        return 0;
    }
    return 1;
}

/***************************************
 * ファイルが存在するか確認する関数
 *
 * ▼引数
 * char filepath[] : 確認するファイルパスのポインタ
 *
 * ▼戻り値
 * 0               : 存在する
 * 1               : 存在しない
 *
***************************************/
int checkExistFile(char filepath[]) {
    struct stat st;
    if (stat(filepath, &st) != 0) {
        return 0;
    }
    return 1;
}

/***************************************
 * CO2 濃度をログへ書き込む関数
 *
 * 記録用 Python ファイルを実行する関数。
 * Python ファイル内部で CO2 濃度を測定とログ用 CSV への書き込み処理を行う。
 *
 * python ファイルが存在しない、インポートエラーが起こった場合などが発生した場合は Python のエラーが画面に出力される。
 *
 * ▼引数
 * なし
 *
 * ▼戻り値
 * なし
 *
***************************************/
void doRecordCo2ConceToLogs(void) {
    system("python3 record_co2_conce.py");
    return;
}

/***************************************
 * CO2 濃度をログから取得して配列を書き換える関数
 *
 * CO2 濃度のログファイル最終行（最新の記録）から 指定行数飛ばしに CO2 濃度を取得し、CO2 の濃度配列へ再代入する。
 * 再代入は配列の後ろから行う。
 *
 * 引数2に指定された値で取得する行数の間隔が変化する。
 * 1 の場合は1行毎（10分毎）、6 の場合は6行毎（60分毎）という感じ。
 * 指定行数の間隔に沿って順に取得していき、取得先の行が存在しなくなったらその時点で取得を終了する。
 *
 * 引数2 の指定が 0 であれば 1 を返す。
 * ファイルの展開に失敗した場合は 2 を返す。
 *
 * ▼引数
 * char filepath[] : CO2 濃度ログファイルのパスのポインタ
 * int co2Conces[] : CO2 濃度を格納する配列のポインタ
 * int len         : CO2 濃度を格納する配列の要素数
 * int getInterval : 取得する行数の間隔
 *
 * ▼戻り値
 * 0               : 正常終了
 * 1               : 異常終了（引数エラー）
 * 2               : 異常終了（ファイルの展開に失敗）
 *
***************************************/
int getCo2ConcesFromLogs(char filepath[] ,int co2Conces[], int len, int getInterval) {

    // 引数が指定外だった場合は 1 を返して終了
    if (getInterval <= 0) {
        return 1;
    }

    // ログファイルの展開
    FILE* fp;
    fp = fopen(filepath, "r");

    // ファイルの展開に失敗したら 2 を返して終了
    if (!fp) {
        fclose(fp);
        return 2;
    }

    // 取得用変数・配列の宣言
    int size = 64;        // バッファのサイズ（要素数）
    char buf[size];       // 取得データ格納先へのポインタ
    int lineCount = 0;    // 行カウント
    int co2Conce;         // CO2 濃度値
    char gotDatetime[64]; // 日付。関数終了時に消滅

    // ヒープを確保
    int* co2ConcesHeap;
    co2ConcesHeap = (int*)malloc(sizeof(int) * 100000); // 1.5年分くらい
    if (co2ConcesHeap == NULL) {
        // ヒープが確保できなかったらプログラムを強制終了する
        exit(0);
    }

    // 行数を数えつつ CO2 濃度をヒープ配列へ代入
    for (int i = 0; fgets(buf, size, fp) != NULL; i++) {
        sscanf(buf, "%[^,], %d", gotDatetime, &co2Conce);
        co2ConcesHeap[i] = co2Conce;
        lineCount++;
    }
    fclose(fp);

    // CO2 配列を 0 埋め
    for (int i = 0; i < len; i++) {
        co2Conces[i] = 0;
    }

    // ヒープ配列の最終要素から指定行数毎に CO2 配列の最終要素から順に再代入する
    int c = lineCount - 1; // lineCount を添え字に合わせる
    for (int i = len - 1; i >= 0 && c >= 0; i--) {
        co2Conces[i] = co2ConcesHeap[c];
        c -= getInterval;
    }
    free(co2ConcesHeap); // ヒープの解放

    return 0;
}

/***************************************
 * 天気取得コマンドを作成する関数
 *
 * ▼引数
 * char wttrCmd[] : 天気取得コマンド配列のポインタ
 *
 * ▼戻り値
 * なし
 *
***************************************/
void makeWttrCmd(char wttrCmd[]) {
    char wttrLocale[160];

    // WTTR_LOCALE 環境変数の読み込み
    const char *temp = getenv("WTTR_LOCALE");
    if (temp == NULL || strlen(temp) > 159) {
        // 環境変数を読み込めなかった もしくは 159 バイトを超過 であればデフォルト設定
        strcpy(wttrLocale, "Tokyo");
        printf("天気表示用の環境変数が読み込めなかったため、デフォルト設定 \"%s\" を使用します。\n", wttrLocale);
        printf("好きな場所の天気予報を表示したい場合、WTTR_LOCALE 環境変数を設定してください。\n");
        printf("example : $ export WTTR_LOCALE=\"Tokyo\" ※値は半角 159 文字以下\n\n");
    } else {
        // 問題なければ環境変数を wttrLocale へ格納
        strcpy(wttrLocale, getenv("WTTR_LOCALE"));
        printf("天気表示用の環境変数を読み込みました。\n");
    }
    printf("wttrLocale : %s\n", wttrLocale);

    // 天気取得コマンドを wttrCmd 配列へ格納
    sprintf(wttrCmd, "curl -fs 'wttr.in/%s?%s", wttrLocale, "0MQT&format=%C\\n%t+%p'");

    return;
}

/***************************************
 * 天気を取得する関数
 *
 * ▼引数
 * char wttrCmd[]               : 天気取得コマンド配列のポインタ
 * int size1                    : 天気情報を格納している二次元配列の1次元目配列の要素数
 * int size2                    : 天気情報を格納している二次元配列の2次元目配列の要素数
 * char wttrLines[size1][size2] : 天気情報を格納している二次元配列のポインタと各要素数
 *
 * ▼戻り値
 * なし
 *
 * TODO: 天気の取得時にサーバーの応答待ちの分、表示が遅れてしまう。スレッド処理にしてラグを無くしたい。
 *
***************************************/
void getWttrLines(char wttrCmd[], int size1, int size2, char wttrLines[size1][size2]) {
    // 天気の格納用配列を宣言
    FILE* fp = NULL;

    // 天気取得コマンドの実行
    fp = popen(wttrCmd, "r"); // コマンド実行結果を fp へ格納

    // fp が NULL だった場合はエラー文を格納して処理を中断
    if (fp == NULL) {
        strcpy(wttrLines[0], "Wttr command result was not returned.");
        strcpy(wttrLines[1], "");
        return;
    }

    for (int i = 0; i < size1; i++) {
        fgets(wttrLines[i], sizeof(wttrLines[i]), fp) != NULL; // 1行ごとに配列へ格納
    }

    pclose(fp);

    return;
}

/***************************************
 * 描画用関数 : 現在時刻を毎秒描画する
 *
 * ▼引数
 * int y      : 描画を行う Y 座標
 * int x      : 描画を行う X 座標
 * time_t now : time(NULL) で取得できる UNIX 時間
 *
 * ▼戻り値
 * なし
 *
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
 * 描画用関数 : 天気を描画する
 *
 * ▼引数
 * int y                        : 描画を行う Y 座標
 * int x                        : 描画を行う X 座標
 * int size1                    : 天気情報を格納している二次元配列の1次元目配列の要素数
 * int size2                    : 天気情報を格納している二次元配列の2次元目配列の要素数
 * char wttrLines[size1][size2] : 天気情報を格納している二次元配列のポインタ
 *
 * ▼戻り値
 * なし
 *
***************************************/
void printWttr(int y, int x, int size1, int size2, char wttrLines[size1][size2]) {
    for (int i = 0; i < size1; i++) {
        mvaddstr(y++, x - strlen(wttrLines[i]), wttrLines[i]);
    }
    return;
}

/***************************************
 * 描画用関数 : CO2グラフの軸を描画する
 *
 * 引数3 で渡された値によって横軸のグラフの表示を変更する。
 *   1 (NARROW) : 10分/1列
 *   3 (MEDIUM) : 30分/1列
 *   6 (BROAD)  : 60分/1列
 *
 * ▼引数
 * int y        : 描画を行う Y 座標
 * int x        : 描画を行う X 座標
 * int co2Style : CO2 定数指定
 *
 * ▼戻り値
 * 0            : 正常終了
 * 1            : 異常終了（引数3が指定外だった場合）
 *
***************************************/
int printCo2GraphBase(int y, int x, int co2Style) {

    char co2GraphBaseLines[][32] = {
        //                             XXXX ←この列に現在の ppm 数値が左詰めで入る
        "(ppm)                       ",
        "  900 |                     ",
        "      |                     ",
        "  600 |                     ",
        "      |                     ",
        "  300 |                     ",
        "      |                     ",
        "    0 +---------------------"
    };

    for (int i = 0; i < 8; i++) {
        mvaddstr(y++, x, co2GraphBaseLines[i]);
    }

    // 引数3に応じて X 軸の数値を置き換え
    switch (co2Style) {
        case 1:
            //               "    0 +--------------------- XXXX"
            mvaddstr(y++, x, "        -3h     -1.5h     Cur.");
            break;
        case 3:
            //               "    0 +--------------------- XXXX"
            mvaddstr(y++, x, "     -10h       -5h       Cur.");
            break;
        case 6:
            //               "    0 +--------------------- XXXX"
            mvaddstr(y++, x, "     -20h      -10h       Cur.");
            break;
        default:
            return 1;
    }

    return 0;
}

/***************************************
 * 描画用関数 : CO2グラフの折れ線グラフを描画する
 *
 * ▼引数
 * int y           : 描画を行う Y 座標
 * int x           : 描画を行う X 座標
 * int co2Conces[] : CO2 濃度の配列
 * int len         : CO2 濃度配列の要素数
 *
 * ▼戻り値
 * なし
 *
***************************************/
void printCo2LineGraph(int y, int x, int co2Conces[], int len) {
    // 境界値配列の初期化
    // 下限値 + (上限値 / (行数 * 3)) * 繰り返し回数（i）で境界値を計算
    int co2BoundaryValue[18] = {0};
    for (int i = 0; i < 18; i++) {
        co2BoundaryValue[i] = 75 + 50 * (i + 1);
        // mvprintw(1 + i   , 1, "%d", co2BoundaryValue[i]); // 確認用
    }

    // 折れ線グラフの描画
    for (int i = 0; i < len; i++) {
        // 0 以下の場合は表示しないので何もせずループをコンティニュー
        if (co2Conces[i] <= 0) {
            x++;
            continue;
        }

        // 1000 以上であれば ! を6行目に描画してコンティニュー
        if (co2Conces[i] >= 1000) {
            mvaddstr(y - 6, x, "!");
            x++;
            continue;
        }

        // 1 ～ 999 の間の描画処理
        // TODO: もう少し良い感じに書き換える

        // 1行目の描画
        if (co2Conces[i] <= co2BoundaryValue[0]) {
            mvaddstr(y    , x, "_"); // 1行目の _ は 1 ～ 境界値配列の下限値を範囲に含む
        } else if (co2Conces[i] <= co2BoundaryValue[1]) {
            mvaddstr(y    , x, "-");
        } else if (co2Conces[i] <= co2BoundaryValue[2]) {
            mvaddstr(y    , x, "`");

        // 2行目の描画
        } else if (co2Conces[i] <= co2BoundaryValue[3]) {
            mvaddstr(y - 1, x, "_");
        } else if (co2Conces[i] <= co2BoundaryValue[4]) {
            mvaddstr(y - 1, x, "-");
        } else if (co2Conces[i] <= co2BoundaryValue[5]) {
            mvaddstr(y - 1, x, "`");

        // 3行目の描画
        } else if (co2Conces[i] <= co2BoundaryValue[6]) {
            mvaddstr(y - 2, x, "_");
        } else if (co2Conces[i] <= co2BoundaryValue[7]) {
            mvaddstr(y - 2, x, "-");
        } else if (co2Conces[i] <= co2BoundaryValue[8]) {
            mvaddstr(y - 2, x, "`");

        // 4行目の描画
        } else if (co2Conces[i] <= co2BoundaryValue[9]) {
            mvaddstr(y - 3, x, "_");
        } else if (co2Conces[i] <= co2BoundaryValue[10]) {
            mvaddstr(y - 3, x, "-");
        } else if (co2Conces[i] <= co2BoundaryValue[11]) {
            mvaddstr(y - 3, x, "`");

        // 5行目の描画
        } else if (co2Conces[i] <= co2BoundaryValue[12]) {
            mvaddstr(y - 4, x, "_");
        } else if (co2Conces[i] <= co2BoundaryValue[13]) {
            mvaddstr(y - 4, x, "-");
        } else if (co2Conces[i] <= co2BoundaryValue[14]) {
            mvaddstr(y - 4, x, "`");

        // 6行目の描画
        } else if (co2Conces[i] <= co2BoundaryValue[15]) {
            mvaddstr(y - 5, x, "_");
        } else if (co2Conces[i] <= co2BoundaryValue[16]) {
            mvaddstr(y - 5, x, "-");
        } else if (co2Conces[i] <= co2BoundaryValue[17]) {
            mvaddstr(y - 5, x, "`");

        // 7行目の描画
        } else {
            mvaddstr(y - 6, x, "_"); // 7行目の _ は 境界値配列の上限超過値 ～ 999 を範囲に含む
        }
        x++;
    }
    return;
}

/***************************************
 * 描画用関数 : CO2グラフ右側へ現在の値を描画する
 *
 * ▼引数
 * int y           : 描画を行う Y 座標
 * int x           : 描画を行う X 座標
 * int co2ValueNow : 現在の CO2 濃度
 *
 * ▼戻り値
 * なし
 *
***************************************/
void printCo2ValueNow(int y, int x, int co2ValueNow) {
        if (co2ValueNow <= 225) {
            mvprintw(y    , x, "%d", co2ValueNow);
        } else if (co2ValueNow <= 375) {
            mvprintw(y - 1, x, "%d", co2ValueNow);
        } else if (co2ValueNow <= 525) {
            mvprintw(y - 2, x, "%d", co2ValueNow);
        } else if (co2ValueNow <= 675) {
            mvprintw(y - 3, x, "%d", co2ValueNow);
        } else if (co2ValueNow <= 825) {
            mvprintw(y - 4, x, "%d", co2ValueNow);
        } else if (co2ValueNow <= 975) {
            mvprintw(y - 5, x, "%d", co2ValueNow);
        } else {
            mvprintw(y - 6, x, "%d", co2ValueNow);
        }
    return;
}

/***************************************
 * 描画用関数 : CO2グラフを1秒毎に点滅させる
 *
 * ▼引数
 * int y           : 描画を行う Y 座標
 * int x           : 描画を行う X 座標
 * int co2ValueNow : time_t now : time(NULL) で取得できる UNIX 時間
 *
 * ▼戻り値
 * なし
 *
***************************************/
void doBlinkCo2Graph(int y, int x, time_t now) {
    if (now % 2 == 0) {
        for (int i = 0; i < 7; i++) {
            mvaddstr(y - i, x - 1, " ");
        }
    }
    return;
}

/***************************************
 * 描画用関数 : もだねちゃん AA を描画する
 *
 * ▼引数
 * int y        : 描画を行う Y 座標
 * int x        : 描画を行う X 座標
 * int eyeNum   : 表示する目の添え字
 * int mouthNum : 表示する口の添え字
 *
 * ▼戻り値
 * なし
 *
***************************************/
void printModane(int y, int x, int eyeNum, int mouthNum) {
    // AA 用配列を初期化
    char modaneTopLines[][32] = {
        "         ____      ",
        "       /      \\    ",
        "      ( ____   ﾚ-、",
        "    ／       -ノ_ﾗ ",
        "   \"  人      ヽ   ",
        " /  ／_ `-_     \\  ",
        "|  /        `-_  | "
    };

    char modaneEyeLines[][32] = {
        "(||   |    |   ||) ",  // Open
        "(|| -==    ==- ||) ",  // Close
        "(||  ＞    ＜  ||) "   // Gyu
    };

    char modaneMouthLines[][32] = {
        " \\) ,,  ワ  ,, (ﾉ  ", // Open
        " \\) ,,  ー  ,, (ﾉ  "  // Close
    };

    char modaneBottomLines[][32] = {
        "  )ヽ________ノ(   ",
        "      \\_父_/      "
    };

    // 頭の描画
    for (int i = 0; i < 7; i++) {
        mvaddstr(y++, x, modaneTopLines[i]);
    }

    // 目の描画（引数の値によって出し分け）
    if (eyeNum <= 11) {
        mvaddstr(y++, x, modaneEyeLines[0]); // Open
    } else if (eyeNum <= 19) {
        mvaddstr(y++, x, modaneEyeLines[1]); // Close
    } else {
        mvaddstr(y++, x, modaneEyeLines[2]); // Gyu
    }

    // 口の描画（引数の値によって出し分け）
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
    /***************************************
     * タイトル表示
    ***************************************/
    printf("------------------------------------------\n");
    printf("                Moda Clock                \n");
    printf("------------------------------------------\n");


    /***************************************
     * CO2 グラフの描画処理準備
    ***************************************/
    printf("=== CO2 グラフの描画処理準備を開始 ===\n");

    // CO2 濃度ログファイルのパスを定義
    char co2ConcesDir[] = "./logs/";
    char co2ConcesPath[] = "./logs/co2_conces.csv";

    // CO2 濃度記録用のログファイルを作成
    printf("CO2 濃度記録用のディレクトリとログファイルの存在確認を行います。\n");

    if (checkExistDir(co2ConcesDir)) {
        printf("ディレクトリが見つかりました。\n%s\n", co2ConcesDir);
    } else {
        printf("ディレクトリが見つかりませんでした。ディレクトリを作成します。\n%s\n", co2ConcesDir);
        // ディレクトリを作成
        if (mkdir(co2ConcesDir, 0755)) {
            // エラーの場合はプログラムを終了する
            perror("mkdir error.");
            exit(0);
        }
    }

    if (checkExistFile(co2ConcesPath)) {
        printf("ファイルが見つかりました。\n%s\n", co2ConcesPath);
    } else {
        printf("ファイルが見つかりませんでした。ファイルを作成します。\n%s\n", co2ConcesPath);
        // 空のファイルを作成
        FILE* fp;
        fp = fopen(co2ConcesPath, "w");
        fclose(fp);
    }

    // 取得行数の定数とグラフの表示形式を定義（初期値は NARROW とする）
    const int NARROW = 1; //  3 時間前まで（ログから1行毎に取得）
    const int MEDIUM = 3; // 10 時間前まで（ログから3行毎に取得）
    const int BROAD  = 6; // 20 時間前まで（ログから6行毎に取得）
    int co2Style = NARROW;

    // CO2 濃度配列を初期化
    int co2Conces[21] = {0};

    // CO2 濃度配列の要素数を取得
    int co2ConcesLen = sizeof(co2Conces) / sizeof(co2Conces[0]);

    // CO2 濃度配列の初期化・ログファイルから過去の CO2 濃度を取得して配列へ格納
    printf("ファイルに記録されている CO2 濃度を取得します。\n");
    getCo2ConcesFromLogs(co2ConcesPath, co2Conces, co2ConcesLen, co2Style);

    // 取得したCO2配列の確認
    printf("取得した CO2 濃度の配列を表示します。\n");
    for (int i = 0; i < co2ConcesLen; i++) {
        printf("co2Conces[%2d] : %4d\n", i, co2Conces[i]);
    }
    printf("\n");



    /***************************************
     * 天気の描画処理準備
    ***************************************/
    printf("=== 天気の描画処理準備を開始 ===\n");

    // wttr コマンドの作成
    char wttrCmd[256];
    makeWttrCmd(wttrCmd);
    printf("wttrCmd    : %s\n\n", wttrCmd);

    // 天気取得コマンドの初回実行
    char wttrLines[2][256];
    getWttrLines(wttrCmd, 2, 256, wttrLines);
    printf("wttrLines :\n%s%s\n\n", wttrLines[0], wttrLines[1]);


    /***************************************
     * もだねちゃんの描画処理準備
    ***************************************/
    printf("=== もだねちゃんの描画処理準備を開始 ===\n");

    // 目と口を出し分けするための乱数用変数を初期化
    int eyeNum       = 1;
    int mouthNum     = 1;


    /***************************************
     * キー入力とステータスバーの描画処理準備
    ***************************************/
    printf("=== キー入力とステータスバーの描画処理準備を開始 ===\n");

    // 変数を初期化
    int StatusCount = 0; // ステータスバーの表示時間を管理する変数
    int PushedKey;       // 押されたキーを保持しておく変数
    int PrevKey;         // 前回押されたキーを保持しておく変数


    /***************************************
     * 画面描画の準備
    ***************************************/
    printf("=== 画面描画の準備を開始 ===\n");

    time_t prev, now;            // 現在とループ前の時間
    int w, h;                    // 描画する画面の幅・高さ
    int x, y;                    // mvaddstr() などで指定する縦横軸の座標
    int	key;                     // 送信されたキー入力の番号を格納する変数。getch() の戻り値は int のため int 型で宣言
    srand((unsigned)time(NULL)); // 乱数用のシードを設定
    setlocale(LC_CTYPE, "");     // 2バイト文字を扱えるようにする


    /***************************************
     * 端末制御の開始
    ***************************************/
    printf("\n描画を開始します。\n終了するには Q キーを押してください。\n");

    sleep(3);                // 設定内容表示のため一旦待機
    initscr();               // cursesを初期化。端末制御の開始
    noecho();                // キー入力した文字を非表示
    timeout(0);              // キー入力を待たない
    curs_set(0);             // カーソルを消す


    // 以降はループ処理で毎秒画面を更新して描画を行う
    while(1) {
        /***************************************
         * ループ確認処理
        ***************************************/
        // 前回の時間を確認して1秒経っていたら処理を開始する
        // 同じ場合は指定マイクロ秒スリープして処理をスキップする
        now = time(NULL);
        if (prev == now) {
            usleep(100000);
            continue;
        }
        prev = now; // 現在日時を次のループの比較用に格納


        /***************************************
         * 画面の高さと幅の取得・描画準備
        ***************************************/
        // 繰り返し毎に高さ幅を取得してレスポンシブにする
        getmaxyx(stdscr, h, w); // 画面の高さと幅の取得。stdscr は端末画面を表す定数。
        erase();                // 画面を消去


        /***************************************
         * 時計の描画処理
        ***************************************/
        // 時間を描画
        printDatetime(1, w - 2, now);


        /***************************************
         * 天気の描画処理
        ***************************************/
        // 天気の描画
        printWttr(2, w - 1, 2, 256, wttrLines); // w - 2 でないのは改行文字分を含めるため

        // 30分毎に天気取得コマンドを再実行して天気情報を更新する
        if (now % (60 * 30) == 0) {
            getWttrLines(wttrCmd, 2, 256, wttrLines);
        }


        /***************************************
         * CO2 グラフの描画処理
        ***************************************/
        // 10分毎にログへ書き込み & 配列の更新
        if (now % (60 * 10) == 0) {
            doRecordCo2ConceToLogs();
            getCo2ConcesFromLogs(co2ConcesPath, co2Conces, co2ConcesLen, co2Style);

        }

        // グラフのベースを描画
        printCo2GraphBase(h - 10, w - 34, co2Style);

        // CO2 濃度配列を折れ線グラフで描画
        printCo2LineGraph(h - 4, w - 27, co2Conces, co2ConcesLen);

        // CO2 濃度配列の現在の数値（配列の最後の要素）を描画
        printCo2ValueNow(h - 4, w - 5, co2Conces[co2ConcesLen - 1]);

        // 1秒毎に現在のグラフ線部分にスペースを上書きして点滅させる
        doBlinkCo2Graph(h - 4, w - 6, now);


        /***************************************
         * もだねちゃんの描画処理
        ***************************************/
        printModane(h / 2 - 5, 2, eyeNum, mouthNum);

        // 10秒毎に次のループ用の乱数を生成
        if (now % 10 == 0) {
            eyeNum   = rand() % 20 + 1;
            mouthNum = rand() % 2 + 1;
        }


        /***************************************
         * キー入力の待ち受け処理
        ***************************************/
        // キー入力の受け取り
        key = getch();

        // Q キーを受け取ったらステータスバーに表示してループを抜ける
        if (key == 'q') {
            mvaddstr(h - 1, 0, "Q キーを受け取りました。プログラムを終了します。");
            refresh();
            sleep(2);
            break;
        }

        // キーに応じた処理
        switch (key) {

            // キーが押されていない（key に -1 が格納されている）なら何もしない
            case -1:
                break;

            // A キーであれば CO2 グラフを 20時間前 までの表示へ切り替え
            case 'a':
                StatusCount = 3;  // ステータスバーの表示ループ数を設定
                PushedKey = key;  // ステータスバー表示用に押されたキーを保存
                co2Style = BROAD; // CO2 表示の切り替え
                getCo2ConcesFromLogs(co2ConcesPath, co2Conces, co2ConcesLen, co2Style); // CO2 配列の再取得
                break;

            // S キーであれば CO2 グラフを 10時間前 までの表示へ切り替え
            case 's':
                StatusCount = 3;
                PushedKey = key;
                co2Style = MEDIUM;
                getCo2ConcesFromLogs(co2ConcesPath, co2Conces, co2ConcesLen, co2Style);
                break;

            // D キーであれば CO2 グラフを 3時間前 までの表示へ切り替え
            case 'd':
                StatusCount = 3;
                PushedKey = key;
                co2Style = NARROW;
                getCo2ConcesFromLogs(co2ConcesPath, co2Conces, co2ConcesLen, co2Style);
                break;

            // 他のキーであれば格納だけを行う
            default:
                StatusCount = 3;
                PushedKey = key;
        }


        /***************************************
         * ステータスバーの処理
        ***************************************/
        // ステータスバーの表示ループ数が残っている間表示処理を行う
        if (StatusCount > 0) {

            // 保存されているキーに応じてステータスバーへメッセージを表示
            switch (PushedKey) {
                case 'a':
                    mvaddstr(h - 1, 0, "CO2 グラフを 20時間前 までの表示へ切り替えます。");
                    break;
                case 's':
                    mvaddstr(h - 1, 0, "CO2 グラフを 10時間前 までの表示へ切り替えます。");
                    break;
                case 'd':
                    mvaddstr(h - 1, 0, "CO2 グラフを 3時間前 までの表示へ切り替えます。");
                    break;
                default:
                    mvaddstr(h - 1, 0, "A S D : CO2 グラフの切り替え  Q : 終了");

            }

            StatusCount--;
        }


        /***************************************
         * 画面の更新
        ***************************************/
        refresh();

    }

    endwin(); // 端末制御の終了
    return 0;
}
