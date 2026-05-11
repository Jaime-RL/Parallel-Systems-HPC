#include <ncurses.h>
#include <unistd.h>
#include "tui.hpp"

// Helper for the history
unsigned char* save_grid(const Grid& grid){
    int real_width = grid.width - 2;
    int real_height = grid.height - 2;
    int i = 0;
    unsigned char* snapshot = (unsigned char*)malloc(real_width * real_height * sizeof(unsigned char));

    for(int y = 1; y <= real_height; ++y){
        for(int x = 1; x <= real_width; ++x){
            snapshot[i] = grid.at(x, y);
            i++;
        }
    }

    return snapshot;
}

// The width and height of the game of life, without the ghost borders
void run_demo(GOL_Base& gol, int width, int height) {
    initscr();
    cbreak();
    keypad(stdscr, TRUE);
    noecho();
    curs_set(0);

    if (has_colors()) {
        start_color();
        use_default_colors(); 
        // Define this color pair for the status bar(s)
        init_pair(1, COLOR_BLACK, COLOR_GREEN); 
    }

    std::vector<unsigned char*> history;
    history.push_back(save_grid(gol.getGrid()));
    int iter = 0;

    while (true) {
        erase(); 
        unsigned char* current = history[iter];

        // ===== Upper status bar =====
        attron(COLOR_PAIR(1)); // Enable status bar colors
        move(0, 0); // Go to upper left corner
        for(int i=0; i<COLS; ++i) addch(' '); // Fill status bar background with empty spaces (for color)
        std::string header = " Game of Life demo | Iter: " + std::to_string(iter) + " / " + std::to_string(history.size()) + " ";
        int header_x = (COLS - header.length()) / 2;
        if (header_x < 0) header_x = 0;
        mvprintw(0, header_x, "%s", header.c_str()); // This way we center the status bar text
        attroff(COLOR_PAIR(1)); // Disable status bar colors

        // ===== Draw the grid =====
        // Offset
        int start_y = 2;
        int start_x = (COLS-(width+2))/2;
        if (start_x < 0) start_x = 0;

        mvaddch(start_y - 1, start_x - 1, '+'); // Upper left corner
        for(int i=0; i < width*2; ++i) addch('-'); // Upper border line
        addch('+'); // Upper right corner

        // Cells
        for (int y = 0; y < height; ++y) {
            mvaddch(start_y + y, start_x - 1, '|'); // Left line
            for (int x = 0; x < width; ++x) {
                if (current[y*width + x] == 1) {
                    addch('@' | A_BOLD);  // # for alive cells
                } else {
                    addch('.'); // . for dead cells (by Motion Twin)
                }
                addch(' '); // Space between cells
            }
            addch('|'); // Right line
        }

        mvaddch(start_y + height, start_x - 1, '+'); // Lower left corner
        for(int i=0; i < width*2; ++i) addch('-'); // Lower border line
        addch('+'); // Lower right corner

        // ===== Lower status bar =====
        int footer_y = LINES - 1; // La última línea disponible
        
        attron(COLOR_PAIR(1)); // Enable status bar colors
        move(footer_y, 0); // Go to lowers left corner
        for(int i=0; i<COLS; ++i) addch(' '); // Fill status bar background with empty spaces (for color)
        std::string footer = " [<-] Previous | [->] Next | [Q] Quit ";
        int footer_x = (COLS - footer.length()) / 2;
        if (footer_x < 0) footer_x = 0;
        mvprintw(footer_y, footer_x, "%s", footer.c_str());  // This way we center the status bar text
        attroff(COLOR_PAIR(1)); // Disable status bar colors

        // ===== User input =====
        int ch = getch();
        if (ch == 'q' || ch == 'Q'){
            break;
        }
        else if(ch == KEY_RIGHT){
            if (iter == (int)history.size() - 1){ 
                gol.run_iteration();
                history.push_back(save_grid(gol.getGrid()));
            }
            iter++;
        }
        else if(ch == KEY_LEFT){
            iter = iter > 0 ? iter - 1 : 0;
        }
    }

    for (unsigned char* ptr : history) {
        free(ptr);
    }

    endwin(); 
}