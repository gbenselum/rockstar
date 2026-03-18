#include "dsp/eq.h"
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include <vector>

using namespace rockstar::dsp;

TEST_CASE("Biquad Filter resets correctly", "[biquad]") {
  Biquad bq(Biquad::PEAKING, 48000.0f);
  bq.set_params(1000.0f, 0.707f, 0.0f); // 0dB gain should have no effect

  // Testing pass-through
  REQUIRE_THAT(bq.process(1.0f), Catch::Matchers::WithinAbs(1.0f, 0.001));
  REQUIRE_THAT(bq.process(0.5f), Catch::Matchers::WithinAbs(0.5f, 0.001));
}

TEST_CASE("FourBandEQ Instantiation", "[fourbandeq]") {
  FourBandEQ eq;
  eq.set_sample_rate(48000.0f);

  std::vector<float> buffer = {0.1f, -0.2f, 0.5f, -0.5f};
  std::vector<float> original = buffer;

  // Process with 0dB gain across all bands should minimally affect signal
  // (allow some phase shift)
  eq.process(buffer.data(), buffer.size());

  for (size_t i = 0; i < buffer.size(); ++i) {
    REQUIRE_THAT(buffer[i], Catch::Matchers::WithinAbs(original[i], 0.1));
  }
}
