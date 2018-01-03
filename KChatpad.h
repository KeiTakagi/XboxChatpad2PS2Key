//
// ファイル: KChatpad.h
// 概    要: Arduino STM32 Xbox Chatpad Clone PS/2 ライブラリ用
// このソースコードは次のソースコードを参考に作成しました
//   たま吉さん      https://github.com/Tamakichi/ArduinoSTM32_PS2Keyboard
//   Cliff L. Biffle http://cliffle.com/project/chatpad
// 作 成 者: Kei Takagi
// 作 成 日: 2017/10/10
// Special Thanks：
//         おおさかJR3
//

#ifndef __KCHATPAD_H__
#define __KCHATPAD_H__

#include <stdint.h>

// ハードウェアシリアルポート
class HardwareSerial;

// 状態管理用
#define BREAK_CODE       0x0100  // BREAKコード
#define KEY_CODE         0x0200  // KEYコード
#define SHIFT_CODE       0x0400  // SHIFTあり
#define CTRL_CODE        0x0800  // CTRLあり
#define ALT_CODE         0x1000  // ALTあり
#define GUI_CODE         0x2000  // GUIあり

#define KEY_NONE          0      // 継続またはバッファに変換対象なし
#define KEY_ERROR         255    // キーコードエラー

#define US                0      // 英語版
#define JP                1      // 日本語版

// キーボードイベント構造体
typedef struct  {
  uint8_t code  : 8; // code
  uint8_t BREAK : 1; // BREAKコード
  uint8_t KEY   : 1; // KEYコード判定
  uint8_t SHIFT : 1; // SHIFTあり
  uint8_t CTRL  : 1; // CTRLあり
  uint8_t ALT   : 1; // ALTあり
  uint8_t GUI   : 1; // GUIあり
  uint8_t FUNC  : 1; // Functionあり
  uint8_t dumy  : 1; // ダミー
} keyEvent;

// キーボードイベント共用体
typedef union {
  uint16_t  value;
  keyEvent  kevt;
} keyinfo;

// クラス定義
class KChatpad {
  private:
    HardwareSerial *_serial;
    uint8_t _keyboardType;
    uint8_t _buffer[8];
    uint8_t _last_key0;
    uint8_t _last_key1;
    uint32_t _last_ping;
  public:
    // キーボード利用開始
    uint8_t begin(HardwareSerial &, int);
    // キーボード利用開始
    uint8_t begin(HardwareSerial &);

    // キーボード利用終了
    void end();
    // キーボード初期化
    uint8_t init();
    // シリアルポートに何バイトのデータが到着しているかを返します。
    int available(void);
    // キーボード入力の読み込み
    keyEvent read();
    // 起きろコマンド送信
    void GetUp();
};

#endif
