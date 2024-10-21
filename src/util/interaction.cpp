#include "util/interaction.h"

TWIN_NAMESPACE_BEGIN

    Interaction::Interaction() : ray(Ray()), relative_pixel(cppgl::vec2(0, 0)), disable_camera_interpolation(false), left_click(false), pressed(false), shift(false) {}

    std::ostream &operator<<(std::ostream &os, const Interaction &intera) {
        os << "[Interaction] Leftclick: " << intera.left_click << " Pressed: " << intera.pressed << " Shift: " << intera.shift << " Disable Cam Interpolation: " << intera.disable_camera_interpolation << " Relative pixel: " << intera.relative_pixel.transpose() << " Ray: " << intera.ray;
        return os;
    }

TWIN_NAMESPACE_END