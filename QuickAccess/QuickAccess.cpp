#include <iostream>
#include <Windows.h>
#include <tchar.h>

#include <thread>


void GetConsoleInput(HWND*, char*);
void SampleCursor(HWND*, char*);


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

    std::string appNameStr;

    char appNameChar[128];

    while(*running == 1) {

        std::cout << "\nEnter the name of the program that you want quick access to:" << std::endl;

        
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

        if (*handle != NULL) {
            std::cout << "Found " << appNameStr << std::endl;
        }
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



            RECT windowBounds;

            // This checks if the cursor is in the bounds of the window. If not, the window will minimize.
            do {

                Sleep(100);

                GetWindowRect(*handle, &windowBounds);
                GetCursorPos(&pos);

                
            } while ((pos.x >= windowBounds.left && pos.x <= windowBounds.right) && (pos.y >= windowBounds.top && pos.y <= windowBounds.bottom));


            ShowWindow(*handle, SW_MINIMIZE);
        }


        // Delay between sampling cursor position.
        Sleep(100);

    }

}
