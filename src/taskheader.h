// タスク間で共有するオブジェクトの定義
#ifndef _TASKHEADER_H
#define _TASKHEADER_H

#include <Arduino.h>

// オーディオ関連定義
#define SAMPLE_RATE     48000
#define FRAME_SAMPLES   4             // Number of samples per frame
#define BUFFER_SAMPLES  FRAME_SAMPLES // Double buffer number per frame
#define MSG_QUEUE_NUM   8             // メッセージキューの数（メモリ確保の関係でFRAME_SAMPLES以下にすること）
#define CHIP_CLOCK      (4 * 1000 * 1000) // 4MHz
#define YM2608_CLOCK    7987200UL     // 8MHz
#define FRAME_BATCH     16            // まとめてDMA転送するフレーム数

extern QueueHandle_t queueAudio;    // AudioタスクとI2Sタスクで共有するキューの定義

#endif
