#include "fontAtlas.h"

#include <Windows.h>
#include <objidl.h>
#include <gdiplus.h>
#include <algorithm>
#include <cmath>
#include <map>
#include <memory>
#include <new>

#pragma comment(lib, "gdiplus.lib")

int SEFontNumberFrameCount(int totalFrames) {
    if (totalFrames <= 0) return 0;
    int frames = 0;
    if (totalFrames % 10 == 0) frames = 10;
    if (totalFrames % 11 == 0) frames = 11;
    if (totalFrames % 24 == 0) frames = 24;
    return frames;
}

std::wstring SEFontAtlasCellText(const SEFontAtlasRequest& request, int cellIndex) {
    if (cellIndex < 0 || request.columns <= 0 || request.rows <= 0 ||
        request.columns > 256 || request.rows > 256 ||
        cellIndex >= request.columns * request.rows) return L"";
    if (!request.isNumber) return request.judgementText;
    const int frameCount = SEFontNumberFrameCount(request.columns * request.rows);
    if (!frameCount) return L"";
    int glyph = cellIndex % frameCount;
    if (frameCount == 24) {
        if (glyph == 11) return L"+";
        if (glyph == 23) return L"-";
        glyph %= 12;
    }
    return glyph < 10 ? std::wstring(1, wchar_t(L'0' + glyph)) : L"";
}

namespace {
struct GdiPlusSession {
    ULONG_PTR token = 0;
    GdiPlusSession() {
        Gdiplus::GdiplusStartupInput input;
        if (Gdiplus::GdiplusStartup(&token, &input, nullptr) != Gdiplus::Ok) token = 0;
    }
    ~GdiPlusSession() { if (token) Gdiplus::GdiplusShutdown(token); }
};

bool RenderFontAtlas(const SEFontAtlasRequest& request,
    SEFontAtlasBitmap& output, std::string& error) {
    if (request.fontPath.empty()) { error = "Choose a TTF font file first."; return false; }
    if (request.columns < 1 || request.rows < 1 || request.columns > 256 || request.rows > 256 ||
        request.columns * request.rows > 256 || request.cellWidth < 8 || request.cellHeight < 8 ||
        request.cellWidth > 2048 || request.cellHeight > 2048 || request.padding < 0 ||
        request.padding > 64 || request.outlinePixels < 0 || request.outlinePixels > 16 ||
        request.shadowPixels < 0 || request.shadowPixels > 16) {
        error = "Use 1-256 cells, 8-2048 pixel cells, padding 0-64 and effects 0-16 pixels.";
        return false;
    }
    const int width = request.columns * request.cellWidth;
    const int height = request.rows * request.cellHeight;
    if (width > 4096 || height > 4096 || (std::uint64_t)width * height > 16 * 1024 * 1024) {
        error = "The font atlas exceeds 4096 x 4096. Reduce the cell size or frame grid.";
        return false;
    }
    if (request.isNumber && !SEFontNumberFrameCount(request.columns * request.rows)) {
        error = "This number source does not use a supported 10, 11 or 24-cell frame layout.";
        return false;
    }
    if (!request.isNumber && (request.judgementText.empty() || request.judgementText.size() > 64)) {
        error = "Enter a judgement label containing 1-64 characters."; return false;
    }
    const int margin = request.padding + request.outlinePixels + request.shadowPixels + 1;
    const float usableWidth = (float)(request.cellWidth - margin * 2);
    const float usableHeight = (float)(request.cellHeight - margin * 2);
    if (usableWidth < 2 || usableHeight < 2) {
        error = "The cell is too small for the padding, outline and shadow."; return false;
    }
    GdiPlusSession session;
    if (!session.token) { error = "GDI+ could not initialize the font renderer."; return false; }
    Gdiplus::PrivateFontCollection fonts;
    if (fonts.AddFontFile(request.fontPath.c_str()) != Gdiplus::Ok) {
        error = "The font file could not be read. Choose a supported TrueType font."; return false;
    }
    Gdiplus::FontFamily family;
    int found = 0;
    if (fonts.GetFamilies(1, &family, &found) != Gdiplus::Ok || found != 1) {
        error = "The selected file contains no readable font family."; return false;
    }
    int fontStyle = -1;
    for (int style : { Gdiplus::FontStyleRegular, Gdiplus::FontStyleBold,
            Gdiplus::FontStyleItalic, Gdiplus::FontStyleBoldItalic }) {
        if (family.IsStyleAvailable(style)) { fontStyle = style; break; }
    }
    if (fontStyle < 0) { error = "No usable outline style was found in this font."; return false; }

    std::map<std::wstring, std::unique_ptr<Gdiplus::GraphicsPath>> paths;
    float top = 0, bottom = 0, widest = 0;
    bool hasBounds = false;
    const int cells = request.columns * request.rows;
    for (int cell = 0; cell < cells; ++cell) {
        const std::wstring text = SEFontAtlasCellText(request, cell);
        if (text.empty() || paths.count(text)) continue;
        auto path = std::make_unique<Gdiplus::GraphicsPath>();
        if (path->AddString(text.c_str(), (INT)text.size(), &family, fontStyle,
            128.0f, Gdiplus::PointF(0, 0), Gdiplus::StringFormat::GenericTypographic()) != Gdiplus::Ok ||
            path->GetPointCount() <= 0) {
            error = "The selected font could not draw the requested label."; return false;
        }
        Gdiplus::RectF bounds;
        if (path->GetBounds(&bounds) != Gdiplus::Ok) { error = "Font bounds could not be measured."; return false; }
        top = hasBounds ? (std::min)(top, bounds.Y) : bounds.Y;
        bottom = hasBounds ? (std::max)(bottom, bounds.GetBottom()) : bounds.GetBottom();
        widest = (std::max)(widest, bounds.Width);
        hasBounds = true;
        paths.emplace(text, std::move(path));
    }
    if (!hasBounds || widest <= 0 || bottom <= top) { error = "The font has no visible glyph outlines."; return false; }
    // A shared scale and baseline keeps narrow digits and signs proportional.
    const float scale = (std::min)(usableWidth / widest, usableHeight / (bottom - top));
    SEFontAtlasBitmap raster;
    raster.width = width;
    raster.height = height;
    raster.pixels.assign((size_t)width * height, 0);
    Gdiplus::Bitmap bitmap(width, height, width * 4, PixelFormat32bppARGB,
        reinterpret_cast<BYTE*>(raster.pixels.data()));
    Gdiplus::Graphics graphics(&bitmap);
    if (bitmap.GetLastStatus() != Gdiplus::Ok || graphics.GetLastStatus() != Gdiplus::Ok) {
        error = "The font bitmap could not be allocated."; return false;
    }
    graphics.SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);
    Gdiplus::SolidBrush fill(Gdiplus::Color(request.fillArgb));
    Gdiplus::SolidBrush shadow(Gdiplus::Color(request.shadowArgb));
    Gdiplus::Pen outline(Gdiplus::Color(request.outlineArgb), (float)request.outlinePixels * 2.0f);
    outline.SetLineJoin(Gdiplus::LineJoinRound);
    for (int cell = 0; cell < cells; ++cell) {
        const std::wstring text = SEFontAtlasCellText(request, cell);
        if (text.empty()) continue;
        std::unique_ptr<Gdiplus::GraphicsPath> path(paths.at(text)->Clone());
        if (!path) { error = "The font outline could not be copied."; return false; }
        Gdiplus::RectF bounds;
        path->GetBounds(&bounds);
        const float x = (float)((cell % request.columns) * request.cellWidth) +
            (request.cellWidth - bounds.Width * scale) * 0.5f - bounds.X * scale;
        const float y = (float)((cell / request.columns) * request.cellHeight) +
            (request.cellHeight - (bottom - top) * scale) * 0.5f - top * scale;
        Gdiplus::Matrix transform(scale, 0, 0, scale, x, y);
        if (path->Transform(&transform) != Gdiplus::Ok) { error = "The font outline could not be positioned."; return false; }
        if (request.shadowPixels > 0) {
            Gdiplus::Matrix offset;
            offset.Translate((float)request.shadowPixels, (float)request.shadowPixels);
            path->Transform(&offset);
            const Gdiplus::Status status = graphics.FillPath(&shadow, path.get());
            offset.Reset();
            offset.Translate((float)-request.shadowPixels, (float)-request.shadowPixels);
            path->Transform(&offset);
            if (status != Gdiplus::Ok) { error = "The font shadow could not be drawn."; return false; }
        }
        if (request.outlinePixels > 0 && graphics.DrawPath(&outline, path.get()) != Gdiplus::Ok) {
            error = "The font outline could not be drawn."; return false;
        }
        if (graphics.FillPath(&fill, path.get()) != Gdiplus::Ok) { error = "The font fill could not be drawn."; return false; }
    }
    graphics.Flush(Gdiplus::FlushIntentionSync);
    output = std::move(raster);
    return true;
}
} // namespace

bool RenderSEFontAtlas(const SEFontAtlasRequest& request,
    SEFontAtlasBitmap& output, std::string& error) {
    error.clear();
    output = SEFontAtlasBitmap();
    try { return RenderFontAtlas(request, output, error); }
    catch (const std::bad_alloc&) { error = "Not enough memory to generate the font atlas. Reduce its size."; return false; }
}

int RunFontAtlasSelfTest() {
    SEFontAtlasRequest request;
    if (SEFontAtlasCellText(request, 0) != L"0" || SEFontAtlasCellText(request, 9) != L"9") return 1;
    request.columns = 11;
    if (!SEFontAtlasCellText(request, 10).empty()) return 2;
    request.columns = 24;
    if (SEFontAtlasCellText(request, 11) != L"+" || SEFontAtlasCellText(request, 23) != L"-" ||
        SEFontAtlasCellText(request, 12) != L"0" || !SEFontAtlasCellText(request, 22).empty()) return 3;
    request.rows = 2;
    if (SEFontAtlasCellText(request, 35) != L"+" || SEFontAtlasCellText(request, 47) != L"-") return 4;
    if (SEFontNumberFrameCount(13) != 0 || SEFontNumberFrameCount(110) != 11 || SEFontNumberFrameCount(120) != 24) return 5;
    wchar_t windowsDirectory[MAX_PATH] = {};
    if (!GetWindowsDirectoryW(windowsDirectory, MAX_PATH)) return 6;
    request.fontPath = std::wstring(windowsDirectory) + L"\\Fonts\\arial.ttf";
    request.columns = 11;
    request.rows = 1;
    SEFontAtlasBitmap raster;
    std::string error;
    if (!RenderSEFontAtlas(request, raster, error)) return 7;
    for (int cell = 0; cell < 11; ++cell) {
        bool hasInk = false;
        for (int y = 0; y < request.cellHeight; ++y)
            for (int x = 0; x < request.cellWidth; ++x)
                hasInk |= (raster.pixels[(size_t)y * raster.width + cell * request.cellWidth + x] >> 24) != 0;
        if (hasInk != (cell < 10)) return 8;
    }
    if (raster.pixels.front() != 0) return 9;
    request.isNumber = false;
    request.columns = 1;
    request.cellWidth = 256;
    request.judgementText = L"PGREAT";
    if (!RenderSEFontAtlas(request, raster, error) || raster.width != 256 ||
        std::none_of(raster.pixels.begin(), raster.pixels.end(), [](std::uint32_t pixel) { return (pixel >> 24) != 0; })) return 10;
    request.fontPath += L".missing";
    if (RenderSEFontAtlas(request, raster, error) || error.empty() || !raster.pixels.empty()) return 11;
    request.columns = 256;
    request.rows = 256;
    if (RenderSEFontAtlas(request, raster, error) || error.empty()) return 12;
    return 0;
}
