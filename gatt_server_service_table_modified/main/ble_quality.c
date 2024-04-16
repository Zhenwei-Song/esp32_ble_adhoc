#include "ble_quality.h"

// 计算的丢包率是 乘积项  跳数是相加项
// lin_quality（）函数返回的是链路质量
// Q R为超参数
double Q = 1; // 系统过程
double R = 1; // 测量
// double SNR = 0;
// double P = 1;
double percentage = 0.1;
// double base = 78.0L / 79.0L;

// 卡尔曼滤波器 对snr进行滤波   一维 初始化p=1 SNRhat初始化为0 P初始化为1
void Kalman(double *SNRhat, double SNR_Z, double *P, double Q, double R)
{

    // 获取测量值 SNR_Z
    // 计算先验估计值
    double SNR_hat = *SNRhat;
    // 计算先验协方差
    double P_ = *P + Q;
    // 计算卡尔曼增益
    double K = P_ / (P_ + R);
    // 计算后验估计值
    *SNRhat = SNR_hat + K * (SNR_Z - SNR_hat);
    // 更新估计误差协方差
    *P = (1 - K) * *P;
    //printf("SNRhat:%f\n", *SNRhat);
    //printf("P:%f\n", *P);
}

// 计算误码率
double calculate_ber(double SNR)
{
    // printf("calculate_ber SNR: %f\n", SNR);
    double ber = (pow(M_E, -SNR / 2)) / 2;
    // printf("calculate_ber ber: %f\n", ber);
    return ber;
}

// 计算误包率 假定包长是Nbit
double calculate_per(double SNR, double N)
{
    double ber = calculate_ber(SNR);
    //printf("calculate_per ber: %f SNR: %f\n", ber, SNR);
    double per = 1 - pow(((pow((1 - ber), 15)) + 15 * ber * pow((1 - ber), 14)), (double)(N) / 15.0);
    // printf("calculate_per per: %f\n", per);
    return per;
}
// m nakagaimi参数  N包长  snr w指的是时隙  n表示邻居个数
//  返回邻居节点的链路质量(只包含乘积项) 16bit
double bluetooth_prr_m(int m, int N, long double E_SNR, int n)
{
    // u_int16_t PRR = 0;
    double prr = 1 - bluetooth_per_approximate_m(N, E_SNR, n);
    // printf("blue prr: %Lf\n", prr);
    // PRR = prr * 65535;
    // printf("blue PRR: %d\n", PRR);
    return prr;
}

// 返回 乘积项和相加项计算得出的8bit链路质量量化值  数值越大 越好
uint16_t link_quality(u_int8_t hop, u_int16_t product_term)
{
    // 将per评级 获得PER
    // 将跳数
    // a和b是比重系数
    // printf("link_quality product_term:%d\n", product_term);

    float prr = (float)(product_term) / 65535;
    // printf("link_quality prr:%f\n", prr);
    float hop_ = 1 - (float)(hop) / MAX_HOP;
    float quality = percentage * prr + (1 - percentage) * hop_;
    // printf("link_quality quality:%f\n", quality);
    uint16_t quality_ = quality * 65535;
    // outfile << quality << endl;
    // printf("cal quality:%d\n", quality_);
    return quality_;
}
// 函数重载
// 返回到目的节点的乘积项  packet_product_term 为packet中的乘积项  neighbor_product_term为邻居表中的乘积项
u_int16_t bluetooth_prr_m_1(u_int16_t packet_product_term, u_int16_t neighbor_product_term)
{
    float f_packet_product_term = (float)(packet_product_term) / (float)(65535);
    //	outfile << "f_packet_product_term: " << f_packet_product_term << endl;
    float f_neighbor_product_term = (float)(neighbor_product_term) / (float)(65535);
    //	outfile << "f_neighbor_product_term: " << f_neighbor_product_term << endl;
    u_int16_t result = f_packet_product_term * f_neighbor_product_term * 65535;
    //	outfile << "result: " << result << endl;
    return result;
}

// 计算丢包率 此处的SNR是比值 不是db    w是单个分组所跨过时隙数 n是不同邻居微微网 可以理解为邻居节点的个数 N是单个分组内的bit数
// 计算的是乘积项
double bluetooth_per_approximate_m(int N, double E_SNR, int n)
{
    // double calculate_per(double SNR, double& per)
    double per = calculate_per(E_SNR, N);
    // printf("bluetooth_per_approximate_m per: %f\n", per);
    double prr_mac = calculate_prr_mac(n);
    // printf("bluetooth_per_approximate_m prr_mac: %f\n", prr_mac);
    double result = 1 - (1 - per) * prr_mac;
    // printf("bluetooth_per_approximate_m result: %f\n", result);
    return result;
}

// 仅考虑了 暴露终端 没有考虑隐藏终端的问题
double calculate_prr_mac(int n)
{
    double result1 = pow(0.5, n - 1);
    double prr_mac = pow(result1, 1);
    return prr_mac;
}

double calculate_snr(double signal_power, double noise_power)
{
    double snr;
    double signal_w = 1000 * pow(10, 0.1 * signal_power);
    double noise_w = 1000 * pow(10, 0.1 * noise_power);
    // printf("calculate_snr signal_power: %lf\n", signal_power);
    // printf("calculate_snr power: %lf\n", signal_w);
    // printf("calculate_snr noise_power: %lf\n", noise_power);
    //  确保噪声功率不为零，避免除以零错误
    if (noise_power != 0.0) {
        // 使用公式计算SNR，以分贝为单位
        snr = 10 * log10(signal_w / noise_w);
    }
    else {
        // 处理噪声功率为零的情况
        snr = INFINITY; // 此时SNR被定义为正无穷大
    }
    // printf("calculate_snr snr: %f\n", snr);
    return snr;
}