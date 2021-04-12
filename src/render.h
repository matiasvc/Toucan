#pragma once

#include <list>

#include "internal.h"

namespace Toucan {

void draw_figure_2d_list(std::list<Figure2D>& figures, ToucanContext& context);
void draw_figure_3d_list(std::list<Figure3D>& figures, ToucanContext& context);
void draw_figure_input_list(std::list<FigureInput>& figures, ToucanContext& context);

} // namespace Toucan
