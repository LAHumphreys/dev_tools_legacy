#ifndef __DEV_TOOLS_CPP_LIBRARIES_LIB_SCREEN_SCREEN_H__
#define __DEV_TOOLS_CPP_LIBRARIES_LIB_SCREEN_SCREEN_H__
#include <string>
#include <vector>
#include <ncurses.h>

// A pane inside the Screen
class Window;
// A specialisation of a window for 
// terminal based input..
class Terminal;

class ScreenLogger;

/*
 * A simplistic wrapper around ncurses which can 
 * be used to drive a basic application with a main
 * terminal and optionally a sider bar and or topbar
 */
class Screen {
public:
    /*********************************
     *          C'tor / D'tor
     ********************************/
    /* * Return the ncurses instance
     */
    static Screen& Instance() {
        static Screen instance;
        return instance;
    }
    
    /*
     * Clean up the ncurses library and any related
     * resources
     */
    virtual ~Screen();
    
    /*********************************
     *          Main
     ********************************/

    /*
     * Return the "main" terminal
     */
    Terminal& MainTerminal() {
        return *main;
    }

    /*********************************
     *          Top Bar
     ********************************/

    /*
     * Return the top bar object
     *
     * If the topbar isn't currently active it is initialised
     */
    Terminal& TopBar();

    // Is the topbar in use?
    bool TopBarShowing() {
        return topbar != nullptr;
    }

    /*
     * Destroy the top bar window 
     */
    void KillTopBar();

    /*********************************
     *          Data Access
     ********************************/

    /*
     * Access fundamental properties of the screen
     */
    int Height() const { return height; }
    int Width() const { return width; }
private:
    /*
     * Initialise the top bar
     */
    void ShowTopBar();

    /*
     * Initialise the ncurses library.
     *
     * We implement as a singleton since it doesn't
     * make any sence to initialise the library multiple
     * times...
     */
    Screen();

    /*
     * Sub-windows
     */
    Terminal*  main;
    Terminal*  topbar;

    /*
     * Screen properties
     */
    int height;
    int width; 
    int topbar_height;

    /*
     * Helper Objects
     */
    ScreenLogger* logger;
};


/*
 * Wraps an ncurses window
 */
class Window {
public:
    struct WIN_INFO{
        int cols;
        int lines;
        int start_col;
        int start_line;
    };

    /*********************************
     *          C'tor / D'tor
     ********************************/
    /*
     * Take ownership of the window _win.
     *
     * Realisticly this should only be called by Screen
     */
    Window(WINDOW* _win,const Window::WIN_INFO& info);

    /*
     * Delete the window, and release all related resources...
     */
    virtual ~Window();

    /*********************************
     *          Output
     ********************************/

     /*
      * Refresh the Window();
      */
     virtual void Refresh();

    /*
     * Place the string at the position.
     *
     * Behaviour is undefined if x,y is not within the window bounds
     */
    virtual void PutString(int x, int y, const std::string& line);

    /*
     * Clear down the screen
     */
    virtual void Clear();

    /*********************************
     *          Move Commands
     ********************************/

    /*
     * Move the window to X, Y
     */
    bool Move( int x, int y);

    /*
     * Move the window to X, Y
     */
    bool Resize( int cols, int lines);

    void Boxed(bool yesno) {
        boxed = yesno;
    }


protected:
    WINDOW* win;
    WIN_INFO info;
    bool boxed;
};

/*
 * Enhance Window with terminal based actions
 */
class Terminal : public Window {
public:
    Terminal(WINDOW* _win, const Window::WIN_INFO& info);

    /*
     * Get a line from the user...
     */
    std::string GetLine(const std::string& prompt);

    /*
     * Put a new line of output...
     */
    void PutString(const std::string& text);

private:
};


#endif