// image_processor.c - 图像处理器：左半黑白，右半红边
// 编译：gcc image_processor.c -o zabianyi.exe -Wall -lm
// 用法：zabianyi.exe <输入图片> [输出图片]

#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#ifdef _WIN32
#include <direct.h>  // Windows 的 _getcwd
#else
#include <unistd.h>  // Linux/Mac 的 getcwd
#endif

#define STB_IMAGE_IMPLEMENTATION
#include "libs/stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "libs/stb_image_write.h"

// Sobel算子
int sobel_x[3][3] = { {-1, 0, 1}, {-2, 0, 2}, {-1, 0, 1} };
int sobel_y[3][3] = { {-1, -2, -1}, {0, 0, 0}, {1, 2, 1} };

// RGB转灰度
unsigned char rgb_to_gray(unsigned char r, unsigned char g, unsigned char b) {
    return (unsigned char)(0.299 * r + 0.587 * g + 0.114 * b);
}

// Sobel边缘检测
unsigned char detect_edge(unsigned char* image, int x, int y, int width, int height, int channels) {
    int gx = 0, gy = 0;

    for (int ky = -1; ky <= 1; ky++) {
        for (int kx = -1; kx <= 1; kx++) {
            int px = x + kx;
            int py = y + ky;
            if (px < 0) px = 0;
            if (px >= width) px = width - 1;
            if (py < 0) py = 0;
            if (py >= height) py = height - 1;

            int idx = (py * width + px) * channels;
            unsigned char gray = rgb_to_gray(image[idx], image[idx + 1], image[idx + 2]);
            gx += gray * sobel_x[ky + 1][kx + 1];
            gy += gray * sobel_y[ky + 1][kx + 1];
        }
    }

    int magnitude = (int)sqrt((float)(gx * gx + gy * gy));
    if (magnitude > 255) magnitude = 255;
    return (unsigned char)magnitude;
}

// 显示帮助信息
void show_help(const char* program_name) {
    printf("========================================\n");
    printf("   图像处理器 - 左黑白右红边效果\n");
    printf("========================================\n\n");
    printf("用法: %s <输入图片> [输出图片]\n\n", program_name);
    printf("参数说明:\n");
    printf("  <输入图片>   必需，要处理的图片文件\n");
    printf("  [输出图片]   可选，处理后的图片文件名（默认: output.jpg）\n\n");
    printf("示例:\n");
    printf("  %s input.jpg              # 输出为 output.jpg\n", program_name);
    printf("  %s photo.png result.jpg   # 输出为 result.jpg\n", program_name);
    printf("  %s -h                     # 显示此帮助信息\n\n", program_name);
    printf("支持格式: JPG, PNG, BMP, TGA\n");
}

int main(int argc, char* argv[]) {
    // 检查命令行参数
    if (argc < 2 || strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0) {
        show_help(argv[0]);
        return 0;
    }

    // 获取输入输出文件名
    const char* input_path = argv[1];
    const char* output_path = (argc >= 3) ? argv[2] : "output.jpg";

    printf("========================================\n");
    printf("   图像处理器 - 左黑白右红边效果\n");
    printf("========================================\n\n");
    printf("输入文件: %s\n", input_path);
    printf("输出文件: %s\n", output_path);

    // 检查输入文件是否存在
    FILE* test_file = fopen(input_path, "rb");
    if (test_file == NULL) {
        printf("\n错误：找不到输入图片 '%s'\n", input_path);
        printf("请确保文件存在且路径正确。\n");
        printf("\n按回车键退出...");
        getchar();
        return 1;
    }
    fclose(test_file);

    int width, height, channels;

    // 加载图片
    printf("\n正在加载图片...\n");
    unsigned char* image = stbi_load(input_path, &width, &height, &channels, 0);
    if (image == NULL) {
        printf("错误：无法加载图片 %s\n", input_path);
        printf("可能的原因：图片格式不支持或文件已损坏。\n");
        printf("支持格式：JPG, PNG, BMP, TGA\n");
        printf("\n按回车键退出...");
        getchar();
        return 1;
    }

    printf("图片信息：%d x %d, %d 通道\n", width, height, channels);

    // 转换为RGB格式
    unsigned char* rgb_image = image;
    int need_free = 0;

    if (channels == 1) {
        printf("转换灰度图 -> RGB...\n");
        rgb_image = (unsigned char*)malloc(width * height * 3);
        need_free = 1;
        for (int i = 0; i < width * height; i++) {
            rgb_image[i * 3] = image[i];
            rgb_image[i * 3 + 1] = image[i];
            rgb_image[i * 3 + 2] = image[i];
        }
        channels = 3;
    }
    else if (channels == 4) {
        printf("转换RGBA -> RGB...\n");
        rgb_image = (unsigned char*)malloc(width * height * 3);
        need_free = 1;
        for (int i = 0; i < width * height; i++) {
            rgb_image[i * 3] = image[i * 4];
            rgb_image[i * 3 + 1] = image[i * 4 + 1];
            rgb_image[i * 3 + 2] = image[i * 4 + 2];
        }
        channels = 3;
    }

    // 创建输出图像
    unsigned char* output = (unsigned char*)malloc(width * height * 3);
    memcpy(output, rgb_image, width * height * 3);

    int mid_x = width / 2;
    int edge_threshold = 50;

    printf("\n处理中...\n");

    // 左半部分：黑白
    printf("  - 左半部分转为黑白...\n");
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < mid_x; x++) {
            int idx = (y * width + x) * 3;
            unsigned char gray = rgb_to_gray(rgb_image[idx], rgb_image[idx + 1], rgb_image[idx + 2]);
            output[idx] = gray;
            output[idx + 1] = gray;
            output[idx + 2] = gray;
        }
    }

    // 右半部分：边缘红边
    printf("  - 右半部分边缘检测并加红...\n");
    for (int y = 0; y < height; y++) {
        for (int x = mid_x; x < width; x++) {
            int idx = (y * width + x) * 3;
            unsigned char edge_strength = detect_edge(rgb_image, x, y, width, height, 3);
            if (edge_strength > edge_threshold) {
                output[idx] = 255;  // 红色增强
            }
        }
    }

    // 保存结果
    printf("\n保存结果...\n");
    int success = 0;
    const char* ext = strrchr(output_path, '.');

    if (ext != NULL && strcmp(ext, ".png") == 0) {
        success = stbi_write_png(output_path, width, height, 3, output, width * 3);
    }
    else {
        success = stbi_write_jpg(output_path, width, height, 3, output, 95);
    }

    if (success) {
        printf("\n✅ 处理完成！\n");
        printf("输出文件：%s\n", output_path);
    }
    else {
        printf("\n❌ 保存图片失败\n");
    }

    // 释放内存
    stbi_image_free(image);
    if (need_free) free(rgb_image);
    free(output);

    printf("\n按回车键退出...");
    getchar();
    return 0;
}