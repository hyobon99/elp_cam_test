main: main.o  v4l2uvc.o  rervision_xu_ctrls.o
	gcc -o main main.o  v4l2uvc.o rervision_xu_ctrls.o 
main.o: main.c  v4l2uvc.h  
	gcc -c main.c  
v4l2uvc.o: v4l2uvc.c v4l2uvc.h  
	gcc -c v4l2uvc.c 
rervision_xu_ctrls.o: rervision_xu_ctrls.c rervision_xu_ctrls.h  
	gcc -c rervision_xu_ctrls.c 
clean :
	rm main main.o  v4l2uvc.o rervision_xu_ctrls.o *.avi
	
