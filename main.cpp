#include "helper.h"

#include "color.h"
#include "material.h"
#include "hittable_list.h"
#include "sphere.h"
#include "camera.h"

#include <iostream>
#include <thread>
#include <chrono>

color ray_color(const ray &r, const hittable &world, int depth) {
    hit_record rec;

    if (depth <= 0)
        return color(0, 0, 0);

    if (world.hit(r, 0.001, infinity, rec)) {
        ray scattered;
        color attenuation;
        if (rec.mat_ptr->scatter(r, rec, attenuation, scattered))
            return attenuation * ray_color(scattered, world, depth - 1);
        return color(0, 0, 0);
    }
    vec3 unit_direction = unit_vector(r.direction());
    double t = 0.5 * (unit_direction.y() + 1.);
    return (1 - t) * color(1., 1., 1.) + t * color(0.5, 0.7, 1.0);
}

hittable_list random_scene() {
    hittable_list world;

    shared_ptr<lambertian> ground_material = make_shared<lambertian>(color(0.5, 0.5, 0.5));
    world.add(make_shared<sphere>(point3(0, -1000, 0), 1000, ground_material));

    double radius = 0.2;
    for (int a = -11; a < 11; a++) {
        for (int b = -11; b < 11; b++) {
            double choose_mat = random_double();
            point3 center(a + 0.9 * random_double(), radius, b + 0.9 * random_double());

            if ((center - point3(4, radius, 0)).length() > 0.9) {
                shared_ptr<material> sphere_material;

                if (choose_mat < 0.8) {
                    // diffuse
                    vec3 albedo = color::random() * color::random();
                    sphere_material = make_shared<lambertian>(albedo);
                    world.add(make_shared<sphere>(center, radius, sphere_material));
                } else if (choose_mat < 0.95) {
                    // metal
                    vec3 albedo = color::random(0.5, 1);
                    double fuzz = random_double(0, 0.5);
                    sphere_material = make_shared<metal>(albedo, fuzz);
                    world.add(make_shared<sphere>(center, radius, sphere_material));
                } else {
                    // glass
                    sphere_material = make_shared<dielectric>(1.5);
                    world.add(make_shared<sphere>(center, radius, sphere_material));
                }
            }
        }
    }

    shared_ptr<dielectric> material1 = make_shared<dielectric>(1.5);
    world.add(make_shared<sphere>(point3(0, 1, 0), 1.0, material1));

    shared_ptr<lambertian> material2 = make_shared<lambertian>(color(0.4, 0.2, 0.1));
    world.add(make_shared<sphere>(point3(-4, 1, 0), 1.0, material2));

    shared_ptr<metal> material3 = make_shared<metal>(color(0.7, 0.6, 0.5), 0.0);
    world.add(make_shared<sphere>(point3(4, 1, 0), 1.0, material3));

    return world;
}

struct render_data {
    render_data(int i, int n, camera &c, hittable_list &w, const int &iw,
                const int &ih, const int &s) : idx(i), num(n), cam(c), world(w), image_width(iw), image_height(ih),
                                   samples_per_pixel(s) {}
    int idx;
    int num;
    camera &cam;
    hittable_list &world;
    const int &image_width;
    const int &image_height;
    const int &samples_per_pixel;
};

void render_scanline(render_data data, std::vector<color> *buf) {
    int max_depth = 50;
    int idx = data.image_height - 1 - data.idx;
    int stop = fmax(idx + 1 - data.num, 0);
    for (int j = idx; j >= stop; --j) {
        for (int i = 0; i < data.image_width; ++i) {
            color pixel_color(0, 0, 0);
            for (int s = 0; s < data.samples_per_pixel; ++s) {
                double u = (i + random_double()) / (data.image_width - 1);
                double v = (j + random_double()) / (data.image_height - 1);

                ray r = data.cam.get_ray(u, v);
                pixel_color += ray_color(r, data.world, max_depth);
            }
            buf->push_back(pixel_color);
        }
    }
    std::cerr << "Finished rendering lines " << data.idx << " to " << data.image_height - stop << std::endl;
}

int main() {

    std::cerr << "Setting up image information... ";

    // Image
    const double aspect_ratio = 16. / 9.;
    const int image_width = 400;
    const int image_height = static_cast<int>(image_width / aspect_ratio);
    const int samples_per_pixel = 100;
    const int max_depth = 50;

//    const double aspect_ratio = 3. / 2.;
//    const int image_width = 1200;
//    const int image_height = static_cast<int>(image_width / aspect_ratio);
//    const int samples_per_pixel = 500;
//    const int max_depth = 50;

    std::cerr << "Done." << std::endl << "Setting up the world... ";

    // World
    // hittable_list world = random_scene();
    hittable_list world;

    shared_ptr<lambertian> material_ground = make_shared<lambertian>(color(0.8, 0.8, 0.0));
    shared_ptr<lambertian> material_center = make_shared<lambertian>(color(0.1, 0.2, 0.5));
    shared_ptr<dielectric> material_left = make_shared<dielectric>(1.5);
    shared_ptr<metal> material_right = make_shared<metal>(color(0.8, 0.6, 0.2), 1.0);

    world.add(make_shared<sphere>(point3(0.0, -100.5, -1.), 100.0, material_ground));
    world.add(make_shared<sphere>(point3(0.0, 0.0, -1.), 0.5, material_center));
    world.add(make_shared<sphere>(point3(-1.0, 0.0, -1.), 0.5, material_left));
    world.add(make_shared<sphere>(point3(1.0, 0.0, -1.), 0.5, material_right));

    std::cerr << "Done." << std::endl << "Setting up the camera... ";

    // Camera

    point3 lookfrom(3, 3, 2);
    point3 lookat(0, 0, -1);
    vec3 vup(0, 1, 0);
    double dist_to_focus = (lookfrom - lookat).length();
    double aperture = 2.0;

    camera cam(lookfrom, lookat, vup, 20, aspect_ratio, aperture, dist_to_focus);

    std::cerr << "Done. " << std::endl << "STARTING RENDER" << std::endl;

    // Render

    std::cout << "P3\n" << image_width << ' ' << image_height << "\n255\n";

    // We have 225 lines to print out
    int num_threads = 15;
    int scanlinesPerThread = ceil(float(image_height) / float(num_threads));

    std::cerr << "Rendering on " << num_threads << " threads ("
              << scanlinesPerThread << " lines per process)" << std::endl;

    // Store our threads and colors for later
    std::vector<std::thread> threads;
    std::vector<std::vector<color>*> buffer;

    auto start = std::chrono::steady_clock::now();
    // Spawn our new threads with the data
    for (int p = 0; p < num_threads; p++) {
        // This could be optimized...
        render_data data(
                p * scanlinesPerThread,
                scanlinesPerThread,
                cam,
                world,
                image_width,
                image_height,
                samples_per_pixel
                );
        // Create a buffer that we will write data to
        std::vector<color> *buf = new std::vector<color>();

        // Spawn the new thread
        std::thread thread(render_scanline, data, buf);

        std::cerr << "Started rendering on lines " << p * scanlinesPerThread << " to "
        << (int)fmin(p * scanlinesPerThread + scanlinesPerThread, image_height) << std::endl;

        // Save the new thread and buffer
        threads.push_back(std::move(thread));
        buffer.push_back(buf);
    }

    int counter = 0;

    // Ensure all jobs are finished then print out the colors
    for (int t = 0; t < threads.size(); t++) {
        threads[t].join();
        for (color line : *buffer[t]){
            write_color(std::cout, line, samples_per_pixel);
            counter ++;
        }
    }

    auto end = std::chrono::steady_clock::now();

    auto diff = end - start;

    std::cerr << counter << " lines written" << std::endl << "Finished rendering in "
              << std::chrono::duration<double, std::milli>(diff).count() << "ms" << std::endl;

    return 0;
}


