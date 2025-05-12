set logging on
set logging file gdb_zoom_debug.log
set logging overwrite on

# Break on key zoom-related functions
break LayoutWidget::mousePressEvent
break LayoutWidget::mouseMoveEvent
break LayoutWidget::mouseReleaseEvent
break LayoutWidget::paintEvent

# Detailed commands for each breakpoint
commands 1
    printf "Mouse Press Event:\n"
    print m_interaction_mode
    print m_is_zooming
    print m_zoom_start_pos
    print m_pan_offset
    print m_zoom_factor
    print m_polygons_to_draw.size()
    continue
end

commands 2
    printf "Mouse Move Event:\n"
    print m_interaction_mode
    print m_is_zooming
    print m_zoom_start_pos
    print m_zoom_current_pos
    print m_pan_offset
    print m_zoom_factor
    continue
end

commands 3
    printf "Mouse Release Event:\n"
    print m_interaction_mode
    print m_is_zooming
    print m_zoom_start_pos
    print m_zoom_current_pos
    print m_pan_offset
    print m_zoom_factor
    continue
end

commands 4
    printf "Paint Event:\n"
    print m_interaction_mode
    print m_is_zooming
    print m_pan_offset
    print m_zoom_factor
    print m_polygons_to_draw.size()
    continue
end

# Run the program
run input_layer.oas
