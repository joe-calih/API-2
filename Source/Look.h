#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include <cmath>

namespace ui
{
using namespace juce;

// ---- palette ---------------------------------------------------------------------
static const Colour bgTop       { 0xff343434 };
static const Colour bgBottom    { 0xff242424 };
static const Colour panelFill   { 0xff262626 };
static const Colour panelEdge   { 0xffc6c6c6 };
static const Colour textWhite   { 0xffeeeeee };
static const Colour textBlue    { 0xff3d8bff };
static const Colour dotBlue     { 0xff2c72e6 };
static const Colour blueTop     { 0xff0a2a98 };
static const Colour blueMid     { 0xff1a4fd2 };
static const Colour blueBottom  { 0xff0a2890 };

constexpr float kA0 = -2.35619449f;   // -135 deg
constexpr float kA1 =  2.35619449f;   // +135 deg

inline Font font (float h, bool bold = false, float kern = 0.f)
{
    Font f ("Arial", h, bold ? Font::bold : Font::plain);
    f.setExtraKerningFactor (kern);
    return f;
}

inline void drawLed (Graphics& g, Point<float> c, float r, Colour on, bool lit)
{
    if (lit)
    {
        const float gr = r * 2.6f;
        g.setGradientFill (ColourGradient (on.withAlpha (0.5f), c, on.withAlpha (0.f), c.translated (gr, 0.f), true));
        g.fillEllipse (c.x - gr, c.y - gr, gr * 2.f, gr * 2.f);
    }
    g.setColour (Colours::black);
    g.fillEllipse (c.x - r - 1.3f, c.y - r - 1.3f, (r + 1.3f) * 2.f, (r + 1.3f) * 2.f);
    const Colour body = lit ? on.brighter (0.15f) : on.darker (0.88f);
    g.setGradientFill (ColourGradient (body.brighter (lit ? 0.7f : 0.4f), c.x - r * 0.35f, c.y - r * 0.35f,
                                       body.darker (0.25f), c.x + r, c.y + r, true));
    g.fillEllipse (c.x - r, c.y - r, r * 2.f, r * 2.f);
}

// ---- look and feel (rotary knobs) -------------------------------------------------
class ApiLookAndFeel : public LookAndFeel_V4
{
public:
    ApiLookAndFeel()
    {
        setColour (PopupMenu::backgroundColourId, Colour (0xff1e1e1e));
        setColour (PopupMenu::textColourId, Colour (0xffe0e0e0));
        setColour (PopupMenu::highlightedBackgroundColourId, Colour (0xff1a4fd2));
        setColour (PopupMenu::headerTextColourId, textBlue);
        setColour (TooltipWindow::backgroundColourId, Colour (0xff101010));
        setColour (TooltipWindow::textColourId, Colours::white);
        setColour (BubbleComponent::backgroundColourId, Colour (0xff101010));
        setColour (BubbleComponent::outlineColourId, Colour (0xff6a6a6a));
    }

    // style: 0 black, 1 grey, 2 red, 3 small black
    void drawRotarySlider (Graphics& g, int x, int y, int w, int h, float pos,
                           float startA, float endA, Slider& s) override
    {
        const int style = (int) s.getProperties().getWithDefault ("style", 0);
        const auto b = Rectangle<float> ((float) x, (float) y, (float) w, (float) h).reduced (1.5f);
        const float R = jmin (b.getWidth(), b.getHeight()) * 0.5f;
        const auto c = b.getCentre();
        const float ang = startA + pos * (endA - startA);

        Colour skirtHi, skirtLo, capHi, capLo;
        switch (style)
        {
            case 1:  skirtHi = Colour (0xff6e6e6e); skirtLo = Colour (0xff2e2e2e);
                     capHi   = Colour (0xffc4c4c4); capLo   = Colour (0xff6c6c6c); break;
            case 2:  skirtHi = Colour (0xffa51a1a); skirtLo = Colour (0xff4a0707);
                     capHi   = Colour (0xffee4a4a); capLo   = Colour (0xff9a1010); break;
            default: skirtHi = Colour (0xff3c3c3c); skirtLo = Colour (0xff0a0a0a);
                     capHi   = Colour (0xff5a5a5a); capLo   = Colour (0xff151515); break;
        }

        g.setColour (Colours::black.withAlpha (0.55f));
        g.fillEllipse (c.x - R + 1.5f, c.y - R + 2.6f, R * 2.f, R * 2.f);

        g.setGradientFill (ColourGradient (skirtHi, c.x - R * 0.5f, c.y - R * 0.7f,
                                           skirtLo, c.x + R * 0.6f, c.y + R * 0.9f, true));
        g.fillEllipse (c.x - R, c.y - R, R * 2.f, R * 2.f);

        if (style == 2 || style == 1)   // knurled edge
        {
            g.setColour (Colours::black.withAlpha (0.30f));
            const int n = style == 2 ? 40 : 48;
            for (int i = 0; i < n; ++i)
            {
                const float a = (float) i * MathConstants<float>::twoPi / (float) n;
                g.drawLine (c.x + std::sin (a) * R * 0.90f, c.y - std::cos (a) * R * 0.90f,
                            c.x + std::sin (a) * R,         c.y - std::cos (a) * R, 1.1f);
            }
        }

        const float capR = R * 0.78f;
        g.setGradientFill (ColourGradient (capHi, c.x - capR * 0.45f, c.y - capR * 0.6f,
                                           capLo, c.x + capR * 0.55f, c.y + capR * 0.8f, true));
        g.fillEllipse (c.x - capR, c.y - capR, capR * 2.f, capR * 2.f);
        g.setColour (Colours::white.withAlpha (0.14f));
        g.drawEllipse (c.x - capR, c.y - capR, capR * 2.f, capR * 2.f, 1.f);
        g.setColour (Colours::black.withAlpha (0.45f));
        g.drawEllipse (c.x - R, c.y - R, R * 2.f, R * 2.f, 1.f);

        // pointer
        Path ptr;
        ptr.addRoundedRectangle (-1.3f, -R * 0.93f, 2.6f, R * 0.72f, 1.2f);
        g.setColour (Colours::white.withAlpha (0.96f));
        g.fillPath (ptr, AffineTransform::rotation (ang).translated (c.x, c.y));
    }
};

// ---- static backdrop (cached) -----------------------------------------------------
class Backdrop : public Component
{
public:
    struct Tick { String label; float p; };

    Backdrop() { setInterceptsMouseClicks (false, false); setBufferedToImage (true); }

    static Point<float> onCircle (Point<float> c, float r, float p)
    {
        const float a = kA0 + p * (kA1 - kA0);
        return { c.x + std::sin (a) * r, c.y - std::cos (a) * r };
    }

    void scale (Graphics& g, Point<float> c, float rLabel, float rDot, const std::vector<Tick>& ticks, float size = 11.f)
    {
        for (auto& t : ticks)
        {
            auto d = onCircle (c, rDot, t.p);
            g.setColour (dotBlue);
            g.fillEllipse (d.x - 1.6f, d.y - 1.6f, 3.2f, 3.2f);
            if (t.label.isNotEmpty())
            {
                auto l = onCircle (c, rLabel, t.p);
                txt (g, t.label, l.x, l.y, size, textWhite, true);
            }
        }
    }

    static void txt (Graphics& g, const String& s, float cx, float cy, float size, Colour col,
                     bool bold = false, float kern = 0.f)
    {
        g.setFont (font (size, bold, kern));
        g.setColour (col);
        g.drawText (s, Rectangle<float> (cx - 70.f, cy - size * 0.75f, 140.f, size * 1.5f), Justification::centred, false);
    }

    static void txtL (Graphics& g, const String& s, float x, float cy, float size, Colour col, bool bold = false)
    {
        g.setFont (font (size, bold));
        g.setColour (col);
        g.drawText (s, Rectangle<float> (x, cy - size * 0.75f, 90.f, size * 1.5f), Justification::centredLeft, false);
    }

    void panel (Graphics& g, Rectangle<float> r, const String& title)
    {
        g.setColour (panelFill);
        g.fillRoundedRectangle (r, 13.f);
        g.setColour (panelEdge);
        g.drawRoundedRectangle (r.reduced (0.7f), 13.f, 1.4f);
        if (title.isNotEmpty())
            txt (g, title, r.getCentreX(), r.getY() + 11.f, 13.5f, textWhite, false, 0.16f);
    }

    void paint (Graphics& g) override
    {
        const float W = (float) getWidth(), H = (float) getHeight();

        g.setGradientFill (ColourGradient (bgTop, 0.f, 0.f, bgBottom, 0.f, H, false));
        g.fillAll();

        // ---- toolbar
        g.setGradientFill (ColourGradient (Colour (0xff3e3e3e), 0.f, 0.f, Colour (0xff1c1c1c), 0.f, 22.f, false));
        g.fillRect (0.f, 0.f, W, 22.f);
        g.setColour (Colour (0xff0c0c0c));
        g.drawLine (0.f, 22.f, W, 22.f, 1.f);

        // ---- logo (original mark)
        {
            Path tri;
            tri.addTriangle (8.f, 62.f, 30.f, 38.f, 30.f, 86.f);
            g.setColour (Colour (0xff1c7bf0));
            g.fillPath (tri);
            g.fillRoundedRectangle (30.f, 52.f, 38.f, 22.f, 2.f);
            txt (g, "SBC", 51.f, 63.5f, 15.f, Colours::white, true, 0.05f);
        }

        // ---- panels
        panel (g, { 10.f, 127.f, 520.f, 126.f }, "COMPRESSOR");
        panel (g, { 10.f, 262.f, 302.f, 128.f }, "TONE");
        panel (g, { 322.f, 262.f, 208.f, 128.f }, "LINK");
        panel (g, { 393.f, 30.f, 137.f, 89.f }, "");
        txt (g, "VU", 449.f, 44.f, 15.f, textWhite, false, 0.1f);

        // VU source
        txt (g, "source", 426.f, 80.f, 9.5f, textBlue);
        txt (g, "METER", 424.f, 94.f, 11.f, textWhite, true);
        txtL (g, "GR",  484.f, 57.f, 9.5f, textWhite, true);
        txtL (g, "OUT", 484.f, 75.f, 9.5f, textWhite, true);
        txtL (g, "IN",  484.f, 94.f, 9.5f, textWhite, true);

        // ---- compressor knobs: scales and labels
        const Point<float> cThr { 73.f, 193.f }, cAtk { 172.f, 193.f }, cRat { 262.f, 193.f },
                           cRel { 355.f, 193.f }, cVar { 447.f, 191.f };

        auto lin = [] (float v) { return (10.f - v) / 30.f; };
        scale (g, cThr, 36.f, 26.f, { { "+10", lin (10) }, { "+8", lin (8) }, { "+4", lin (4) }, { "0", lin (0) },
                                      { "-4", lin (-4) }, { "-8", lin (-8) }, { "-12", lin (-12) }, { "-20", lin (-20) } });
        const float s7 = 1.f / 6.f;
        scale (g, cAtk, 36.f, 26.f, { { ".03", 0 }, { ".1", s7 }, { ".3", 2 * s7 }, { "1", 3 * s7 },
                                      { "3", 4 * s7 }, { "10", 5 * s7 }, { "30", 1 } });
        scale (g, cRat, 36.f, 26.f, { { "1.5", 0 }, { "2", s7 }, { "3", 2 * s7 }, { "4", 3 * s7 },
                                      { "6", 4 * s7 }, { "10", 5 * s7 }, { String::fromUTF8 ("\xe2\x88\x9e"), 1 } });
        scale (g, cRel, 36.f, 26.f, { { ".05", 0 }, { ".1", s7 }, { ".2", 2 * s7 }, { ".5", 3 * s7 },
                                      { "1", 4 * s7 }, { "2", 5 * s7 }, { "", 1 } });
        {   // "50m" label + bracket to the variable knob
            auto p = onCircle (cRel, 36.f, 1.f);
            txt (g, "50m", p.x + 4.f, p.y + 4.f, 11.f, textWhite, true);
            g.setColour (textBlue);
            g.drawLine (p.x - 4.f, p.y - 7.f, p.x + 26.f, p.y - 7.f, 1.f);
            g.drawLine (p.x + 26.f, p.y - 7.f, p.x + 26.f, p.y - 24.f, 1.f);
        }
        auto vp = [] (float t) { return std::log (t / 0.05f) / std::log (60.f); };
        scale (g, cVar, 39.f, 31.f, { { "50m", 0.f }, { ".15s", vp (0.15f) }, { ".25s", vp (0.25f) },
                                      { ".75s", vp (0.75f) }, { "1.5s", vp (1.5f) }, { "3s", 1.f } }, 10.f);

        auto name = [&] (float cx, const String& sub, const String& n, float subY = 222.f, float nameY = 236.f)
        {
            txt (g, sub, cx, subY, 9.5f, textBlue);
            txt (g, n,   cx, nameY, 11.5f, textWhite, true);
        };
        name (73.f,  "dB",     "THRESH");
        name (172.f, "m/sec",  "ATTACK");
        name (262.f, "x:1",    "RATIO");
        name (355.f, "sec",    "RELEASE");
        txt (g, "sec", 447.f, 215.f, 9.5f, textBlue);
        txt (g, "RELEASE", 447.f, 231.f, 11.5f, textWhite, true);
        txt (g, "variable", 447.f, 244.f, 9.5f, textBlue);

        // ---- TONE
        auto bracket = [&] (float keyX, float ledX)
        {
            g.setColour (Colour (0xffc8c8c8));
            g.drawLine (keyX + 10.f, 325.f, ledX - 12.f, 325.f, 1.f);
            g.drawLine (ledX - 12.f, 302.f, ledX - 12.f, 346.f, 1.f);
            for (float y : { 302.f, 346.f }) g.drawLine (ledX - 12.f, y, ledX - 6.f, y, 1.f);
            g.drawLine (ledX - 12.f, 325.f, ledX - 6.f, 325.f, 1.f);
        };
        bracket (30.f, 59.f);   bracket (130.f, 162.f);   bracket (236.f, 262.f);
        txtL (g, "HARD", 66.f, 302.f, 10.f, textWhite, true);
        txtL (g, "MED",  66.f, 325.f, 10.f, textWhite, true);
        txtL (g, "SOFT", 66.f, 347.f, 10.f, textWhite, true);
        txtL (g, "LOUD", 169.f, 302.f, 10.f, textWhite, true);
        txtL (g, "MED",  169.f, 325.f, 10.f, textWhite, true);
        txtL (g, "NORM", 169.f, 347.f, 10.f, textWhite, true);
        txtL (g, "NEW",  269.f, 302.f, 10.f, textWhite, true);
        txtL (g, "OLD",  269.f, 347.f, 10.f, textWhite, true);
        txtL (g, "F FWD",  270.f, 313.f, 8.5f, textBlue);
        txtL (g, "F BACK", 270.f, 337.f, 8.5f, textBlue);
        name (30.f,  "thresh",   "KNEE",   365.f, 377.f);
        name (136.f, "detector", "THRUST", 365.f, 377.f);
        name (240.f, "tone",     "TYPE",   365.f, 377.f);

        // ---- LINK
        txt (g, "BP", 357.f, 292.f, 10.f, textWhite, true);
        txt (g, "HP", 392.f, 292.f, 10.f, textWhite, true);
        txt (g, "LP", 392.f, 339.f, 10.f, textWhite, true);
        g.setColour (Colour (0xffc8c8c8));
        g.drawLine (347.f, 325.f, 354.f, 325.f, 1.f);
        g.drawLine (354.f, 303.f, 354.f, 337.f, 1.f);
        for (float y : { 303.f, 337.f }) g.drawLine (354.f, y, 358.f, y, 1.f);
        name (352.f, "", "SHAPE", 365.f, 377.f);
        const Point<float> cLink { 466.f, 325.f };
        scale (g, cLink, 35.f, 26.f, { { "IND", 0 }, { "50", s7 }, { "60", 2 * s7 }, { "70", 3 * s7 },
                                       { "80", 4 * s7 }, { "90", 5 * s7 }, { "100", 1 } }, 10.5f);
        txt (g, "percent", 466.f, 366.f, 9.5f, textBlue);
        txt (g, "L/R LINK", 466.f, 378.f, 11.5f, textWhite, true);

        // ---- blue output panel
        {
            Rectangle<float> r (543.f, 22.f, 170.f, H - 22.f + 14.f);
            g.setGradientFill (ColourGradient (blueTop, 0.f, r.getY(), blueBottom, 0.f, r.getBottom(), false));
            g.fillRoundedRectangle (r, 14.f);
            ColourGradient glow (blueMid.withAlpha (0.9f), 612.f, 190.f, blueMid.withAlpha (0.f), 612.f, 40.f, true);
            g.setGradientFill (glow);
            g.fillRoundedRectangle (r, 14.f);
            g.setColour (panelEdge);
            g.drawRoundedRectangle (r.reduced (0.7f), 14.f, 1.4f);
        }
        txt (g, "ANALOG", 612.f, 38.f, 12.5f, textWhite, true);
        txt (g, "ON",  612.f, 55.f, 10.f, textWhite, true);
        txt (g, "OFF", 612.f, 106.f, 10.f, textWhite, true);
        txt (g, "OUTPUT", 612.f, 129.f, 13.f, textWhite, true, 0.08f);
        {
            const Point<float> cOut { 612.f, 177.f };
            for (float p : { 0.f, 1.f })
            {
                auto d = onCircle (cOut, 38.f, p);
                g.setColour (dotBlue.brighter (0.5f));
                g.fillEllipse (d.x - 1.8f, d.y - 1.8f, 3.6f, 3.6f);
            }
            txt (g, "-24", 588.f, 217.f, 12.f, textWhite, true);
            txt (g, "+24", 638.f, 217.f, 12.f, textWhite, true);
        }
        txt (g, "IN", 597.f, 267.f, 11.f, textWhite, true);
        txt (g, "manual", 632.f, 264.f, 9.f, Colour (0xff6fa8ff));
        txt (g, "make-up", 632.f, 275.f, 9.f, Colour (0xff6fa8ff));

        scale (g, { 580.f, 314.f }, 19.f, 0.f, { { "0", 0 }, { "100", 1 } }, 10.f);
        scale (g, { 646.f, 314.f }, 20.f, 0.f, { { "-18", 0 }, { "0", 0.5f }, { "+18", 1 } }, 10.f);
        txt (g, "MIX",  580.f, 352.f, 11.f, textWhite, true);
        txt (g, "TRIM", 646.f, 352.f, 11.f, textWhite, true);
        txt (g, "COMPRESSOR", 607.f, 385.f, 10.f, Colour (0xff8fa3d8), false, 0.12f);
    }
};

// ---- LED ---------------------------------------------------------------------------
class Led : public Component
{
public:
    Led (Colour c) : colour (c) { setInterceptsMouseClicks (false, false); }
    void setLit (bool b) { if (b != lit) { lit = b; repaint(); } }
    void paint (Graphics& g) override
    {
        drawLed (g, getLocalBounds().toFloat().getCentre(), 3.1f, colour, lit);
    }
private:
    Colour colour; bool lit = false;
};

// ---- VU meter ----------------------------------------------------------------------
class VuMeter : public Component
{
public:
    void setNeedle (float vu) { if (std::abs (vu - value) > 0.02f) { value = vu; repaint(); } }

    static float pos (float vu)
    {
        const float amp = std::pow (10.f, vu / 20.f);
        return jlimit (-0.04f, 1.06f, (amp - 0.063f) / (1.413f - 0.063f));
    }

    void paint (Graphics& g) override
    {
        auto b = getLocalBounds().toFloat();
        g.setColour (Colour (0xff0d0d0d));
        g.fillRoundedRectangle (b, 7.f);
        g.setColour (Colour (0xff555555));
        g.drawRoundedRectangle (b.reduced (0.5f), 7.f, 1.f);

        auto face = b.reduced (4.f);
        Path clip;  clip.addRoundedRectangle (face, 4.f);
        Graphics::ScopedSaveState ss (g);
        g.reduceClipRegion (clip);

        g.setGradientFill (ColourGradient (Colour (0xfff3ecc8), face.getCentreX(), face.getY(),
                                           Colour (0xffd9ca8c), face.getCentreX(), face.getBottom(), false));
        g.fillRect (face);
        for (float x : { face.getX(), face.getRight() })
        {
            g.setGradientFill (ColourGradient (Colour (0xffffcf4a).withAlpha (0.9f), x, face.getBottom(),
                                               Colour (0xffffcf4a).withAlpha (0.f), x + (x < face.getCentreX() ? 48.f : -48.f), face.getBottom() - 10.f, true));
            g.fillRect (face);
        }

        const Point<float> pv { face.getCentreX(), face.getBottom() + 22.f };
        auto at = [&] (float vu, float r)
        {
            const float a = (-42.f + pos (vu) * 84.f) * MathConstants<float>::pi / 180.f;
            return Point<float> (pv.x + std::sin (a) * r, pv.y - std::cos (a) * r);
        };

        // scale arc + ticks
        Path arc, red;
        for (float v = -24.f; v <= 3.01f; v += 0.5f)
        {
            auto p = at (v, 66.f);
            if (v == -24.f) arc.startNewSubPath (p); else arc.lineTo (p);
        }
        for (float v = 0.f; v <= 3.01f; v += 0.25f)
        {
            auto p = at (v, 66.f);
            if (v == 0.f) red.startNewSubPath (p); else red.lineTo (p);
        }
        g.setColour (Colour (0xff222222));
        g.strokePath (arc, PathStrokeType (1.f));
        g.setColour (Colour (0xffc8201c));
        g.strokePath (red, PathStrokeType (3.f));

        const float majors[] = { -24.f, -12.f, -6.f, -3.f, 0.f, 3.f };
        for (float v = -24.f; v <= 3.01f; v += 1.f)
        {
            bool major = false;
            for (float m : majors) major |= std::abs (v - m) < 0.01f;
            const bool isRed = v >= 0.f;
            g.setColour (isRed ? Colour (0xffc8201c) : Colour (0xff222222));
            auto a = at (v, 66.f), b2 = at (v, major ? 59.f : 62.f);
            g.drawLine (a.x, a.y, b2.x, b2.y, major ? 1.4f : 0.8f);
        }
        for (float m : { -24.f, -12.f, -6.f, -3.f, 0.f })
        {
            auto p = at (m, 51.f);
            g.setFont (font (8.5f, true));
            g.setColour (m >= 0.f ? Colour (0xffc8201c) : Colour (0xff222222));
            g.drawText (String ((int) m), Rectangle<float> (p.x - 12.f, p.y - 6.f, 24.f, 12.f), Justification::centred, false);
        }
        g.setFont (font (11.f, true));
        g.setColour (Colour (0xff222222));
        g.drawText ("VU", Rectangle<float> (face.getRight() - 30.f, face.getBottom() - 24.f, 24.f, 12.f), Justification::centred, false);

        // needle
        auto tip = at (value, 80.f);
        g.setColour (Colours::black.withAlpha (0.25f));
        g.drawLine (pv.x + 1.5f, pv.y, tip.x + 1.5f, tip.y, 1.6f);
        g.setColour (Colour (0xff111111));
        g.drawLine (pv.x, pv.y, tip.x, tip.y, 1.4f);

        // hub plate
        g.setColour (Colour (0xff141414));
        g.fillRoundedRectangle (face.getCentreX() - 15.f, face.getBottom() - 8.f, 30.f, 10.f, 3.f);
        g.fillRect (face.getX(), face.getBottom() - 2.f, face.getWidth(), 2.f);
        // glass sheen
        g.setGradientFill (ColourGradient (Colours::white.withAlpha (0.18f), face.getX(), face.getY(),
                                           Colours::white.withAlpha (0.f), face.getX(), face.getCentreY(), false));
        g.fillRect (face.withHeight (face.getHeight() * 0.5f));
    }
private:
    float value = -30.f;
};

// ---- square key (multi-state, bound to a parameter) -------------------------------
class CycleKey : public Component
{
public:
    CycleKey (AudioProcessorValueTreeState& s, const char* paramId, int numStates_, Colour c,
              bool dimWhenZero_, UndoManager* um, std::function<void (int)> onChange_ = nullptr)
        : numStates (numStates_), colour (c), dimWhenZero (dimWhenZero_), onChange (std::move (onChange_))
    {
        att = std::make_unique<ParameterAttachment> (*s.getParameter (paramId),
            [this] (float v) { state = (int) std::lround (v); repaint(); if (onChange) onChange (state); }, um);
        att->sendInitialUpdate();
    }

    void paint (Graphics& g) override
    {
        auto b = getLocalBounds().toFloat().reduced (1.f);
        if (down) b = b.translated (0.f, 1.f);
        const bool lit = ! dimWhenZero || state != 0;
        Colour face = lit ? colour : colour.darker (0.7f);
        g.setColour (Colours::black.withAlpha (0.6f));
        g.fillRoundedRectangle (b.translated (0.f, 1.5f), 3.f);
        g.setGradientFill (ColourGradient (face.brighter (0.4f), b.getX(), b.getY(),
                                           face.darker (0.3f), b.getX(), b.getBottom(), false));
        g.fillRoundedRectangle (b, 3.f);
        g.setColour (Colours::white.withAlpha (lit ? 0.45f : 0.15f));
        g.drawRoundedRectangle (b.reduced (1.f), 2.5f, 0.8f);
        g.setColour (Colours::black.withAlpha (0.85f));
        g.drawRoundedRectangle (b, 3.f, 1.f);
    }

    void mouseDown (const MouseEvent&) override { down = true; repaint(); }
    void mouseUp (const MouseEvent& e) override
    {
        down = false; repaint();
        if (contains (e.getPosition()) && numStates > 0)
            att->setValueAsCompleteGesture ((float) ((state + 1) % numStates));
    }
private:
    std::unique_ptr<ParameterAttachment> att;
    int numStates, state = 0;
    Colour colour;
    bool dimWhenZero, down = false;
    std::function<void (int)> onChange;
};

// ---- toggle switch (Analog) ----------------------------------------------------------
class ToggleSwitch : public Component
{
public:
    ToggleSwitch (AudioProcessorValueTreeState& s, const char* paramId, UndoManager* um)
    {
        att = std::make_unique<ParameterAttachment> (*s.getParameter (paramId),
            [this] (float v) { on = v > 0.5f; repaint(); }, um);
        att->sendInitialUpdate();
    }
    void paint (Graphics& g) override
    {
        const auto c = getLocalBounds().toFloat().getCentre();
        // hex nut
        Path hex;
        for (int i = 0; i < 6; ++i)
        {
            const float a = MathConstants<float>::pi / 6.f + (float) i * MathConstants<float>::pi / 3.f;
            Point<float> p (c.x + std::cos (a) * 17.f, c.y + std::sin (a) * 17.f);
            if (i == 0) hex.startNewSubPath (p); else hex.lineTo (p);
        }
        hex.closeSubPath();
        g.setColour (Colours::black.withAlpha (0.6f));
        g.fillPath (hex, AffineTransform::translation (1.f, 2.f));
        g.setGradientFill (ColourGradient (Colour (0xffe6e6e6), c.x - 12.f, c.y - 14.f, Colour (0xff5a5a5a), c.x + 12.f, c.y + 14.f, false));
        g.fillPath (hex);
        g.setColour (Colour (0xff222222));
        g.strokePath (hex, PathStrokeType (1.f));
        g.setGradientFill (ColourGradient (Colour (0xff2a2a2a), c.x - 9.f, c.y - 9.f, Colour (0xff0a0a0a), c.x + 9.f, c.y + 9.f, false));
        g.fillEllipse (c.x - 10.f, c.y - 10.f, 20.f, 20.f);

        // lever
        const float dir = on ? -1.f : 1.f;
        Path lever;
        lever.addRoundedRectangle (-3.6f, 0.f, 7.2f, 24.f, 3.f);
        auto t = AffineTransform::rotation (dir > 0.f ? MathConstants<float>::pi : 0.f).translated (c.x, c.y);
        // draw lever pointing up (on) or down (off)
        Path leverUp;  leverUp.addRoundedRectangle (-3.6f, -24.f, 7.2f, 24.f, 3.f);
        g.setColour (Colours::black.withAlpha (0.5f));
        g.fillPath (leverUp, AffineTransform::scale (1.f, on ? 1.f : -1.f).translated (c.x + 1.5f, c.y + 2.f));
        g.setGradientFill (ColourGradient (Colour (0xfff2f2f2), c.x - 4.f, c.y, Colour (0xff7c7c7c), c.x + 4.f, c.y, false));
        g.fillPath (leverUp, AffineTransform::scale (1.f, on ? 1.f : -1.f).translated (c.x, c.y));
        const float ty = c.y + (on ? -24.f : 24.f);
        g.setGradientFill (ColourGradient (Colours::white, c.x - 2.f, ty - 2.f, Colour (0xff8a8a8a), c.x + 4.f, ty + 4.f, true));
        g.fillEllipse (c.x - 5.f, ty - 5.f, 10.f, 10.f);
        juce::ignoreUnused (t, lever);
    }
    void mouseUp (const MouseEvent& e) override
    {
        if (contains (e.getPosition())) att->setValueAsCompleteGesture (on ? 0.f : 1.f);
    }
private:
    std::unique_ptr<ParameterAttachment> att;
    bool on = true;
};

// ---- toolbar button ------------------------------------------------------------------
class ToolButton : public Component
{
public:
    enum Kind { Text, Undo, Redo, Left, Right, Menu };
    ToolButton (Kind k, String t = {}) : kind (k), text (std::move (t)) {}
    std::function<void()> onClick;
    void setText (const String& t) { if (t != text) { text = t; repaint(); } }

    void paint (Graphics& g) override
    {
        auto b = getLocalBounds().toFloat().reduced (0.5f);
        g.setGradientFill (ColourGradient (down ? Colour (0xff262626) : Colour (0xff4a4a4a), 0.f, b.getY(),
                                           down ? Colour (0xff1a1a1a) : Colour (0xff2c2c2c), 0.f, b.getBottom(), false));
        g.fillRoundedRectangle (b, 3.f);
        g.setColour (Colour (0xff0e0e0e));
        g.drawRoundedRectangle (b, 3.f, 1.f);
        g.setColour (Colours::white.withAlpha (0.1f));
        g.drawLine (b.getX() + 3.f, b.getY() + 1.5f, b.getRight() - 3.f, b.getY() + 1.5f);

        const auto c = b.getCentre();
        g.setColour (Colour (0xffdcdcdc));
        switch (kind)
        {
            case Text:
                g.setFont (font (11.f)); g.drawText (text, b, Justification::centred, false); break;
            case Left:  { Path p; p.addTriangle (c.x + 4.f, c.y - 5.f, c.x + 4.f, c.y + 5.f, c.x - 5.f, c.y); g.fillPath (p); break; }
            case Right: { Path p; p.addTriangle (c.x - 4.f, c.y - 5.f, c.x - 4.f, c.y + 5.f, c.x + 5.f, c.y); g.fillPath (p); break; }
            case Menu:  for (int i = -1; i <= 1; ++i) g.fillRect (c.x - 5.f, c.y + (float) i * 4.f - 0.75f, 10.f, 1.5f); break;
            case Undo:
            case Redo:
            {
                const float s = kind == Undo ? -1.f : 1.f;
                Path p;
                p.addCentredArc (c.x, c.y + 1.f, 5.f, 5.f, 0.f, s < 0 ? -1.2f : 1.2f, s < 0 ? -4.4f : 4.4f, true);
                g.strokePath (p, PathStrokeType (1.6f));
                Path h;
                h.addTriangle (c.x + s * -5.f, c.y - 7.f, c.x + s * -5.f, c.y - 1.f, c.x + s * -10.f * -0.f - s * 0.f, c.y - 4.f);
                h.clear();
                h.addTriangle (c.x + s * 5.f, c.y - 6.f, c.x + s * 5.f, c.y, c.x + s * 9.f, c.y - 3.f);
                g.fillPath (h);
                break;
            }
        }
    }
    void mouseDown (const MouseEvent&) override { down = true; repaint(); }
    void mouseUp (const MouseEvent& e) override
    {
        down = false; repaint();
        if (contains (e.getPosition()) && onClick) onClick();
    }
private:
    Kind kind; String text; bool down = false;
};

// ---- preset name field ------------------------------------------------------------------
class PresetBox : public Component
{
public:
    std::function<void()> onClick;
    void setText (const String& t) { if (t != text) { text = t; repaint(); } }
    void paint (Graphics& g) override
    {
        auto b = getLocalBounds().toFloat().reduced (0.5f);
        g.setGradientFill (ColourGradient (Colour (0xff1a1a1a), 0.f, b.getY(), Colour (0xff2c2c2c), 0.f, b.getBottom(), false));
        g.fillRoundedRectangle (b, 3.f);
        g.setColour (Colour (0xff0a0a0a));
        g.drawRoundedRectangle (b, 3.f, 1.f);
        g.setColour (Colour (0xffe4e4e4));
        g.setFont (font (11.5f));
        g.drawText (text, b, Justification::centred, true);
        Path t;
        t.addTriangle (b.getRight() - 15.f, b.getCentreY() - 3.f, b.getRight() - 7.f, b.getCentreY() - 3.f, b.getRight() - 11.f, b.getCentreY() + 3.f);
        g.setColour (Colour (0xffbbbbbb));
        g.fillPath (t);
    }
    void mouseUp (const MouseEvent& e) override { if (contains (e.getPosition()) && onClick) onClick(); }
private:
    String text;
};

} // namespace ui
