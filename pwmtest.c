#include <stdio.h>
#include <wiringPi.h>
#include <stdlib.h>

int main(){

	char *buffer = malloc(10);

	wiringPiSetup();
	pwmSetMode(PWM_MODE_MS);
	pwmSetClock(3840);
	pwmSetRange(100);
	pinMode(1, PWM_OUTPUT);

	while(1){

		printf("Enter Dutycycle: ");
		gets(buffer);
		pwmWrite(1, atoi(buffer));
		printf("\r \t \t \t \r");

	}

	return(0);
}
