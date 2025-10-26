#include "slickdl/sensor.hpp"

#include <SDL3/SDL.h>

#include <vector>

#include "test.hpp"

using namespace slickdl::sensors;

struct sensorsTest : testing::Test {
    void SetUp() override {
        // initialize sensors subsystem (safe minimal choice for sensor APIs)
        ASSERT_TRUE( SDL_Init( SDL_INIT_SENSOR ) );
    }

    void TearDown() override { SDL_Quit(); }
};

TEST_F( sensorsTest, ConstantsAndEnumRoundtrip ) {
    // constant
    EXPECT_FLOAT_EQ( g_standardGravity, 9.80665f );

    // enum round-trip through legacy conversions
    EXPECT_EQ( type_t::invalid, fromLegacy( toLegacy( type_t::invalid ) ) );
    EXPECT_EQ( type_t::unknown, fromLegacy( toLegacy( type_t::unknown ) ) );
    EXPECT_EQ( type_t::accelerometer,
               fromLegacy( toLegacy( type_t::accelerometer ) ) );
    EXPECT_EQ( type_t::gyro, fromLegacy( toLegacy( type_t::gyro ) ) );
    EXPECT_EQ( type_t::accelerometerLeft,
               fromLegacy( toLegacy( type_t::accelerometerLeft ) ) );
    EXPECT_EQ( type_t::gyroLeft, fromLegacy( toLegacy( type_t::gyroLeft ) ) );
    EXPECT_EQ( type_t::accelerometerRight,
               fromLegacy( toLegacy( type_t::accelerometerRight ) ) );
    EXPECT_EQ( type_t::gyroRight, fromLegacy( toLegacy( type_t::gyroRight ) ) );
}

TEST_F( sensorsTest, AllDoesNotCrashAndMayBeEmpty ) {
    auto l_ids = id::all();
    // may be empty on many CI/dev machines. Just assert it returns a vector.
    EXPECT_TRUE( ( std::is_same_v< decltype( l_ids ), std::vector< id > > ));
}

TEST_F( sensorsTest, PerDeviceWrappersMatchRawIfPresent ) {
    auto l_ids = id::all();
    for ( auto l_id : l_ids ) {
        sensor_t l_sensor = l_id;

        // name: call raw API first to avoid wrapper assertions
        const char* l_rawName = SDL_GetSensorNameForID( l_id );
        if ( l_rawName ) {
            auto l_wrappedName = l_sensor.name();
            EXPECT_EQ( l_wrappedName, std::string_view( l_rawName ) );
        }

        // type: raw value might be SDL_SENSOR_INVALID (-1) for some ids; guard
        // it
        SDL_SensorType l_rawType = SDL_GetSensorTypeForID( l_id );
        if ( l_rawType != SDL_SENSOR_INVALID ) {
            auto l_wrappedType = l_sensor.type();
            EXPECT_EQ( l_wrappedType, fromLegacy( l_rawType ) );
        }

        // nonPortableType: raw returns -1 on failure; guard it
        int l_rawNonPortable = SDL_GetSensorNonPortableTypeForID( l_id );
        if ( l_rawNonPortable != -1 ) {
            auto l_wrappedNonPortable = l_sensor.nonPortableType();
            EXPECT_EQ( l_wrappedNonPortable, l_rawNonPortable );
        }

        // Try opening the sensor only if SDL_OpenSensor succeeds
        SDL_Sensor* l_rawHandle = SDL_OpenSensor( l_id );
        if ( l_rawHandle ) {
            // close raw handle immediately; we'll use wrapper::open to test
            // that path
            SDL_CloseSensor( l_rawHandle );

            EXPECT_EQ( l_id, l_sensor.id() ); // wrapper::id vs SDL_GetSensorID

            // properties may be 0 on some sensors; check raw first
            uint32_t l_rawProps = SDL_GetSensorProperties( l_sensor );
            if ( l_rawProps != 0 ) {
                auto l_wrappedProps = l_sensor.properties();
                EXPECT_EQ( l_wrappedProps, l_rawProps );
            }

            // test data reading only if raw API returns true for N = 3
            std::array< float, 3 > l_probe{};
            if ( SDL_GetSensorData( l_sensor, l_probe.data(),
                                    l_probe.size() ) ) {
                auto l_got = l_sensor.data< 3 >();
                EXPECT_EQ( l_got.size(), l_probe.size() );
                for ( size_t l_i = 0; l_i < l_probe.size(); ++l_i ) {
                    EXPECT_FLOAT_EQ( l_got.at( l_i ), l_probe.at( l_i ) );
                }
            }

#if 0
            // close via wrapper
            close( l_sensor );
#endif
        }
    }
}

TEST_F( sensorsTest, UpdateDoesNotCrash ) {
    // Should be safe to call regardless of devices
    update();
    SUCCEED();
}
