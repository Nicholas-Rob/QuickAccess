#include <iostream>
#include <Windows.h>
#include <tchar.h>
#include <Psapi.h>
#include <thread>


void GetConsoleInput(HWND*, char*);
void SampleCursor(HWND*, char*);
static BOOL CALLBACK MonitorEnum(HMONITOR hMon, HDC hdc, LPRECT lprcMonitor, LPARAM lParam);


// A struct to hold the screen area of a monitor.
typedef struct monitor {

    RECT area;
    RECT work;

} Monitor;

// A struct to hold every monitor enumerated over.
typedef struct monitors_cont {

    int amount = 0;
    Monitor* monitors;

} MonitorContainer;


// Flag for if the program should be paused.
static char pauseFlag = 0;

// Monitors found.
static MonitorContainer monCon;

// Holds the original placement of the window being "quick accessed"
static WINDOWPLACEMENT originalPlacement;





int main()
{

    // This stops a console window from being open.
    FreeConsole();


    // monitor container set up to hold 3 potential monitors.
    monCon = { 0, new Monitor[3] };

    // Find any monitors connected to computer.
    if (EnumDisplayMonitors(NULL, NULL, MonitorEnum, (LPARAM)&monCon) == 0) {
        return 1;
    }


    // The handle for the window being "quick accessed"
    HWND handle = NULL;

    // Flag used to tell threads when they should no longer be running.
    char* running = new char;
    *running = 1;


    // This thread is used to detect the "F4" key press that would set a
    // window to be "quick accessed". If the console is enabled, it will also handle input.
    std::thread console_in (GetConsoleInput, &handle, running);


    // This thread is used to sample the placement of the cursor and see if it is
    // in the top left corner of any found monitor. If so, open the "quick accessed" window.
    // If the cursor leaves the opened window, the window will then close.
    std::thread sample (SampleCursor, &handle, running);

    
    
    // Wait for the threads to finish, and then let program shutdown.
    console_in.join();
    sample.join();

    return 0;
}




void GetConsoleInput(HWND* handle, char* running) {



    //std::cout << "Press F4 to enable quick access to the window in focus.\n" << std::endl;

    // While the thread should be running...
    while(*running == 1) {

        // Win Key + F4 toggles the quick access interaction. The window being
        // "quick accessed" will not open when cursor is in position.
        if (GetKeyState(VK_F4) < 0 && GetKeyState(VK_LWIN) < 0) {
            pauseFlag = ~pauseFlag;

            Sleep(500);

        }
        else if (GetKeyState(VK_F4) < 0) { // If F4 is pressed, this will save the handle of the window in the foreground, making it "quick accessable".


        HWND newHandle = GetForegroundWindow();

            if (newHandle != *handle) {

                *handle = newHandle;

                GetWindowPlacement(newHandle, &originalPlacement);

            }
           

        }

        Sleep(100);
        
    }

}


void SampleCursor(HWND* handle, char* running) {

    // While there is no window being quick accessable, delay for 1 second and check again.
    while (*handle == NULL && *running == 1 ) {

        // Breaks out of while-loop if user decides to quit before selecting window.
        

        Sleep(1000);
    }


    // Used later to store the cursor placement information.
    POINT pos;

    // Used later to store any monitor screen area information.
    RECT rect;



    // Holds info of any window that got pushed out of focus in order to
    // put the "quick accessed" window into focus.
    HWND prevWin = NULL;

    // Used to store the placement of a window that gets pushed out of focus.
    WINDOWPLACEMENT prevPlacement;
    prevPlacement.length = sizeof(WINDOWPLACEMENT);


    // Used to store information of where a quick accessed window should appear.
    // This will change depending on what monitor the window is meant to pop up in.
    WINDOWPLACEMENT quickWinPlacement;
    quickWinPlacement.length = sizeof(WINDOWPLACEMENT);

    // Used to store the area that the quick accessed window should cover.
    RECT quickRect = { 0,0,0,0 };


    // The width and height of what the quick accessed window should appear as.
    const int QUICK_WIDTH = 800;
    const int QUICK_HEIGHT = 600;



    // While the thread should be running...
    while(*running == 1) {

        // if the program has not been paused by the previous thread...
        if ((pauseFlag & 0x1) == 0x0) {

            // Look for cursor position.
            GetCursorPos(&pos);



            // Find the monitor that the cursur is currently on.
            int monitor;

            for (monitor = 0; monitor < monCon.amount; monitor++) {
                
                rect = monCon.monitors[monitor].area;

                if (pos.x >= rect.left && pos.x <= rect.right &&
                    pos.y >= rect.top && pos.y <= rect.bottom) {

                    break;
                }
            }



            // If it is within bounds, display window for quick access.
            if (pos.x <= rect.left && pos.y <= rect.top) {


                // Save the window that is getting pushed out of focus.
                prevWin = GetForegroundWindow();
                GetWindowPlacement(prevWin, &prevPlacement);


                // Save the original position data of the window being quick accessed
                // in order to restore it afterwards.
                GetWindowPlacement(*handle, &originalPlacement);


                
                // Align the window rect to be at the top-left of the monitor
                // that the cursor was found on.
                quickRect.left = rect.left;
                quickRect.top = rect.top;
                quickRect.right = rect.left + QUICK_WIDTH;
                quickRect.bottom = rect.top + QUICK_HEIGHT;

                // Used to tell Windows to not have the window set to "maximized" or "minimized".
                quickWinPlacement.rcNormalPosition = quickRect;
                quickWinPlacement.showCmd = SW_SHOWNORMAL;


                // Finally, pop the window open at the selected position and put it into focus.
                SetWindowPlacement(*handle, &quickWinPlacement);
                SetForegroundWindow(*handle);


                // Used to keep track of if the window being quick accessed gets maximized.
                // If it has been, then it is no longer being considered "quick accessed" and
                // will not minimize if the cursor is moved out of its bounds.
                WINDOWPLACEMENT placement;
                placement.length = sizeof(WINDOWPLACEMENT);

                RECT windowBounds;



                // This checks if the cursor is in the bounds of the window. If not, the window will minimize.
                do {

                    // Small delay between samples.
                    Sleep(100);


                    // Checks to see if window is maximized. 
                    // If so, break and then don't minimize window after the cursor leaves the bounds.
                    GetWindowPlacement(*handle, &placement);

                    if (placement.showCmd == SW_SHOWMAXIMIZED) {
                        break;
                    }


                    // This is used to keep track of the current window bounds, and the current
                    // cursor placement. If the window is resized (not maximized), it will update the bounds
                    // that the cursor is allowed to be in without minimizing the window.
                    GetWindowRect(*handle, &windowBounds);
                    GetCursorPos(&pos);


                } while (((pos.x >= windowBounds.left && pos.x <= windowBounds.right) &&
                    (pos.y >= windowBounds.top && pos.y <= windowBounds.bottom)));





                // Check if the window has been maximized during the previous do-while loop.
                // If so, do nothing.
                // If not, then restore the "quick accessed" window to its previous position and then minimize it.
                if (placement.showCmd != SW_SHOWMAXIMIZED) {

                    
                    originalPlacement.showCmd = SW_SHOWMINNOACTIVE;
                    SetWindowPlacement(*handle, &originalPlacement);


                    // If a window was pushed out of focus to bring up the quick accessed window,
                    // restore it into focus.
                    if (prevWin != *handle) {
                        SetWindowPlacement(prevWin, &prevPlacement);
                    }
                    

                }


            }
        }

        // Delay between sampling cursor position.
        Sleep(100);

    }

}


// This function is used by the Win32 API to enumerate through all found monitors.
static BOOL CALLBACK MonitorEnum(HMONITOR hMon, HDC hdc, LPRECT lprcMonitor, LPARAM lParam) {
   
    // The lParam is used to hold a pointer to a previously made monitor container struct.
    MonitorContainer* monCon = (MonitorContainer*)lParam;

    MONITORINFO info = {};

    ZeroMemory(&info, sizeof(MONITORINFO));

    info.cbSize = sizeof(MONITORINFO);

    // If a monitor is found that for some reason this callback does not have info for... it failed.
    if (GetMonitorInfo(hMon, &info) == 0) {
        printf("\nfailed\n");
    }


    // Saves total screen area values of each monitor
    RECT* screen = &(monCon->monitors[monCon->amount].area);

    screen->left = (*lprcMonitor).left;
    screen->right = (*lprcMonitor).right;
    screen->top = (*lprcMonitor).top;
    screen->bottom = (*lprcMonitor).bottom;


    // Saves work area values of each monitor
    RECT* work = &(monCon->monitors[monCon->amount].work);

    work->left = info.rcWork.left;
    work->right = info.rcWork.right;
    work->top = info.rcWork.top;
    work->bottom = info.rcWork.bottom;


    // Increments the monitor count in the given monitor container struct.
    monCon->amount++;


    // This callback will only return true in order for the Win32 API to continue
    // enumerating through every monitor it can find.
    return TRUE;

}

