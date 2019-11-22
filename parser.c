#include "led-matrix.h"

#include <thread>
#include <chrono>

#include <stdlib.h>

#include <unistd.h>
#include <stdio.h>
#include <signal.h>
#include <pthread.h>
#include <sys/shm.h>

#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <linux/input.h>
#include <string.h>

#include <getopt.h>

#define PPM_SIZE 70


void usage(char *name){
	printf("Usage: %s --filebuffer FILENAME --fps FPS --format [txt|ppm]\n", name);
	exit(1);
}

enum format_e {
	TXT,
	PPM
};

typedef struct parser_arguments{
	char filename[2048];
	unsigned short fps;
	enum format_e format;
} parser_arguments;


parser_arguments get_arguments(int argc, char ** argv){
	int opt;
	int option_index = 0;
	parser_arguments retour = {"buffer", 60, TXT};

	static struct option long_options[] = {
		{"filebuffer", required_argument, 0,  'b' },
		{"fps",  			 required_argument, 0,  'f' },
		{"format",  	 required_argument, 0,  't' },
		{0,            0,                 0,   0 }
	};

	while ((opt = getopt_long(argc, argv, "b:f:t:",long_options, &option_index)) != -1 ){
		switch(opt){
			case 'b':
				strncpy(retour.filename, optarg, sizeof(retour.filename));
				break;
			case 'f':
				retour.fps = atoi(optarg);
				break;
			case 't':
				if (strcmp(optarg, "ppm") == 0){
					retour.format = PPM;	
				}else{
					retour.format = TXT;	
				}
				break;
			default:
				usage(argv[0]);
				exit(1);
				break;
		}
		if (opt == -1){
			usage(argv[0]);
		}
	}
	if (optind < argc) {
		usage(argv[0]);
	}else{
		if (argc == 1){
			usage(argv[0]);
		}
	}
	return retour;
}




int MATRIX[64][64][3]={0};

void show_matrix(){
	for (int line =0; line < 64; line++){
		for (int column=0; column < 64 ; column++){
			printf("(%u,%u,%u)", MATRIX[line][column][0],MATRIX[line][column][1],MATRIX[line][column][2]);
		}
		printf("\n");
	}
}


void showArg(parser_arguments * arg){
	printf("== Retour ==\n");
	printf("Filename : %s\n", arg->filename);
	printf("FPS : %d\n", arg->fps);
	printf("Format :%d\n", arg->format);
}

void parse_file(FILE * filename, enum format_e format ){
	char buff[PPM_SIZE];
	char * ptr;	
	int width, height;

	int position_x = 0;
	int position_y = 0;
	int rgb_pos = 0;


	if (filename != NULL){
		if ( format == PPM ){
			ptr = fgets(buff, PPM_SIZE, filename);
			// Header
			if ( (ptr != NULL) ){
				do{
					ptr = fgets(buff, PPM_SIZE, filename);
					if (ptr == NULL){
						fprintf(stderr, "An error occurs during parsing\n");
						exit(4);
					}
				}while ((strncmp(buff, "#", 1)) == 0);		
				if ( sscanf(buff, "%d %d", &width, &height) < 2){
						fprintf(stderr, "An error occurs during parsing\n");
						exit(4);
				}
				if ((width != 64 )||(height != 64)){
						fprintf(stderr, "An error occurs during parsing. Bad matrix size\n");
						exit(4);
				}
				int pixel=0;
				do {
					pixel = 0;
					if (fscanf(filename, "%d", &pixel) == 1){
						if (pixel <= 255){
							MATRIX[position_y][position_x][rgb_pos] = pixel;
							rgb_pos +=1;
							if (rgb_pos == 3){
								rgb_pos = 0;
								position_x +=1;
								if (position_x == 64){
									position_x = 0;
									position_y += 1;
								}
							}
						}
					}
				}while((position_x < 64) && (position_y < 64 ));
			}
		}else{
			unsigned int r = 0;
			unsigned int g = 0;
			unsigned int b = 0;
			do {
				if (fscanf(filename, "(%d,%d,%d)", &r,&g,&b) == 3){
					fseek(filename, 1, SEEK_CUR);
					if ((r <= 255) && (g <= 255) && (b <= 255)){
						MATRIX[position_y][position_x][0] = r;
						MATRIX[position_y][position_x][1] = g;
						MATRIX[position_y][position_x][2] = b;
						position_x +=1;
						if (position_x == 64){
							position_x = 0;
							position_y += 1;
						}
					}
				}
			}while((position_x < 64) && (position_y < 64 ));
		}
	}
}


int main(int argc, char ** argv){
	parser_arguments arg = get_arguments(argc, argv);
	FILE * fp;
	if ((fp = fopen(arg.filename, "r")) != NULL){
		while(true){
			parse_file(fp, arg.format);
			fseek(fp, 0, SEEK_SET);
			std::this_thread::sleep_for(std::chrono::milliseconds(1000/arg.fps));
//			sleep(0.5);
			printf("In\n");
		}
		fclose(fp);
		show_matrix();
	}else{
		fprintf(stderr, "Buffer file '%s' is not created\n", arg.filename);
	}
	return 0;
}
