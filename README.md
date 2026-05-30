# where-pochacco

## ゲームの概要
`where-pochacco` は、OLED上に並ぶキャラクターの中から `pochacco` を探すタイムアタックゲームです。  
カーソルを4方向ボタンで動かし、`pochacco` の位置に合わせるとステージクリアになります。

- ステージ数: 3
- ステージ1: `3x3`
- ステージ2: `5x5`
- ステージ3: `5x5`（キャラクター画像が `90/180/270` 度回転）
- クリア条件: カーソル座標と `pochacco` 座標が一致
- スコア: 各ステージ時間の合計（短いほど良い）

## ハード構成

### 対応ボード
- Seeed Studio XIAO ESP32C3

### 使用デバイス
- OLEDディスプレイ（SSD1306, 128x64, I2C, アドレス `0x3C`）
- 4方向ボタン（UP/DOWN/LEFT/RIGHT）
- ブザー（効果音用）

### ピン割り当て（`config.h`）
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
- 以下ライブラリが導入済み
  - `Adafruit GFX Library`
  - `Adafruit SSD1306`

### コンパイル
プロジェクトルートで実行:

```powershell
arduino-cli compile --fqbn esp32:esp32:XIAO_ESP32C3 where-pochacco.ino
```

`.arduino-build` 配下に出力したい場合は例として:

```powershell
arduino-cli compile --fqbn esp32:esp32:XIAO_ESP32C3 --output-dir .arduino-build where-pochacco.ino
```

### 書き込み（例）
接続ポートを確認して実行:

```powershell
arduino-cli upload -p <PORT> --fqbn esp32:esp32:XIAO_ESP32C3 where-pochacco.ino
```

## ゲームの操作方法

- タイトル画面
  - いずれかの矢印ボタンを押すとゲーム開始
- プレイ中
  - `UP`: 上へ1マス移動
  - `DOWN`: 下へ1マス移動
  - `LEFT`: 左へ1マス移動
  - `RIGHT`: 右へ1マス移動
  - 枠線で表示されるカーソルを `pochacco` に合わせるとクリア
- オールクリア後
  - いずれかの矢印ボタンでタイトルへ戻る
