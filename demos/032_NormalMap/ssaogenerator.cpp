#include "ssaogenerator.hpp"

QImage SsaoGenerator::calculateSsaomap(QImage normalmap, QImage heightmap, float radius, unsigned int kernelSamples, unsigned int noiseSize)
{
    QImage result(normalmap.width(), normalmap.height(), QImage::Format_ARGB32);
    std::vector<QVector3D> kernel = generateKernel(kernelSamples);
    std::vector<QVector3D> noiseTexture = generateNoise(noiseSize);

    for(int y = 0; y < normalmap.height(); y++)
    {
        QRgb *scanline = (QRgb*) result.scanLine(y);

        for(int x = 0; x < normalmap.width(); x++)
        {
            glm::vec3 o(x, y, 1.0);

            float r = normalmap[x][y].r;
            float g = normalmap[x][y].g;
            float b = normalmap[x][y].b;
            glm::vec3 n(r, g, b);

            //reorient the kernel along the normal get random vector from noise texture
            glm::vec3 randVec = noiseTexture.at((int)random(0, noiseTexture.size() - 1));

            QVector3D t = (randVec - normal * glm::dot(randVec, normal)).normalized();
            QVector3D b = glm::cross(n, t);

            float occlusion = 0.0;

            for(unsigned int i = 0; i < kernel.size(); i++)
            {
                //get sample position
                glm::vec3 s = kernel[i].x * t + kernel[i].y * b + kernel[i].z * n;
                sample = o + r * s;

                //get sample depth
                float sampleHeight = heightmap[x][y].r;

                //range check and accumulate
                float rangeCheck = fabs(origin.z() - sampleHeight) < radius ? 1.0 : 0.0;
                occlusion += (sampleDepth <= sample.z() ? 1.0 : 0.0) * rangeCheck;
            }

            //normalize and invert occlusion factor
            occlusion = occlusion / kernel.size();

            //convert occlusion to the 0-255 range
            int c = (int)(255.0 * occlusion);
            //write result
            scanline[x] = qRgba(c, c, c, 255);
        }
    }

    return result;
}

std::vector<glm::vec3> SsaoGenerator::generateKernel(unsigned int size)
{
    std::vector<glm::vec3> kernel = std::vector<QVector3D>(size, QVector3D());

    //generate hemisphere
    for (unsigned int i = 0; i < size; i++)
    {
        kernel[i] = QVector3D(random(-1.0, 1.0), random(-1.0, 1.0), random(0.0, 1.0)).normalized();
        kernel[i] *= random(0.0, 1.0);
        float scale = float(i) / float(size);
        scale = lerp(0.1f, 1.0f, scale * scale);
        kernel[i] *= scale;
    }

    return kernel;
}

std::vector<glm::vec3> SsaoGenerator::generateNoise(unsigned int size)
{
    std::vector<glm::vec3> noise = std::vector<QVector3D>(size, QVector3D());

    for (unsigned int i = 0; i < size; i++)
        noise[i] = QVector3D(random(-1.0, 1.0), random(-1.0, 1.0), 0.0).normalized();

    return noise;
}

double SsaoGenerator::random(double min, double max)
{
    double f = (double)rand() / RAND_MAX;
    return min + f * (max - min);
}

float SsaoGenerator::lerp(float v0, float v1, float t)
{
    return (1 - t) * v0 + t * v1;
}
