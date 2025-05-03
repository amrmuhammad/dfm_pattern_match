import klayout.db as pya

layout = pya.Layout()
top = layout.create_cell("TOP")
layer_index = layout.layer(10, 0)  # Layer 10, datatype 0

# Define grid parameters
num_x, num_y = 1000, 1000
rect_w, rect_h = 1000, 1000  # in database units (e.g., nm)
spacing_x, spacing_y = 1200, 1200

for ix in range(num_x):
    for iy in range(num_y):
        x = ix * spacing_x
        y = iy * spacing_y
        box = pya.Box(x, y, x + rect_w, y + rect_h)
        top.shapes(layer_index).insert(box)

layout.write("mask_grid.oas")

