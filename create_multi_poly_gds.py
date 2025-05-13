import gdstk

# Create a library
lib = gdstk.Library(name='MultiPolyLib', unit=1e-6, precision=1e-9)

# Create a cell
cell = lib.new_cell('MultiPolyCell')

# Polygon 1: Square
points1 = [(0, 0), (10, 0), (10, 10), (0, 10)]
polygon1 = gdstk.Polygon(points1, layer=1, datatype=0)
cell.add(polygon1)

# Polygon 2: Rectangle (offset)
points2 = [(15, 0), (25, 0), (25, 5), (15, 5)]
polygon2 = gdstk.Polygon(points2, layer=2, datatype=1) # Different layer/datatype for variety
cell.add(polygon2)

# Polygon 3: Triangle (further offset)
points3 = [(0, 15), (5, 20), (0, 20)]
polygon3 = gdstk.Polygon(points3, layer=3, datatype=2)
cell.add(polygon3)

# Write to GDS file
output_filename = 'test_multi_poly.gds'
lib.write_gds(output_filename)
print(f"Saved {output_filename} with 3 polygons in cell 'MultiPolyCell'.")
