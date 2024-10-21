
#ifndef TWIN_INTERACTION_H
#define TWIN_INTERACTION_H

#include "twinDefines.h"
#include "util/ray.h"
#include <memory>
#include <cppgl.h>

TWIN_NAMESPACE_BEGIN

    class Interaction {
    public:
        Interaction();

        Ray ray;
        cppgl::vec2 relative_pixel;
        bool disable_camera_interpolation, left_click, pressed, shift;

        friend std::ostream &operator<<(std::ostream &os, const Interaction &intera);
    };
TWIN_NAMESPACE_END

#endif //TWIN_INTERACTION_H
