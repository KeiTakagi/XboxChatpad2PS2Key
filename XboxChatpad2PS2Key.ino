//
// ファイル: XboxChatpad2PS2Key.ino
// 概    要: 3.3V PS/2インターフェース for Arduino STM32
// 作 成 者: Kei Takagi
// 作 成 日: 2017/10/10
// Special Thanks：
//         おおさかJR3
//

#include "ps2dev.h"
#include "KChatpad.h"

#define LED_PIN   PC13
#define DATA_PIN  PB5
#define CLOCK_PIN PB4

KChatpad kb;

// PS2dev object (2:data, 3:clock)
PS2dev keyboard(CLOCK_PIN, DATA_PIN);

void ack() {
  // acknowledge commands
  // ECHO、RESEND以外のコマンドに対する正しく受け取られた応答メッセージ(初期化コマンド、ACK応答)
  while (keyboard.write(0xFA));
}

// キーボードがパソコンへ発行するコマンドコード
int kbdCmd(int command) {
  uint8_t val;
  switch (command) {
    case 0xFF:
      // キーボードリセットコマンド。
      // 正しく受け取った場合ACKを返す。その後キーボードはセルフテストを実行する。
      ack();
      delay(50);
      while (keyboard.write(0xAA) != 0);// BAT(Basic Assurance Test)が正しく終了した
      break;
    case 0xFE:
      // 再送要求
      // Reset:
      ack();
      break;
    case 0xF6:
      // 再送要求
      // set defaults
      // enter stream mode
      ack();
      break;
    case 0xF5:
      // 起動時の状態へ戻し、キースキャンを停止する
      // disable data reporting
      // FM
      ack();
      break;
    case 0xF4:
      // キースキャンを開始する
      // enable data reporting
      // FM
      ack();
      break;
    case 0xF3:
      // リピートレートの設定
      // Rate=1 / ((8 + (bit2:0) ) * (2 ** (bit4:3)) * 0.00417 )  MakeCode/Sec
      //set typematic rate/delay :
      ack();
      keyboard.read(&val); //do nothing with the rate
      ack();
      break;
    case 0xF2:
      // ID読み出し
      // get device id :
      // 識別情報を要求  ACK ('L') ('H')
      ack();
      keyboard.write(0xAB);// ID(L)識別IDのLowバイト
      keyboard.write(0x83);// ID(H)識別IDのHighバイト
      break;
    case 0xF0:
      // スキャンコード選択(廃止?)
      // Opt=00 現在のスキャンコードを返す  ACK(Code)
      //     01 コードセット1              ACK x ACK
      //     02 コードセット2              ACK x ACK
      //     03 コードセット4              ACK x ACK
      // set scan code set
      ack();
      keyboard.read(&val); //do nothing with the rate
      ack();
      break;
    case 0xEE:
      // リセットと自己診断
      // Reset(廃止?)
      // echo
      // ack();
      keyboard.write(0xEE);
      break;
    case 0xED:
      // キーボードのLEDの点灯/消灯要求。
      // これに続くオプションバイトでLEDを指定する。
      // ⇒ChatpadにはLEDが無いので実装しない。
      // 2Byte目  指定LED
      // Bit2  Caps Lock LED
      // Bit1  Numeric Lock LED
      // Bit0  Scroll Lock LED
      // set/reset LEDs
      ack();
      keyboard.read(&val); //do nothing with the rate
      ack();
      break;
  }
}

void setup() {
  pinMode(LED_PIN, OUTPUT);
  kb.begin(Serial2);

  // establish ps/2 connection with target
  while (keyboard.write(0xAA) != 0) {
    digitalWrite(LED_PIN, HIGH);
    delay(500);
    digitalWrite(LED_PIN, LOW);
    delay(500);
  }
  delay(100);
}

void loop() {
  keyEvent  inkey;        // チャットパッドのからの入力
  uint8_t   c, code;      // 接続先機器からの応答用

  if ( (digitalRead(CLOCK_PIN) == LOW) || (digitalRead(DATA_PIN) == LOW)) {
    while (keyboard.read(&c)) ;
    kbdCmd(c);
  } else {
    if (kb.available() > 0 ) {
      // スキャンコード送信
      //    キーボードのキーが押された時と、キーを離した時にスキャンコードを送信します。
      //    キー・スキャンコードは３種類でデフォルトはスキャンコードセット２です。
      //    これはASCIIコードではなくキー番号（VFキーコード)です。
      //    キーが押された時はMakeコードを、離した時にはBreakコードを送信します。
      //    BreakコードはほとんどがMakeコードに0F0hを加えた2バイトコードで、オートリピートはMakeコードの送信です。
      //    Exp:'A'を押すと'1Ch'送信、離すと'0F0h,1Ch'の2バイト送信になります。

      code = 0;
      inkey = kb.read();
      code = inkey.code;

      if ( code == KEY_NONE)return;
      if ( code == KEY_ERROR)return;

      if ( inkey.KEY == 0) {
        if ( inkey.ALT == 1) {
          if (inkey.BREAK == 0 ) {
            keyboard.write(0x11);  // 左ALTが同時に押されている
          } else {
            keyboard.write(0xF0);  // 左ALTキーを離した
            keyboard.write(0x11);
          }
        }
        else if (inkey.SHIFT == 1 ) {
          if (inkey.BREAK == 0 ) {
            keyboard.write(0x12);  // 左シフトキーが同時に押されている
          } else {
            keyboard.write(0xF0);  // 左シフトキーを離した
            keyboard.write(0x12);
          }
        }
        else if (inkey.CTRL == 1 ) {
          if (inkey.BREAK == 0 ) {
            keyboard.write(0x14);  // 左Ctrlキーが同時に押されている
          } else {
            keyboard.write(0xF0);  // 左Ctrlキーを離した
            keyboard.write(0x14);
          }
        }
      } else {
        if (inkey.FUNC == 1 ) {
          keyboard.write(0xE0);    // 機能キー
        }
      }
      if (inkey.BREAK == 0 ) {
        keyboard.write(code);    // キーが押されている
        digitalWrite(LED_PIN, HIGH);
      } else {
        keyboard.write(0xF0);    // キーを離した
        keyboard.write(code);
        digitalWrite(LED_PIN, LOW);
      }
    }
  }
  kb.GetUp();
}
