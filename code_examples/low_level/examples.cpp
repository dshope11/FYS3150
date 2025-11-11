//
// Created by Nils Enric Canut Taugbøl on 10/11/2025.
//
#include "precomp.h"

// we set it as volatile to avoid Dead Code Elimination (DCE)
// by setting it to volatile we tell the compiler not to eliminate it even if it is not used directly in the final results of the present code
static volatile double volatile_res;

int main() {
    std::random_device rd;
    std::mt19937 gen(rd());

    std::uniform_real_distribution massDistribution(MIN_MASS, MAX_MASS);
    std::uniform_real_distribution posXDistribution((float) SCRNWIDTH, (float) SCRNWIDTH);
    std::uniform_real_distribution posYDistribution((float) SCRNHEIGHT, (float) SCRNHEIGHT);

    Timer t;
    int N = 1000000;

    std::vector<arma::vec2> distances(N);
    std::vector<double> GM(N);
    for (size_t i = 0; i < N; ++i) {
        distances[i]  = arma::vec2{posXDistribution(gen) - posXDistribution(gen), posYDistribution(gen) - posYDistribution(gen)};
        GM[i] = G * massDistribution(gen);
    }

    // warm-up cache
    double warmUpSum = 0.0;
    for (int i = 0; i < N; ++i) {

        double r2 = distances[i](0) * distances[i](0) + distances[i](1) * distances[i](1);
        double r = std::sqrt(r2);
        double gmWreciprocal = GM[i] / (r * r2);

        arma::vec2 acceleration = {distances[i](0) * gmWreciprocal, distances[i](1) * gmWreciprocal};

        warmUpSum += acceleration(0) + 0.5*acceleration(1);  // keep the compiler on its toes with hard-to-predict operations
    }
    volatile_res = warmUpSum;

    // Baseline
    double baseSum = 0.0;
    double sum = 0.0;
    double baselineTime = 0.0;
    t.reset();
    for (int i = 0; i < N; ++i) {

        arma::vec2 acc{0,0};

        double r2 = distances[i](0) * distances[i](0) + distances[i](1) * distances[i](1);
        double r = std::sqrt(r2);
        double gmWreciprocal = GM[i] / (r2 * r);

        acc(0) += distances[i](0) * gmWreciprocal;
        acc(1) += distances[i](1) * gmWreciprocal;

        sum += acc(0) + 0.5*acc(1);  // keep the compiler on its toes with hard-to-predict operations
    }
    baselineTime += t.elapsed();
    volatile_res = sum;
    baseSum = sum/N;

    // Cost of a sqrt vs multiplication
    double costSum = 0.0;
    sum = 0.0;
    double costTime = 0.0;
    t.reset();
    for (int i = 0; i < N; ++i) {

        arma::vec2 acc{0,0};

        double r2 = distances[i](0) * distances[i](0) + distances[i](1) * distances[i](1);
        // double r = r2 * 3.; //std::sqrt(r2);
        double gmWreciprocal = GM[i] / (0.25 * r2);

        acc(0) += distances[i](0) * gmWreciprocal;
        acc(1) += distances[i](1) * gmWreciprocal;

        sum += acc(0) + 0.5*acc(1);  // keep the compiler on its toes with hard-to-predict operations
    }
    costTime += t.elapsed();
    volatile_res = sum;
    costSum = sum/N;

    // Braided2
    double braidSum = 0.0;
    sum = 0.0;
    double braidTime = 0.0;
    t.reset();
    for (int i = 0; i+1 < N; i+=2) {

        arma::vec2 acc_1{0,0};
        arma::vec2 acc_2{0,0};

        double r2_1 = distances[i](0) * distances[i](0) + distances[i](1) * distances[i](1);
        double r2_2 = distances[i+1](0) * distances[i+1](0) + distances[i+1](1) * distances[i+1](1);

        double r_1 = std::sqrt(r2_1);
        double r_2 = std::sqrt(r2_2);

        double gmWreciprocal_1 = GM[i] / (r2_1 * r_1);
        double gmWreciprocal_2 = GM[i+1] / (r2_2 * r_2);

        acc_1(0) += distances[i](0) * gmWreciprocal_1;
        acc_1(1) += distances[i](1) * gmWreciprocal_1;
        acc_2(0) += distances[i+1](0) * gmWreciprocal_2;
        acc_2(1) += distances[i+1](1) * gmWreciprocal_2;

        sum += acc_1(0) + 0.5*acc_1(1);  // keep the compiler on its toes with hard-to-predict operations
        sum += 0.5*acc_2(0) + acc_2(1);

    }
    braidTime += t.elapsed();
    volatile_res = sum;
    braidSum = sum/N;

    // Braided4 - with a reduction of fetching
    double braidSmartSum = 0.0;
    sum = 0.0;
    double braidSmartTime = 0.0;
    t.reset();
    for (int i = 0; i+3 < N; i+=4) {

        arma::vec2 acc_1{0,0};
        arma::vec2 acc_2{0,0};
        arma::vec2 acc_3{0,0};
        arma::vec2 acc_4{0,0};

        double dx_1 = distances[i](0);
        double dy_1 = distances[i](1);
        double dx_2 = distances[i+1](0);
        double dy_2 = distances[i+1](1);
        double dx_3 = distances[i+2](0);
        double dy_3 = distances[i+2](1);
        double dx_4 = distances[i+3](0);
        double dy_4 = distances[i+3](1);

        double r2_1 = dx_1 * dx_1 + dy_1 * dy_1;
        double r2_2 = dx_2 * dx_2 + dy_2 * dy_2;
        double r2_3 = dx_3 * dx_3 + dy_3 * dy_3;
        double r2_4 = dx_4 * dx_4 + dy_4 * dy_4;

        double r_1 = std::sqrt(r2_1);
        double r_2 = std::sqrt(r2_2);
        double r_3 = std::sqrt(r2_3);
        double r_4 = std::sqrt(r2_4);

        double gmWreciprocal_1 = GM[i] / (r2_1 * r_1);
        double gmWreciprocal_2 = GM[i+1] / (r2_2 * r_2);
        double gmWreciprocal_3 = GM[i+1] / (r2_3 * r_3);
        double gmWreciprocal_4 = GM[i+1] / (r2_4 * r_4);

        acc_1(0) += dx_1 * gmWreciprocal_1;
        acc_1(1) += dy_1 * gmWreciprocal_1;
        acc_2(0) += dx_2 * gmWreciprocal_2;
        acc_2(1) += dy_2 * gmWreciprocal_2;
        acc_3(0) += dx_3 * gmWreciprocal_3;
        acc_3(1) += dy_3 * gmWreciprocal_3;
        acc_4(0) += dx_4 * gmWreciprocal_4;
        acc_4(1) += dy_4 * gmWreciprocal_4;

        sum += acc_1(0) + 0.5*acc_1(1);  // keep the compiler on its toes with hard-to-predict operations
        sum += 0.5*acc_2(0) + acc_2(1);
        sum += 0.5*acc_3(0) + acc_3(1);
        sum += acc_4(0) + 0.25*acc_4(1);
    }
    braidSmartTime += t.elapsed();
    volatile_res = sum;
    braidSmartSum = sum/N;

    // sanity: print checksuns so compiler can’t elide, and you can sanity-check equality
    std::printf("Arithmetic:\n  baseline  ns=%8.6f  checksum=%.8e\n"
                "  simplified ns=%8.6f  checksum=%.8e  speedup=%.2fx\n",
                baselineTime, baseSum, costTime, costSum, baselineTime / costTime);

    std::printf("ILP:\n  scalar    ns=%8.6f  checksum=%.8e\n"
                "  braided2  ns=%8.6f  checksum=%.8e  speedup=%.2fx\n",
                baselineTime, baseSum, braidTime, braidSum, baselineTime / braidTime);

    std::printf("ILP:\n  scalar    ns=%8.6f  checksum=%.8e\n"
                "  braided4 with reduced fetching  ns=%8.6f  checksum=%.8e  speedup=%.2fx\n",
                baselineTime, baseSum, braidSmartTime, braidSmartSum, baselineTime / braidSmartTime);

    return (volatile_res == 0.12345) ? 0 : 1; // touch volatile to keep compiler honest
}