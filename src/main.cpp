#include <iostream>
#include <SDL2/SDL.h>
#include <random>
#include <cmath>

#define H_FOV 80
#define SCREEN_WIDTH 1920
#define SCREEN_HEIGHT 1080

using namespace std;

typedef struct Line Line;
struct Line {
	int x1;
	int y1;
	int x2;
	int y2;
};

typedef struct DLine DLine;
struct DLine {
	double x1;
	double y1;
	double x2;
	double y2;
};

typedef struct Point Point;
struct Point {
	int x;
	int y;
};

typedef struct DPoint DPoint;
struct DPoint {
	double x;
	double y;
};

// ------------------------------------------------
bool initSDL(SDL_Window** window, SDL_Renderer** renderer)
{
	if (SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		printf("Error initializing SDL");
        return false;
	}
	
	*window = SDL_CreateWindow("RayCasting", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_FULLSCREEN);
	if (*window == nullptr)
	{
    	printf("Error initializing Window");
        return false;
	}
		
	*renderer = SDL_CreateRenderer(*window, -1, SDL_RENDERER_PRESENTVSYNC|SDL_RENDERER_ACCELERATED);
	if (*renderer == nullptr)
	{
    	printf("Error initializing Renderer");
        return false;
	}
	return true;
}
// ------------------------------------------------
// Credit to Gavin (https://stackoverflow.com/a/1968345/9962106) for this algorithm
char get_line_intersection(double p0_x, double p0_y, double p1_x, double p1_y, double p2_x, double p2_y, double p3_x, double p3_y, double *i_x, double *i_y)
{
    double s1_x, s1_y, s2_x, s2_y;
    s1_x = p1_x - p0_x;     s1_y = p1_y - p0_y;
    s2_x = p3_x - p2_x;     s2_y = p3_y - p2_y;

    double s, t;
    s = (-s1_y * (p0_x - p2_x) + s1_x * (p0_y - p2_y)) / (-s2_x * s1_y + s1_x * s2_y);
    t = ( s2_x * (p0_y - p2_y) - s2_y * (p0_x - p2_x)) / (-s2_x * s1_y + s1_x * s2_y);

    if (s >= 0 && s <= 1 && t >= 0 && t <= 1)
    {
        // Collision detected
        if (i_x != nullptr)
            *i_x = p0_x + (t * s1_x);
        if (i_y != nullptr)
            *i_y = p0_y + (t * s1_y);
        return 1;
    }

    return 0; // No collision
}
// ------------------------------------------------
double radian(double value)
{
	return value*M_PI/180.0;
}
// ------------------------------------------------
void getIndexFromAngle(double angle, double distance, double* x, double* y)
{
	*x = distance * sqrt(2) * cos(radian(angle));
	*y = distance * sqrt(2) * sin(radian(angle));
}
// ------------------------------------------------
void render(SDL_Renderer* renderer, Line (&lines)[5], DPoint pos, double watchingDirection)
{
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
	SDL_RenderFillRect(renderer, nullptr);
	
	SDL_SetRenderDrawColor(renderer, 53, 53, 53, 255);
	static SDL_Rect top = {0, 0, SCREEN_WIDTH, SCREEN_HEIGHT/2};
	SDL_RenderFillRect(renderer, &top);
	SDL_SetRenderDrawColor(renderer, 113, 113, 113, 255);
	static SDL_Rect bot = {0, SCREEN_HEIGHT/2, SCREEN_WIDTH, SCREEN_HEIGHT/2};
	SDL_RenderFillRect(renderer, &bot);
	
	SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
	DPoint index;
	static bool display = false;
	DPoint intersection = {0, 0};
	static const int res = SCREEN_WIDTH;
	for (int i=0; i<res; i++)
	{
		index = {0, 0};
		double angle = watchingDirection-H_FOV/2.0 + (double)i*(H_FOV/(double)res);
		getIndexFromAngle(angle, 10000, &index.x, &index.y);
		
		DLine ray = {pos.x, pos.y, pos.x + index.x, pos.y + index.y};
		
		display = false;
		for(Line &line : lines)
		{
			
			if (get_line_intersection(line.x1, line.y1, line.x2, line.y2, ray.x1, ray.y1, ray.x2, ray.y2, &intersection.x, &intersection.y))
			{
				display = true;
				ray.x2 = intersection.x;
				ray.y2 = intersection.y;
			}
			
			if (display)
			{
				double distanceRAW = hypot(ray.x2 - ray.x1, ray.y2 - ray.y1);
				double distanceFIXED = distanceRAW * cos(radian(angle-watchingDirection));
				if (distanceFIXED > 0)
				{
					double taille = ((50.0*300.0)/distanceFIXED)/0.043703125;
					int haut = (int)(SCREEN_HEIGHT/2.0 - taille/2);
					int bas = (int)(SCREEN_HEIGHT/2.0 + taille/2);
					SDL_RenderDrawLine(renderer, i, haut, i, bas);
				}
			}
		}
	}
	
	SDL_RenderPresent(renderer);
}


int main()
{
	SDL_Window* window = nullptr;
	SDL_Renderer* renderer = nullptr;
	SDL_Event event;
	
	random_device rd;
	auto mt = mt19937(rd());
	uniform_real_distribution<double> dist(0.0, 5000.0);
	
	if (!initSDL(&window, &renderer))
	{
		return EXIT_FAILURE;
	}
	
	bool running = true;
	DPoint pos = {0.0, 0.0};
	double watchingDirection = 30.0;
	
	Line lines[5]= {{0, 0, 0, 0}};
	for(Line &line : lines)
	{
		line.x1 = (int)dist(mt);
		line.y1 = (int)dist(mt);
		line.x2 = (int)dist(mt);
		line.y2 = (int)dist(mt);
	}
	
	SDL_DisplayMode current;
	SDL_GetCurrentDisplayMode(0, &current);
	int REFRESH_RATE = current.refresh_rate;
	
	SDL_SetRelativeMouseMode(SDL_TRUE);
	double walking_speed;
	
	while(running)
	{
		while(SDL_PollEvent(&event))
		{
			switch(event.type)
			{
				case SDL_QUIT:
				{
					running = false;
					break;
				}
				case SDL_KEYDOWN:
				{
					if (event.key.keysym.scancode == SDL_SCANCODE_ESCAPE)
						running = false;
					break;
				}
				case SDL_MOUSEMOTION:
				{
					watchingDirection += event.motion.xrel*(60.0/REFRESH_RATE)/4.5;
					break;
				}
				default:
					break;
			}
		}
		
		static DPoint temp;
		
		const Uint8* keys = SDL_GetKeyboardState(nullptr);
		if (keys[SDL_SCANCODE_LSHIFT])
		{
			walking_speed = 2.0;
		}
		else
		{
			walking_speed = 1.0;
		}
		if (keys[SDL_SCANCODE_W])
		{
			getIndexFromAngle(watchingDirection, 5.0*(60.0/REFRESH_RATE)*walking_speed, &temp.x, &temp.y);
			pos.x += temp.x;
			pos.y += temp.y;
		}
		if (keys[SDL_SCANCODE_A])
		{
			getIndexFromAngle(watchingDirection-90.0, 5.0*(60.0/REFRESH_RATE)*walking_speed, &temp.x, &temp.y);
			pos.x += temp.x;
			pos.y += temp.y;
		}
		if (keys[SDL_SCANCODE_S])
		{
			getIndexFromAngle(watchingDirection+180.0, 5.0*(60.0/REFRESH_RATE)*walking_speed, &temp.x, &temp.y);
			pos.x += temp.x;
			pos.y += temp.y;
		}
		if (keys[SDL_SCANCODE_D])
		{
			getIndexFromAngle(watchingDirection+90.0, 5.0*(60.0/REFRESH_RATE)*walking_speed, &temp.x, &temp.y);
			pos.x += temp.x;
			pos.y += temp.y;
		}
		
		
		render(renderer, lines, pos, watchingDirection);
		
	}
	SDL_Quit();
	return EXIT_SUCCESS;
}
