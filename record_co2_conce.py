import csv
from datetime import datetime as dt
import logging
import logging.handlers
import os

import mh_z19


def verify_co2_conce(co2_conce: int) -> int:
    """CO2 濃度を検証する関数
    メインプログラム側のメモリサイズ超過などの対策用。

    Args:
        co2_conce (int): 検証する CO2 濃度

    Returns:
        int: 正常終了または異常終了を識別する数値。以下いずれかの値が出力される。
            - 0 ... 正常値である
            - 1 ... 5桁以上の異常値
            - 2 ... マイナス値である異常値
    """
    if co2_conce > 9999:
        return 1

    if co2_conce < 0:
        return 2

    return 0


def log_conce(got_datetime, co2_conce, dir_path = './logs/', filename = 'co2_conces.csv', max_bytes = 108000, backup_count = 12):
    """CO2 濃度をログへ保存する関数
    dir_path + filename で指定されたパスへ CO2 濃度を記録したログファイルを出力する。

    Args:
        got_datetime (str):           CSV の 1 列目に挿入する日付文字列。
        co2_conce (int):              CSV の 2 列目に挿入する CO2 濃度の値。
        dir_path (str, optional):     ログファイルのディレクトリパス。デフォルト値は './logs/' 。
        filename (str, optional):     ログファイルのファイル名。デフォルト値は 'co2_conces.csv' 。
        max_bytes (int, optional):    ログファイルのファイルサイズ上限。デフォルト値は 108,000  byte 。
        backup_count (int, optional): ログファイルのローテーションするファイル数。デフォルト値は 12 個。

    Exsample:
        2023-01-08 07:00:00,1\n
        2023-01-08 07:10:00,700\n
        2023-01-08 07:20:00,1025\n
        ...

    Note:
        max_bytes のデフォルト値 10,8000 byte（※1）かつ Exsample の形式でログを出力した場合の最大・最小値は以下のとおり。

        - 1 行毎の最大 byte 数は 25 byte 。この時の最大行数は 4,319 行。
        - 1 行毎の最小 byte 数は 22 byte 。この時の最大行数は 4,909 行（※2）。

        ※1) この 108,000 byte は取得約 1 カ月分の数値。
        25 byte * 6 * 24 * 30 = 108,000 byte で計算している。内訳は以下のとおり。

        - 25 byte ... 1 行毎の最大 byte 数 25 byte 。
        -  6      ... 毎時間の最大取得回数。名プログラム側で 10 分毎に 1 回 record_co2_conce.py が呼び出されるため。
        - 25      ... 24 時間分。
        - 30      ... 30 日分。

        ※2) このため、メインプログラム > getCo2ConcesFromLogs() で確保するメモリヒープ量は 4909 行分を基準にして設定する。

        TODO: このあたりもう少しシンプルな実装にする
    """
    # ロガーを取得
    log = logging.getLogger(__name__)

    # ログ出力レベルの設定
    log.setLevel(logging.INFO)

    # ログの出力先パスを初期化
    os.makedirs(dir_path, exist_ok=True) # logs ディレクトリが無ければ作成
    log_path = dir_path + filename

    # ローテーティングファイルハンドラを作成
    rh = logging.handlers.RotatingFileHandler(
        log_path,
        encoding='utf-8',
        maxBytes=max_bytes,
        backupCount=backup_count
    )

    # ロガーへ追加
    log.addHandler(rh)

    # ログを出力
    log.info(f'{got_datetime},{co2_conce}')


"""
■ メイン処理
"""
# 現在日時を取得（YYYY-MM-DD hh:mm:ss）
now = dt.now().strftime('%Y-%m-%d %H:%M:%S')

# CO2 濃度の値を取得
co2_conce = mh_z19.read_all()["co2"]
# co2_conce = 1

# 取得した値の検証と格納
if verify_co2_conce(co2_conce) == 1:
    # 5桁以上の場合（異常値）は -1 に変更
    verified_co2_conce = -1
if verify_co2_conce(co2_conce) == 2:
    # 負の数値の場合（異常値）は -2 に変更
    verified_co2_conce = -2
else:
    # 正常な値の場合はそのまま代入する
    verified_co2_conce = co2_conce

# CO2 濃度のログを出力
log_conce(now, verified_co2_conce)
