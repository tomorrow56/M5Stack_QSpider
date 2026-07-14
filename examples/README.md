# Examples

This folder contains Arduino sketches for controlling the M5Stack Q-Spider quadruped robot.

このフォルダには、M5Stack Q-Spider（小型四脚ロボット）を制御するためのArduinoスケッチが入っています。

---

## Folder Structure / フォルダ構成

| Folder | Description / 説明 |
|---|---|
| `M5_Spider_Webserver` | Control the robot from a web browser via WiFi. WiFi経由のWebブラウザから操作します。 |
| `M5_Spider_esp-now` | Control the robot with a game controller using ESP-NOW communication. ESP-NOW通信でゲームコントローラから操作します。 |

---

## Common Requirements / 共通要件

- **M5Stack Basic** (ESP32 based)
- **SG-90** or compatible 9g servos × 9
- **PCA9685** servo driver module (I2C address: `0x72`)
- Arduino IDE with the following libraries / 以下のライブラリが必要です:
  - `M5Stack` / `M5Unified`
  - `Adafruit PWM Servo Driver Library` (for PCA9685)
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

This sketch starts the M5Stack as a WiFi access point or connects to an existing access point, and serves a simple web controller on port 80.

このスケッチは、M5StackをWiFiアクセスポイントとして起動するか、既存のアクセスポイントに接続して、ポート80で簡易Webコントローラを提供します。

### Features / 機能

- WiFi setup via `WiFiManager` (captive portal)
- OTA (Over The Air) firmware update support
- Web UI with buttons for forward, backward, left/right shift, spin, and stop
- Face animation (eye blink, mouth open) on the M5Stack LCD
- LINE Notify message on successful WiFi connection
- Calibration mode (`adjMode`) triggered when powered on while charging

### How to Use / 使い方

1. Open `M5_Spider_Webserver.ino` in Arduino IDE.
2. Select `M5Stack-Core-ESP32` board.
3. Update the LINE Notify token (`token`) if you want IP notifications.
4. Upload the sketch to M5Stack.
5. Connect to the displayed AP name or configure via WiFiManager portal.
6. Open the IP address shown on the LCD with a browser.

### Main Files / 主なファイル

- `M5_Spider_Webserver.ino` : Main sketch / メインスケッチ
- `WebPage.h` : HTML for the web controller / WebコントローラのHTML
- `wifi_setup.cpp/h` : WiFi connection helper and OTA setup / WiFi接続とOTAのヘルパー
- `avatar.cpp/h` : Face animation class / 顔アニメーションクラス

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

### Main Files / 主なファイル

- `M5_Spider_esp-now.ino` : Main sketch / メインスケッチ
- `clappyavator.cpp/h` : Face animation class / 顔アニメーションクラス
- `ip5306.cpp/h` : Battery and charging status / バッテリ・充電状態

For the transmitter side, see the M5AtomPad repository.

送信側はM5AtomPadリポジトリを参照してください。

<https://github.com/tomorrow56/M5AtomPad>

---

## Calibration / キャリブレーション

All sketches provide a calibration mode that moves every servo to its center position. This is useful during mechanical assembly.

いずれのスケッチでも、すべてのサーボをセンター位置に移動するキャリブレーションモードがあります。機械的な組み立て時に便利です。

- **Web versions**: Enter calibration mode automatically when powered on while M5Stack is charging.
- **ESP-NOW version**: Define `chgDET` at the top of the sketch to enable the same behavior.

| Sketch | Calibration Trigger / トリガ |
|---|---|
| `M5_Spider_Webserver` | Power on while charging / 充電中に電源ON |
| `M5_Spider_Webserver_task` | Power on while charging / 充電中に電源ON |
| `M5_Spider_esp-now` | Enable `chgDET` then power on while charging / `chgDET`を有効にして充電中に電源ON |

Press the **B button** to exit calibration mode.

**Bボタン**でキャリブレーションモードを終了します。
