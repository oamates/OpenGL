#ifndef SIMD_LEVEL_H
#error Dont include this file without defining SIMD_LEVEL_H
#else
#define FASTNOISE_SIMD_CLASS2(x) FastNoiseSIMD_L##x
#define FASTNOISE_SIMD_CLASS(level) FASTNOISE_SIMD_CLASS2(level)

namespace FastNoiseSIMD_internal
{
	class FASTNOISE_SIMD_CLASS(SIMD_LEVEL_H) : public FastNoiseSIMD
	{
	public:
		// Do not call this, use SetSIMDLevel(int) to have NewFastNoiseSIMD() return the level you want
		FASTNOISE_SIMD_CLASS(SIMD_LEVEL_H)(int seed = 1337);

		static float* GetEmptySet(int size);
		static int AlignedSize(int size);

		void FillSampledNoiseSet(float* noiseSet, int xStart, int yStart, int zStart, int xSize, int ySize, int zSize, int sampleScale) override;
		void FillSampledNoiseSet(float* noiseSet, FastNoiseVectorSet* vectorSet, float xOffset = 0.0f, float yOffset = 0.0f, float zOffset = 0.0f) override;

		void FillWhiteNoiseSet(float* floatSet, int xStart, int yStart, int zStart, int xSize, int ySize, int zSize, float scaleModifier = 1.0f) override;
		void FillWhiteNoiseSet(float* noiseSet, FastNoiseVectorSet* vectorSet, float xOffset = 0.0f, float yOffset = 0.0f, float zOffset = 0.0f) override;

		void FillValueSet(float* floatSet, int xStart, int yStart, int zStart, int xSize, int ySize, int zSize, float scaleModifier = 1.0f) override;
		void FillValueFractalSet(float* floatSet, int xStart, int yStart, int zStart, int xSize, int ySize, int zSize, float scaleModifier = 1.0f) override;
		void FillValueSet(float* noiseSet, FastNoiseVectorSet* vectorSet, float xOffset = 0.0f, float yOffset = 0.0f, float zOffset = 0.0f) override;
		void FillValueFractalSet(float* noiseSet, FastNoiseVectorSet* vectorSet, float xOffset = 0.0f, float yOffset = 0.0f, float zOffset = 0.0f) override;

		void FillPerlinSet(float* floatSet, int xStart, int yStart, int zStart, int xSize, int ySize, int zSize, float scaleModifier = 1.0f) override;
		void FillPerlinFractalSet(float* floatSet, int xStart, int yStart, int zStart, int xSize, int ySize, int zSize, float scaleModifier = 1.0f) override;
		void FillPerlinSet(float* noiseSet, FastNoiseVectorSet* vectorSet, float xOffset = 0.0f, float yOffset = 0.0f, float zOffset = 0.0f) override;
		void FillPerlinFractalSet(float* noiseSet, FastNoiseVectorSet* vectorSet, float xOffset = 0.0f, float yOffset = 0.0f, float zOffset = 0.0f) override;

		void FillSimplexSet(float* floatSet, int xStart, int yStart, int zStart, int xSize, int ySize, int zSize, float scaleModifier = 1.0f) override;
		void FillSimplexFractalSet(float* floatSet, int xStart, int yStart, int zStart, int xSize, int ySize, int zSize, float scaleModifier = 1.0f) override;
		void FillSimplexSet(float* noiseSet, FastNoiseVectorSet* vectorSet, float xOffset = 0.0f, float yOffset = 0.0f, float zOffset = 0.0f) override;
		void FillSimplexFractalSet(float* noiseSet, FastNoiseVectorSet* vectorSet, float xOffset = 0.0f, float yOffset = 0.0f, float zOffset = 0.0f) override;

		void FillCellularSet(float* floatSet, int xStart, int yStart, int zStart, int xSize, int ySize, int zSize, float scaleModifier = 1.0f) override;
		void FillCellularSet(float* noiseSet, FastNoiseVectorSet* vectorSet, float xOffset = 0.0f, float yOffset = 0.0f, float zOffset = 0.0f) override;
	
	};
}
#undef SIMD_LEVEL_H
#endif
