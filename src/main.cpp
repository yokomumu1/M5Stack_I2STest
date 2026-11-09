#include <M5Unified.h>      // M5Unified統合ライブラリをインクルード
#include "taskheader.h"     // タスク共通ヘッダーファイルをインクルード
#include "taskI2S.h"        // I2Sタスクのヘッダーファイルをインクルード

void setup() {
  auto cfg = M5.config(); // デフォルト設定を取得
  M5.begin(cfg);          // M5Stackの初期化
  M5.Display.setTextSize(3);        // テキストサイズを2倍に設定
  M5.Display.print("Sound Check!"); // 画面に文字列を表示

  //------------------------------------
  // シリアルポートの初期化
  //------------------------------------
  Serial.begin(115200);   // シリアルポート初期化
  Serial.println("Setup completed");

  //------------------------------------
  // I2Sの初期化
  //------------------------------------
  setupI2S1();

  //------------------------------------
  // キューの作成と初期化
  //------------------------------------
  queueAudio = xQueueCreate(MSG_QUEUE_NUM, sizeof(int16_t *));
  if (queueAudio == NULL) {
    Serial.println("Error creating the queue");
    while (1);
  }

  //------------------------------------
  // タスクの生成
  //------------------------------------
  // I2Sの処理 → Core 1 に割り当て
  //（M5Stackのディスプレイ描画はCore 0で行うため、I2S処理はCore 1に割り当てる）
  xTaskCreatePinnedToCore(
    TaskI2SMain,   // タスクのメイン関数
    "TaskI2S",     // タスク名
    2 * 1024,      // スタックサイズ（バイト単位）
    NULL,          // 引数
    1,             // 優先度
    &tskI2S,       // タスクハンドル
    1              // Core 1
  );
}

void loop() {
  // M5Stackの更新処理（タッチやボタンの状態を更新する）
  M5.update();
    
  // メモリプールからオーディオバッファを確保
  int16_t *ptr = (int16_t *)pvPortMalloc(sizeof(int16_t) * BUFFER_SAMPLES * FRAME_BATCH * 2);

  if (ptr == NULL) {
    // メモリ確保失敗時の処理
    // メッセージキューの数が多すぎて確保できない場合を想定（音切れが無いならば減らすと良いはず）
    Serial.println("[Error] Audio buffer malloc error");
    return;
  }

  // 確保したバッファにサイン波を生成して書き込む
  static float phase = 0.0f;
  float frequency = 440.0f; // サイン波の周波数（Hz）
  float increment = 2.0f * PI * frequency / SAMPLE_RATE;
  for (int i = 0; i < BUFFER_SAMPLES * FRAME_BATCH; i++) {
    ptr[i * 2] = (int16_t)(32767.0f * sin(phase)); // 左チャンネル
    ptr[i * 2 + 1] = (int16_t)(32767.0f * sin(phase)); // 右チャンネル
    phase += increment;
    if (phase >= 2.0f * PI) {
      phase -= 2.0f * PI;
    }
  }

  // レンダリング済みのバッファIndexをキューにつなぐ
  xQueueSend(queueAudio, &ptr, portMAX_DELAY);  // キューがいっぱいなら待機する
}
