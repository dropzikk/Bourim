#include <windows.h>
#include <winhttp.h>
#include <string>
#include "HtmlTokenizer.h"
#include "DomParser.h"
#include "CssExtractor.h"
#include "CssParser.h"
#include <functional>

#pragma comment(lib, "winhttp.lib")

HWND hUrlEdit;
HWND hGoButton;
HWND hOutput;

std::string DownloadURL(const std::wstring& url)
{
    std::string result;

    URL_COMPONENTS urlComp{};
    urlComp.dwStructSize = sizeof(urlComp);

    wchar_t host[256];
    wchar_t path[1024];

    urlComp.lpszHostName = host;
    urlComp.dwHostNameLength = _countof(host);
    urlComp.lpszUrlPath = path;
    urlComp.dwUrlPathLength = _countof(path);

    if (!WinHttpCrackUrl(url.c_str(), 0, 0, &urlComp)) {
        return "URL parse error";
    }

    HINTERNET hSession = WinHttpOpen(L"MiniBrowser/1.0",
        WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
        WINHTTP_NO_PROXY_NAME,
        WINHTTP_NO_PROXY_BYPASS, 0);

    if (!hSession) return "WinHttpOpen failed";

    HINTERNET hConnect = WinHttpConnect(hSession,
        urlComp.lpszHostName,
        urlComp.nPort, 0);
    if (!hConnect) {
        WinHttpCloseHandle(hSession);
        return "Connect failed";
    }

    DWORD flags = 0;
    if (urlComp.nScheme == INTERNET_SCHEME_HTTPS)
        flags |= WINHTTP_FLAG_SECURE;

    HINTERNET hRequest = WinHttpOpenRequest(
        hConnect, L"GET", urlComp.lpszUrlPath,
        NULL, WINHTTP_NO_REFERER,
        WINHTTP_DEFAULT_ACCEPT_TYPES, flags);

    if (!hRequest) {
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        return "Request failed";
    }

    BOOL sent = WinHttpSendRequest(hRequest,
        WINHTTP_NO_ADDITIONAL_HEADERS, 0,
        WINHTTP_NO_REQUEST_DATA, 0, 0, 0);

    if (!sent || !WinHttpReceiveResponse(hRequest, NULL)) {
        WinHttpCloseHandle(hRequest);
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        return "Send/Receive failed";
    }

    DWORD dwSize = 0;
    do {
        char buf[1024];
        DWORD downloaded = 0;

        if (!WinHttpReadData(hRequest, buf, sizeof(buf), &downloaded))
            break;

        result.append(buf, downloaded);

        dwSize = downloaded;
    } while (dwSize > 0);

    WinHttpCloseHandle(hRequest);
    WinHttpCloseHandle(hConnect);
    WinHttpCloseHandle(hSession);

    return result;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_CREATE:
        hUrlEdit = CreateWindow(L"EDIT", L"https://example.com",
            WS_CHILD | WS_VISIBLE | WS_BORDER,
            10, 10, 400, 25,
            hwnd, (HMENU)1, NULL, NULL);

        hGoButton = CreateWindow(L"BUTTON", L"Go",
            WS_CHILD | WS_VISIBLE,
            420, 10, 50, 25,
            hwnd, (HMENU)2, NULL, NULL);

        hOutput = CreateWindow(L"EDIT", L"",
            WS_CHILD | WS_VISIBLE | WS_BORDER |
            ES_MULTILINE | ES_AUTOVSCROLL | ES_AUTOHSCROLL |
            WS_VSCROLL,
            10, 45, 560, 350,
            hwnd, (HMENU)3, NULL, NULL);
        break;

    case WM_COMMAND:
        if (LOWORD(wParam) == 2)
        {
            wchar_t url[512];
            GetWindowText(hUrlEdit, url, 512);

            std::string html = DownloadURL(url);
            auto tokens = HtmlTokenizer::Tokenize(html);
            auto dom = DomParser::Parse(tokens);

            auto styles = CssExtractor::ExtractStyles(dom);

            CSSStyleSheet sheet;

            for (auto& cssText : styles)
            {
                auto cssTokens = CssTokenizer::Tokenize(cssText);
                CSSStyleSheet part = CssParser::Parse(cssTokens);

                for (auto& rule : part.rules)
                    sheet.rules.push_back(rule);
            }

            std::string output;

            for (auto& rule : sheet.rules)
            {
                output += "Selector: " + rule.selector + "\n";
                for (auto& d : rule.declarations)
                    output += "  " + d.property + ": " + d.value + "\n";
                output += "\n";
            }

            std::wstring wo(output.begin(), output.end());
            SetWindowText(hOutput, wo.c_str());
        }
        break;
    case WM_SIZE:
    {
        int width = LOWORD(lParam);
        int height = HIWORD(lParam);

        MoveWindow(hUrlEdit, 10, 10, width - 80, 25, TRUE);
        MoveWindow(hGoButton, width - 65, 10, 55, 25, TRUE);
        MoveWindow(hOutput, 10, 45, width - 20, height - 55, TRUE);
    }
    break;

    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    }

    return DefWindowProc(hwnd, msg, wParam, lParam);
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, PWSTR, int nCmdShow)
{
    const wchar_t CLASS_NAME[] = L"MiniBrowserClass";

    WNDCLASS wc{};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);

    RegisterClass(&wc);

    HWND hwnd = CreateWindowEx(
        0,
        CLASS_NAME,
        L"Mini Browser",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,
        600, 450,
        NULL,
        NULL,
        hInstance,
        NULL
    );

    ShowWindow(hwnd, nCmdShow);

    MSG msg{};
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}