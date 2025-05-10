import gdstk
import time

def create_mask_layer_oasis(filename="mask_layer.oas", layer_num=1, datatype_num=0):
    '''Creates an OASIS file with a single large rectangle for the mask layer.'''
    print(f"Creating {filename}...")
    lib = gdstk.Library()
    cell = lib.new_cell("MASK_CELL")

    # Define a large rectangle
    # Using large coordinates to simulate a somewhat "huge" dimension
    mask_rect = gdstk.rectangle((0, 0), (100000, 100000), layer=layer_num, datatype=datatype_num)
    cell.add(mask_rect)

    lib.write_oas(filename)
    print(f"{filename} created successfully.")

def create_input_layer_oasis(filename="input_layer.oas", layer_num=2, datatype_num=0, grid_dim=100, square_size=500, grid_origin_x=50000, grid_origin_y=50000):
    '''
    Creates an OASIS file with a grid of many small squares for the input layer.
    A grid_dim of 100 means 100x100 = 10,000 polygons.
    Spacing between squares is implicitly defined by their origin.
    If square_size is 500, and they are placed at intervals of 1000 (i.e. next_origin = origin + 1000),
    there will be a 500 unit gap.
    Here, we'll make them placed at square_size * 2 intervals for some spacing.
    '''
    print(f"Creating {filename}...")
    lib = gdstk.Library()
    cell = lib.new_cell("INPUT_CELL")

    # Calculate spacing to be equal to square_size, so total step is 2 * square_size
    step_size = square_size * 2

    polygons = []
    for i in range(grid_dim):
        for j in range(grid_dim):
            x_start = grid_origin_x + i * step_size
            y_start = grid_origin_y + j * step_size
            square = gdstk.rectangle(
                (x_start, y_start),
                (x_start + square_size, y_start + square_size),
                layer=layer_num,
                datatype=datatype_num
            )
            polygons.append(square)
    
    cell.add(*polygons)
    print(f"Added {len(polygons)} polygons to {filename}.")

    lib.write_oas(filename)
    print(f"{filename} created successfully.")

if __name__ == "__main__":
    start_time = time.time()

    # Mask layer parameters
    mask_filename = "mask_layer.oas"
    mask_layer_num = 1
    
    # Input layer parameters
    input_filename = "input_layer.oas"
    input_layer_num = 2
    
    # Parameters to make it "huge" but still manageable for a first test
    # 200x200 grid = 40,000 polygons.
    # If this is too slow or files too big, reduce grid_dim.
    # If too small, increase grid_dim.
    input_grid_dim = 200  # Creates grid_dim * grid_dim polygons
    input_square_size = 100 # Microns or DB units
    # Position the input grid to overlap with the mask (0,0) to (100000, 100000)
    # Let mask be (0,0) to (100k, 100k)
    # Let input squares be 100x100 units.
    # Let input grid start at 50k, 50k. Grid of 200 squares, step 200.
    # So input goes from 50k to 50k + 200*200 = 50k + 40k = 90k. This ensures overlap.
    input_grid_origin_x = 50000 
    input_grid_origin_y = 50000

    print("Starting OASIS file generation...")
    create_mask_layer_oasis(filename=mask_filename, layer_num=mask_layer_num)
    create_input_layer_oasis(
        filename=input_filename,
        layer_num=input_layer_num,
        grid_dim=input_grid_dim,
        square_size=input_square_size,
        grid_origin_x=input_grid_origin_x,
        grid_origin_y=input_grid_origin_y
    )

    end_time = time.time()
    print(f"Finished generating OASIS files in {end_time - start_time:.2f} seconds.")
    print(f"Files generated in: /home/amrmuhammad/dev/dfm_pattern_match/")
    print(f"Mask file: {mask_filename} (Layer {mask_layer_num})")
    print(f"Input file: {input_filename} (Layer {input_layer_num}, {input_grid_dim*input_grid_dim} polygons)")
    print("\nTo run this script, activate the virtual environment first:")
    print("  cd /home/amrmuhammad/dev/dfm_pattern_match")
    print("  source .venv/bin/activate")
    print("  python generate_oasis_test_files.py")
