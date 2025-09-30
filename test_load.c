#include <windows.h>
#include <stdio.h>
int main(void){
    HMODULE h = LoadLibraryA("SDL3.dll");
    if(!h){
        DWORD e = GetLastError();
        LPVOID msg = NULL;
        FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                       NULL, e, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&msg, 0, NULL);
        fprintf(stderr, "LoadLibrary failed: code=%u msg=%s\n", (unsigned)e, msg ? (char*)msg : "<null>");
        if(msg) LocalFree(msg);
        return 1;
    }
    printf("LoadLibrary OK\n");
    FreeLibrary(h);
    return 0;
}
