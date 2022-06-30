import csv
from datetime import datetime as dt
import os

# import mh_z19

# 現在日時を取得（YYYY-MM-DD hh:mm:ss）
now = dt.now().strftime('%Y-%m-%d %H:%M:%S')
print(now)

# CO2 濃度の値を取得
# co2_conce = mh_z19.read_all()["co2"]
co2_conce = 111
print(co2_conce)

# logs ディレクトリが無ければ作成
dir_path = './logs/'
os.makedirs(dir_path, exist_ok=True) 

# CSV ファイルへ書き込み
filename = 'co2_conces.csv'
with open(dir_path + filename, 'a') as f:
    writer = csv.writer(f)
    writer.writerow([now, co2_conce])
