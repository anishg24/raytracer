//
// Created by anish on 3/4/23.
//

#ifndef RAYTRACING_COLOR_H
#define RAYTRACING_COLOR_H

#include "vec3.h"

#include <iostream>

void write_color(std::ostream &out, color pixel_color, int samples_per_pixel) {
    double r = pixel_color.x();
    double g = pixel_color.y();
    double b = pixel_color.z();

    // Gamma correction (gamma = 2) and anti-aliasing calculation
    double scale = 1. / samples_per_pixel;
    r = sqrt(scale * r);
    g = sqrt(scale * g);
    b = sqrt(scale * b);

    // Write translated [0, 255] value of each color component
    out << static_cast<int>(256 * clamp(r, 0.0, 0.999)) << ' '
        << static_cast<int>(256 * clamp(g, 0.0, 0.999)) << ' '
        << static_cast<int>(256 * clamp(b, 0.0, 0.999)) << '\n';
}

#endif //RAYTRACING_COLOR_H
