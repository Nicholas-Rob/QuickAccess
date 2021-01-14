#include <iostream>
#include <Windows.h>
#include <tchar.h>

#include <thread>


void GetConsoleInput(HWND*);
void SampleCursor(HWND*);


int main()
{

    HWND handle = NULL;

    std::thread console_in (GetConsoleInput, &handle);

    std::thread sample (SampleCursor, &handle);
    

    // If this is reached, user typed in "quit" to the console.
    console_in.join();

    return 0;
}


void GetConsoleInput(HWND* handle) {


    std::string appNameStr;

    char appNameChar[128];

    while(1) {

        std::cout << "\nEnter the name of the program that you want quick access to:" << std::endl;

        //std::cin >> appNameStr;
        std::cin.getline(appNameChar, sizeof(appNameChar));


        appNameStr.assign(appNameChar, 128);


        // Checks if user wants to quit.
        if (appNameStr.compare("quit") == 0) break;



        TCHAR* appName = new TCHAR[appNameStr.size() + 1];
        appName[appNameStr.size()] = 0;

        std::copy(appNameStr.begin(), appNameStr.end(), appName);


        *handle = FindWindow(NULL, appName);

        if (*handle != NULL) {
            std::cout << "Found " << appNameStr << std::endl;
        }
    }

}


void SampleCursor(HWND* handle) {

    while (*handle == NULL) {
        // Blocks thread until a handle is found for a window.
        Sleep(1000);
    }


    int X_BOUND = GetSystemMetrics(SM_XVIRTUALSCREEN);
    int Y_BOUND = GetSystemMetrics(SM_YVIRTUALSCREEN);


    POINT pos;

    while(1) {

        // Look for cursor position.
        GetCursorPos(&pos);


        // If it is within bounds, display window for quick access.
        if (pos.x <= X_BOUND && pos.y <= Y_BOUND) {

            ShowWindow(*handle, SW_SHOWNORMAL);

            SetWindowPos(*handle, HWND_TOP, 0, 0, 0, 0, SWP_NOSIZE);

            SetForegroundWindow(*handle);

        }

        // Delay between sampling cursor position.
        Sleep(100);

    }
}
