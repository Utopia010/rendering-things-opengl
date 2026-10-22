#pragma once

#include <complex>
#include <cstddef>
#include <cstdint>
#include <vector>

// Horizontal coordinates use x and z; y is the vertical height.
struct OceanConfig {
    std::size_t samplesX = 64;
    std::size_t samplesZ = 64;
    double lengthX = 100.0;       // metres
    double lengthZ = 100.0;       // metres
    double gravity = 9.81;        // m/s^2
    double windSpeed = 12.0;      // m/s
    double windX = 1.0;           // horizontal direction; normalized internally
    double windZ = 0.0;
    double spectrumScale = 1e-7;  // Phillips spectrum's A, not a wave amplitude
    double shortWaveCutoff = 0.5; // ell in equation (41), metres
    double waterDepth = 0.0;      // <= 0 means deep water
    double capillaryLength = 0.0; // <= 0 disables surface tension
    double loopPeriod = 0.0;      // <= 0 means no frequency quantization
    std::uint64_t seed = 2026;
};

struct WaveVector {
    double x;
    double z;
};

// [x][z] sampled values. Fourier coefficients remain separate complex arrays.
using FloatGrid = std::vector<std::vector<float>>;

class OceanSurfaceSimulator {
public:
    explicit OceanSurfaceSimulator(OceanConfig config);

    // Evaluate the same random realization at any time t, in seconds.
    // No previous update is needed: update(10) and then update(0) is valid.
    void update(double time);

    [[nodiscard]] const FloatGrid& heights() const noexcept;
    [[nodiscard]] const FloatGrid& slopesX() const noexcept;
    [[nodiscard]] const FloatGrid& slopesZ() const noexcept;
    [[nodiscard]] const OceanConfig& config() const noexcept;

    // These indices select Fourier modes, not positions on the output grid.
    [[nodiscard]] WaveVector waveVector(std::size_t modeX, std::size_t modeZ) const;
    [[nodiscard]] std::complex<double> coefficient(std::size_t modeX,
                                                   std::size_t modeZ) const;

private:
    [[nodiscard]] std::size_t flatIndex(std::size_t xIndex, std::size_t zIndex) const noexcept;
    [[nodiscard]] double angularFrequency(double waveNumber) const;
    [[nodiscard]] double phillipsSpectrum(WaveVector waveVector) const;

    OceanConfig config_;
    std::vector<WaveVector> waveVectors_;              // fixed k for each Fourier mode
    std::vector<double> frequencies_;                  // fixed omega(k), equations (31)-(35)
    std::vector<std::complex<double>> initialAmplitudes_; // h_0(k), equation (42)
    std::vector<std::complex<double>> coefficients_;      // h(k,t), equation (43)
    FloatGrid heights_;
    FloatGrid slopesX_;
    FloatGrid slopesZ_;
};
