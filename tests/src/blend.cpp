#include "slickdl/blend.hpp"

#include "slickdl/init_quit.hpp"
#include "test.hpp"

using namespace slickdl;

TEST( BlendTest, BlendEnumMapping ) {
    EXPECT_EQ( init::fromLegacy( static_cast< SDL_InitFlags >( 0 ) ),
               init::fromLegacy( static_cast< SDL_InitFlags >(
                   0 ) ) ); // silence potential unused include warnings

    // blend_t -> legacy -> back
    EXPECT_EQ( blend::none, fromLegacy( toLegacy( blend::none ) ) );
    EXPECT_EQ( blend::blend, fromLegacy( toLegacy( blend::blend ) ) );
    EXPECT_EQ( blend::blendPremultiplied,
               fromLegacy( toLegacy( blend::blendPremultiplied ) ) );
    EXPECT_EQ( blend::add, fromLegacy( toLegacy( blend::add ) ) );
    EXPECT_EQ( blend::addPremultiplied,
               fromLegacy( toLegacy( blend::addPremultiplied ) ) );
    EXPECT_EQ( blend::modulate, fromLegacy( toLegacy( blend::modulate ) ) );
    EXPECT_EQ( blend::multiply, fromLegacy( toLegacy( blend::multiply ) ) );
    EXPECT_EQ( blend::invalid, fromLegacy( toLegacy( blend::invalid ) ) );
}

TEST( BlendTest, BlendOperationMapping ) {
    EXPECT_EQ( blendOperation::add,
               fromLegacy( toLegacy( blendOperation::add ) ) );
    EXPECT_EQ( blendOperation::subtract,
               fromLegacy( toLegacy( blendOperation::subtract ) ) );
    EXPECT_EQ( blendOperation::revSubtract,
               fromLegacy( toLegacy( blendOperation::revSubtract ) ) );
    EXPECT_EQ( blendOperation::minimum,
               fromLegacy( toLegacy( blendOperation::minimum ) ) );
    EXPECT_EQ( blendOperation::maximum,
               fromLegacy( toLegacy( blendOperation::maximum ) ) );
}

TEST( BlendTest, BlendFactorMapping ) {
    EXPECT_EQ( blendFactor::zero, fromLegacy( toLegacy( blendFactor::zero ) ) );
    EXPECT_EQ( blendFactor::one, fromLegacy( toLegacy( blendFactor::one ) ) );
    EXPECT_EQ( blendFactor::sourceColor,
               fromLegacy( toLegacy( blendFactor::sourceColor ) ) );
    EXPECT_EQ( blendFactor::oneMinusSourceColor,
               fromLegacy( toLegacy( blendFactor::oneMinusSourceColor ) ) );
    EXPECT_EQ( blendFactor::sourceAlpha,
               fromLegacy( toLegacy( blendFactor::sourceAlpha ) ) );
    EXPECT_EQ( blendFactor::oneMinusSourceAlpha,
               fromLegacy( toLegacy( blendFactor::oneMinusSourceAlpha ) ) );
    EXPECT_EQ( blendFactor::destinationColor,
               fromLegacy( toLegacy( blendFactor::destinationColor ) ) );
    EXPECT_EQ(
        blendFactor::oneMinusDestinationColor,
        fromLegacy( toLegacy( blendFactor::oneMinusDestinationColor ) ) );
    EXPECT_EQ( blendFactor::destinationAlpha,
               fromLegacy( toLegacy( blendFactor::destinationAlpha ) ) );
    EXPECT_EQ(
        blendFactor::oneMinusDestinationAlpha,
        fromLegacy( toLegacy( blendFactor::oneMinusDestinationAlpha ) ) );
}

TEST( BlendTest, ComposeCustomBlendModeMatchesSDL ) {
    // Choose a few representative combinations and ensure wrapper equals SDL
    {
        auto l_wrapper = composeCustomBlendMode(
            blendFactor::one, blendFactor::zero, blendOperation::add,
            blendFactor::one, blendFactor::zero, blendOperation::add );
        auto l_legacy = SDL_ComposeCustomBlendMode(
            toLegacy( blendFactor::one ), toLegacy( blendFactor::zero ),
            toLegacy( blendOperation::add ), toLegacy( blendFactor::one ),
            toLegacy( blendFactor::zero ), toLegacy( blendOperation::add ) );
        EXPECT_EQ( toLegacy( l_wrapper ), l_legacy );
    }

    {
        auto l_wrapper = composeCustomBlendMode(
            blendFactor::sourceAlpha, blendFactor::oneMinusSourceAlpha,
            blendOperation::subtract, blendFactor::destinationColor,
            blendFactor::oneMinusDestinationColor,
            blendOperation::revSubtract );
        auto l_legacy = SDL_ComposeCustomBlendMode(
            toLegacy( blendFactor::sourceAlpha ),
            toLegacy( blendFactor::oneMinusSourceAlpha ),
            toLegacy( blendOperation::subtract ),
            toLegacy( blendFactor::destinationColor ),
            toLegacy( blendFactor::oneMinusDestinationColor ),
            toLegacy( blendOperation::revSubtract ) );
        EXPECT_EQ( toLegacy( l_wrapper ), l_legacy );
    }

    {
        // premultiplied style
        auto l_wrapper = composeCustomBlendMode(
            blendFactor::one, blendFactor::one, blendOperation::add,
            blendFactor::one, blendFactor::one, blendOperation::add );
        auto l_legacy = SDL_ComposeCustomBlendMode(
            toLegacy( blendFactor::one ), toLegacy( blendFactor::one ),
            toLegacy( blendOperation::add ), toLegacy( blendFactor::one ),
            toLegacy( blendFactor::one ), toLegacy( blendOperation::add ) );
        EXPECT_EQ( toLegacy( l_wrapper ), l_legacy );
    }
}
