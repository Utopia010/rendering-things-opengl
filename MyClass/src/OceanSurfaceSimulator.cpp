#include "OceanSurfaceSimulator.hpp"

#include <algorithm>
#include <cmath>
#include <complex>
#include <limits>
#include <numbers>
#include <random>
#include <stdexcept>
#include <utility>

namespace {
using Complex = std::complex<double>;

bool isPowerOfTwo(std::size_t value) {
  return value >= 4 && (value & (value - 1)) == 0;
}

// Inverse FFT: positive exponent, deliberately without 1/N normalization.
// Equation (36) is a sum rather than an average, so an unnormalized transform
// maps its Fourier coefficients directly to the sampled height field.
void inverseFft1D(std::vector<Complex> &values) {
  const std::size_t count = values.size();
  for (std::size_t i = 1, j = 0; i < count; ++i) {
    std::size_t bit = count >> 1;
    while ((j & bit) != 0) {
      j ^= bit;
      bit >>= 1;
    }
    j ^= bit;
    if (i < j) {
      std::swap(values[i], values[j]);
    }
  }

  for (std::size_t blockSize = 2; blockSize <= count; blockSize <<= 1) {
    const double angle =
        2.0 * std::numbers::pi / static_cast<double>(blockSize);
    const Complex root = std::polar(1.0, angle);
    for (std::size_t start = 0; start < count; start += blockSize) {
      Complex weight{1.0, 0.0};
      for (std::size_t offset = 0; offset < blockSize / 2; ++offset) {
        const Complex even = values[start + offset];
        const Complex odd = weight * values[start + offset + blockSize / 2];
        values[start + offset] = even + odd;
        values[start + offset + blockSize / 2] = even - odd;
        weight *= root;
      }
    }
  }
}

// This function performs the sum in equation (36). Although there are N*M
// input modes and N*M output points, no mode belongs to one output point:
// every output point receives contributions from *all* input modes. The FFT
// evaluates that full sum efficiently via transforms along z and then x.
// The flat layout is [x][z] in both domains.
void inverseFft2D(std::vector<Complex> &values, std::size_t samplesX,
                  std::size_t samplesZ) {
  std::vector<Complex> line(std::max(samplesX, samplesZ));
  for (std::size_t x = 0; x < samplesX; ++x) {
    line.resize(samplesZ);
    for (std::size_t z = 0; z < samplesZ; ++z) {
      line[z] = values[x * samplesZ + z];
    }
    inverseFft1D(line);
    for (std::size_t z = 0; z < samplesZ; ++z) {
      values[x * samplesZ + z] = line[z];
    }
  }
  for (std::size_t z = 0; z < samplesZ; ++z) {
    line.resize(samplesX);
    for (std::size_t x = 0; x < samplesX; ++x) {
      line[x] = values[x * samplesZ + z];
    }
    inverseFft1D(line);
    for (std::size_t x = 0; x < samplesX; ++x) {
      values[x * samplesZ + z] = line[x];
    }
  }
}

double signedWaveNumber(std::size_t index, std::size_t count, double length) {
  // FFT indices N/2 ... N-1 represent negative spatial frequencies.
  const auto signedIndex = index < count / 2
                               ? static_cast<std::ptrdiff_t>(index)
                               : static_cast<std::ptrdiff_t>(index) -
                                     static_cast<std::ptrdiff_t>(count);
  return 2.0 * std::numbers::pi * static_cast<double>(signedIndex) / length;
}

void requireFinitePositive(double value, const char *name) {
  if (!std::isfinite(value) || value <= 0.0) {
    throw std::invalid_argument(name);
  }
}

void requireFiniteNonnegative(double value, const char *name) {
  if (!std::isfinite(value) || value < 0.0) {
    throw std::invalid_argument(name);
  }
}
} // namespace

OceanSurfaceSimulator::OceanSurfaceSimulator(OceanConfig config)
    : config_(std::move(config)),
      waveVectors_(config_.samplesX * config_.samplesZ),
      frequencies_(config_.samplesX * config_.samplesZ),
      initialAmplitudes_(config_.samplesX * config_.samplesZ),
      coefficients_(config_.samplesX * config_.samplesZ),
      heights_(config_.samplesX, std::vector<float>(config_.samplesZ)),
      slopesX_(config_.samplesX, std::vector<float>(config_.samplesZ)),
      slopesZ_(config_.samplesX, std::vector<float>(config_.samplesZ)) {
  if (!isPowerOfTwo(config_.samplesX) || !isPowerOfTwo(config_.samplesZ)) {
    throw std::invalid_argument(
        "samplesX and samplesZ must be powers of two >= 4");
  }
  requireFinitePositive(config_.lengthX, "lengthX must be positive and finite");
  requireFinitePositive(config_.lengthZ, "lengthZ must be positive and finite");
  requireFinitePositive(config_.gravity, "gravity must be positive and finite");
  requireFiniteNonnegative(config_.windSpeed,
                           "windSpeed must be nonnegative and finite");
  requireFiniteNonnegative(config_.spectrumScale,
                           "spectrumScale must be nonnegative and finite");
  requireFiniteNonnegative(config_.shortWaveCutoff,
                           "shortWaveCutoff must be nonnegative and finite");
  if (!std::isfinite(config_.windX) || !std::isfinite(config_.windZ) ||
      (config_.windSpeed > 0.0 &&
       std::hypot(config_.windX, config_.windZ) == 0.0)) {
    throw std::invalid_argument(
        "wind direction must be finite and nonzero when windSpeed > 0");
  }
  if (!std::isfinite(config_.waterDepth) ||
      !std::isfinite(config_.capillaryLength) ||
      !std::isfinite(config_.loopPeriod)) {
    throw std::invalid_argument("optional physical parameters must be finite");
  }

  // Equation (42): draw one complex Gaussian for every spatial Fourier mode.
  // The random engine lives across all draws; re-seeding for each mode would
  // accidentally give every mode the same random value.
  std::mt19937_64 generator(config_.seed);
  std::normal_distribution<double> normal(0.0, 1.0);
  for (std::size_t modeX = 0; modeX < config_.samplesX; ++modeX) {
    for (std::size_t modeZ = 0; modeZ < config_.samplesZ; ++modeZ) {
      const std::size_t index = flatIndex(modeX, modeZ);
      const WaveVector k{
          signedWaveNumber(modeX, config_.samplesX, config_.lengthX),
          signedWaveNumber(modeZ, config_.samplesZ, config_.lengthZ)};
      waveVectors_[index] = k;
      frequencies_[index] = angularFrequency(std::hypot(k.x, k.z));
      const double power = phillipsSpectrum(k);
      const double scale = std::sqrt(power / 2.0);
      initialAmplitudes_[index] =
          scale * Complex{normal(generator), normal(generator)};
    }
  }
  update(0.0);
}

void OceanSurfaceSimulator::update(double time) {
  if (!std::isfinite(time)) {
    throw std::invalid_argument("time must be finite");
  }

  std::vector<Complex> heightSpectrum(coefficients_.size());
  std::vector<Complex> slopeXSpectrum(coefficients_.size());
  std::vector<Complex> slopeZSpectrum(coefficients_.size());
  const Complex imaginaryUnit{0.0, 1.0};

  // Each (modeX, modeZ) identifies one whole-surface wave basis.
  // It does not identify a single physical surface point.
  for (std::size_t modeX = 0; modeX < config_.samplesX; ++modeX) {
    for (std::size_t modeZ = 0; modeZ < config_.samplesZ; ++modeZ) {
      const std::size_t index = flatIndex(modeX, modeZ);
      const std::size_t oppositeX =
          (config_.samplesX - modeX) % config_.samplesX;
      const std::size_t oppositeZ =
          (config_.samplesZ - modeZ) % config_.samplesZ;
      const WaveVector k = waveVectors_[index];
      const Complex forwardPhase = std::polar(1.0, frequencies_[index] * time);

      // Equation (43): the opposite mode's conjugate is needed so that
      // h(-k,t) = conj(h(k,t)) and the reconstructed height is real.
      coefficients_[index] =
          initialAmplitudes_[index] * forwardPhase +
          std::conj(initialAmplitudes_[flatIndex(oppositeX, oppositeZ)]) *
              std::conj(forwardPhase);
      heightSpectrum[index] = coefficients_[index];

      // Equation (37): differentiating exp(i k.x) multiplies by i*k.
      slopeXSpectrum[index] = imaginaryUnit * k.x * coefficients_[index];
      slopeZSpectrum[index] = imaginaryUnit * k.z * coefficients_[index];
    }
  }

  // These inverse FFTs perform equation (36)'s all-modes-to-all-points sum.
  // The first transforms the height coefficients; the other two transform
  // the differentiated coefficients from equation (37).
  inverseFft2D(heightSpectrum, config_.samplesX, config_.samplesZ);
  inverseFft2D(slopeXSpectrum, config_.samplesX, config_.samplesZ);
  inverseFft2D(slopeZSpectrum, config_.samplesX, config_.samplesZ);
  for (std::size_t gridX = 0; gridX < config_.samplesX; ++gridX) {
    for (std::size_t gridZ = 0; gridZ < config_.samplesZ; ++gridZ) {
      const std::size_t index = flatIndex(gridX, gridZ);
      heights_[gridX][gridZ] = static_cast<float>(heightSpectrum[index].real());
      slopesX_[gridX][gridZ] = static_cast<float>(slopeXSpectrum[index].real());
      slopesZ_[gridX][gridZ] = static_cast<float>(slopeZSpectrum[index].real());
    }
  }
}

const FloatGrid &OceanSurfaceSimulator::heights() const noexcept {
  return heights_;
}

const FloatGrid &OceanSurfaceSimulator::slopesX() const noexcept {
  return slopesX_;
}

const FloatGrid &OceanSurfaceSimulator::slopesZ() const noexcept {
  return slopesZ_;
}

const OceanConfig &OceanSurfaceSimulator::config() const noexcept {
  return config_;
}

WaveVector OceanSurfaceSimulator::waveVector(std::size_t modeX,
                                             std::size_t modeZ) const {
  if (modeX >= config_.samplesX || modeZ >= config_.samplesZ) {
    throw std::out_of_range("waveVector index is outside the Fourier grid");
  }
  return waveVectors_[flatIndex(modeX, modeZ)];
}

std::complex<double>
OceanSurfaceSimulator::coefficient(std::size_t modeX, std::size_t modeZ) const {
  if (modeX >= config_.samplesX || modeZ >= config_.samplesZ) {
    throw std::out_of_range("coefficient index is outside the Fourier grid");
  }
  return coefficients_[flatIndex(modeX, modeZ)];
}

std::size_t
OceanSurfaceSimulator::flatIndex(std::size_t xIndex,
                                 std::size_t zIndex) const noexcept {
  return xIndex * config_.samplesZ + zIndex;
}

double OceanSurfaceSimulator::angularFrequency(double waveNumber) const {
  if (waveNumber == 0.0) {
    return 0.0;
  }
  const double capillary = waveNumber * config_.capillaryLength;
  const double depthFactor = config_.waterDepth > 0.0
                                 ? std::tanh(waveNumber * config_.waterDepth)
                                 : 1.0;
  double frequency = std::sqrt(config_.gravity * waveNumber *
                               (1.0 + capillary * capillary) * depthFactor);
  if (config_.loopPeriod > 0.0) {
    const double base = 2.0 * std::numbers::pi / config_.loopPeriod;
    frequency = std::floor(frequency / base) * base; // equation (35)
  }
  return frequency;
}

double OceanSurfaceSimulator::phillipsSpectrum(WaveVector k) const {
  const double waveNumber = std::hypot(k.x, k.z);
  if (waveNumber == 0.0 || config_.windSpeed == 0.0 ||
      config_.spectrumScale == 0.0) {
    return 0.0;
  }
  const double windLength =
      config_.windSpeed * config_.windSpeed / config_.gravity;
  const double windMagnitude = std::hypot(config_.windX, config_.windZ);
  const double alignment = (k.x * config_.windX + k.z * config_.windZ) /
                           (waveNumber * windMagnitude);
  const double kL = waveNumber * windLength;
  const double shortScale = waveNumber * config_.shortWaveCutoff;
  return config_.spectrumScale * std::exp(-1.0 / (kL * kL)) *
         (alignment * alignment) * std::exp(-shortScale * shortScale) /
         std::pow(waveNumber, 4.0);
}
