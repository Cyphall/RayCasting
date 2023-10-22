#include <iostream>
#include <SDL2/SDL.h>
#include <random>
#include <glm/glm.hpp>

const int SCREEN_WIDTH = 1920;
const int SCREEN_HEIGHT = 1080;
const double H_FOV = glm::radians(80.0);
const double V_FOV = 2.0 * glm::atan(glm::tan(H_FOV * 0.5) / (static_cast<double>(SCREEN_WIDTH) / static_cast<double>(SCREEN_HEIGHT)));
const double WALL_HEIGHT = 2.0;

struct Ray
{
	glm::dvec2 origin;
	glm::dvec2 direction;
};

struct Wall
{
	glm::dvec2 start;
	glm::dvec2 end;
	glm::u8vec3 color;
};

bool initSDL(SDL_Window** window, SDL_Renderer** renderer)
{
	if (SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		printf("Error initializing SDL");
        return false;
	}
	
	*window = SDL_CreateWindow("RayCasting", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_BORDERLESS);
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

glm::dvec2 getDirFromAngle(double angle)
{
	return {
		glm::sin(angle),
		glm::cos(angle)
	};
}

struct IntersectionResult
{
	double distance;
	const Wall* intersectedWall;
};

bool intersect(const Ray& ray, const std::vector<Wall>& walls, IntersectionResult& result)
{
	double closestDist;
	const Wall* closestWall = nullptr;
	
	glm::dvec2 intersection;
	
	for(const Wall& wall : walls)
	{
		if (get_line_intersection(wall.start.x, wall.start.y, wall.end.x, wall.end.y, ray.origin.x, ray.origin.y, ray.origin.x + ray.direction.x * 1000, ray.origin.y + ray.direction.y * 1000, &intersection.x, &intersection.y))
		{
			double dist = glm::length(intersection - ray.origin);
			if (dist <= 0) continue;
			
			if (closestWall == nullptr || dist < closestDist)
			{
				closestWall = &wall;
				closestDist = dist;
			}
		}
	}
	
	if (closestWall != nullptr)
	{
		result.distance = closestDist;
		result.intersectedWall = closestWall;
		return true;
	}
	return false;
}

void render(SDL_Renderer* renderer, std::vector<Wall>& walls, glm::dvec2 pos, double camAngle)
{
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
	SDL_RenderFillRect(renderer, nullptr);
	
	SDL_SetRenderDrawColor(renderer, 53, 53, 53, 255);
	static SDL_Rect top = {0, 0, SCREEN_WIDTH, SCREEN_HEIGHT/2};
	SDL_RenderFillRect(renderer, &top);
	SDL_SetRenderDrawColor(renderer, 113, 113, 113, 255);
	static SDL_Rect bot = {0, SCREEN_HEIGHT/2, SCREEN_WIDTH, SCREEN_HEIGHT/2};
	SDL_RenderFillRect(renderer, &bot);
	
	Ray ray{};
	ray.origin = pos;
	
	IntersectionResult result{};
	
	double distanceToHeightRatio = (1.0 * glm::tan(V_FOV / 2.0)) * 2.0;
	
	glm::dvec2 centerRay = getDirFromAngle(camAngle);
	glm::dvec2 leftRay = getDirFromAngle(camAngle - H_FOV / 2.0);
	glm::dvec2 rightRay = getDirFromAngle(camAngle + H_FOV / 2.0);
	
	for (int i = 0; i < SCREEN_WIDTH; i++)
	{
		double uvX = (i + 0.5) / SCREEN_WIDTH;
		ray.direction = glm::normalize(glm::mix(leftRay, rightRay, uvX));
		
		if (intersect(ray, walls, result))
		{
			// corrects fisheye effect
			double angleFromCenter = glm::acos(glm::dot(ray.direction, centerRay));
			double correctedDistance = result.distance * glm::cos(angleFromCenter);
			
			double normalizedHeight = WALL_HEIGHT / (distanceToHeightRatio * correctedDistance);
			double pixelHeight = normalizedHeight * SCREEN_HEIGHT;
			double up = SCREEN_HEIGHT / 2.0 - pixelHeight / 2.0;
			double down = SCREEN_HEIGHT / 2.0 + pixelHeight / 2.0;
			SDL_SetRenderDrawColor(renderer, result.intersectedWall->color.r, result.intersectedWall->color.g, result.intersectedWall->color.b, 255);
			SDL_RenderDrawLine(renderer, i, (int)up, i, (int)down);
		}
	}
	
	SDL_RenderPresent(renderer);
}

int main(int argc, char* argv[])
{
	SDL_Window* window = nullptr;
	SDL_Renderer* renderer = nullptr;
	if (!initSDL(&window, &renderer))
	{
		return EXIT_FAILURE;
	}
	
	bool running = true;
	glm::dvec2 pos = {0.0, -25.0};
	double watchingDirection = 0.0;
	
	std::random_device rd;
	auto mt = std::mt19937(rd());
	std::uniform_real_distribution<double> posDist(-25.0, 25.0);
	std::independent_bits_engine<std::default_random_engine, CHAR_BIT, unsigned long> colorDist(rd());
	
	std::vector<Wall> walls(50);
	for(Wall& wall : walls)
	{
		wall.start = {posDist(mt), posDist(mt)};
		wall.end = {posDist(mt), posDist(mt)};
		
		wall.color.r = colorDist();
		wall.color.g = colorDist();
		wall.color.b = colorDist();
	}
	
	SDL_DisplayMode current;
	SDL_GetCurrentDisplayMode(0, &current);
	double deltaTime = 1.0 / current.refresh_rate;
	
	SDL_SetRelativeMouseMode(SDL_TRUE);
	
	double cameraSensitivity = 10;
	SDL_Event event{};
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
					watchingDirection += event.motion.xrel * cameraSensitivity * deltaTime;
					break;
				}
				default:
					break;
			}
		}
		
		
		double walkingSpeed = 5;
		const Uint8* keys = SDL_GetKeyboardState(nullptr);
		
		if (keys[SDL_SCANCODE_LSHIFT])
		{
			walkingSpeed *= 2.0;
		}
		
		if (keys[SDL_SCANCODE_W])
		{
			pos += getDirFromAngle(glm::radians(watchingDirection)) * walkingSpeed * deltaTime;
		}
		if (keys[SDL_SCANCODE_A])
		{
			pos += getDirFromAngle(glm::radians(watchingDirection - 90.0)) * walkingSpeed * deltaTime;
		}
		if (keys[SDL_SCANCODE_S])
		{
			pos += getDirFromAngle(glm::radians(watchingDirection + 180.0)) * walkingSpeed * deltaTime;
		}
		if (keys[SDL_SCANCODE_D])
		{
			pos += getDirFromAngle(glm::radians(watchingDirection + 90.0)) * walkingSpeed * deltaTime;
		}
		
		
		render(renderer, walls, pos, glm::radians(watchingDirection));
		
	}
	SDL_Quit();
	return EXIT_SUCCESS;
}
