#pragma once

#include <SDL3/SDL_render.h>

#include <gsl/pointers>

namespace slickdl {

using renderer_t = gsl::not_null< SDL_Renderer* >;
using texture_t = gsl::not_null< SDL_Texture* >;

} // namespace slickdl
