#ifndef CAMERA_H
#define CAMERA_H

#include "hittable.h"
#include "material.h"

class camera {
    public:
        double aspect_ratio = 1.0;
        int    image_width  = 100;
        int    samples_per_pixel = 10;
        int    max_depth = 50;

        double vfov = 90;
        point3 lookfrom = point3(0, 0, 0);
        point3 lookat = point3(0, 0, -1);

        vec3 vup = vec3(0, 1, 0);

        double defocus_angle = 0;
        double focus_dist = 10;

        void render(const hittable& world) {
            initialize();

            //Render
            std::cout << "P3\n" << image_width << " " << image_height << "\n255\n";

            for (size_t j = 0; j < image_height; ++j) {
                std::clog << "\rScanlines remaining: " << (image_height - j) << ' ' << std::flush;
                for (size_t i = 0; i < image_width; ++i) {
                    color pixel_color;
                    for (size_t sample = 0; sample < samples_per_pixel; ++sample) {
                        ray r = get_ray(i, j);
                        pixel_color += ray_color(r, max_depth, world);
                    }

                    write_color(std::cout, pixel_color * pixel_samples_ratio);
                }
            }

            std::clog << "\rDone.                      \n";
        }

    private:
        int    image_height;
        point3 center;
        point3 pixel00_loc;
        vec3 pixel_delta_u;
        vec3 pixel_delta_v;       
        double pixel_samples_ratio; //Color scale factor for a sum of pixel samples
        vec3 u,v,w;
        vec3 defocus_disk_u;
        vec3 defocus_disk_v;

        void initialize() {
            image_height = int(image_width * (1 / aspect_ratio));
            image_height = (image_height < 1) ? 1 : image_height;

            pixel_samples_ratio = 1.0 / samples_per_pixel;

            center = lookfrom;

            //Calculate viewport dimensions
            auto theta = degree_to_radians(vfov);
            auto h = std::tan(theta / 2);
            auto viewport_height = 2 * h * focus_dist;
            auto viewport_width = viewport_height * (double(image_width) / image_height);

            //Calculate the u,v,w unit basis vectors for the camerao coordinate frame.
            w = unit_vector(lookfrom - lookat);
            u = unit_vector(cross(vup, w));
            v = cross(w, u);


            //Calculate the vectors across the horizontal and down the vertical viewport edges.
            vec3 viewport_u = viewport_width * u;
            vec3 viewport_v = viewport_height * -v;

            //Calculate the delta vectors horizontally and vertically.
            pixel_delta_u = viewport_u / image_width;
            pixel_delta_v = viewport_v / image_height;

            //Calculate the upper-left pixel location;
            pixel00_loc = center - focus_dist * w - viewport_u/2 - viewport_v/2 + 0.5 * (pixel_delta_u + pixel_delta_v);

            //Calculate the camera defocus disk basis vectors 
            auto defocus_radius = focus_dist * std::tan(degree_to_radians(defocus_angle / 2));
            defocus_disk_u = defocus_radius * u;
            defocus_disk_v = defocus_radius * v;
        }

        color ray_color(const ray& r, int depth, const hittable& world) const {
            if (depth <= 0) {
                return color(0, 0, 0);
            }

            hit_record rec;
            if (world.hit(r, interval(0.001, infinity), rec)) {
                ray scattered;
                color attenuation;
                if (rec.mat->scatter(r, rec, attenuation, scattered)) {
                    return attenuation * ray_color(scattered, depth - 1, world);
                }

                return color(0, 0, 0);
            }

            vec3 unit_direction = unit_vector(r.direction());
            auto a = 0.5 * (unit_direction.y() + 1.0);
            return (1.0 -a) * color(1.0, 1.0, 1.0) + a * color (0.5, 0.7, 1.0);
        }

        ray get_ray(int i, int j) const {
            auto offset = sample_square();
            auto pixel_sample = pixel00_loc + (i + offset.x()) * pixel_delta_u + (j + offset.y()) * pixel_delta_v;

            auto ray_origin = (defocus_angle <= 0) ? center : defocus_disk_sample();
            auto ray_direction = pixel_sample - ray_origin;

            return ray(ray_origin, ray_direction);
        }

        vec3 sample_square() const {
            //Return the vetor to a random point in the [-0.5, -0.5] to [0.5, 0.5] range
            return vec3(random_double() - 0.5, random_double() - 0.5, 0);
        }

        vec3 defocus_disk_sample() const {
            auto p = random_in_unit_disk();
            return center + (p[0] * defocus_disk_u) + (p[1] * defocus_disk_v);
        }
}; 

#endif