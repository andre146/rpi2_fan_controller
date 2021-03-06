#include <stdio.h>
#include <wiringPi.h>
#include <unistd.h>
#include <math.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>

#define TEMPFILE_PATH "/sys/class/thermal/thermal_zone0/temp" //where to get the temp from
#define FAN_PIN 1 //default pin
#define PWM_RANGE 1024 //default pwm range
#define SLEEP_TIME 3
#define CONFIGFILE_PATH "fan.conf"
#define DEFAULT_PROP_GAIN 1
#define DEFAULT_INT_GAIN 0

unsigned char fanPin = FAN_PIN; //sorry I won't do it again I promise...

float getTemp(){ //reads the temperature and returns it in degree celsius

	FILE *tempFile = fopen(TEMPFILE_PATH, "r"); //open the file
	long fileLen;

	if(tempFile == NULL){ //check if opened correctly
                return(-1);
        }

	fseek(tempFile, 0, SEEK_END); //go to the end
	fileLen = ftell(tempFile) + 1;//get the length
	fseek(tempFile, 0, SEEK_SET);//go to the start
	char buffer[fileLen + 1];

	if(buffer == NULL){
	  fclose(tempFile);
	  return(-1);
	}

	fgets(buffer, fileLen, tempFile);
	fclose(tempFile);

	return((float)atof(buffer) / 1000);

}

void cleanup(){ //resets the gpio pins

	pwmWrite(fanPin, 0);
	pinMode(fanPin, INPUT);
	printf("Exit...\n");
	exit(0);
}

int main(int argc, char **argv){

	if(argc <= 1){ //one argument is required which is the set temperatue
	  printf("Usage: <temp>\n");
	  return(1);
	}
	if(geteuid() != 0){ //check if run with root privileges. Running as a normal user leads to a system crash!
	  printf("This program must be run as root!\n");
	  return(1);
	}

	/*declaring and initializing some variables*/
	
	int setTemp = atoi(argv[1]);
	int effort = 0; // the controller output
	int sleepTime = SLEEP_TIME;
	float error = 0; // error, in this case actual value - setpoint
	float errorSum = 0; // error sum for integration
	float propEffort = 0; // proportional output
	float intEffort = 0; // integral output
	float tmpIntEffort = 0; //temporary integral output storage
	float propGain = DEFAULT_PROP_GAIN; // the proportinal controller gain
	float intGain = DEFAULT_INT_GAIN; // the integral controller gain

	printf("Starting PI-Based fan controller by Andre Picker\n");

	/*
	Now follows the parsing of the config file
	*/

	FILE *confFile = fopen(CONFIGFILE_PATH, "rb"); 
	long fileLen = 0;
	long filePos = 0;
	char *lineBuffer; //later holds one single line
	char *cmdBuffer; //contains the variable to be set
	char *argBuffer; //contains the value

	if(confFile == NULL){
	  printf("Failed to open config file %s!\n", CONFIGFILE_PATH);
		return(1);
	}

	fseek(confFile, 0, SEEK_END); //getting file length
	fileLen = ftell(confFile) + 1;
	fseek(confFile, 0, SEEK_SET);
	lineBuffer = malloc(fileLen + 1); //allocating enough memory
	cmdBuffer = malloc(fileLen + 1);
	argBuffer = malloc(fileLen + 1);

	while(filePos < fileLen - 1){ //walking through the file line by line by utilising fgets() which stops at \n
		fgets(lineBuffer, fileLen, confFile);
		filePos = ftell(confFile);

		if(lineBuffer != NULL || lineBuffer[0] != '#'){ //ignores line if it is a comment or NULL
			cmdBuffer = strtok(lineBuffer, " "); // split at space character
			argBuffer = strtok(NULL, " ");
			
			if(argBuffer == NULL || cmdBuffer == NULL){
				continue;
			}
			if(strcmp(cmdBuffer, "sleep") == 0){ //switch depending on variable
				sleepTime = atoi(argBuffer);
			} else if (strcmp(cmdBuffer, "fanPin") == 0){
				fanPin = atoi(argBuffer);
			} else if (strcmp(cmdBuffer, "propGain") == 0){
                         	propGain = (float)atof(argBuffer);
                        } else if (strcmp(cmdBuffer, "intGain") == 0){
                                intGain = (float)atof(argBuffer);
                        }
		}
	}

//	free(lineBuffer); //does not work for some reason
	free(cmdBuffer);
	free(argBuffer);
	fclose(confFile);

	printf("Fan Pin: %i\n", fanPin);
	printf("Sleeptime: %i\n", sleepTime);
	printf("Proportional gain: %.4f\n", propGain);
	printf("Integral gain: %.4f\n\n", intGain);

	signal(SIGINT, cleanup); //initializing a signal handler to call cleanup() when an interrupt signal is sent

        wiringPiSetup(); //setting up the wiringPi lib
        pwmSetRange(PWM_RANGE);
//	pwmSetClock(3840); //does this even work at all?
        pwmSetMode(PWM_MODE_MS);
	pinMode(fanPin, PWM_OUTPUT);

	while(1){

		error = getTemp() - setTemp;
		errorSum += error;

		propEffort = propGain * error;
		intEffort = intGain * errorSum * sleepTime;
		effort = (int)round(propEffort + intEffort);

		if(effort > PWM_RANGE){ //clamp the effort to the range of the pwm value
			effort = PWM_RANGE;
			intEffort = tmpIntEffort;
		} else if(effort < 0){
			effort = 0;
			intEffort = tmpIntEffort;
		} else{
			tmpIntEffort = intEffort;
		}	
		
		printf("\rEffort: %i\t", effort);
		fflush(stdout);
		pwmWrite(fanPin, effort);
		sleep(sleepTime);
	}

	return 0;
}
