#include <iostream>
#include <Windows.h>
#include <tchar.h>
#include <Psapi.h>
#include <thread>


/*
struct win_data {
    std::wstring name;
    HWND handle;
}; */


void GetConsoleInput(HWND*, char*);
void SampleCursor(HWND*, char*);

//BOOL CALLBACK EnumWindowsCallback(HWND, LPARAM);


int main()
{

    HWND handle = NULL;

    char* running = new char;

    *running = 1;

    std::thread console_in (GetConsoleInput, &handle, running);

    std::thread sample (SampleCursor, &handle, running);
    

    // If this is reached, user typed in "quit" to the console.
    console_in.join();
    sample.join();

    return 0;
}


void GetConsoleInput(HWND* handle, char* running) {

    /*
    std::string appNameStr;

    char appNameChar[128];
    */

    std::cout << "Press F4 to enable quick access to the window in focus.\n" << std::endl;

    while(*running == 1) {

        
        

        /*
        std::cin.getline(appNameChar, sizeof(appNameChar));


        appNameStr.assign(appNameChar, 128);

        
        // Checks if user wants to quit.
        if (appNameStr.compare("quit") == 1) {

            *running = 0;
            break;
        }

        
        TCHAR* appName = new TCHAR[appNameStr.size() + 1];
        appName[appNameStr.size()] = 0;

        std::copy(appNameStr.begin(), appNameStr.end(), appName);


        *handle = FindWindow(NULL, appName);
        */

        /*
        std::wstring appNameW = std::wstring(appNameStr.begin(), appNameStr.end());

        win_data data = { appNameW, *handle };

        EnumWindows(EnumWindowsCallback, (LPARAM)&data);
        */

        if (GetKeyState(VK_F4) < 0) {

            

            HWND newHandle = GetForegroundWindow();

            if (newHandle != *handle) {

                *handle = newHandle;

                /*
                WCHAR title[128];

                GetWindowText(*handle, title, 128);

                if (*handle != NULL) {
                    std::wcout << "Found " << title << std::endl;
                } */

            }

        }

        Sleep(100);
        
    }

}


void SampleCursor(HWND* handle, char* running) {


    while (*handle == NULL && *running == 1) {

        // Breaks out of while-loop if user decides to quit before selecting window.
        

        // Blocks thread until a handle is found for a window.
        Sleep(1000);
    }


    int X_BOUND = GetSystemMetrics(SM_XVIRTUALSCREEN);
    int Y_BOUND = GetSystemMetrics(SM_YVIRTUALSCREEN);


    POINT pos;

    while(*running == 1) {

        // Look for cursor position.
        GetCursorPos(&pos);


        // If it is within bounds, display window for quick access.
        if (pos.x <= X_BOUND && pos.y <= Y_BOUND) {

            ShowWindow(*handle, SW_SHOWNORMAL);

            SetWindowPos(*handle, HWND_TOP, 0, 0, 0, 0, SWP_NOSIZE);

            SetForegroundWindow(*handle);


            // Used to keep track of whether the window is maximized or not.
            WINDOWPLACEMENT placement;
            placement.length = sizeof(WINDOWPLACEMENT);

            RECT windowBounds;

            // This checks if the cursor is in the bounds of the window. If not, the window will minimize.
            do {

                Sleep(100);

                // Checks to see if window is maximized. 
                // If so, break and then don't minimize window after the cursor leaves the bounds.
                GetWindowPlacement(*handle, &placement);

                if (placement.showCmd == SW_SHOWMAXIMIZED) {
                    break;
                }


                GetWindowRect(*handle, &windowBounds);
                GetCursorPos(&pos);

                
            } while (((pos.x >= windowBounds.left && pos.x <= windowBounds.right) && 
                      (pos.y >= windowBounds.top && pos.y <= windowBounds.bottom)) );


            if (placement.showCmd != SW_SHOWMAXIMIZED) {
                ShowWindow(*handle, SW_MINIMIZE);
            }
        }


        // Delay between sampling cursor position.
        Sleep(100);

    }

}

