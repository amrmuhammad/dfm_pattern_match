import gdstk

# Create a library
lib = gdstk.Library(name='MySimpleLib', unit=1e-6, precision=1e-9) # Standard units

# Create a cell
cell = lib.new_cell('SIMPLE_CELL')

# Define points for a simple rectangle (e.g., 10x20 microns)
# Points should be (x, y) tuples or lists
points = [(0, 0), (10, 0), (10, 20), (0, 20)]

# Create the polygon
polygon = gdstk.Polygon(points, layer=1, datatype=0)

# Add the polygon to the cell
cell.add(polygon)

# Add the cell to the library (not strictly necessary if only one cell, but good practice)
# lib.add(cell) # Already done by new_cell if library is specified

# Save the library to an OASIS file
lib.write_oas('simple_test.oas')

print("simple_test.oas created successfully.")
