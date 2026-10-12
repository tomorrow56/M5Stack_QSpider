# Examples

This folder contains Arduino sketches for controlling the M5Stack Q-Spider quadruped robot.

このフォルダには、M5Stack Q-Spider（小型四脚ロボット）を制御するためのArduinoスケッチが入っています。

---

## Folder Structure / フォルダ構成

| Folder | Description / 説明 |
|---|---|
| `M5_Spider_Webserver` | Control the robot from a web browser via WiFi. WiFi経由のWebブラウザから操作します。 |
| `M5_Spider_esp-now` | Control the robot with a game controller using ESP-NOW communication. ESP-NOW通信でゲームコントローラから操作します。 |
| `ESP32_ChipInfo` | Utility sketch to check the ESP32 chip info and MAC address. ESP32のチップ情報とMACアドレスを確認するユーティリティスケッチです。 |

---

## Common Requirements / 共通要件

- **M5Stack Basic** (ESP32 based)
- **SG-90** or compatible 9g servos × 9
- **PCA9685** servo driver module (I2C address: `0x72`)
- Arduino IDE with the following libraries:
  - `M5Stack` / `M5Unified`
  - `Adafruit PWM Servo Driver Library` (for PCA9685)
  - `M5StackUpdater`

以下のライブラリをインストールしたArduino IDEが必要です。

- `M5Stack` / `M5Unified`
- `Adafruit PWM Servo Driver Library`（PCA9685用）
- `M5StackUpdater`

The servo channel assignment is common across all sketches.

サーボのチャンネル割り当てはすべてのスケッチで共通です。

| Channel | Function | 機能 |
|---|---|---|
| 0 | FR0 | Front Right knee / 右前脚膝関節 |
| 1 | FR1 | Front Right leg swing / 右前脚振り |
| 2 | FL0 | Front Left knee / 左前脚膝関節 |
| 3 | FL1 | Front Left leg swing / 左前脚振り |
| 4 | TILT | Head tilt / 頭部チルト |
| 6 | RR0 | Rear Right knee / 右後脚膝関節 |
| 7 | RR1 | Rear Right leg swing / 右後脚振り |
| 8 | RL0 | Rear Left knee / 左後脚膝関節 |
| 9 | RL1 | Rear Left leg swing / 左後脚振り |

---

## M5_Spider_Webserver

By default, this sketch starts the M5Stack as a standalone WiFi access point and serves a simple web controller on port 80.

このスケッチは、デフォルトではM5Stackを独立したWiFiアクセスポイントとして起動し、ポート80で簡易Webコントローラを提供します。

### Features / 機能

- Standalone WiFi access point mode by default
- Optional WiFi setup via `WiFiManager` (captive portal)
- OTA (Over The Air) firmware updates in WiFiManager mode
- Web UI with buttons for forward, backward, left/right shift, spin, and stop
- Face animation (eye blink, mouth open) on the M5Stack LCD
- Calibration mode (`adjMode`) triggered when powered on while charging

- デフォルトで独立WiFiアクセスポイントとして動作
- `WiFiManager`（キャプティブポータル）によるWiFi設定も選択可能
- WiFiManagerモードではOTAによるファームウェア更新に対応
- 前進、後退、左右移動、旋回、停止を操作できるWeb UI
- M5Stack LCDで目や口の顔アニメーションを表示
- 充電中に電源を入れるとキャリブレーションモード（`adjMode`）を開始

### WiFi Mode / WiFiモード

`usesoftAP` is defined by default near the top of `M5_Spider_Webserver.ino`. In this mode, the M5Stack creates an access point named `ESP32-xxxx` with the password `esp32`. Connect directly to it and open `http://192.168.4.1/`.

`M5_Spider_Webserver.ino`の先頭付近では、デフォルトで`usesoftAP`が定義されています。このモードでは、M5Stackが`ESP32-xxxx`という名前、パスワード`esp32`のアクセスポイントを作成します。直接接続し、`http://192.168.4.1/`を開いてください。

To connect the M5Stack to an existing WiFi access point instead, comment out `#define usesoftAP`. The sketch then uses the WiFiManager portal for network configuration and enables OTA updates.

既存のWiFiアクセスポイントへM5Stackを接続する場合は、`#define usesoftAP`をコメントアウトしてください。その場合はWiFiManagerの設定ポータルを使用し、OTA更新も有効になります。

### How to Use / 使い方

1. Open `M5_Spider_Webserver.ino` in Arduino IDE.
2. Select the `M5Stack-Core-ESP32` board.
3. Keep `#define usesoftAP` enabled for standalone AP mode, or comment it out for WiFiManager mode.
4. Upload the sketch to M5Stack.
5. Connect to the AP name displayed on the LCD.
6. Open the IP address shown on the LCD with a browser.

1. Arduino IDEで`M5_Spider_Webserver.ino`を開きます。
2. `M5Stack-Core-ESP32`ボードを選択します。
3. 独立APモードでは`#define usesoftAP`を有効のままにし、WiFiManagerモードではコメントアウトします。
4. スケッチをM5Stackへ書き込みます。
5. LCDに表示されたアクセスポイント名へ接続します。
6. LCDに表示されたIPアドレスをブラウザで開きます。

### Main Files / 主なファイル

- `M5_Spider_Webserver.ino`: Main sketch
- `WebPage.h`: HTML for the web controller
- `wifi_setup.cpp/h`: WiFi connection helper and OTA setup
- `avatar.cpp/h`: Face animation class

- `M5_Spider_Webserver.ino`: メインスケッチ
- `WebPage.h`: WebコントローラのHTML
- `wifi_setup.cpp/h`: WiFi接続とOTAのヘルパー
- `avatar.cpp/h`: 顔アニメーションクラス

---

## M5_Spider_esp-now

This sketch receives control commands wirelessly from a custom ESP-NOW controller (such as M5AtomPad) and drives the robot.

このスケッチは、ESP-NOW対応のカスタムコントローラ（M5AtomPadなど）からの無線コマンドを受信してロボットを駆動します。

### Features / 機能

- ESP-NOW peer-to-peer communication
- NES (Famicom) style gamepad input support
- Continuous walking/spinning controlled by flags
- FreeRTOS face animation task on Core 0
- Two machine presets (`modeNES = 1` for SpiderW, `modeNES = 2` for SpiderB) for per-unit calibration
- Calibration mode triggered by charging detection (define `chgDET` to enable)

- ESP-NOWによるピアツーピア通信
- NES（ファミコン）形式のゲームパッド入力に対応
- フラグ制御による連続歩行・旋回
- Core 0で動作するFreeRTOS顔アニメーションタスク
- 個体調整用の2種類のプリセット（SpiderWは`modeNES = 1`、SpiderBは`modeNES = 2`）
- 充電検出で開始するキャリブレーションモード（`chgDET`の定義で有効化）

### Controller Button Mapping / コントローラのボタン割り当て

| NES Button | Command | Action / 動作 |
|---|---|---|
| UP | rNES_UP | Walk forward / 前進 |
| DOWN | rNES_DOWN | Walk backward / 後退 |
| LEFT | rNES_LEFT | Shift left / 左移動 |
| RIGHT | rNES_RIGHT | Shift right / 右移動 |
| A | rNES_A | Spin right / 右旋回 |
| B | rNES_B | Spin left / 左旋回 |
| START/SELECT | - | Reserved / 予約 |
| (no input) | rNES_STOP | Stop / 停止 |

### How to Use / 使い方

1. Open `M5_Spider_esp-now.ino` in Arduino IDE.
2. Set `modeNES` to match your robot unit (1 or 2).
3. Make sure the controller MAC address is registered in the `OnDataRecv` filter.
4. Upload the sketch to M5Stack.
5. Power on the ESP-NOW controller and gamepad.

1. Arduino IDEで`M5_Spider_esp-now.ino`を開きます。
2. ロボットの個体に合わせて`modeNES`を1または2に設定します。
3. コントローラのMACアドレスが`OnDataRecv`フィルタに登録されていることを確認します。
4. スケッチをM5Stackへ書き込みます。
5. ESP-NOWコントローラとゲームパッドの電源を入れます。

### Main Files / 主なファイル

- `M5_Spider_esp-now.ino`: Main sketch
- `avatar.cpp/h`: Face animation class

- `M5_Spider_esp-now.ino`: メインスケッチ
- `avatar.cpp/h`: 顔アニメーションクラス

For the transmitter side, see the M5AtomPad repository.

送信側はM5AtomPadリポジトリを参照してください。

<https://github.com/tomorrow56/M5AtomPad>

When registering a controller in the `OnDataRecv` MAC filter, you can use `ESP32_ChipInfo` on each M5Stack to check its MAC address.

`OnDataRecv`のMACアドレスフィルタに登録する際は、各M5Stackで`ESP32_ChipInfo`を実行してMACアドレスを確認できます。

---

## ESP32_ChipInfo

A simple utility sketch that prints ESP32 chip information (model, features, core count, revision, flash size, and most importantly the MAC address) to the Serial Monitor.

ESP32のチップ情報（モデル、機能、コア数、リビジョン、フラッシュサイズ、そして重要なMACアドレス）をシリアルモニタに出力するシンプルなユーティリティスケッチです。

### Use Case / 用途

Useful for checking the MAC address of an M5Stack unit before registering it as an ESP-NOW peer (see `M5_Spider_esp-now`) or before hardcoding a peer's MAC address in a controller sketch.

M5StackユニットをESP-NOWのピアとして登録する前（`M5_Spider_esp-now`を参照）や、コントローラ側にピアのMACアドレスをハードコードする前に、MACアドレスを確認するのに使えます。

### How to Use / 使い方

1. Open `ESP32_ChipInfo.ino` in Arduino IDE.
2. Upload the sketch to the M5Stack.
3. Open the Serial Monitor at 115200 baud.
4. The chip model, features, core count, revision, MAC address, flash size, and chip ID are printed once at startup.

1. Arduino IDEで`ESP32_ChipInfo.ino`を開きます。
2. スケッチをM5Stackへ書き込みます。
3. シリアルモニタを115200 baudで開きます。
4. 起動時にチップモデル、機能、コア数、リビジョン、MACアドレス、フラッシュサイズ、チップIDが一度出力されます。

### Main Files / 主なファイル

- `ESP32_ChipInfo.ino`: Main sketch

- `ESP32_ChipInfo.ino`: メインスケッチ

---

## Calibration / キャリブレーション

All sketches provide a calibration mode that moves every servo to its center position. This is useful during mechanical assembly.

いずれのスケッチでも、すべてのサーボをセンター位置に移動するキャリブレーションモードがあります。機械的な組み立て時に便利です。

- **Web versions**: Enter calibration mode automatically when powered on while M5Stack is charging.
- **ESP-NOW version**: Define `chgDET` at the top of the sketch to enable the same behavior.

- **Web版**：M5Stackの充電中に電源を入れると、自動的にキャリブレーションモードへ移行します。
- **ESP-NOW版**：スケッチ先頭で`chgDET`を定義すると、同じ動作が有効になります。

| Sketch | Calibration Trigger / トリガ |
|---|---|
| `M5_Spider_Webserver` | Power on while charging / 充電中に電源ON |
| `M5_Spider_Webserver_task` | Power on while charging / 充電中に電源ON |
| `M5_Spider_esp-now` | Enable `chgDET` then power on while charging / `chgDET`を有効にして充電中に電源ON |

Press the **B button** to exit calibration mode.

**Bボタン**でキャリブレーションモードを終了します。
