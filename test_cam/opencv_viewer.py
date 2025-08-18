#!/usr/bin/env python3
import cv2
import sys
import time

def main():
    # 카메라 디바이스 (기본값: 0)
    device = 0
    if len(sys.argv) > 1:
        device = sys.argv[1]
    
    # 카메라 열기
    cap = cv2.VideoCapture(device)
    
    if not cap.isOpened():
        print(f"Error: Could not open camera device {device}")
        return
    
    # 카메라 설정
    cap.set(cv2.CAP_PROP_FRAME_WIDTH, 640)
    cap.set(cv2.CAP_PROP_FRAME_HEIGHT, 480)
    cap.set(cv2.CAP_PROP_FPS, 30)
    
    print("Camera opened successfully!")
    print("Press 'q' to quit, 's' to save frame")
    
    frame_count = 0
    start_time = time.time()
    
    while True:
        # 프레임 읽기
        ret, frame = cap.read()
        
        if not ret:
            print("Error: Could not read frame")
            break
        
        frame_count += 1
        
        # FPS 계산
        elapsed_time = time.time() - start_time
        fps = frame_count / elapsed_time if elapsed_time > 0 else 0
        
        # 프레임 정보 표시
        info_text = f"Frame: {frame_count}, FPS: {fps:.1f}, Size: {frame.shape[1]}x{frame.shape[0]}"
        cv2.putText(frame, info_text, (10, 30), cv2.FONT_HERSHEY_SIMPLEX, 0.7, (0, 255, 0), 2)
        
        # 화면에 표시
        cv2.imshow('USB Webcam - Real-time Viewer', frame)
        
        # 키 입력 처리
        key = cv2.waitKey(1) & 0xFF
        if key == ord('q'):
            break
        elif key == ord('s'):
            # 프레임 저장
            filename = f"opencv_frame_{frame_count:06d}.jpg"
            cv2.imwrite(filename, frame)
            print(f"Saved: {filename}")
    
    # 정리
    cap.release()
    cv2.destroyAllWindows()
    print(f"Total frames captured: {frame_count}")
    print(f"Average FPS: {frame_count / elapsed_time:.1f}")

if __name__ == "__main__":
    main() 