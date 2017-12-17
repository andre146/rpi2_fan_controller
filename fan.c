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
#define LOWEST_ERROR -5
#define CONFIGFILE_PATH "fan.conf"
#define LINE_BUF_LEN 1024
#define DEFAULT_PROP_GAIN 1
#define DEFAULT_INT_GAIN 1

float getTemp(){

	FILE *tempFile = fopen(TEMPFILE_PATH, "r");
	char buffer[TEMP_STRING_LEN + 1];

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
	int sleepTime = SLEEP_TIME;
	char fanPin = FAN_PIN;
	float lowestError = LOWEST_ERROR;
	float error = 0;
	float errorSum = 0;
	float propEffort = 0;
	float intEffort = 0;
	float propGain = DEFAULT_PROP_GAIN;
	float intGain = DEFAULT_INT_GAIN;

	printf("Starting PI-Based fan controller v1.0 by Andre Picker\n");

	FILE *confFile = fopen(CONFIGFILE_PATH, "rb");
	long fileLen = 0;
	long filePos = 0;
	char *lineBuffer;
	char *cmdBuffer;
	char *argBuffer;
	if(confFile == NULL){
		printf("Failed to open config file fan.conf!\n");
		return(1);
	}

	fseek(confFile, 0, SEEK_END);
	fileLen = ftell(confFile) + 1;
	fseek(confFile, 0, SEEK_SET);
	lineBuffer = malloc(fileLen + 1);
	cmdBuffer = malloc(fileLen + 1);
	argBuffer = malloc(fileLen + 1);
	printf("Debug0\n");
	while(filePos < fileLen - 1){
		fgets(lineBuffer, fileLen, confFile);
		printf("reading...\n");
		if(lineBuffer[0] != '#'){
			cmdBuffer = strtok(lineBuffer, " ");
			argBuffer = strtok(NULL, " ");

			if(strcmp(cmdBuffer, "sleep") == 0){
				sleepTime = atoi(argBuffer);
				printf("Sleeptime: %i\n", sleepTime);
			} else if (strcmp(cmdBuffer, "fanPin") == 0){
				fanPin = atoi(argBuffer);
				printf("Fan Pin: %i\n", fanPin);
			} else if (strcmp(cmdBuffer, "propGain") == 0){
                                propGain = (float)atof(argBuffer);
				printf("Proportional gain: %f\n", propGain);
                        } else if (strcmp(cmdBuffer, "intGain") == 0){
                                intGain = (float)atof(argBuffer);
				printf("Integral gain: %f\n", intGain);
                        } else if (strcmp(cmdBuffer, "lowestError") == 0){
                                lowestError = atof(argBuffer);
				printf("Lowest Error: %f\n", lowestError);
                        }
		}

	}

	free(lineBuffer);
	free(cmdBuffer);
	free(argBuffer);
	fclose(confFile);

	printf("Fan Pin: %i\n", fanPin);
	printf("Sleeptime: %i\n", sleepTime);
	printf("Proportional gain: %f\n", propGain);
	printf("Integral gain: %f\n", intGain);
	printf("Lowest Error: %f\n", lowestError);

	signal(SIGINT, cleanup);
        wiringPiSetup();
        pwmSetRange(PWM_RANGE);
        //pwmSetClock(3840);
        pwmSetMode(PWM_MODE_MS);
        pinMode(fanPin, PWM_OUTPUT);

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
