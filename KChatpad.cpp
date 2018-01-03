//
// ファイル: KChatpad.cpp
// 概    要: Arduino STM32 Xbox Chatpad Clone PS/2 ライブラリ用
// このソースコードは次のソースコードを参考に作成しました
//   たま吉さん      https://github.com/Tamakichi/ArduinoSTM32_PS2Keyboard
//   Cliff L. Biffle http://cliffle.com/project/chatpad
// 作 成 者: Kei Takagi
// 作 成 日: 2017/10/10
// Special Thanks：
//         おおさかJR3
//
// 2017/12/10 チャットパッド日本語キーボード対応

#include "HardwareSerial.h"
#include "wiring_private.h"
#include "wirish_time.h"
#include "KChatpad.h"

static const uint8_t ShiftMask = (1 << 0);
static const uint8_t GreenSquareMask = (1 << 1);
static const uint8_t OrangeCircleMask = (1 << 2);
static const uint8_t PeopleMask = (1 << 3);

// Xbox Chatpad 初期化コマンド
static const uint8_t InitMessage[]      = { 0x87, 0x02, 0x8C, 0x1F, 0xCC };
// Xbox Chatpad 起きているか監視コマンド
static const uint8_t KeepAwakeMessage[]  = { 0x87, 0x02, 0x8C, 0x1B, 0xD0 };

// Normal, Shift  , Ctrl , Alt    , Win
// Normal, Shift  , Green, Orange , People
uint16_t AsciiTableJP[] PROGMEM = {
  0x003D  , 0x013D  ,  0x0000 ,  0x0000 ,  0x0000 ,   /* 11 :7 */
  0x0036  , 0x0136  ,  0x0000 ,  0x0000 ,  0x0000 ,   /* 12 :6 */
  0x002E  , 0x012E  ,  0x0000 ,  0x0000 ,  0x0000 ,   /* 13 :5 */
  0x0025  , 0x0125  ,  0x0000 ,  0x0000 ,  0x0000 ,   /* 14 :4 */
  0x0026  , 0x0126  ,  0x0000 ,  0x0000 ,  0x0000 ,   /* 15 :3 */
  0x001E  , 0x011E  ,  0x0000 ,  0x0000 ,  0x0000 ,   /* 16 :2 */
  0x0016  , 0x0116  ,  0x0000 ,  0x0000 ,  0x0000 ,   /* 17 :1 */
  0x0000  , 0x0000  ,  0x0000 ,  0x0000 ,  0x0000 ,   /* 18 Unused */

  0x003C  , 0x013C  ,  0x013E ,  0x014C ,  0x0000 ,   /* 21 :U */
  0x0035  , 0x0135  ,  0x0136 ,  0x0000 ,  0x0000 ,   /* 22 :Y */
  0x002C  , 0x012C  ,  0x012E ,  0x016A ,  0x000D ,   /* 23 :T (0x000D:TAB)*/
  0x002D  , 0x012D  ,  0x0125 ,  0x0000 ,  0x0000 ,   /* 24 :R */
  0x0024  , 0x0124  ,  0x0126 ,  0x0000 ,  0x0076 ,   /* 25 :E */
  0x001D  , 0x011D  ,  0x011E ,  0x013D ,  0x0000 ,   /* 26 :W */
  0x0015  , 0x0115  ,  0x0151 ,  0x0054 ,  0x0000 ,   /* 27 :Q */
  0x0000  , 0x0000  ,  0x0000 ,  0x0000 ,  0x0000 ,   /* 28 Unused */

  0x003B  , 0x013B  ,  0x0051 ,  0x015B ,  0x0000 ,   /* 31 :J */
  0x0033  , 0x0133  ,  0x005D ,  0x0149 ,  0x0000 ,   /* 32 :H */
  0x0034  , 0x0134  ,  0x005B ,  0x0141 ,  0x0000 ,   /* 33 :G */
  0x002b  , 0x012b  ,  0x0051 ,  0x0000 ,  0x0000 ,   /* 34 :F */
  0x0023  , 0x0123  ,  0x0000 ,  0x0000 ,  0x0000 ,   /* 35 :D */
  0x001B  , 0x011B  ,  0x0000 ,  0x0000 ,  0x0000 ,   /* 36 :S */
  0x001C  , 0x011C  ,  0x0000 ,  0x0000 ,  0x0000 ,   /* 37 :A */
  0x0000  , 0x0000  ,  0x0000 ,  0x0000 ,  0x0000 ,   /* 38 Unused */

  0x0031  , 0x0131  ,  0x013D ,  0x0000 ,  0x0000 ,   /* 41 :N */
  0x0032  , 0x0132  ,  0x014A ,  0x0000 ,  0x0000 ,   /* 42 :B */
  0x002A  , 0x012A  ,  0x0116 ,  0x0000 ,  0x0000 ,   /* 43 :V */
  0x0021  , 0x0121  ,  0x0000 ,  0x0000 ,  0x0000 ,   /* 44 :C */
  0x0022  , 0x0122  ,  0x0000 ,  0x0000 ,  0x0000 ,   /* 45 :X */
  0x001A  , 0x011A  ,  0x0000 ,  0x0000 ,  0x0000 ,   /* 46 :Z */
  0x0000  , 0x0000  ,  0x0000 ,  0x0000 ,  0x0000 ,   /* 47 Unused */
  0x0000  , 0x0000  ,  0x0000 ,  0x0000 ,  0x0000 ,   /* 48 Unused */

  0xE06B  , 0x0000  ,  0xE072 ,  0x0000 ,  0x0000 ,   /* 51 :Left DOWN */
  0x003A  , 0x013A  ,  0x0049 ,  0x0041 ,  0x0000 ,   /* 52 :M */
  0x0049  , 0x0000  ,  0x0000 ,  0x0000 ,  0x0000 ,   /* 53 :カタ */
  0x0029  , 0x0000  ,  0x0000 ,  0x0000 ,  0x0000 ,   /* 54 :Space  */
  0x0066  , 0x0000  ,  0x0000 ,  0x0000 ,  0x0000 ,   /* 55 :Backspace */
  0x0000  , 0x0000  ,  0x0000 ,  0x0000 ,  0x0000 ,   /* 56 Unused */
  0x0000  , 0x0000  ,  0x0000 ,  0x0000 ,  0x0000 ,   /* 57 Unused */
  0x0000  , 0x0000  ,  0x0000 ,  0x0000 ,  0x0000 ,   /* 58 Unused */

  0x0000  , 0x0000  ,  0x0000 ,  0x0000 ,  0x0000 ,   /* 61 Unused */
  0x0041  , 0x0000  ,  0x0000 ,  0x0000 ,  0x0000 ,   /* 62 :漢字 */
  0x005A  , 0x005A  ,  0x005A ,  0x005A ,  0x005A ,   /* 63 :Enter */
  0x004d  , 0x014d  ,  0x004E ,  0x014E ,  0x0000 ,   /* 64 :P */
  0x0045  , 0x0000  ,  0x0000 ,  0x0000 ,  0x0000 ,   /* 65 :0 */
  0x0046  , 0x0146  ,  0x0000 ,  0x0000 ,  0x0000 ,   /* 66 :9 */
  0x003E  , 0x013E  ,  0x0000 ,  0x0000 ,  0x0000 ,   /* 67 :8 */
  0x0000  , 0x0000  ,  0x0000 ,  0x0000 ,  0x0000 ,   /* 68 Unused */

  0xE074  , 0x0000  ,  0xE075 ,  0x0000 ,  0x0000 ,   /* 71 :Right  UP*/
  0x004B  , 0x014B  ,  0x0052 ,  0x004C ,  0x024B ,   /* 72 :L */
  0x0000  , 0x0000  ,  0x0000 ,  0x0000 ,  0x0000 ,   /* 73 Unused */
  0x0000  , 0x0000  ,  0x0000 ,  0x0000 ,  0x0000 ,   /* 74 Unused */
  0x0044  , 0x0144  ,  0x0055 ,  0x0155 ,  0x0000 ,   /* 75 :O */
  0x0043  , 0x0143  ,  0x0146 ,  0x0152 ,  0x0000 ,   /* 76 :I */
  0x0042  , 0x0142  ,  0x004A ,  0x015D ,  0x0000 ,   /* 77 :K */
  0x0000  , 0x0000  ,  0x0000 ,  0x0000 ,  0x0000     /* 78 Unused */
};

// US Key
// Normal, Shift  , Ctrl , Alt    , Win
// Normal, Shift  , Green, Orange , People
static const uint16_t AsciiTableUS[] PROGMEM = {
  0x003D  , 0x013D  ,  0x0000 ,  0x0000 ,  0x0000 ,   /* 11 :7 */
  0x0036  , 0x0136  ,  0x0000 ,  0x0000 ,  0x0000 ,   /* 12 :6 */
  0x002E  , 0x012E  ,  0x0000 ,  0x0000 ,  0x0000 ,   /* 13 :5 */
  0x0025  , 0x0125  ,  0x0000 ,  0x0000 ,  0x0000 ,   /* 14 :4 */
  0x0026  , 0x0126  ,  0x0000 ,  0x0000 ,  0x0000 ,   /* 15 :3 */
  0x001E  , 0x011E  ,  0x0000 ,  0x0000 ,  0x0000 ,   /* 16 :2 */
  0x0016  , 0x0116  ,  0x0000 ,  0x0000 ,  0x0000 ,   /* 17 :1 */
  0x0000  , 0x0000  ,  0x0000 ,  0x0000 ,  0x0000 ,   /* 18 Unused */

  0x003C  , 0x013C  ,  0x0136 ,  0x0000 ,  0x0000 ,   /* 21 :U */
  0x0035  , 0x0135  ,  0x0055 ,  0x0000 ,  0x0000 ,   /* 22 :Y */
  0x002C  , 0x012C  ,  0x012E ,  0x0000 ,  0x000D ,   /* 23 :T (0x000D:TAB)*/
  0x002D  , 0x012D  ,  0x0126 ,  0x0125 ,  0x0000 ,   /* 24 :R */
  0x0024  , 0x0124  ,  0x0000 ,  0x0000 ,  0x0076 ,   /* 25 :E */
  0x001D  , 0x011D  ,  0x0054 ,  0x0000 ,  0x0000 ,   /* 26 :W */
  0x0015  , 0x0115  ,  0x0116 ,  0x0000 ,  0x0000 ,   /* 27 :Q */
  0x0000  , 0x0000  ,  0x0000 ,  0x0000 ,  0x0000 ,   /* 28 Unused */

  0x003B  , 0x013B  ,  0x013D ,  0x011E ,  0x0000 ,   /* 31 :J */
  0x0033  , 0x0133  ,  0x004A ,  0x0051 ,  0x0000 ,   /* 32 :H */
  0x0034  , 0x0134  ,  0x0000 ,  0x006A ,  0x0000 ,   /* 33 :G */
  0x002b  , 0x012b  ,  0x015D ,  0x0000 ,  0x0000 ,   /* 34 :F */
  0x0023  , 0x0123  ,  0x015B ,  0x0000 ,  0x0000 ,   /* 35 :D */
  0x001B  , 0x011B  ,  0x0000 ,  0x0000 ,  0x0000 ,   /* 36 :S */
  0x001C  , 0x011C  ,  0x0155 ,  0x0000 ,  0x0000 ,   /* 37 :A */
  0x0000  , 0x0000  ,  0x0000 ,  0x0000 ,  0x0000 ,   /* 38 Unused */

  0x0031  , 0x0131  ,  0x0141 ,  0x0000 ,  0x0000 ,   /* 41 :N */
  0x0032  , 0x0132  ,  0x016A ,  0x014C ,  0x0000 ,   /* 42 :B */
  0x002A  , 0x012A  ,  0x004E ,  0x0151 ,  0x0000 ,   /* 43 :V */
  0x0021  , 0x0121  ,  0x0000 ,  0x0000 ,  0x0000 ,   /* 44 :C */
  0x0022  , 0x0122  ,  0x0000 ,  0x0000 ,  0x0000 ,   /* 45 :X */
  0x001A  , 0x011A  ,  0x0000 ,  0x0000 ,  0x0000 ,   /* 46 :Z */
  0x0000  , 0x0000  ,  0x0000 ,  0x0000 ,  0x0000 ,   /* 47 Unused */
  0x0000  , 0x0000  ,  0x0000 ,  0x0000 ,  0x0000 ,   /* 48 Unused */

  0xE074  , 0x0000  ,  0xE075 ,  0x0000 ,  0x0000 ,   /* 51 :Right  UP*/
  0x003A  , 0x013A  ,  0x0149 ,  0x0000 ,  0x0000 ,   /* 52 :M */
  0x0049  , 0x0000  ,  0x014A ,  0x0000 ,  0x0000 ,   /* 53 :Period (.) */
  0x0029  , 0x0000  ,  0x0000 ,  0x0000 ,  0x0000 ,   /* 54 :Space  */
  0xE06B  , 0x0000  ,  0xE072 ,  0x0000 ,  0x0000 ,   /* 55 :Left DOWN */
  0x0000  , 0x0000  ,  0x0000 ,  0x0000 ,  0x0000 ,   /* 56 Unused */
  0x0000  , 0x0000  ,  0x0000 ,  0x0000 ,  0x0000 ,   /* 57 Unused */
  0x0000  , 0x0000  ,  0x0000 ,  0x0000 ,  0x0000 ,   /* 58 Unused */

  0x0000  , 0x0000  ,  0x0000 ,  0x0000 ,  0x0000 ,   /* 61 Unused */
  0x0041  , 0x0000  ,  0x0052 ,  0x004C ,  0x0000 ,   /* 62 :Comma(,) */
  0x005A  , 0x005A  ,  0x005A ,  0x005A ,  0x005A ,   /* 63 :Enter */
  0x004d  , 0x014d  ,  0x0146 ,  0x014E ,  0x0000 ,   /* 64 :P */
  0x0045  , 0x0000  ,  0x0000 ,  0x0000 ,  0x0000 ,   /* 65 :0 */
  0x0046  , 0x0146  ,  0x0000 ,  0x0000 ,  0x0000 ,   /* 66 :9 */
  0x003E  , 0x013E  ,  0x0000 ,  0x0000 ,  0x0000 ,   /* 67 :8 */
  0x0000  , 0x0000  ,  0x0000 ,  0x0000 ,  0x0000 ,   /* 68 Unused */

  0x0066  , 0x0000  ,  0x0000 ,  0x0000 ,  0x0000 ,   /* 71 :Backspace */
  0x004B  , 0x014B  ,  0x005D ,  0x0000 ,  0x024B ,   /* 72 :L */
  0x0000  , 0x0000  ,  0x0000 ,  0x0000 ,  0x0000 ,   /* 73 Unused */
  0x0000  , 0x0000  ,  0x0000 ,  0x0000 ,  0x0000 ,   /* 74 Unused */
  0x0044  , 0x0144  ,  0x013E ,  0x0000 ,  0x0000 ,   /* 75 :O */
  0x0043  , 0x0143  ,  0x0152 ,  0x0000 ,  0x0000 ,   /* 76 :I */
  0x0042  , 0x0142  ,  0x005B ,  0x0000 ,  0x0000 ,   /* 77 :K */
  0x0000  , 0x0000  ,  0x0000 ,  0x0000 ,  0x0000     /* 78 Unused */
};

// KEYコード判定用
uint8_t keyCode[] = {
  0x1c,// a
  0x32,// b
  0x21,// c
  0x23,// d
  0x24,// e
  0x2b,// f
  0x34,// g
  0x33,// h
  0x43,// i
  0x3b,// j
  0x42,// k
  0x4b,// l
  0x3a,// m
  0x31,// n
  0x44,// o
  0x4d,// p
  0x15,// q
  0x2d,// r
  0x1b,// s
  0x2c,// t
  0x3c,// u
  0x2a,// v
  0x1d,// w
  0x22,// x
  0x35,// y
  0x1a,// z
  0x45,// 0
  0x16,// 1
  0x1e,// 2
  0x26,// 3
  0x25,// 4
  0x2e,// 5
  0x36,// 6
  0x3d,// 7
  0x3e,// 8
  0x46,// 9
  0x4E,// -
  0x55,// ^
  0x6A,// \
  0x54,// @
  0x5B,// [
  0x4C,// ;
  0x52,// :
  0x5D,// ]
  0x41,// ,
  0x49,// .
  0x4A,// /
  0x51,// Unused
  0x00 //END
};

//
// 利用開始(初期化)
// 引数
//   serial    : シリアル番号
// 戻り値
//  0:正常終了 0以外: 異常終了
uint8_t KChatpad::begin(HardwareSerial &serial) {
  return begin(serial, (int)US);
}

// 利用開始(初期化)
// 引数
//   serial       : シリアル番号
//   keyboardType : キーボードタイプ
// 戻り値
//  0:正常終了 0以外: 異常終了
uint8_t KChatpad::begin(HardwareSerial &serial, int keyboardType) {
  uint8_t i, err = 0;
  err = init();
  if ( err != 0 )goto ERROR;
  _serial = &serial;
  _keyboardType = keyboardType;

  // 電源投入後500ms以上待って
  // InitMessage（87 02 8C 1F CC）を送信する必要があります
  // ※Xbox Chatpad clone (TYX-517PCB1 VER2.5 2015-12-08)
  // では初期化を3回しないと、初期化の応答受けても正常な動作を確認できませんでした
  // 動作しない場合は初期化の回数、時間を調整してみてください
  // Cliff L. Biffle氏は500ms待たずに初期を1回だけ実行しています
  // またキーボードが使用できるまで電源ONから約5秒必要です

  delay(500);
  for (i = 0; i < 3; i++) {
    delay(30);
    _serial->begin(19200);
    while (!_serial) delay(100);
    _serial->write(InitMessage, sizeof(InitMessage));
  }

ERROR:
  return err;
}

// 利用終了
void KChatpad::end() {
  if (_serial == NULL)return;
  _serial->end();
  return;
}

// キーボード初期化
// 戻り値 0:正常終了、 0以外:異常終了
uint8_t KChatpad::init() {
  _serial = NULL;
  _last_key0 = 0;
  _last_key1 = 0;
  _last_ping = 0;
  return 0;
}

// シリアルポートに何バイトのデータが到着しているかを返します。
// 戻り値 シリアルバッファにあるデータのバイト数を返します
int KChatpad::available(void) {
  if (_serial == NULL)return 0;
  return _serial->available();
}

//
// 入力キー情報の取得(CapsLock、NumLock、ScrollLockを考慮)
// 仕様
//  ・入力したキーに対応するASCIIコード文字またはキーコードと付随する情報を返す。
//    - エラーまたは入力なしの場合、下位8ビットに以下の値を設定する
//     0x00で入力なし、0xFFでエラー
//
// 戻り値: 入力情報
// キー入力情報(キーイベント構造体のメンバーは下記の通り)
// k.code  : アスキーコード or キーコード(AsciiTable参照)
//            0の場合は入力無し、255の場合はエラー
// k.BREAK : キー押し情報                  => 0: 押した、　　　　1:離した
// k.KEY   : キーコード/アスキーコード種別 => 0: アスキーコード、1:キーコード
// k.SHIFT : SHIFTキー押し判定             => 0: 押していない 、 1:押している
// k.CTRL  : CTRLキー押し判定              => 0: 押していない 、 1:押している
// k.ALT   : ALTキー押し判定               => 0: 押していない 、 1:押している
// k.GUI   : GUI(Windowsキー)押し判定      => 0: 押していない 、 1:押している
//
keyEvent KChatpad::read() {
  keyinfo  in = {.value = 0};// キーボード状態
  uint8_t i, code, checksum = 0, err, function;
  uint16_t index, inCode;
  int len;

  if (_serial == NULL)goto ERROR;

  // キーコードの初期化
  in.value = KEY_NONE;
  in.kevt.KEY = 1;

  // チャットパッドは、最大2つの同時キーを検出できます。
  // 押された1番目のキーのキーコードが4バイト目
  // 2つのキーが押されていると、押された2番目のキーのキーコードが5バイト目に格納されます
  len = available();
  if (8 <= len ) {
    //Chatpadからシリアル通信で8バイトのバケットを受け取ります。
    err = 1;
    for (i = 0; i < 8; i++) {
      //受信データを1バイト読み込みますが、バッファ中の読み取り位置は変更しません
      //受信データから0xB4を見つけるまでパケットを捨てます
      if ( _serial->peek() == 0xB4 ) {
        if (i == 0) {
          err = 0;
          break;
        }
      } else {
        _serial->read();
      }
    }
    if (err == 1)goto ERROR;

    //Chatpadからシリアル通信で8バイトのバケットを受け取ります。
    for (i = 0; i < 8; i++) _buffer[i] = _serial->read();
    //0xA5で始まる「ステータスレポート」パケットは使い方が不明なので破棄します。
    //0xB4から始まらないパケットは無視します
    //2バイト目が0xC5以外のパケットも無視します
    if (_buffer[1] != 0xC5) goto ERROR;

    //チェックサムの確認
    //7バイト目はチェックサムです。
    //0-6バイトを合計し、結果を否定（2の補数）することによって計算されます。
    for (i = 0; i < 7; i++) checksum += _buffer[i];
    checksum = -checksum;
    if (checksum != _buffer[7])goto ERROR;

    //正常なパケットの処理
    uint8_t modifiers = _buffer[3];
    uint8_t key0      = _buffer[4];
    uint8_t key1      = _buffer[5];

    code = 0;
    if (key0 && key0 != _last_key0 && key0 != _last_key1) {
      code = key0;
      in.kevt.BREAK = 0;
    }
    if (key1 && key1 != _last_key0 && key1 != _last_key1) {
      code = key1;
      in.kevt.BREAK = 0;
    }
    if (_last_key0 && _last_key0 != key0 && _last_key0 != key1) {
      code = _last_key0;
      in.kevt.BREAK = 1;
    }
    if (_last_key1 && _last_key1 != key0 && _last_key1 != key1) {
      code = _last_key1;
      in.kevt.BREAK = 1;
    }
    _last_key0 = key0;
    _last_key1 = key1;

    index = (((code - 0x11) & 0x70) >> 1) | ((code - 0x11) & 0x7);
    if (_keyboardType == JP) {
      if (index >= (sizeof(AsciiTableJP) / 5)) goto ERROR;
    } else {
      if (index >= (sizeof(AsciiTableUS) / 5)) goto ERROR;
    }


    in.kevt.SHIFT = 0;  // Shiftキー
    in.kevt.CTRL  = 0;  // Ctrlキー
    in.kevt.ALT   = 0;  // Altキー
    in.kevt.GUI   = 0;  // Windowsキー
    in.kevt.FUNC  = 0;  // 機能キー

    //Xbox Chatpadの入力値を文字コードに変換
    index = index * 5;
    if ( modifiers & ShiftMask )        index += 1; // Shiftキー ON
    if ( modifiers & GreenSquareMask )  index += 2; // Ctrlキー ON
    if ( modifiers & OrangeCircleMask ) index += 3; // Altキー ON
    if ( modifiers & PeopleMask )       index += 4; // Windowsキー ON

    in.kevt.code =  0;
    if (_keyboardType == JP) {
      inCode = pgm_read_word_near(AsciiTableJP + index);//2バイト読み込み
    } else {
      inCode = pgm_read_word_near(AsciiTableUS + index);//2バイト読み込み
    }
    in.kevt.code = inCode & 0x00FF;
    function = inCode >> 8;

    if ( function == 0xE0 )in.kevt.FUNC           = 1; // 機能キー ON
    else if ( function & ShiftMask )in.kevt.SHIFT      = 1; // Shiftキー ON
    else if ( function & GreenSquareMask )in.kevt.CTRL = 1; // Ctrlキー ON
    else if ( function & OrangeCircleMask )in.kevt.ALT = 1; // Altキー ON
    else if ( function & PeopleMask )in.kevt.GUI       = 1; // Windowsキー ON

    for (i = 0; keyCode[i] != 0x00; i++) {
      if (keyCode[i] == in.kevt.code) {
        in.kevt.KEY = 0;//表示可能文字コード
        break;
      }
    }
  }

  goto DONE;
ERROR:
  in.value = KEY_ERROR;

DONE:
  GetUp();
  return in.kevt;
}


// 起きろコマンド送信
void KChatpad::GetUp() {
  if (_serial == NULL)return;
  // KeepAwakeMessageを定期的に送信する必要があります。
  // 送信しない場合、チャットパッドはスリープ状態に戻ります。
  // 毎秒KeepAwakeMessageを送信します。
  uint32_t time = millis();
  if (time - _last_ping > 1000) {
    _last_ping = time;
    _serial->write(KeepAwakeMessage, sizeof(KeepAwakeMessage));
  }
}
