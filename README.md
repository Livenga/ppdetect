# ppdetect

画像処理関係は, CUDAもしくはOpenCLで並列処理を行う予定.


### TODO
* 線形補間(縦横比は固定. 横幅, 縦幅大きい方を基準とする.)

### 雑記
2018/09/17  
画像サイズを縮小することで, 計算量を減少させるとともにプレゼンテーション画面内の
情報を劣化させることができコーナー検出の際に不必要な点を削除することができる.  
これによりもっともらしい直線を正確に判断できる.(推定: 画像の種類によっては無効な処理かもしれない.)

最も投票率が多い直線を基準として処理を行う?
