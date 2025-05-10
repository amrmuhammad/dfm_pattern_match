#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <stdexcept> // For std::stoi, std::runtime_error

#include <boost/geometry.hpp>
#include <boost/geometry/geometries/point_xy.hpp>
#include <boost/geometry/geometries/polygon.hpp>
#include <boost/geometry/io/io.hpp>

#include "gdstk/gdstk.hpp" // GDSTK header

namespace bg = boost::geometry;

// Define types for Boost.Geometry
typedef bg::model::d2::point_xy<double> point_type;
typedef bg::model::polygon<point_type> polygon_type;
typedef std::vector<polygon_type> layer_type; // A layer is a vector of polygons

// Function to load a layer from an OASIS file
// Function to load a layer from an OASIS file
layer_type load_layer_from_oasis(const std::string& filename, int layer_number, int datatype_number = 0) {
    std::cout << "Loading layer " << layer_number << ":" << datatype_number << " from " << filename << std::endl;
    layer_type loaded_layer;
    gdstk::Library lib; // Will be populated by the return value of gdstk::read_oas
    gdstk::ErrorCode local_error_code_val;
    // Use 0.0 for unit and precision for gdstk to read them from the OASIS file itself.
    lib = gdstk::read_oas(filename.c_str(), 0.0, 0.0, &local_error_code_val);

    if (local_error_code_val != gdstk::ErrorCode::NoError) {
        std::cerr << "Error reading OASIS file: " << filename << " (Error code: " << (int)local_error_code_val << ")" << std::endl;
        lib.clear(); // Even if read_oas failed and returned an empty/default lib, clear it.
        return loaded_layer; // Return empty layer on error
    }

    // Iterate through all cells in the library
    for (uint64_t i = 0; i < lib.cell_array.count; ++i) {
        gdstk::Cell* cell = lib.cell_array[i];
        if (cell) {
            // Iterate through polygons in the current cell
            for (uint64_t j = 0; j < cell->polygon_array.count; ++j) {
                gdstk::Polygon* gdstk_poly = cell->polygon_array[j];
                if (gdstk_poly && gdstk::get_layer(gdstk_poly->tag) == (uint32_t)layer_number && gdstk::get_type(gdstk_poly->tag) == (uint32_t)datatype_number) {
                    polygon_type boost_poly; // Create a Boost polygon
                    gdstk::Array<gdstk::Vec2>& points = gdstk_poly->point_array;
                    
                    if (points.count < 3) { // A polygon needs at least 3 points
                        std::cerr << "Warning: Skipping polygon with < 3 points in cell '" << (cell->name ? cell->name : "Unnamed") << "'." << std::endl;
                        continue;
                    }

                    for (uint64_t k = 0; k < points.count; ++k) {
                        bg::append(boost_poly.outer(), point_type(points[k].x, points[k].y));
                    }
                    
                    // Ensure the polygon is closed for Boost.Geometry by adding the first point at the end
                    // if it's not already closed. GDSTK polygons are typically closed by definition.
                    // However, Boost.Geometry's intersection might behave better with explicitly closed polygons.
                    if (points.count > 0 && (points[0].x != points[points.count - 1].x || points[0].y != points[points.count - 1].y)) {
                         bg::append(boost_poly.outer(), point_type(points[0].x, points[0].y));
                    }
                    bg::correct(boost_poly); // Correct winding order if necessary for Boost.Geometry
                    loaded_layer.push_back(boost_poly);
                }
            }
        }
    }

    if (loaded_layer.empty()) {
        std::cerr << "Warning: No polygons loaded from " << filename << " for layer " << layer_number << ":" << datatype_number << std::endl;
    }

    lib.clear(); // Free memory allocated by GDSTK for the library contents
    return loaded_layer;
}

// Function to save a layer to an OASIS file
// Function to save a layer to an OASIS file
void save_layer_to_oasis(const layer_type& layer_to_save, const std::string& filename, int layer_number, int datatype_number = 0) {
    std::cout << "Saving " << layer_to_save.size() << " polygons to layer " << layer_number << ":" << datatype_number << " in " << filename << std::endl;

    gdstk::Library lib = {};
    lib.unit = 1e-6;      // Default unit (1 micron)
    lib.precision = 1e-9; // Default precision (1 nanometer)
    // lib.name = gdstk::copy_string("DEFAULT_LIB", NULL); // Optional: name the library

    gdstk::Cell* cell = (gdstk::Cell*)gdstk::allocate_clear(sizeof(gdstk::Cell));
    cell->name = gdstk::copy_string("RESULT_CELL", NULL); // It's good practice to name cells

    for (const auto& boost_poly : layer_to_save) {
        if (boost_poly.outer().empty()) {
            std::cerr << "Warning: Skipping an empty polygon during save." << std::endl;
            continue;
        }

        gdstk::Array<gdstk::Vec2> gdstk_points = {}; // Temporary array for points of one polygon
        // Reserve memory if a typical number of points is known, e.g., gdstk_points.ensure_slots(boost_poly.outer().size());
        
        for (const auto& pt : boost_poly.outer()) {
            gdstk_points.append({pt.x(), pt.y()});
        }

        // GDSTK expects polygons to not have the last point same as the first (it implies closure).
        // Boost polygons often do. If gdstk_points has this, remove the last point.
        if (gdstk_points.count > 1 && 
            gdstk_points[0].x == gdstk_points[gdstk_points.count - 1].x && 
            gdstk_points[0].y == gdstk_points[gdstk_points.count - 1].y) {
            gdstk_points.count--; // Effectively remove the last point if it closes the loop
        }

        if (gdstk_points.count < 3) {
             std::cerr << "Warning: Skipping polygon with < 3 unique points for OASIS output." << std::endl;
             gdstk_points.clear(); // free the points array
             continue;
        }

        gdstk::Polygon* gdstk_poly = (gdstk::Polygon*)gdstk::allocate_clear(sizeof(gdstk::Polygon));
        gdstk_poly->tag = gdstk::make_tag(layer_number, datatype_number);
        // gdstk_poly->point_array.copy_from(gdstk_points); // This copies the data
        // Or, if we want to transfer ownership of gdstk_points internal buffer (more advanced, ensure gdstk_points is not cleared later):
        gdstk_poly->point_array = gdstk_points; // This should assign the array structure, potentially sharing data or copying
                                              // For safety and clarity with gdstk_allocate_clear, a copy is safer if ownership is murky.
                                              // Let's re-verify gdstk behavior. A common pattern is to fill poly->point_array directly or ensure it takes ownership.
                                              // Given typical C library patterns, assigning the struct and then ensuring `gdstk_points` is not cleared (or only its container, not data) would work if point_array becomes owner.
                                              // However, `gdstk.h` shows `Array<T>` has its own `items` pointer. So `gdstk_poly->point_array.extend(gdstk_points)` or `copy_from` is better.
        
        // Let's use copy_from for safety, ensuring gdstk_poly has its own copy of the points.
        gdstk_poly->point_array.ensure_slots(gdstk_points.count); // Ensure capacity
        for(size_t i=0; i < gdstk_points.count; ++i) {
            gdstk_poly->point_array.append(gdstk_points[i]);
        }
        // After points are copied to gdstk_poly->point_array, the temporary gdstk_points can be cleared.
        gdstk_points.clear(); 

        cell->polygon_array.append(gdstk_poly); // Add the new polygon to the cell
    }

    lib.cell_array.append(cell);

    gdstk::ErrorCode error_code = lib.write_oas(filename.c_str(), 0, 0, 0);
    if (error_code != gdstk::ErrorCode::NoError) {
        std::cerr << "Error writing OASIS file: " << filename << " (Error code: " << (int)error_code << ")" << std::endl;
        // gdstk::print_error() could be useful here if available
    }

    // lib.clear() will free the cell, its name, polygons, and their point arrays.
    // It will also free the library name if it was set.
    lib.clear(); 
}

// Function to perform AND operation between mask and input layers (unchanged)
layer_type layer_and(const layer_type& mask_layer, const layer_type& input_layer) {
    layer_type result;
    for (const auto& mask_poly : mask_layer) {
        for (const auto& input_poly : input_layer) {
            std::vector<polygon_type> output; // intersection can produce multiple polygons
            try {
                bg::intersection(mask_poly, input_poly, output);
                result.insert(result.end(), output.begin(), output.end());
            } catch (const bg::exception& e) {
                std::cerr << "Boost.Geometry intersection error: " << e.what() << std::endl;
                // Potentially log problematic polygons or skip them
            }
        }
    }
    return result;
}

int main(int argc, char* argv[]) {
    if (argc != 7) {
        std::cerr << "Usage: " << argv[0] << " <mask_oasis_file> <mask_layer_num> <input_oasis_file> <input_layer_num> <output_oasis_file> <output_layer_num>" << std::endl;
        return 1;
    }

    try {
        std::string input_file = argv[1];
        int input_layer_num = std::stoi(argv[2]);
        std::string mask_file = argv[3];
        int mask_layer_num = std::stoi(argv[4]);
        std::string output_file = argv[5];
        int output_layer_num = std::stoi(argv[6]);

        // For now, datatype is hardcoded to 0 as per our generated files
        int default_datatype = 0;

        std::cout << "--- Configuration ---" << std::endl;
        std::cout << "Mask File: " << mask_file << ", Layer: " << mask_layer_num << std::endl;
        std::cout << "Input File: " << input_file << ", Layer: " << input_layer_num << std::endl;
        std::cout << "Output File: " << output_file << ", Layer: " << output_layer_num << std::endl;
        std::cout << "Datatype (fixed): " << default_datatype << std::endl;
        std::cout << "---------------------" << std::endl;

        // Load layers from OASIS files
        std::cout << "\n--- Loading Layers ---" << std::endl;
        layer_type mask_layer = load_layer_from_oasis(mask_file, mask_layer_num, default_datatype);
        layer_type input_layer = load_layer_from_oasis(input_file, input_layer_num, default_datatype);
        std::cout << "Mask layer loaded with " << mask_layer.size() << " polygons." << std::endl;
        std::cout << "Input layer loaded with " << input_layer.size() << " polygons." << std::endl;

        if (mask_layer.empty() || input_layer.empty()) {
            std::cerr << "Error: One or both input layers are empty. Cannot perform AND operation." << std::endl;
            // Save an empty output file or handle as an error
            layer_type empty_result;
            save_layer_to_oasis(empty_result, output_file, output_layer_num, default_datatype);
            std::cout << "Saved an empty result file." << std::endl;
            return 1; // Indicate an error or abnormal termination
        }

        // Perform AND operation
        std::cout << "\n--- Performing AND Operation ---" << std::endl;
        layer_type result_layer = layer_and(mask_layer, input_layer);
        std::cout << "AND operation resulted in " << result_layer.size() << " polygons." << std::endl;

        // Save the result layer to an OASIS file
        std::cout << "\n--- Saving Result Layer ---" << std::endl;
        save_layer_to_oasis(result_layer, output_file, output_layer_num, default_datatype);
        std::cout << "Result layer saved to " << output_file << std::endl;

    } catch (const std::invalid_argument& e) {
        std::cerr << "Error: Invalid layer number argument. Please provide integers." << std::endl;
        return 1;
    } catch (const std::out_of_range& e) {
        std::cerr << "Error: Layer number argument out of range." << std::endl;
        return 1;
    } catch (const std::exception& e) {
        std::cerr << "An unexpected error occurred: " << e.what() << std::endl;
        return 1;
    }

    std::cout << "\nProcessing finished." << std::endl;
    return 0;
}
