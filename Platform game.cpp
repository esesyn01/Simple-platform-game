#define _USE_MATH_DEFINES
#define _CRT_SECURE_NO_WARNINGS
#include<math.h>
#include<stdio.h>
#include<string.h>

extern "C" {
#include"./SDL2-2.0.10/include/SDL.h"
#include"./SDL2-2.0.10/include/SDL_main.h"
}

#define SCREEN_WIDTH	640
#define SCREEN_HEIGHT	480
#define PLAYER_WIDTH 20
#define PLAYER_HEIGHT 20
#define BONUSV 240
#define DASH 1200
#define DASH_FACTOR 4600
#define JUMP_BASE_VEL -250
#define POSX_FACTOR 110
#define BASESPEED 65
#define FACTORX 2.5
#define FACTORY 300
#define TIMER_X 523
#define TIMER_Y 4
#define TIMER_W 113
#define TIMER_H 36
#define POS_ABOVE_1ST 30
#define TIME_Y 20

SDL_Rect Init(int PosX, int PosY, int w, int h)    // funkcja inicjalizujaca strukture SDL_Rect
{
	SDL_Rect rectangle;
	rectangle.x = PosX;
	rectangle.y = PosY;
	rectangle.w = w;
	rectangle.h = h;
	return rectangle;
}

// narysowanie napisu txt na powierzchni screen, zaczynając od punktu (x, y)
// charset to bitmapa 128x128 zawierająca znaki

void DrawString(SDL_Surface* screen, int x, int y, const char* text,
	SDL_Surface* charset) {
	int px, py, c;
	SDL_Rect s, d;
	s.w = 8;
	s.h = 8;
	d.w = 8;
	d.h = 8;
	while (*text) {
		c = *text & 255;
		px = (c % 16) * 8;
		py = (c / 16) * 8;
		s.x = px;
		s.y = py;
		d.x = x;
		d.y = y;
		SDL_BlitSurface(charset, &s, screen, &d);
		x += 8;
		text++;
	};
};


// narysowanie na ekranie screen powierzchni sprite w punkcie (x, y)
// (x, y) to punkt środka obrazka sprite na ekranie

void DrawSurface(SDL_Surface* screen, SDL_Surface* sprite, int x, int y) {
	SDL_Rect dest;
	dest.x = x - sprite->w / 2;
	dest.y = y - sprite->h / 2;
	dest.w = sprite->w;
	dest.h = sprite->h;
	SDL_BlitSurface(sprite, NULL, screen, &dest);
};


// rysowanie pojedynczego pixela
void DrawPixel(SDL_Surface* surface, int x, int y, Uint32 color) {
	int bpp = surface->format->BytesPerPixel;
	Uint8* p = (Uint8*)surface->pixels + y * surface->pitch + x * bpp;
	*(Uint32*)p = color;
};


// rysowanie linii o długości l w pionie (gdy dx = 0, dy = 1) 
// bądź poziomie (gdy dx = 1, dy = 0)
void DrawLine(SDL_Surface* screen, int x, int y, int l, int dx, int dy, Uint32 color) {
	for (int i = 0; i < l; i++) {
		DrawPixel(screen, x, y, color);
		x += dx;
		y += dy;
	};
};


// rysowanie prostokąta o długości boków l i k
void DrawRectangle(SDL_Surface* screen, int x, int y, int l, int k,
	Uint32 outlineColor, Uint32 fillColor) {
	int i;
	DrawLine(screen, x, y, k, 0, 1, outlineColor);
	DrawLine(screen, x + l - 1, y, k, 0, 1, outlineColor);
	DrawLine(screen, x, y, l, 1, 0, outlineColor);
	DrawLine(screen, x, y + k - 1, l, 1, 0, outlineColor);
	for (i = y + 1; i < y + k - 1; i++)
		DrawLine(screen, x + 1, i, l - 2, 1, 0, fillColor);
};

void DrawPlatform(SDL_Surface* screen, int x, int y, int l, int k, Uint32 outlineColor, Uint32 fillcolor, double position, int levelwidth)   // rysowanie platformy na ekranie
{
	int posX, roznica;
	if (x + l < position || position + SCREEN_WIDTH < x)  // jesli platformy nie ma w kadrze, to jej nie rysuj
	{
		DrawRectangle(screen, 0, 0, 0, 0, outlineColor, fillcolor);
	}
	else
	{
		if (x < position && x + l > position + SCREEN_WIDTH) // jesli platforma wystaje z obu stron ekranu, to narysuj jej widoczna czesc
		{
			DrawRectangle(screen, 0, y, SCREEN_WIDTH, k, outlineColor, fillcolor);
		}
		else
		{
			if (x < position)      // jesli platforma wystaje z lewej strony, to narysuj jej czesc przy lewej krawedzi
			{
				roznica = position - x;
				DrawRectangle(screen, 0, y, l - roznica, k, outlineColor, fillcolor);
			}
			else
			{
				posX = x - position;
				if (x + l > position + SCREEN_WIDTH)    // jesli platofrma wystaje z prawej to narysuj jej czesc po prawej
				{
					roznica = posX + l;
					roznica -= SCREEN_WIDTH;
					DrawRectangle(screen, posX, y, l - roznica, k, outlineColor, fillcolor);
				}
				else
				{
					DrawRectangle(screen, posX, y, l, k, outlineColor, fillcolor);   // jesli cala platforma jest w kadrze, to ja narysuj
				}
			}
		}
	}
}
bool ifstands(SDL_Rect player, SDL_Rect* platform)  // sprawdzanie, czy sprite stoi
{
	for (int i = 0; i < player.w; i++)
	{
		SDL_Rect pixel = Init(player.x + i, player.y + player.h - 1, 1, 1);
		if (SDL_HasIntersection(platform, &pixel) == SDL_TRUE)  // kolizja od dolu(sprite stoi na platformie)
		{
			return 1;
		}
	}
	return 0;
}
bool ifcolidesr(SDL_Rect player, SDL_Rect* platform) // sprawdzanie, czy nie ma kolizji od prawej
{
	for (int i = 0; i < player.h - 1; i++)
	{
		SDL_Rect pixel = Init(player.x + player.w, player.y + i, 1, 1);
		if (SDL_HasIntersection(platform, &pixel) == SDL_TRUE)   // jest kolizja
		{
			return 1;
		}
	}
	return 0;
}
bool ifcolidesup(SDL_Rect player, SDL_Rect* platform)  // sprawdzenie, czy nie ma kolizji od góry
{
	for (int i = 0; i < player.w; i++)
	{
		SDL_Rect pixel = Init(player.x + i, player.y, 1, 1);
		if (SDL_HasIntersection(platform, &pixel) == SDL_TRUE)  // jest kolizja
		{
			return 1;
		}
	}
	return 0;
}

void collision(int platformsnumber, int* quit, SDL_Rect player, SDL_Rect* platformy, int* stand, bool* jump, bool* djump, bool* djumpreq, bool* dash)
{
	for (int i = 0; i < platformsnumber; i++)
	{
		*quit = ifcolidesr(player, &platformy[i]);
		if (*quit == 1)
		{
			break;
		}
		*quit = ifcolidesup(player, &platformy[i]);
		if (*quit == 1)
		{
			break;
		}
	}
	for (int i = 0; i < platformsnumber; i++)
	{
		*stand = ifstands(player, &platformy[i]);
		if (*stand == 1)
		{
			*jump = 1;
			*djump = 1;
			*djumpreq = 0;
			*dash = 1;
			break;
		}
	}
}

void dashing(double* speed, double delta, double* velY, double copyX, double copyY, bool* indash)
{
	*speed -= DASH_FACTOR * delta;
	if (*speed < copyX)
	{
		*speed = copyX;
		*indash = 0;
		*velY = copyY;
	}
}
// main
#ifdef __cplusplus
extern "C"
#endif
int main(int argc, char** argv) {
	int t1, t2, quit, rc, stand, t3, t4, steer, help1, help2;
	bool jump, djump, djumpreq, dash, indash;
	double delta, delta1, worldTime, speed, velY, factorY, posY, bonus, copyY, copyX, factorX, position, scale = 1, keypressed = 0;
	FILE* platforms;
	platforms = fopen("platforms.txt", "r");
	int mapw = 0; int maph = 0; int platformsnumber = 0, temph = 0, tempy = 0;
	fscanf(platforms, "%d %d %d", &mapw, &maph, &platformsnumber);
	if (maph != SCREEN_HEIGHT)  // ustawianie skalowania (wysokosc okna to 480px)
	{
		scale = SCREEN_HEIGHT;
		scale /= maph;
		maph = SCREEN_HEIGHT;
	}
	SDL_Rect* platformy = new SDL_Rect[platformsnumber];  // inicjalizacja pamieci na platformy z pliku tekstowego
	for (int i = 0; i < platformsnumber; i++)
	{
		if (scale == 1)
		{
			fscanf(platforms, "%d %d %d %d", &platformy[i].x, &platformy[i].y, &platformy[i].w, &platformy[i].h);
		}
		else
		{
			fscanf(platforms, "%d %d %d %d", &platformy[i].x, &tempy, &platformy[i].w, &temph);  // skalowanie wmiarow platform
			tempy *= scale;
			temph *= scale;
			platformy[i].y = tempy;
			platformy[i].h = temph;
		}
	}
	SDL_Event event;
	SDL_Surface* screen, * charset;
	SDL_Surface* eti;
	SDL_Texture* scrtex;
	SDL_Window* window;
	SDL_Renderer* renderer;
	printf("wyjscie printfa trafia do tego okienka\n");
	printf("printf output goes here\n");

	if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
		printf("SDL_Init error: %s\n", SDL_GetError());
		return 1;
	}
	rc = SDL_CreateWindowAndRenderer(SCREEN_WIDTH, SCREEN_HEIGHT, 0,
		&window, &renderer);
	if (rc != 0) {
		SDL_Quit();
		printf("SDL_CreateWindowAndRenderer error: %s\n", SDL_GetError());
		return 1;
	};

	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");
	SDL_RenderSetLogicalSize(renderer, SCREEN_WIDTH, SCREEN_HEIGHT);
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);

	SDL_SetWindowTitle(window, "Robot Unicorn Attack - project 2");


	screen = SDL_CreateRGBSurface(0, SCREEN_WIDTH, SCREEN_HEIGHT, 32,
		0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);

	scrtex = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888,
		SDL_TEXTUREACCESS_STREAMING,
		SCREEN_WIDTH, SCREEN_HEIGHT);


	// wyłączenie widoczności kursora myszy
	SDL_ShowCursor(SDL_DISABLE);

	// wczytanie obrazka cs8x8.bmp
	charset = SDL_LoadBMP("./cs8x8.bmp");
	if (charset == NULL) {
		printf("SDL_LoadBMP(cs8x8.bmp) error: %s\n", SDL_GetError());
		SDL_FreeSurface(screen);
		SDL_DestroyTexture(scrtex);
		SDL_DestroyWindow(window);
		SDL_DestroyRenderer(renderer);
		SDL_Quit();
		return 1;
	};
	SDL_SetColorKey(charset, true, 0x000000);

	eti = SDL_LoadBMP("./player.bmp");
	if (eti == NULL) {
		printf("SDL_LoadBMP(player.bmp) error: %s\n", SDL_GetError());
		SDL_FreeSurface(charset);
		SDL_FreeSurface(screen);
		SDL_DestroyTexture(scrtex);
		SDL_DestroyWindow(window);
		SDL_DestroyRenderer(renderer);
		SDL_Quit();
		return 1;
	};

	char text[128];
	int gold = SDL_MapRGB(screen->format, 0xFF, 0xD7, 0x00);  // ustawienie kolorów 
	int pink = SDL_MapRGB(screen->format, 0xFF, 0x14, 0x93);
	int bialy = SDL_MapRGB(screen->format, 0xFF, 0xFF, 0xFF);
	int czarny = SDL_MapRGB(screen->format, 0x00, 0x00, 0x00);
	int zielony = SDL_MapRGB(screen->format, 0x00, 0xFF, 0x00);
	int czerwony = SDL_MapRGB(screen->format, 0xFF, 0x00, 0x00);
	int niebieski = SDL_MapRGB(screen->format, 0x11, 0x11, 0xCC);

	t1 = SDL_GetTicks();
	quit = 0;
	worldTime = 0;
	stand = 0;
	position = 1;
	velY = 0;
	factorY = FACTORY;
	speed = BASESPEED;
	posY = platformy[0].y-POS_ABOVE_1ST;    // ustawianie parametrów startowych
	jump = 0;
	djump = 1;
	djumpreq = 0;
	dash = 1;
	indash = 0;
	copyX = 0;
	copyY = 0;
	factorX = FACTORX;
	steer = 0;

	while (!quit) {
		t2 = SDL_GetTicks();   // obliczenie czasu
		delta = (t2 - t1) * 0.001;
		t1 = t2;
		worldTime += delta;
		if (indash == 1)    // zmniejszanie predkosci podczas dasha
		{
			dashing(&speed, delta, &velY, copyX, copyY, &indash);
		}
		if (steer == 1)   // jezeli sterowanie jest zaawansowane, to automatycznie przemieszczaj gracza
		{
			position += speed * delta;
			speed += delta * factorX;
		}
		else
		{
			if (keypressed == 1)
			{
				position += delta*POSX_FACTOR; // jezeli sterowanie podstawowe i wcisnieto przycisk, to poruszaj gracza
				if (position > mapw - SCREEN_WIDTH)
				{
					position - mapw + SCREEN_WIDTH;
				}
			}
		}
		if (position >= mapw - SCREEN_WIDTH)
		{
			position -= mapw;                  // zapetlanie planszy
			position += SCREEN_WIDTH;
		}
		if (stand == 1)
		{
			velY = 0;
		}
		posY += velY * delta;   // fizyka skokow i upadkow
		velY += factorY * delta;
		SDL_FillRect(screen, NULL, czarny);
		SDL_Rect player = Init(position, posY, PLAYER_WIDTH, PLAYER_HEIGHT + 1);
		DrawSurface(screen, eti, (PLAYER_WIDTH / 2) - 1, posY + (PLAYER_HEIGHT / 2));  // narysowanie sprite'a w odpowiedniej pozycji wzgledem platform
		for (int i = 0; i < platformsnumber; i++)
		{
			DrawPlatform(screen, platformy[i].x, platformy[i].y, platformy[i].w, platformy[i].h, bialy, bialy, position, mapw);  // rysowanie platofrm
		}
		collision(platformsnumber, &quit, player, platformy, &stand, &jump, &djump, &djumpreq, &dash);
		if (stand == 0) //brak mozliwosci skoku kiedy gracz nie stoi na platformie
		{
			jump = 0;
		}
		if (posY >= SCREEN_HEIGHT) // jesli gracz wypadnie poza ekran to skoncz gre
		{
			quit = 1;
		}
		DrawRectangle(screen, TIMER_X, TIMER_Y, TIMER_W, TIMER_H, czerwony, niebieski);
		sprintf(text, " czas = %.1lf s ", worldTime);  // wypisanie czasu na ekran
		DrawString(screen, TIMER_X+1, TIME_Y, text, charset);

		SDL_UpdateTexture(scrtex, NULL, screen->pixels, screen->pitch);
		//		SDL_RenderClear(renderer);
		SDL_RenderCopy(renderer, scrtex, NULL, NULL);
		SDL_RenderPresent(renderer);

		// obsługa zdarzeń (o ile jakieś zaszły) / handling of events (if there were any)
		while (SDL_PollEvent(&event)) {
			switch (event.type) {
			case SDL_KEYDOWN:
				if (event.key.keysym.sym == SDLK_ESCAPE) quit = 1; // wyjdz z gry
				else if (event.key.keysym.sym == SDLK_z)
				{
					if (steer == 1)  // jesli zaawansowane sterowanie
					{
						if (jump == 1 || djump == 1)
						{
							t3 = SDL_GetTicks();  // zmmierz czas i zabierz mozliwosc (podwojnego)skoku
							if (jump == 1)
							{
								jump = 0;
							}
							else
							{
								djump = 0;
								djumpreq = 1;
							}
						}
					}
				}
				else if (event.key.keysym.sym == SDLK_x)
				{
					if (dash == 1 && steer == 1) // ustaw predkosc i flagi jezeli gracz chce i moze wykonac dasha
					{
						dash = 0;
						copyY = velY;
						copyX = speed;
						speed = DASH;
						velY = 0;
						indash = 1;
						djump = 1;

					}
				}
				else if (event.key.keysym.sym == SDLK_n)
				{
					quit = 0;
					worldTime = 0;
					stand = 0;
					position = 1;
					velY = 0;
					speed = BASESPEED;    // ustawienie parametrow tak jak na poczatku
					posY = platformy[0].y - POS_ABOVE_1ST;
					jump = 0;
					djump = 1;
					djumpreq = 0;
					dash = 1;
					indash = 0;
				}
				else if (event.key.keysym.sym == SDLK_d)
				{
					steer++;   // przelaczanie sterowania miedzy podstawowym a zaawansowanym
					if (steer > 1)
					{
						steer = 0;
					}
				}
				else if (event.key.keysym.sym == SDLK_RIGHT)   // informacja ze gracz porusza sie w klaszycznym sterowaniu
				{
					if (steer == 0)
					{
						keypressed = 1;
					}
				}
				else if (event.key.keysym.sym == SDLK_UP)  // podstawowy skok (bez bonusu za dlugosc trzymania)
				{
					if (steer == 0 && jump == 1)
					{
						jump = 0;
						stand = 0;
						velY = JUMP_BASE_VEL;
					}
				}
				break;
			case SDL_KEYUP:
				if (event.key.keysym.sym == SDLK_z)
				{
					if (steer == 1)
					{
						if (stand == 1 || djumpreq == 1)  // wykonanie skoku w zaawansowanym sterowaniu (mierzenie czasu nacisniecia i doliczany bonus)
						{
							t4 = SDL_GetTicks();
							delta1 = (t4 - t3) * 0.001;
							if (delta1 > 0.25)
							{
								delta1 = 0.25;
							}
							bonus = delta1 * BONUSV;
							velY = JUMP_BASE_VEL - bonus;
							stand = 0;
							djumpreq = 0;
						}
					}
				}
				else if (event.key.keysym.sym == SDLK_RIGHT)  // gracz przestał poruszac sie w podstawowym sterowaniu
				{
					if (steer == 0)
					{
						keypressed = 0;
					}
				}
				break;
			case SDL_QUIT:  // wyjdz z gry
				quit = 1;
				break;
			};
		};
	};
	// zwolnienie powierzchni / freeing all surfaces
	SDL_FreeSurface(charset);
	SDL_FreeSurface(screen);
	SDL_DestroyTexture(scrtex);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	fclose(platforms);
	delete[]platformy;
	SDL_Quit();
	return 0;
};