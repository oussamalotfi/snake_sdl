#include <SDL2/SDL.h>
#include <SDL2/SDL_joystick.h>
#include <SDL2/SDL_rect.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_scancode.h>
#include <SDL2/SDL_timer.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// parameters
#define WINDOW_NAME "sdl_snake"
#define WIDTH 800
#define HEIGHT 800
#define PI 3.14159265359
#define GRID_SIZE 20
#define RECT_WIDTH (WIDTH / GRID_SIZE)
#define RECT_HEIGHT (HEIGHT / GRID_SIZE)
#define QSIZE = GRID_SIZE * GRID_SIZE

// structures
typedef struct square {
  int x;
  int y;
  SDL_Rect rect;
} square;

const square DEFAULT_SQUARE = {INT_MIN, INT_MIN};
void print_square_data(square s) { printf("(x = %d, y = %d)\n", s.x, s.y); }

typedef struct food {
  int x;
  int y;
} food;

// linked list

typedef struct node {
  square s;
  struct node *next;
} node;

// linked list functions

void printlist(node *head) {
  node *tmp = head;
  int i = 0;
  while (tmp != NULL) {
    printf("list[%d] = ", i);
    print_square_data(tmp->s);
    tmp = tmp->next;
    i++;
  }
}
void printuntil(node *head, int index) {
  node *tmp = head;
  int i = 0;
  while (tmp != NULL && i < index) {
    printf("list[%d] = ", i);
    print_square_data(tmp->s);
    tmp = tmp->next;
    i++;
  }
}

node *create_new_node(square sq) {
  node *result = malloc(sizeof(node));
  (*result).s = sq;
  (*result).next = NULL;
  return result;
}

node *insert_at_head(node **head, square sq) {
  node *result = malloc(sizeof(node));
  (*result).s = sq;
  (*result).next = *head;
  *head = result;
  return result;
}
square indx(node *head, int index) {
  node *tmp = head;
  int i = 0;
  while (tmp != NULL) {
    if (i == index) {
      return tmp->s;
    }
    tmp = tmp->next;
    i++;
  }
  return DEFAULT_SQUARE;
}

// functions

int positive_modulo(int i, int n) { return (i % n + n) % n; }

void clearscreen(SDL_Renderer *renderer) {
  SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
  SDL_RenderClear(renderer);
}

// game functions

square snakedata(node *head, int xspeed, int yspeed) {
  square snekhead =
      (square){indx(head, 0).x + xspeed, indx(head, 0).y + yspeed,
               (SDL_Rect){positive_modulo(snekhead.x, GRID_SIZE) * RECT_WIDTH,
                          positive_modulo(snekhead.y, GRID_SIZE) * RECT_HEIGHT,
                          RECT_WIDTH, RECT_HEIGHT}};
  return snekhead;
}

void check_eat(food food, node *head, bool *foodeaten, int *size) {
  if (food.x == positive_modulo(indx(head, 0).x, GRID_SIZE) &&
      food.y == positive_modulo(indx(head, 0).y, GRID_SIZE)) {
    *foodeaten = true;
    *size += 1;
  }
}

void collsioncheck(node *head, int size, bool *lose, int *xspeed, int *yspeed) {
  for (int i = 1; i < size; i++) {
    if (positive_modulo(indx(head, 0).x, GRID_SIZE) ==
            positive_modulo(indx(head, i).x, GRID_SIZE) &&
        positive_modulo(indx(head, 0).y, GRID_SIZE) ==
            positive_modulo(indx(head, i).y, GRID_SIZE)) {
      *lose = true;
      *xspeed = 0;
      *yspeed = 0;
    }
  }
}

food randomize_food_position(int size, node *head) {
  food f;
  f.x = rand() % GRID_SIZE;
  f.y = rand() % GRID_SIZE;
  for (int i = 0; i < size; i++) {
    for (int j = 0; j <= size; j++) {
      if (f.x == indx(head, i).x && f.y == indx(head, i).y) {
        f.x = rand() % GRID_SIZE;
        f.y = rand() % GRID_SIZE;
      }
    }
  }
  return f;
}

// drawing functions

void draw_grid(SDL_Renderer *renderer) {
  SDL_SetRenderDrawColor(renderer, 80, 80, 80, 255);
  for (int i = 1; i < HEIGHT; i++) {
    SDL_RenderDrawLine(renderer, 0, i * HEIGHT / GRID_SIZE, WIDTH,
                       i * HEIGHT / GRID_SIZE);
  }
  for (int i = 1; i < WIDTH; i++) {
    SDL_RenderDrawLine(renderer, i * WIDTH / GRID_SIZE, 0,
                       i * WIDTH / GRID_SIZE, HEIGHT);
  }
}

void drawfood(SDL_Renderer *renderer, food food) {
  SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255);
  SDL_Rect food_rectangle = {food.x * RECT_WIDTH, food.y * RECT_HEIGHT,
                             RECT_WIDTH, RECT_HEIGHT};
  SDL_RenderFillRect(renderer, &food_rectangle);
}

void drawsnake(SDL_Renderer *renderer, node *head, int size) {
  SDL_Rect r;
  for (int i = 0; i < size; i++) {
    r = indx(head, i).rect;
    SDL_RenderFillRect(renderer, &r);
  }
}

// input functions
void getuserinput(const Uint8 *state, int size, node *head, int *xspeed,
                  int *yspeed) {
  if (state[SDL_SCANCODE_RIGHT] &&
      (size == 0 || (size > 0 && indx(head, 1).x != indx(head, 0).x + 1))) {
    *xspeed = 1;
    *yspeed = 0;
    // printf("<right> is pressed.\n");

  } else if (state[SDL_SCANCODE_LEFT] &&
             (size == 0 ||
              (size > 0 && indx(head, 1).x != indx(head, 0).x - 1))) {
    *xspeed = -1;
    *yspeed = 0;
    // printf("<left> is pressed.\n");

  } else if (state[SDL_SCANCODE_UP] &&
             (size == 0 ||
              (size > 0 && indx(head, 1).y != indx(head, 0).y - 1))) {
    *xspeed = 0;
    *yspeed = -1;
    // printf("<up> is pressed.\n");

  } else if (state[SDL_SCANCODE_DOWN] &&
             (size == 0 ||
              (size > 0 && indx(head, 1).y != indx(head, 0).y + 1))) {
    *xspeed = 0;
    *yspeed = 1;
    // printf("<down> is pressed.\n");
  }
}

int main(int argc, char *argv[]) {
  // create window and renderer
  SDL_Window *window = SDL_CreateWindow(WINDOW_NAME, SDL_WINDOWPOS_UNDEFINED,
                                        SDL_WINDOWPOS_UNDEFINED, WIDTH, HEIGHT,
                                        SDL_WINDOW_OPENGL);

  SDL_Renderer *renderer = SDL_CreateRenderer(
      window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

  bool running = true;
  SDL_Event event;

  // game speed
  // int framecount, timerfps, lastframe, fps, lasttime;

  // unsigned int a = SDL_GetTicks();
  // unsigned int b = SDL_GetTicks();
  // double delta = 0;
  // int fps = 10;
  // int frameDelay = 1000 / fps;
  // Uint32 frameStart;
  // int frameTime;


  float frametime = 0;
  int prevtime = 0;
  int currenttime = 0;
  float delta_time = 0;

  srand(time(0));

  // linked list pointers
  node *head, *tmp;

  // game data
  food food;
  food.x = rand() % GRID_SIZE;
  food.y = rand() % GRID_SIZE;

  int size = 1; // initial size
  int xspeed = 0;
  int yspeed = 0;
  bool lose = false;
  bool foodeaten = false;

  // randomize head of the snake
  square snekhead = {rand() % GRID_SIZE, rand() % GRID_SIZE,
                     (SDL_Rect){snekhead.x * RECT_WIDTH,
                                snekhead.y * RECT_HEIGHT, RECT_WIDTH,
                                RECT_HEIGHT}};

  tmp = create_new_node(snekhead);
  head = tmp;
  printuntil(head, size);

  while (running) {

    while (SDL_PollEvent(&event))
      if (event.type == SDL_QUIT)
        running = false;

    //getting user input
    const Uint8 *state = SDL_GetKeyboardState(NULL);
    if (lose == false)
      getuserinput(state, size, head, &xspeed, &yspeed);


    //fixed input delay issue
    //see https://www.youtube.com/watch?v=yRpen8jOa08

    // new frame 
    prevtime = currenttime;
    currenttime = SDL_GetTicks();
    delta_time = (currenttime - prevtime)/1000.0f;

    frametime += delta_time;
    
    
    if(frametime >= 0.1f){
      frametime = 0;
    
    
    // clear screen
    clearscreen(renderer);



    // limit framerate
    // frameStart = SDL_GetTicks();

    // update snek :)
    if (lose == false) {
      snekhead = snakedata(head, xspeed, yspeed);
      tmp = insert_at_head(&head, snekhead);
      printuntil(head, size);
    }

    // check if food is eaten
    // changes the value of foodeaten and size
    check_eat(food, head, &foodeaten, &size);

    // check for collsion
    // changes the values of lose,xspeed and yspeed
    collsioncheck(head, size, &lose, &xspeed, &yspeed);

    // drawing

    // draw grid
    draw_grid(renderer);

    // draw food
    drawfood(renderer, food);

    // change color if losing
    if (lose == false) {
      SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    } else {
      SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
    }

    // draw snek
    drawsnake(renderer, head, size);

    
    // framecount ++;
    // int timerfps = SDL_GetTicks()-lastframe;
    // if(timerfps < (1000/10)){
    //   SDL_Delay((1000/10)-timerfps);
    // }
    // }

    // get user input
    // changes xspeed and yspeed
    
    SDL_RenderPresent(renderer);


    


    // randomize food position
    if (foodeaten)
      food = randomize_food_position(size, head);

    // reset foodeaten value
    foodeaten = false;
    // show what was drawn

    // frameTime = SDL_GetTicks() - frameStart;

    // if(frameDelay > frameTime){
    //    SDL_Delay(frameDelay-frameTime);
    // }
    }
  }



  // release resources
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  SDL_Quit();

  return (0);
}