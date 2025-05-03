#include <iostream>
#include <vector>
#include <boost/geometry.hpp>
#include <boost/geometry/geometries/point_xy.hpp>
#include <boost/geometry/geometries/polygon.hpp>
#include <boost/geometry/io/io.hpp>

namespace bg = boost::geometry;

// Define types
typedef bg::model::d2::point_xy<double> point_type;
typedef bg::model::polygon<point_type> polygon_type;
typedef std::vector<polygon_type> layer_type;

// Function to perform AND operation between mask and input layers
layer_type layer_and(const layer_type& mask_layer, const layer_type& input_layer) {
    layer_type result;
    for (const auto& mask_poly : mask_layer) {
        for (const auto& input_poly : input_layer) {
            std::vector<polygon_type> output;
            bg::intersection(mask_poly, input_poly, output);
            result.insert(result.end(), output.begin(), output.end());
        }
    }
    return result;
}

// Helper function to print a layer
void print_layer(const layer_type& layer) {
    for (size_t i = 0; i < layer.size(); ++i) {
        std::cout << "Polygon " << i+1 << ":\n";
        std::cout << bg::wkt(layer[i]) << "\n\n";
    }
}

int main() {
    // Create mask layer (rectangle from (0,0) to (50,50))
    polygon_type mask_poly;
    bg::append(mask_poly.outer(), point_type(0, 0));
    bg::append(mask_poly.outer(), point_type(50, 0));
    bg::append(mask_poly.outer(), point_type(50, 50));
    bg::append(mask_poly.outer(), point_type(0, 50));
    bg::append(mask_poly.outer(), point_type(0, 0));
    layer_type mask_layer = {mask_poly};

    // Create input layer (rectangle from (25,25) to (75,75))
    polygon_type input_poly;
    bg::append(input_poly.outer(), point_type(25, 25));
    bg::append(input_poly.outer(), point_type(75, 25));
    bg::append(input_poly.outer(), point_type(75, 75));
    bg::append(input_poly.outer(), point_type(25, 75));
    bg::append(input_poly.outer(), point_type(25, 25));
    layer_type input_layer = {input_poly};

    // Perform AND operation
    layer_type result = layer_and(mask_layer, input_layer);

    // Print results
    std::cout << "Mask Layer:\n";
    print_layer(mask_layer);

    std::cout << "Input Layer:\n";
    print_layer(input_layer);

    std::cout << "Result Layer (AND):\n";
    print_layer(result);

    return 0;
}

