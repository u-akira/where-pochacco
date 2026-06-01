# where-pochacco

## ゲームの概要
`where-pochacco` は、OLED 画面に表示されたキャラクターの中から `pochacco` を見つけるタイムアタックゲームです。  
カーソルを4方向ボタンで動かし、`pochacco` の位置に合わせるとステージクリアになります。

- ステージ数: 3
- ステージ1: `3x3`
- ステージ2: `5x5`
- ステージ3: `5x5`（キャラクター画像を `90/180/270` 度回転）
- クリア条件: カーソル位置と `pochacco` の位置が一致
- スコア: 各ステージのクリア時間合計（短いほど良い）

## ハードウェア構成

### 対応ボード
- Seeed Studio XIAO ESP32C3

### 使用デバイス
- OLEDディスプレイ（SSD1306, 128x64, I2C, アドレス `0x3C`）
- 4方向ボタン（`UP` / `DOWN` / `LEFT` / `RIGHT`）
- ブザー（効果音用）

### ピン設定（`config.h`）
- OLED
  - `OLED_SDA`: GPIO6（D4）
  - `OLED_SCL`: GPIO7（D5）
  - `OLED_RESET`: `-1`（未使用）
- ボタン
  - `BUTTON_UP`: GPIO20（D7）
  - `BUTTON_DOWN`: GPIO8（D8）
  - `BUTTON_LEFT`: GPIO9（D9）
  - `BUTTON_RIGHT`: GPIO10（D10）
- ブザー
  - `BUZZER_PIN`: GPIO5（D3）

## ビルド方法

### 前提
- Arduino CLI がインストール済み
- ESP32 ボード定義（`esp32:esp32`）がインストール済み
- 以下ライブラリがインストール済み
  - `Adafruit GFX Library`
  - `Adafruit SSD1306`

### コンパイル
プロジェクトルートで実行:

```powershell
arduino-cli compile --fqbn esp32:esp32:XIAO_ESP32C3 where-pochacco.ino
```

`.arduino-build` 配下に出力したい場合:

```powershell
arduino-cli compile --fqbn esp32:esp32:XIAO_ESP32C3 --output-dir .arduino-build where-pochacco.ino
```

### 書き込み（アップロード）
接続ポートを指定して実行:

```powershell
arduino-cli upload -p <PORT> --fqbn esp32:esp32:XIAO_ESP32C3 where-pochacco.ino
```

## ゲームの操作方法
- タイトル画面
  - いずれかの方向ボタンを押すとゲーム開始
- プレイ中
  - `UP`: 上へ1マス移動
  - `DOWN`: 下へ1マス移動
  - `LEFT`: 左へ1マス移動
  - `RIGHT`: 右へ1マス移動
  - 枠線で表示されるカーソルを `pochacco` に合わせるとクリア
- クリア画面
  - いずれかの方向ボタンで次ステージへ進行

ドット絵の作成には以下のドット絵ディタを利用しています。
https://yugi-tech-lab.github.io/BM-GamePod/sw/tools/oled_dot_editor.html
