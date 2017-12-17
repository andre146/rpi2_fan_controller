#include <stdio.h>
#include <wiringPi.h>
#include <unistd.h>
#include <math.h>
#include <stdlib.h>
#include <signal.h>

#define TEMPFILE_PATH "/sys/class/thermal/thermal_zone0/temp"
#define TEMP_STRING_LEN 5
#define FAN_PIN 1
#define PWM_RANGE 1024
#define SLEEP_TIME 3
#define LOWEST_ERROR -3

#define PROP_GAIN 1
#define INT_GAIN 0.2

float getTemp(){

	FILE *tempFile = fopen(TEMPFILE_PATH, "r");
	char buffer[TEMP_STRING_LEN + 1];
	float temp;

	if(tempFile == NULL){
		return(-1);
	}

	fgets(buffer, TEMP_STRING_LEN, tempFile);
	fclose(tempFile);

	return((float)atof(buffer) / 100);

}

void cleanup(){

	pwmWrite(FAN_PIN, 0);
	printf("Exit...\n");
	exit(0);
}

int main(int argc, char **argv){

	if(argc <= 1){
		printf("Usage: <temp>\n");
		return(1);
	}

	int setTemp = atoi(argv[1]);
	int effort = 0;
	float error = 0;
	float errorSum = 0;
	float propEffort = 0;
	float intEffort = 0;

	signal(SIGINT, cleanup);
	wiringPiSetup();
	pwmSetRange(PWM_RANGE);
	//pwmSetClock(3840);
	pwmSetMode(PWM_MODE_MS);
	pinMode(FAN_PIN, PWM_OUTPUT);

	printf("Starting PI-Based fan controller v1.0 by Andre Picker\n");

	while(1){

		error = getTemp() - setTemp;
		errorSum += error;

		propEffort = PROP_GAIN * error;
		intEffort = INT_GAIN * errorSum * SLEEP_TIME;
		effort = (int)round(propEffort + intEffort);

		if(effort > PWM_RANGE){
			effort = PWM_RANGE;
		} else if(effort < 0){
			effort = 0;
		}

		if(intEffort > PWM_RANGE){
			errorSum = PWM_RANGE / SLEEP_TIME / INT_GAIN;
		}
		if(error <= LOWEST_ERROR){
			errorSum = 0;
		}

		pwmWrite(FAN_PIN, effort);
		sleep(SLEEP_TIME);
	}

	return 0;
}
