fan: fan.c
	gcc -o fan fan.c -lwiringPi -std=c11 -lm -Wall

pwmtest: pwmtest.c
	gcc -o pwmtest pwmtest.c -lwiringPi -std=c11 -lm
