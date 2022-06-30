import csv
from datetime import datetime as dt
# import mh_z19

# 現在日時を取得（YYYY-MM-DD hh:mm:ss）
now = dt.now().strftime('%Y-%m-%d %H:%M:%S')
print(now)

# CO2 濃度の値を取得
# co2_conce = mh_z19.read_all()["co2"]
co2_conce = 111
print(co2_conce)

# CSV ファイルへ書き込み
with open('./logs/co2_conces.csv', 'a') as f:
    writer = csv.writer(f)
    writer.writerow([now, co2_conce])
