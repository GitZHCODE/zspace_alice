#include "zSMin.h"

namespace zSpace {

    void zSMin::smin(
        const zScalarArray& a,
        const zScalarArray& b,
        zScalarArray& result,
        float k,
        MODE mode)
    {
        for (size_t i = 0; i < result.size(); i++)
        {
            float ai = a[i];
            float bi = b[i];

            switch (mode)
            {
            case MODE::min:
                // hard minimum
                result[i] = (ai < bi) ? ai : bi;
                break;

            case MODE::exponential:
                result[i] = smin_exponential(ai, bi, k);
                break;

            case MODE::root:
                result[i] = smin_root(ai, bi, k);
                break;

            case MODE::sigmoid:
                result[i] = smin_sigmoid(ai, bi, k);
                break;

            case MODE::polynomial_quadratic:
                result[i] = smin_polyQuadratic(ai, bi, k);
                break;

            case MODE::polynomial_cubic:
                result[i] = smin_polyCubic(ai, bi, k);
                break;

            case MODE::polynomial_quartic:
                result[i] = smin_polyQuartic(ai, bi, k);
                break;

            case MODE::circular:
                result[i] = smin_circular(ai, bi, k);
                break;

            case MODE::circular_geometrical:
                result[i] = smin_circularGeometrical(ai, bi, k);
                break;

            default:
                result[i] = (ai < bi) ? ai : bi;
                break;
            }
        }
    }

    void zSMin::smin_multiple(
        const std::vector<zScalarArray>& inputs,
        zScalarArray& result,
        float k,
        MODE mode)
    {
        result = inputs[0];

        for (size_t i = 1; i < inputs.size(); i++)
        {
            smin(result, inputs[i], result, k, mode);
        }
    }

    void zSMin::smin_exponential_weighted(
        const zScalarArray& a,
        const zScalarArray& b,
        zScalarArray& result,
        float k,
        float wt
    )
    {
        if (a.size() != b.size()) return;

        result.resize(a.size());

        for (size_t i = 0; i < a.size(); i++)
        {
            float valA = a[i];
            float valB = b[i];

            result[i] = smin_exponential_wt(valA, valB, k, wt);
        }
    }

    // SMIN Helper Functions implementations
    //-------------------------------------------------------------

    // Exponential
    inline float zSMin::smin_exponential(float a, float b, float k)
    {
        // from the article: k *= 1.0; (implicitly the same)
        float r = exp2(-a / k) + exp2(-b / k);
        return -k * log2(r);
    }

    inline float zSMin::smin_exponential_wt(float a, float b, float k, float wt)
    {
        //   (1-wt)*exp(-a/k) + wt*exp(-b/k)
        float termA = (1.0f - wt) * exp2(-a / k);
        float termB = wt * exp2(-b / k);
        float r = termA + termB;

        // Avoid log(0)
        if (r < 1e-14f)
        {
            // Return something large negative or handle underflow
            return -1e6f;
        }

        // Weighted exponential SMin formula:
        //    -k * log2( (1-wt)*exp2(-a/k) + wt*exp2(-b/k) )
        return -k * log2(r);
    }

    // Root
    inline float zSMin::smin_root(float a, float b, float k)
    {
        k *= 2.0f;
        float x = b - a;
        return 0.5f * (a + b - sqrtf(x * x + k * k));
    }

    // Sigmoid
    inline float zSMin::smin_sigmoid(float a, float b, float k)
    {
        k *= logf(2.0f);
        float x = b - a;
        return a + x / (1.0f - exp2(x / k));
    }

    // Quadratic Polynomial
    inline float zSMin::smin_polyQuadratic(float a, float b, float k)
    {
        k *= 4.0f;
        float h = max(k - fabs(a - b), 0.0f) / k;
        return std::min(a, b) - h * h * k * 0.25f; // (1.0/4.0)
    }

    // Cubic Polynomial
    inline float zSMin::smin_polyCubic(float a, float b, float k)
    {
        k *= 6.0f;
        float h = max(k - fabs(a - b), 0.0f) / k;
        return std::min(a, b) - h * h * h * k * (1.0f / 6.0f);
    }

    // Quartic Polynomial
    inline float zSMin::smin_polyQuartic(float a, float b, float k)
    {
        k *= (16.0f / 3.0f);
        float h = max(k - fabs(a - b), 0.0f) / k;
        return std::min(a, b) - h * h * h * (4.0f - h) * k * (1.0f / 16.0f);
    }

    // Circular
    inline float zSMin::smin_circular(float a, float b, float k)
    {
        k *= 1.0f / (1.0f - sqrtf(0.5f));
        float h = max(k - fabs(a - b), 0.0f) / k;
        return std::min(a, b)
            - k * 0.5f * (1.0f + h - sqrtf(1.0f - h * (h - 2.0f)));
    }

    // Circular Geometrical
    inline float zSMin::smin_circularGeometrical(float a, float b, float k)
    {
        k *= 1.0f / (1.0f - sqrtf(0.5f));
        float mAB = std::min(a, b);
        float dx = max(k - a, 0.0f);
        float dy = max(k - b, 0.0f);
        float l = sqrtf(dx * dx + dy * dy);

        return max(k, mAB) - l;
    }

} 