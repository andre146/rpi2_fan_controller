fan: fan.c
	gcc -o fan fan.c -lwiringPi -std=c11 -lm

pwmtest: pwmtest.c
	gcc -o pwmtest pwmtest.c -lwiringPi -std=c11 -lm
