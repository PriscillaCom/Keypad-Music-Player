#include "avr.h"
#include "lcd.h"
#include <stdio.h>
#include <math.h>

#define q 8

struct Note {
	int frequency;
	int duration;
	int pitch;
};

struct Note record1[40];
int rec1_count = 0;

struct Note record2[40];
int rec2_count = 0;

struct Note record3[40];
int rec3_count = 0;

int Low_Freq[9] = {220,233,246,261,277,293,329,349,391};
int Mid_Freq[9] = {440,466,493,523,554,587,659,698,783};
int High_Freq[9] = {880,932,987,1046,1108,1174,1318,1396,1567};
	
int num_recorded = 0;
int end1,end2,end3;

int is_pressed(int r, int c){
	DDRC = 0;
	PORTC = 0;
	
	SET_BIT(DDRC,r);
	SET_BIT(PORTC,c + 4);
	avr_wait(1);
	
	if(GET_BIT(PINC,c + 4)){ //returns 1 for weak 1
		return 0;
	}
	else{ //returned 0 for strong 0
		return 1;
	}
}

int get_key(void){
	int r,c;
	for(r = 0; r < 4; ++r){
		for(c = 0; c < 4; ++c){
			if(is_pressed(r,c)){
				return (r*4)+c+ 1; //which button got pressed
			}
		}
	}
	return 0; //no keys pressed
}

void play_note(int freq, int dur){
	int period = (int)20000/freq;
	int TH = period/2;
	int TL = period - TH;
	int k = dur*100/period;
	
	for(int i = 0; i < k; ++i){
		SET_BIT(PORTB,3);
		avr_wait(TH); //high
		CLR_BIT(PORTB,3);
		avr_wait(TL); //low
	}
}

void play_song(const struct Note Song[], int N){
	int f,d;
		f = Song[N].frequency;
		d = 800/Song[N].duration;
		char buffer[17];
		sprintf(buffer,"F:%d D:%d",f,d);
		lcd_pos(0,0);
		lcd_puts(buffer);
	play_note(f,d);
}

void display_title(int song){
	if(num_recorded > 0){
		char buffer[17];
		sprintf(buffer, "Recording %d %d",song+1,num_recorded);
		lcd_pos(0,0);
		lcd_puts(buffer);
	}
}

void get_note(int key){
	char note;
	lcd_clr();
	if(key == 1)
		note = 'A';
	else if(key == 2)
		note = 'a';
	else if(key == 3)
		note = 'B';
	else if(key == 5)
		note = 'C';
	else if(key == 6)
		note = 'c';
	else if(key == 7)
		note = 'D';
	else if(key == 9)
		note = 'E';
	else if(key == 10)
		note = 'F';
	else 
		note = 'G';
		
	char buffer[17];
	sprintf(buffer,"Note: %c",note);
	lcd_pos(0,4);
	lcd_puts(buffer);
}

int get_freq(int key,int pitch){
	int k;
	
	if(key == 1)
		k = 220;
	else if(key == 2)
		k = 233;
	else if(key == 3)
		k = 246;
	else if(key == 5)
		k = 261;
	else if(key == 6)
		k = 277;
	else if(key == 7)
		k = 293;
	else if(key == 9)
		k = 329;
	else if(key == 10)
		k = 349;
	else
		k = 391;
		
	if(pitch == 1)
		k = k*2;
	if(pitch == 2){
		k = k*2;
		k = k*2;
	}
	
	return k;
}

int main(){
	lcd_init();
	SET_BIT(DDRB,3);
	
	int key = 0;

	//current song notes
	int song1 = 0;
	int song2 = 0;
	int song3 = 0;
	
	int displayTitle = 0;
	int selectSong = 0;
	int play = 0;
	int record = 0;
	int pitch = 0;
	int getNote = 0;
	
	for(;;){
		key = get_key();
		if((key > 0 && key <= 3 || (key >= 5 && key <= 7) || (key >= 9 && key <= 11)) && displayTitle == 0 && play == 0){
			get_note(key);
			int k = get_freq(key,pitch);
			play_note(k,q);
			getNote = 1;
		}
		
		if(key == 4){     //Display Song
			if(displayTitle == 0){
				lcd_clr();
				displayTitle = 1;
				avr_wait(3000);
			}
			else{
				lcd_clr();
				displayTitle = 0;
				avr_wait(3000);
			}
		}
		
		else if(key == 8){ // play/pause song
			if(play == 0)
				play = 1;
			else
				play = 0;
			avr_wait(3000);
			lcd_clr();
		}
		
		else if(key == 12){ // record song
			record = 1;
			avr_wait(3000);
		}
		
		else if(key == 13){ //prev song
			lcd_clr();
			if(selectSong == 0 && num_recorded != 1 && num_recorded != 0)
			selectSong = num_recorded-1;
			else if(num_recorded != 1 && num_recorded != 0)
			selectSong = (selectSong - 1);
			avr_wait(3000);
		}
		
		else if(key == 14){ // pitch change
			pitch = (pitch + 1) % 3;
			avr_wait(3000);
		}
		
		else if(key == 15){ //next song
			lcd_clr();
			if(num_recorded > 0)
				selectSong = (selectSong + 1) % num_recorded;
			avr_wait(3000);
		}
		
		else if(key == 16){ //stop record
			record = 0;
			avr_wait(4000);
			num_recorded++;
		}

		if(record == 1 && getNote == 1){
			
			int f;
			f = get_freq(key,pitch);
					
			if(num_recorded > 3){
				char buffer[17];
				sprintf(buffer,"Recording exceeded");
				lcd_pos(0,0);
				lcd_puts(buffer);
			}
			
			if(num_recorded == 0){
				record1[rec1_count].frequency = f;
				record1[rec1_count].duration = q;
				record1[rec1_count].pitch = pitch;
				rec1_count++;
				avr_wait(3000);
			}
			else if(num_recorded == 1){
				record2[rec2_count].frequency = f;
				record2[rec2_count].duration = q;
				record2[rec2_count].pitch = pitch;
				rec2_count++;
				avr_wait(3000);
			}
			else if(num_recorded == 2){
				record3[rec3_count].frequency = f;
				record3[rec3_count].duration = q;
				record3[rec3_count].pitch = pitch;
				rec3_count++;
				avr_wait(3000);
			}
			
			getNote = 0;
			
		}
	
		else if(displayTitle == 1){
			display_title(selectSong);
			char buffer[17];
			
			if(play == 1){
				
				if((selectSong == 0 && song1 == rec1_count) || (selectSong == 1 && song2 == rec2_count)|| (selectSong == 2 && song3 == rec3_count))
					play = 0;
			
				else if(selectSong == 0 && song1 < rec1_count){
					play_song(record1,song1);
					song1++;
				}
				else if(selectSong == 1 && song2 < rec2_count){
					play_song(record2,song2);
					song2++;
				}
				else if(selectSong == 2 && song3 < rec3_count){
					play_song(record3,song3);
					song3++;
				}		
			}
			
			else if(song1 >= rec1_count)
				song1 = 0;
			else if(song2 >= rec2_count)
				song2 = 0;
			else if(song3 >= rec3_count)
				song3 = 0;
		}
		
		char buffer[17];
		sprintf(buffer,"D:%d P:%d R:%d S:%d",displayTitle,play,record,selectSong);
		lcd_pos(1,0);
		lcd_puts(buffer);
		
	}
	avr_wait(2000);
	
}