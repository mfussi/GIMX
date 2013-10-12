/*
 Copyright (c) 2012 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#include <string.h>
#include <stdlib.h>
#ifdef WIN32
#include <windows.h>
#include <curses.h>
#else
#include <ncursesw/ncurses.h>
#include <sys/time.h>
#endif

#include "display.h"

#include "calibration.h"
#include <GE.h>
#include "emuclient.h"

#define CROSS_CHAR '*'
#define SHIFT_ESC _("Press Shift+Esc to exit.")

#define RATE_PERIOD 500000 //ms

#ifndef WIN32
#define STICK_Y_L 11
#else
#define STICK_Y_L 13
#endif
#define STICK_X_L 21

//#define BUTTON_Y_L 3
//#define BUTTON_X_L 3

#define BUTTON_Y_L STICK_Y_L
#define BUTTON_X_L 16

#define LSTICK_Y_P 2
#define LSTICK_X_P 2

#define RSTICK_Y_P LSTICK_Y_P
#define RSTICK_X_P LSTICK_X_P + STICK_X_L + 1

#define BUTTON_Y_P LSTICK_Y_P
#define BUTTON_X_P RSTICK_X_P + STICK_X_L + 1

#define CAL_Y_P LSTICK_Y_P + STICK_Y_L
#define CAL_X_P 2

//#define BUTTON_Y_P LSTICK_Y_P + STICK_Y_L
//#define BUTTON_X_P LSTICK_X_P

#define BUTTON_W LABEL_LENGTH + BUTTON_X_L + 2

#define LABEL_LENGTH sizeof("triangle")
#define BUTTON_LENGTH LABEL_LENGTH + sizeof(": 255")

static WINDOW *lstick = NULL, *rstick = NULL, *wbuttons = NULL, *wcal = NULL;

static int cross[2][2] = { {STICK_X_L / 2, STICK_Y_L / 2}, {STICK_X_L / 2, STICK_Y_L / 2} };

#ifndef WIN32
struct timeval t0, t1;
#else
LARGE_INTEGER t0, t1, freq;
#endif

void display_calibration()
{
  char line[COLS];
  s_mouse_cal* mcal = cal_get_mouse(current_mouse, current_conf);

  if(current_cal == NONE)
  {
    mvaddstr(CAL_Y_P, CAL_X_P + 1, _("Mouse calibration (Ctrl+F1 to edit)"));
  }
  else
  {
    mvaddstr(CAL_Y_P, CAL_X_P + 1, _("Mouse calibration (Ctrl+F1 to save)(mouse wheel to change values)"));
  }
  clrtoeol();
  wmove(wcal, 1, 1);
  if(GE_GetMKMode() == GE_MK_MODE_MULTIPLE_INPUTS)
  {
    waddstr(wcal, "Mouse:");
    if(current_cal == MC)
    {
      wattron(wcal, COLOR_PAIR(4));
    }
    snprintf(line, COLS, " %s (%d) (F1) ", GE_MouseName(current_mouse), GE_MouseVirtualId(current_mouse));
    waddstr(wcal, line);
    if(current_cal == MC)
    {
      wattron(wcal, COLOR_PAIR(1));
    }
  }
  waddstr(wcal, _("Profile:"));
  if(current_cal == CC)
  {
    wattron(wcal, COLOR_PAIR(4));
  }
  snprintf(line, COLS, " %d (F2)", current_conf + 1);
  waddstr(wcal, line);
  if(current_cal == CC)
  {
    wattron(wcal, COLOR_PAIR(1));
  }
  wclrtoeol(wcal);
  mvwaddstr(wcal, 2, 1, _("Dead zone:"));
  if(current_cal == DZX)
  {
    wattron(wcal, COLOR_PAIR(4));
  }
  waddstr(wcal, " x=");
  if(mcal->dzx)
  {
    snprintf(line, COLS, "%d", *mcal->dzx);
    waddstr(wcal, line);
  }
  else
  {
    waddstr(wcal, _("N/A"));
  }
  waddstr(wcal, " (F3)");
  if(current_cal == DZX)
  {
    wattron(wcal, COLOR_PAIR(1));
  }
  if(current_cal == DZY)
  {
    wattron(wcal, COLOR_PAIR(4));
  }
  waddstr(wcal, " y=");
  if(mcal->dzy)
  {
    snprintf(line, COLS, "%d", *mcal->dzy);
    waddstr(wcal, line);
  }
  else
  {
    waddstr(wcal, _("N/A"));
  }
  waddstr(wcal, " (F4)");
  if(current_cal == DZY)
  {
    wattron(wcal, COLOR_PAIR(1));
  }
  if(current_cal == DZS)
  {
    wattron(wcal, COLOR_PAIR(4));
  }
  waddstr(wcal, _(" shape="));
  if(mcal->dzs)
  {
    if (*mcal->dzs == E_SHAPE_CIRCLE)
    {
      waddstr(wcal, _("circle"));
    }
    else
    {
      waddstr(wcal, _("rectangle"));
    }
  }
  else
  {
    waddstr(wcal, _(" N/A"));
  }
  waddstr(wcal, " (F5)");
  if(current_cal == DZS)
  {
    wattron(wcal, COLOR_PAIR(1));
  }
  wclrtoeol(wcal);
  mvwaddstr(wcal, 3, 1, _("Acceleration:"));
  if(current_cal == TEST)
  {
    wattron(wcal, COLOR_PAIR(4));
  }
  waddstr(wcal, _(" test (F6)"));
  if(current_cal == TEST)
  {
    wattron(wcal, COLOR_PAIR(1));
  }
  if(current_cal == EX)
  {
    wattron(wcal, COLOR_PAIR(4));
  }
  waddstr(wcal, " x=");
  if(mcal->ex)
  {
    snprintf(line, COLS, "%.2f", *mcal->ex);
    waddstr(wcal, line);
  }
  else
  {
    waddstr(wcal, _("N/A"));
  }
  waddstr(wcal, " (F7)");
  if(current_cal == EX)
  {
    wattron(wcal, COLOR_PAIR(1));
  }
  if(current_cal == EY)
  {
    wattron(wcal, COLOR_PAIR(4));
  }
  waddstr(wcal, " y=");
  if(mcal->ey)
  {
    snprintf(line, COLS, "%.2f", *mcal->ey);
    waddstr(wcal, line);
  }
  else
  {
    waddstr(wcal, _("N/A"));
  }
  waddstr(wcal, " (F8)");
  if(current_cal == EY)
  {
    wattron(wcal, COLOR_PAIR(1));
  }
  wclrtoeol(wcal);
  mvwaddstr(wcal, 4, 1, _("Sensitivity:"));
  if(current_cal == MX)
  {
    wattron(wcal, COLOR_PAIR(4));
  }
  if(mcal->mx)
  {
    snprintf(line, COLS, " %.2f", *mcal->mx);
    waddstr(wcal, line);
  }
  else
  {
    waddstr(wcal, _(" N/A"));
  }
  waddstr(wcal, " (F9)");
  if(current_cal == MX)
  {
    wattron(wcal, COLOR_PAIR(1));
  }
  wclrtoeol(wcal);
  mvwaddstr(wcal, 5, 1, "X/Y:");
  if(current_cal == RD || current_cal == VEL)
  {
    wattron(wcal, COLOR_PAIR(4));
  }
  waddstr(wcal, _(" circle test"));
  if(current_cal == RD || current_cal == VEL)
  {
    wattron(wcal, COLOR_PAIR(1));
  }
  waddstr(wcal, ", ");
  if(current_cal == RD)
  {
    wattron(wcal, COLOR_PAIR(4));
  }
  snprintf(line, COLS, _("radius=%d (F10)"), mcal->rd);
  waddstr(wcal, line);
  if(current_cal == RD)
  {
    wattron(wcal, COLOR_PAIR(1));
  }
  waddstr(wcal, ", ");
  if(current_cal == VEL)
  {
    wattron(wcal, COLOR_PAIR(4));
  }
  snprintf(line, COLS, _("velocity=%d (F11)"), mcal->vel);
  waddstr(wcal, line);
  if(current_cal == VEL)
  {
    wattron(wcal, COLOR_PAIR(1));
  }
  if(current_cal == MY)
  {
    wattron(wcal, COLOR_PAIR(4));
  }
  waddstr(wcal, _(" ratio="));
  if(mcal->mx && mcal->my)
  {
    snprintf(line, COLS, "%.2f", *mcal->my / *mcal->mx);
    waddstr(wcal, line);
  }
  else
  {
    waddstr(wcal, _("N/A"));
  }
  waddstr(wcal, " (F12)");
  if(current_cal == MY)
  {
    wattron(wcal, COLOR_PAIR(1));
  }
  wclrtoeol(wcal);
  box(wcal, 0 , 0);
  wnoutrefresh(wcal);
}

void display_init()
{
  initscr();

  if(has_colors() == FALSE)
  {
    endwin();
    printf("Your terminal does not support color\n");
    exit(1);
  }
  start_color();
  init_pair(1, COLOR_WHITE, COLOR_BLACK);
  init_pair(2, COLOR_WHITE, COLOR_WHITE);
  init_pair(3, COLOR_BLACK, COLOR_RED);
  init_pair(4, COLOR_RED, COLOR_BLACK);

  wrefresh(stdscr);//first call clears the screen
  
  mvaddstr(1, LSTICK_X_P + 1, _("Left stick"));
  mvaddstr(1, RSTICK_X_P + 1, _("Right stick"));
  mvaddstr(1, BUTTON_X_P + 1, _("Buttons"));

  lstick = newwin(STICK_Y_L, STICK_X_L, LSTICK_Y_P, LSTICK_X_P);
  box(lstick, 0 , 0);
  wnoutrefresh(lstick);

  rstick = newwin(STICK_Y_L, STICK_X_L, RSTICK_Y_P, RSTICK_X_P);
  box(rstick, 0 , 0);
  wnoutrefresh(rstick);

  wbuttons = newwin(BUTTON_Y_L, BUTTON_X_L, BUTTON_Y_P, BUTTON_X_P);
  box(wbuttons, 0 , 0);
  wnoutrefresh(wbuttons);

  mvaddstr(CAL_Y_P, CAL_X_P + 1, _("Mouse calibration (Ctrl+F1 to edit)"));

  wcal = newwin(7, COLS-3, CAL_Y_P + 1, CAL_X_P);
  box(wcal, 0 , 0);
  wnoutrefresh(wcal);

  mvaddstr(LINES-1, 1, _("Refresh rate:"));
  mvaddstr(LINES-1, COLS-strlen(SHIFT_ESC), SHIFT_ESC);

  doupdate();

#ifndef WIN32
  gettimeofday(&t0, NULL);
#else
  QueryPerformanceCounter(&t0);
  QueryPerformanceFrequency(&freq);
#endif
}

void display_end()
{
  if(wcal)
  {
    endwin();
  }
}

int last_button_nb = 0;
int cpt = 0;
int cpt_total = 0;

void display_run(int axis[])
{
  int i;
  int d;
  char label[BUTTON_LENGTH];
  char rate[COLS];
  int tdiff;

  cpt++;

#ifndef WIN32
  gettimeofday(&t1, NULL);

  tdiff = (t1.tv_sec * 1000000 + t1.tv_usec) - (t0.tv_sec * 1000000 + t0.tv_usec);
#else
  QueryPerformanceCounter(&t1);

  tdiff = (t1.QuadPart - t0.QuadPart) * 1000000 / freq.QuadPart;
#endif

  if(tdiff > RATE_PERIOD)
  {
    cpt_total += cpt;
    sprintf(rate, _("Processing time: current=%dus average=%dus worst=%dus"), proc_time/cpt, proc_time_total/cpt_total, proc_time_worst);
    mvaddstr(LINES-2, 1, rate);
    clrtoeol();
    sprintf(rate, _("Refresh rate: %dHz  "), cpt*1000000/RATE_PERIOD);
    mvaddstr(LINES-1, 1, rate);
    t0 = t1;
    cpt = 0;
    proc_time = 0;
  }

  d = 0;

  for(i=sa_acc_x; i<SA_MAX; ++i)
  {
    if(axis[i])
    {
      snprintf(label, sizeof(label), "%8s: %4d", get_axis_name(i), axis[i]);
      mvwaddstr(wbuttons, 1 + d, 1, label);
      d++;
    }
    wnoutrefresh(wbuttons);
    if(d == BUTTON_Y_L - 3)
    {
      break;
    }
  }
  memset(label, ' ', sizeof(label));
  label[sizeof(label)-1] = '\0';
  for(i=d; i<last_button_nb; ++i)
  {
    mvwaddstr(wbuttons, 1 + i, 1, label);
  }
  last_button_nb = d;

  mvwaddch(lstick, cross[0][1], cross[0][0], ' ');
  cross[0][0] = STICK_X_L / 2 + (double)axis[sa_lstick_x] / get_max_signed(sa_lstick_x) * (STICK_X_L / 2 - 1);
  cross[0][1] = STICK_Y_L / 2 + (double)axis[sa_lstick_y] / get_max_signed(sa_lstick_y) * (STICK_Y_L / 2 - 1);
  if(cross[0][0] <= 0 || cross[0][0] >= STICK_X_L-1 || cross[0][1] <= 0 || cross[0][1] >= STICK_Y_L-1)
  {
    mvwaddch(lstick, cross[0][1], cross[0][0], CROSS_CHAR | COLOR_PAIR(3));
  }
  else
  {
    mvwaddch(lstick, cross[0][1], cross[0][0], CROSS_CHAR);
  }
  wnoutrefresh(lstick);

  mvwaddch(rstick, cross[1][1], cross[1][0], ' ');
  cross[1][0] = STICK_X_L / 2 + (double)axis[sa_rstick_x] / get_max_signed(sa_rstick_x) * (STICK_X_L / 2 - 1);
  cross[1][1] = STICK_Y_L / 2 + (double)axis[sa_rstick_y] / get_max_signed(sa_rstick_y) * (STICK_Y_L / 2 - 1);
  if(cross[1][0] <= 0 || cross[1][0] >= STICK_X_L-1 || cross[1][1] <= 0 || cross[1][1] >= STICK_Y_L-1)
  {
    mvwaddch(rstick, cross[1][1], cross[1][0], CROSS_CHAR | COLOR_PAIR(3));
  }
  else
  {
    mvwaddch(rstick, cross[1][1], cross[1][0], CROSS_CHAR);
  }
  wnoutrefresh(rstick);

  move(LINES-1, COLS-1);
  wnoutrefresh(stdscr);
  doupdate();
}

/*int main(int argc, char* argv[])
{
  int i;
  display_init();
  int axes[4] = {16,16,0,0};
  int buttons[BUTTON_NB] = {1, 0, 1};

  for(i=0; i<10000; ++i)
  {
    display_run(axes, 32, buttons, 1);
    usleep(10000);
  }

  display_end();

  return 0;
}*/
