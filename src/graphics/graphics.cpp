#include "graphics.h"
#include <iostream>

gbs_opus::graphics::graphics() : m_target(), m_bordercol(SDL_Color{255, 255, 255, 255}),
    m_fillcol(SDL_Color{100, 100, 100, 255}), m_linethickness(1.f)
{

}


void
gbs_opus::graphics::draw_rect(const GPU_Rect &rect)
{
    GPU_Rectangle2(m_target, rect, m_bordercol);

}

void gbs_opus::graphics::draw_circle(float x, float y, float radius)
{
    GPU_Circle(m_target, x, y, radius, m_bordercol);
}

void gbs_opus::graphics::draw_rect_filled(const GPU_Rect &rect, bool bordered)
{
    GPU_RectangleFilled2(m_target, rect, m_fillcol);
    if (bordered)
        draw_rect(rect);
}

void gbs_opus::graphics::draw_circle_filled(float x, float y, float radius, bool bordered)
{
    GPU_CircleFilled(m_target, x, y, radius, m_fillcol);
    if (bordered)
        draw_circle(x, y, radius);
}

void gbs_opus::graphics::clear()
{
    GPU_ClearColor(m_target, m_clearcol);
}

bool gbs_opus::graphics::init()
{
    return init(GPU_GetContextTarget());
}

bool gbs_opus::graphics::init(GPU_Target *t)
{
    if (!t)
    {
        std::cerr << "Error: tried to initialize graphics with context target, "
                     "but it was null!\n";
        return false;
    }

    m_target = t;
    return true;
}

void gbs_opus::graphics::draw_line(float start_x, float start_y, float end_x, float end_y)
{
    GPU_SetLineThickness(m_linethickness);
    GPU_Line(m_target, start_x, start_y, end_x, end_y, m_fillcol);
}

void gbs_opus::graphics::draw_image(image *img, float x, float y)
{
    GPU_Rect src { 0, 0, (float)img->img()->w, (float)img->img()->h };
    GPU_Blit(img->img(), &src, m_target, x, y);
}

void gbs_opus::graphics::present()
{
    GPU_Flip(m_target);
}
