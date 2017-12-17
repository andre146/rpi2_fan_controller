#include <stdio.h>
#include <wiringPi.h>
#include <unistd.h>
#include <math.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>

#define TEMPFILE_PATH "/sys/class/thermal/thermal_zone0/temp"
#define TEMP_STRING_LEN 5
#define FAN_PIN 1
#define PWM_RANGE 1024
#define SLEEP_TIME 3
#define LOWEST_ERROR -3
#define CONFIGFILE_PATH "fan.conf"
#define LINE_BUF_LEN 1024

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

	pwmWrite(1, 0);
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
	int sleepTime = 5;
	char fanPin = 1;
	float error = 0;
	float errorSum = 0;
	float propEffort = 0;
	float intEffort = 0;
	float propGain = 1;
	float intGain = 1;

	signal(SIGINT, cleanup);
	wiringPiSetup();
	pwmSetRange(PWM_RANGE);
	//pwmSetClock(3840);
	pwmSetMode(PWM_MODE_MS);
	pinMode(fanPin, PWM_OUTPUT);

	printf("Starting PI-Based fan controller v1.0 by Andre Picker\n");

	FILE *confFile = fopen(CONFIGFILE_PATH, "rb");
	long fileLen;
	char *fileBuffer;
	char *lineBuffer;
	char *cmdBuffer;
	if(confFile == NULL){
		printf("Failed to open config file fan.conf!\n");
		return(1);
	}

	fseek(confFile, 0, SEEK_END);
	fileLen = ftell(confFile) + 1;
	fseek(confFile, 0, SEEK_SET);
	fileBuffer = malloc(fileLen + 1);
	fread(fileBuffer, 1, fileLen, confFile);
	fclose(confFile);
	lineBuffer = malloc(LINE_BUF_LEN);
	cmdBuffer = malloc(LINE_BUF_LEN);

	lineBuffer = strtok(fileBuffer, "\n");


	while(1){

		error = getTemp() - setTemp;
		errorSum += error;

		propEffort = propGain * error;
		intEffort = intGain * errorSum * sleepTime;
		effort = (int)round(propEffort + intEffort);

		if(effort > PWM_RANGE){
			effort = PWM_RANGE;
		} else if(effort < 0){
			effort = 0;
		}

		if(intEffort > PWM_RANGE){
			errorSum = PWM_RANGE / sleepTime / intGain;
		}
		if(error <= LOWEST_ERROR){
			errorSum = 0;
		}

		pwmWrite(fanPin, effort);
		sleep(sleepTime);
	}

	return 0;
}
