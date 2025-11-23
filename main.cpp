#include <windows.h>
#include <windowsx.h>
#include <winhttp.h>
#include <string>
#include <memory>
#include <algorithm>

#include "HtmlTokenizer.h"
#include "DomParser.h"
#include "CssExtractor.h"
#include "CssTokenizer.h"
#include "CssParser.h"
#include "CssOM.h"
#include "Style.h"
#include "Layout.h"

#pragma comment(lib, "winhttp.lib")

HWND hUrlEdit;
HWND hGoButton;

std::shared_ptr<LayoutBox> g_layoutRoot;
HFONT g_hFont = NULL;
int g_scrollY = 0;
std::wstring g_currentUrl;

const int VIEW_OFFSET_Y = 45;

std::wstring ToWide(const std::string& s)
{
    std::wstring ws;
    ws.reserve(s.size());
    for (unsigned char c : s)
        ws.push_back((wchar_t)c);
    return ws;
}

std::string ToNarrow(const std::wstring& ws)
{
    std::string s;
    s.reserve(ws.size());
    for (wchar_t c : ws)
        s.push_back((char)(c <= 0xFF ? c : '?'));
    return s;
}

std::wstring MakeAbsoluteUrl(const std::wstring& pageUrlW, const std::string& href)
{
    if (href.rfind("http://", 0) == 0 || href.rfind("https://", 0) == 0)
        return ToWide(href);

    std::string pageUrl = ToNarrow(pageUrlW);
    size_t schemePos = pageUrl.find("://");
    if (schemePos == std::string::npos)
        return ToWide(href);

    std::string scheme = pageUrl.substr(0, schemePos);
    std::string rest = pageUrl.substr(schemePos + 3);

    size_t slashPos = rest.find('/');
    std::string host = slashPos == std::string::npos ? rest : rest.substr(0, slashPos);
    std::string path = slashPos == std::string::npos ? "/" : rest.substr(slashPos);

    std::string abs;

    if (!href.empty() && href[0] == '/')
    {
        abs = scheme + "://" + host + href;
    }
    else
    {
        std::string dir = path;
        size_t lastSlash = dir.rfind('/');
        if (lastSlash != std::string::npos)
            dir = dir.substr(0, lastSlash + 1);
        else
            dir = "/";
        abs = scheme + "://" + host + dir + href;
    }

    return ToWide(abs);
}

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

COLORREF ParseColor(const std::string* v, COLORREF def)
{
    if (!v || v->empty())
        return def;

    std::string s = *v;
    std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) { return (char)std::tolower(c); });

    if (!s.empty() && s[0] == '#' && s.size() == 7)
    {
        auto hex = [&](char c) -> int {
            if (c >= '0' && c <= '9') return c - '0';
            if (c >= 'a' && c <= 'f') return 10 + (c - 'a');
            return 0;
            };

        int r = hex(s[1]) * 16 + hex(s[2]);
        int g = hex(s[3]) * 16 + hex(s[4]);
        int b = hex(s[5]) * 16 + hex(s[6]);
        return RGB(r, g, b);
    }

    if (s == "red") return RGB(255, 0, 0);
    if (s == "green") return RGB(0, 128, 0);
    if (s == "blue") return RGB(0, 0, 255);
    if (s == "black") return RGB(0, 0, 0);
    if (s == "white") return RGB(255, 255, 255);
    if (s == "gray" || s == "grey") return RGB(128, 128, 128);
    if (s == "yellow") return RGB(255, 255, 0);
    if (s == "cyan") return RGB(0, 255, 255);
    if (s == "magenta") return RGB(255, 0, 255);

    return def;
}

void DrawLayoutBox(HDC hdc, std::shared_ptr<LayoutBox> box, int offsetY, int scrollY)
{
    int drawX = box->rect.x;
    int drawY = box->rect.y - scrollY + offsetY;
    int w = box->rect.width;
    int h = box->rect.height;

    RECT r;
    r.left = drawX;
    r.top = drawY;
    r.right = drawX + w;
    r.bottom = drawY + h;

    if (box->styled->dom->type == NodeType::Element)
    {
        const std::string* bg = box->styled->styles.get("background-color");
        if (!bg)
            bg = box->styled->styles.get("background");

        if (bg && !bg->empty())
        {
            COLORREF bgCol = ParseColor(bg, RGB(255, 255, 255));
            HBRUSH brush = CreateSolidBrush(bgCol);
            FillRect(hdc, &r, brush);
            DeleteObject(brush);
        }
    }
    else
    {
        const std::string* col = box->styled->styles.get("color");
        COLORREF textCol = ParseColor(col, RGB(0, 0, 0));
        SetTextColor(hdc, textCol);
        SetBkMode(hdc, TRANSPARENT);

        std::wstring wtext(box->styled->dom->name.begin(), box->styled->dom->name.end());
        DrawTextW(hdc, wtext.c_str(), (int)wtext.size(), &r, DT_LEFT | DT_TOP | DT_WORDBREAK);
    }

    for (auto& child : box->children)
        DrawLayoutBox(hdc, child, offsetY, scrollY);
}

void LoadPage(HWND hwnd, const std::wstring& pageUrlW)
{
    g_currentUrl = pageUrlW;

    std::string html = DownloadURL(pageUrlW);
    auto tokens = HtmlTokenizer::Tokenize(html);
    auto dom = DomParser::Parse(tokens);

    auto styleTexts = CssExtractor::ExtractStyles(dom);
    auto styleLinks = CssExtractor::ExtractStyleLinks(dom);

    CSSStyleSheet sheet;
    int nextOrder = 0;

    for (auto& cssText : styleTexts)
    {
        auto cssTokens = CssTokenizer::Tokenize(cssText);
        CSSStyleSheet part = CssParser::Parse(cssTokens);

        for (auto& rule : part.rules) {
            rule.order = nextOrder++;
            sheet.rules.push_back(rule);
        }
    }

    for (auto& href : styleLinks)
    {
        std::wstring absUrl = MakeAbsoluteUrl(pageUrlW, href);
        std::string css = DownloadURL(absUrl);

        auto cssTokens = CssTokenizer::Tokenize(css);
        CSSStyleSheet part = CssParser::Parse(cssTokens);

        for (auto& rule : part.rules) {
            rule.order = nextOrder++;
            sheet.rules.push_back(rule);
        }
    }

    auto styledRoot = StyleEngine::BuildTree(dom, sheet);

    RECT rc;
    GetClientRect(hwnd, &rc);
    int clientWidth = rc.right - rc.left;
    int viewportWidth = clientWidth - 20;
    if (viewportWidth < 100)
        viewportWidth = 100;

    g_layoutRoot = LayoutEngine::Build(styledRoot, viewportWidth);
    g_scrollY = 0;

    InvalidateRect(hwnd, NULL, TRUE);

    SetWindowTextW(hUrlEdit, pageUrlW.c_str());
}

std::shared_ptr<LayoutBox> HitTestLayout(std::shared_ptr<LayoutBox> box, int x, int y, int offsetY, int scrollY)
{
    if (!box) return nullptr;

    for (auto& child : box->children)
    {
        auto hit = HitTestLayout(child, x, y, offsetY, scrollY);
        if (hit)
            return hit;
    }

    int sx = box->rect.x;
    int sy = box->rect.y - scrollY + offsetY;
    int ex = sx + box->rect.width;
    int ey = sy + box->rect.height;

    if (x >= sx && x < ex && y >= sy && y < ey)
        return box;

    return nullptr;
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

        g_hFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
        SendMessage(hUrlEdit, WM_SETFONT, (WPARAM)g_hFont, TRUE);
        SendMessage(hGoButton, WM_SETFONT, (WPARAM)g_hFont, TRUE);
        break;
    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);

        HGDIOBJ oldFont = NULL;
        if (g_hFont)
            oldFont = SelectObject(hdc, g_hFont);

        RECT rc;
        GetClientRect(hwnd, &rc);

        HBRUSH bk = (HBRUSH)GetStockObject(WHITE_BRUSH);
        FillRect(hdc, &rc, bk);

        if (g_layoutRoot)
        {
            DrawLayoutBox(hdc, g_layoutRoot, VIEW_OFFSET_Y, g_scrollY);
        }

        if (oldFont)
            SelectObject(hdc, oldFont);

        EndPaint(hwnd, &ps);
    }
    break;
    case WM_COMMAND:
        if (LOWORD(wParam) == 2)
        {
            wchar_t urlW[512];
            GetWindowText(hUrlEdit, urlW, 512);
            LoadPage(hwnd, urlW);
        }
        break;
    case WM_MOUSEWHEEL:
    {
        int delta = GET_WHEEL_DELTA_WPARAM(wParam);
        int step = 40;
        if (delta > 0)
            g_scrollY -= step;
        else if (delta < 0)
            g_scrollY += step;

        if (g_scrollY < 0)
            g_scrollY = 0;

        if (g_layoutRoot)
        {
            RECT rc;
            GetClientRect(hwnd, &rc);
            int viewHeight = (rc.bottom - rc.top) - VIEW_OFFSET_Y;
            if (viewHeight < 0) viewHeight = 0;

            int contentHeight = g_layoutRoot->rect.height;
            int maxScroll = contentHeight - viewHeight;
            if (maxScroll < 0) maxScroll = 0;

            if (g_scrollY > maxScroll)
                g_scrollY = maxScroll;
        }

        InvalidateRect(hwnd, NULL, TRUE);
    }
    break;
    case WM_LBUTTONDOWN:
    {
        int x = GET_X_LPARAM(lParam);
        int y = GET_Y_LPARAM(lParam);

        if (g_layoutRoot)
        {
            auto hitBox = HitTestLayout(g_layoutRoot, x, y, VIEW_OFFSET_Y, g_scrollY);

            std::shared_ptr<StyledNode> node;
            if (hitBox)
                node = hitBox->styled;

            while (node)
            {
                auto dom = node->dom;
                if (dom->type == NodeType::Element && dom->name == "a")
                {
                    const DomAttribute* hrefAttr = dom->getAttribute("href");
                    if (hrefAttr && !hrefAttr->value.empty())
                    {
                        std::wstring absUrl = MakeAbsoluteUrl(g_currentUrl, hrefAttr->value);
                        LoadPage(hwnd, absUrl);
                    }
                    break;
                }

                if (node->parent.expired())
                    break;
                node = node->parent.lock();
            }
        }
    }
    break;
    case WM_SIZE:
    {
        int width = LOWORD(lParam);
        int height = HIWORD(lParam);

        MoveWindow(hUrlEdit, 10, 10, width - 80, 25, TRUE);
        MoveWindow(hGoButton, width - 65, 10, 55, 25, TRUE);
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