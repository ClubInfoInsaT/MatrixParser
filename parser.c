#include "led-matrix.h"

// Sleeping thread
#include <thread>
#include <chrono>

#include <stdlib.h>

#include <unistd.h>
#include <stdio.h>

#include <string.h>

#include <getopt.h>

using rgb_matrix::GPIO;
using rgb_matrix::RGBMatrix;
using rgb_matrix::Canvas;
using std::vector;

#define PPM_SIZE 70

int MATRIX[64][64][3]={0};


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
	int width, height, max_pixel;
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
				if ( sscanf(buff, "%d %d", &width, &height) != 2){
					fprintf(stderr, "An error occurs during parsing\n");
					exit(4);
				}
				if ((width != 64 )||(height != 64)){
					fprintf(stderr, "An error occurs during parsing. Bad matrix size (%d,%d)\n",width, height);
					exit(4);
				}
				if ((fscanf(filename, "%d", &max_pixel)) != 1){
					fprintf(stderr, "An error occurs during parsing. Max pixel size\n");
					exit(4);
				}else{
					printf("Max pixel width : %d\n", max_pixel);
				}
				int pixel=0;
				for (int line=0; line<64; line++){
					for (int column=0; column<64; column++){
						for (int rgb = 0; rgb<3; rgb++){
							pixel = 0;
							if (fscanf(filename, "%d", &pixel) == 1){
								MATRIX[line][column][rgb] = pixel;
							}
						}
					}
				}
			}
		}else{
			unsigned int r = 0;
			unsigned int g = 0;
			unsigned int b = 0;
			for (int line=0; line<64; line++){
				for (int column=0; column<64; column++){
					if (fscanf(filename, "(%u,%u,%u)", &r,&g,&b) == 3){
						fseek(filename, 1, SEEK_CUR);
						if ((r <= 255) && (g <= 255) && (b <= 255)){
							MATRIX[line][column][0] = r;
							MATRIX[line][column][1] = g;
							MATRIX[line][column][2] = b;
						}
					}
				}
			}
		}
	}
}


void draw_matrix(RGBMatrix * canvas){
	for (int line =0; line < 64; line++){
		for (int column=0; column < 64 ; column++){
			canvas->SetPixel(column, line, MATRIX[line][column][0], MATRIX[line][column][1], MATRIX[line][column][2]);
		}
	}
}

int main(int argc, char ** argv){
	parser_arguments arg = get_arguments(argc, argv);
	RGBMatrix::Options defaults;
	defaults.hardware_mapping = "regular";
	defaults.rows = 64;
	defaults.chain_length = 2;
	defaults.parallel = 2;
	defaults.show_refresh_rate = true;

	rgb_matrix::RuntimeOptions run_opt;
	run_opt.drop_privileges = 0;
	run_opt.daemon = 0;
	run_opt.do_gpio_init = 1;
	RGBMatrix * canvas = rgb_matrix::CreateMatrixFromOptions(defaults,run_opt);
	if (canvas  == NULL){
		fprintf(stderr, "An error occurs when initializing Led Matrix\n");
		exit(3);
	}
	FILE * fp;
	while(true){
		if ((fp = fopen(arg.filename, "r")) != NULL){
			parse_file(fp, arg.format);
			fseek(fp, 0, SEEK_SET);
			std::this_thread::sleep_for(std::chrono::milliseconds(1000/arg.fps));
			draw_matrix(canvas);
			fclose(fp);
//			show_matrix();
		}else{
			fprintf(stderr, "Buffer file '%s' is not created\n", arg.filename);
		}
	}
	return 0;
}
