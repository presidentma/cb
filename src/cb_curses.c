#include "include/cb_curses.h"

static bool terminalHasColors=FALSE;

void cb_curses_start()
{
    initscr();
    keypad(stdscr, TRUE);
    cbreak();
    intrflush(stdscr,FALSE);
    noecho();
    nonl();
    terminalHasColors=has_colors();
    if(terminalHasColors){
        start_color();
        use_default_colors();
    }
}
void exit_curses()
{
    clear();
    refresh();
    doupdate();
    endwin();
}