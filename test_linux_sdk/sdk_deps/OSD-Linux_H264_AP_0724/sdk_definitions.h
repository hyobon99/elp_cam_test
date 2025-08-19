#ifndef SDK_DEFINITIONS_H
#define SDK_DEFINITIONS_H

// 누락된 전역 변수 정의
extern int Dbg_Param;

// H.264 포맷 구조체 정의
struct H264Format {
    unsigned short wWidth;
    unsigned short wHeight;
    unsigned int FrameSize;
    unsigned short fpsCnt;
    unsigned int FrPay[32];  // Frame Payload 배열
};

// 전역 H.264 포맷 변수
extern struct H264Format *gH264fmt;

// 기본값 설정
#define DEFAULT_DBG_PARAM 0

#endif // SDK_DEFINITIONS_H
