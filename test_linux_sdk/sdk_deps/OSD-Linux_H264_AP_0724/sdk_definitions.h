#ifndef SDK_DEFINITIONS_H
#define SDK_DEFINITIONS_H

// 누락된 전역 변수 정의
extern int Dbg_Param;

// 전역 H.264 포맷 변수 (구조체는 h264_xu_ctrls.h에 정의됨)
extern struct H264Format *gH264fmt;

// 기본값 설정
#define DEFAULT_DBG_PARAM 0

#endif // SDK_DEFINITIONS_H
