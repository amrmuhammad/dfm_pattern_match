break LayoutWidget::mousePressEvent
break LayoutWidget::mouseMoveEvent
break LayoutWidget::mouseReleaseEvent
break LayoutWidget::paintEvent

commands 1
    print m_interaction_mode
    print m_is_zooming
    print m_zoom_start_pos
    continue
end

commands 2
    print m_interaction_mode
    print m_is_zooming
    print m_zoom_start_pos
    print m_zoom_current_pos
    continue
end

commands 3
    print m_interaction_mode
    print m_is_zooming
    print m_zoom_start_pos
    print m_zoom_current_pos
    print m_pan_offset
    print m_zoom_factor
    continue
end

commands 4
    print m_interaction_mode
    print m_is_zooming
    print m_polygons_to_draw.size()
    print m_zoom_factor
    print m_pan_offset
    continue
end
