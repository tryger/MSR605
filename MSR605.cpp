/*
 *  MSR605.cpp
 * 
 *  MSR605 Application, for operating a MSR605 magstripe reader/writer.
 *  Copyright (C) 2013 Pentura
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <iostream>
#include <stdio.h>
#include <signal.h>
#include <cstdlib>
#include <stdlib.h>
#include <string.h>
#include "msr605.h"

#define DEVICE "/dev/ttyUSB0"

static const char* VERSION="0.1";

MSR605 *msr = new MSR605();

void sigproc(int x)
{ 		
		printf("Quitting...");
		msr->disconnect();
		delete(msr);
		exit(0);
}

void printTrack(const char *msg, unsigned char *buf, unsigned int len)
{
	printf("size: %d",len);
	printf("[%s]: ", msg);
	for(int x = 0; x < len; x++) printf("%02X", buf[x]);
	printf("\n");
}

void printTrackiso(const char *msg, unsigned char *buf, unsigned int len)
{
	printf("size: %d",len);
	printf("[%s]: ", msg);
	for(int x = 0; x < len; x++) printf("%c", buf[x]);
	printf("\n");
}

void license(){
	printf("MSR605 %s Copyright (C) 2013 Pentura \nThis program comes with ABSOLUTELY NO WARRANTY; \nThis is free software, and you are welcome to redistribute it under certain conditions;\n\n",VERSION);
}

int main (int argc, const char * const argv[]) {
  
	if ( argc > 7 || argc < 6 ){
	      license();
	      printf( "\tusage: %s <track1 bit> <track2 bit> <track3 bit> <mode> <action> <data>\n", argv[0] );
	      printf( "\t\t <track bit> 5|7|8\n");
	      printf( "\t\t <mode> 1 = RAW, 2= ISO\n");
	      printf( "\t\t <action> r = read, w = write, e = erase\n");
	      printf( "\t\t <data> if mode = w; what to write (8bpc)\n");
	}else{
	  int t1 = atoi(argv[1]);
	  int t2 = atoi(argv[2]);
	  int t3 = atoi(argv[3]);
	  int mode = atoi(argv[4]);
	  char op = argv[5][0];
	  unsigned char tarja[26];

	  if (t1 < 5|| t1 > 8 || t1 == 6) {
	    printf("ERROR: Track 1 bits must be either 5,7 or 8\n");
	  
	  }
	  if (t2 < 5 || t2 > 8|| t2== 6) {
	    printf("ERROR: Track 2 bits must be either 5,7 or 8\n");
	  
	  }
	  if (t3 < 5 || t3 > 8|| t3 == 6) {
	    printf("ERROR: Track 3 bits must be either 5,7 or 8\n"); 
	  }

	  if(op == 'w') {
	    const char *ptr = argv[6];
	    for(int i = 0; i < strlen(argv[6]) / 2; i++) {
              sscanf(ptr, "%02hhx", &tarja[i]);
              ptr += 2;
	    }


	  }
  
	license();
	signal(SIGINT, sigproc);	
	try 
	{
		/* connect to specified device */
		magnetic_stripe_t *data = NULL;
		msr->connect(DEVICE);
		printf("Connected to %s\n", DEVICE);
		msr->sendReset();
		msr->getFirmware();
		msr->getModel();
		msr->sendReset();
                
		/* initialize the msr */
		msr->init();
		printf("Initialized MSR605.\n");

		/* read card */

		if(op == 'w') {
			data = (magnetic_stripe_t *) malloc(sizeof(magnetic_stripe_t));

			msr->setHiCo();
		
			//data->track1 = (unsigned char *)malloc(6);
			data->track2 = tarja;
			data->t2_len = 25;

			data->track1 = NULL; //(unsigned char *)malloc(4);
			//strncpy((char *)data->track2, "\xde\xad\xbe\xef", 4);
			data->t1_len = 0;
			//data->track2 = NULL;
			//data->t2_len = 0;
		
			data->track3 = NULL;
			data->t3_len = 0;

			printf("Waiting for swipe...\n");

			msr->writeCard_raw(data, t1, t2, t3);

			msr->free_ms_data(data);
		} else if (op == 'r') {
		  msr->setAllLEDOff();
		  printf("Waiting for swipe...\n");
		  switch(mode) {
		    case 1:
			    data=msr->readCard_raw(t1, t2, t3);
			    printTrack("Track 1", data->track1, data->t1_len);
			    printTrack("Track 2", data->track2, data->t2_len);
			    printTrack("Track 3", data->track3, data->t3_len);
			    break;
		    case 2:
			    data=msr->readCard_iso(t1, t2, t3);
			    printTrackiso("Track 1", data->track1, data->t1_len);
			    printTrackiso("Track 2", data->track2, data->t2_len);
			    printTrackiso("Track 3", data->track3, data->t3_len);
			    break;
		  }
		  msr->free_ms_data(data);
		} else if (op == 'e') { // erase
			printf("Waiting for swipe...\n");
			msr->eraseCard(true, false, false);
		}
				
		/* close connection */
		msr->disconnect();

	}
	catch(const char *msg)
	{
		printf("MSR605 Error: %s\n", msg);
	}
	
	
	delete(msr);
    
	return 0;

	  
	}
}
