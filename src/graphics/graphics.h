//
// Created by Aaron Ishibashi on 11/26/22.
//

#ifndef GBS_OPUS_GRAPHICS_H
#define GBS_OPUS_GRAPHICS_H
#include <SDL_gpu.h>
#include "image.h"


namespace gbs_opus
{
    class graphics {
    public:
        graphics();

        /// Uses default target
        bool init();
        /// Init with a set target
        bool init(GPU_Target *t);

        // ===== Setters/Getters ===========================================
        void border_color(SDL_Color color) { m_bordercol = color; }
        [[nodiscard]] SDL_Color border_color() const { return m_bordercol; }
        void fill_color(SDL_Color color) { m_fillcol = color; }
        [[nodiscard]] SDL_Color fill_color() const { return m_fillcol; }
        void clear_color(SDL_Color color) { m_clearcol = color; }
        [[nodiscard]] SDL_Color clear_color() const { return m_clearcol; }
        void image_tint(SDL_Color color) { m_img_tint = color; }
        [[nodiscard]] SDL_Color image_tint() const { return m_img_tint; }
        void line_thickness(float thickness) { m_linethickness = thickness; }
        [[nodiscard]] float line_thickness() const { return m_linethickness; }

        GPU_Target *target() { return m_target; }
        void target(GPU_Target *t) { m_target = t; }

        // ===== Drawing Funcs ================================================
        void draw_rect(const GPU_Rect &rect);
        void draw_rect_filled(const GPU_Rect &rect, bool bordered);
        void draw_circle(float x, float y, float radius);
        void draw_circle_filled(float x, float y, float radius, bool bordered);
        void draw_line(float start_x, float start_y, float end_x, float end_y);

        void draw_image(image *img, float x, float y);

        void clear();
        void present();
    private:
        SDL_Color m_fillcol, m_bordercol, m_clearcol, m_img_tint;
        float m_linethickness;
        /// Does not manage this target
        GPU_Target *m_target;
    };
}



#endif //GBS_OPUS_GRAPHICS_H
