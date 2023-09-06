#include <filesystem>
#include <fstream>
#include <cstdlib>

#include <CLI/CLI.hpp>
#include <libraw/libraw.h>
#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <spdlog/spdlog.h>

namespace fs = std::filesystem;

void convert_raw_to_bgr(const cv::Mat &raw_image, cv::Mat &bgr8_image, int depth, uint8_t bayer_pattern, uint32_t black_level = 0) {
    spdlog::debug("convert_raw_to_bgr(_, _, {}, {}, {})", depth, bayer_pattern, black_level);
    LibRaw libraw;
    auto *output_params = libraw.output_params_ptr();
    output_params->use_auto_wb = 1;
    output_params->no_auto_bright = 0;
    output_params->user_qual = 0;
    output_params->output_bps = 8;

    uint8_t raw_bits_per_pixel = depth <= 8 ? 8 : 16;
    uint8_t raw_effective_bits_per_pixel = depth;
    uint32_t unused_bits = raw_bits_per_pixel - raw_effective_bits_per_pixel;
    uint32_t datalen = raw_image.total() * (raw_bits_per_pixel / 8);

    bgr8_image.create(raw_image.rows, raw_image.cols, CV_8UC3);

    spdlog::debug("libraw.open_bayer(_, {}, {}, {}, _, _, _, _, _, {}, {}, _, {})", datalen, raw_image.cols, raw_image.rows, bayer_pattern, unused_bits, black_level);
    libraw.open_bayer(
        reinterpret_cast<unsigned char*>(raw_image.data),
        datalen,
        static_cast<uint16_t>(raw_image.cols), static_cast<uint16_t>(raw_image.rows),
        0, 0, 0, 0, // margins left top right bottom
        0, // procflags
        bayer_pattern,
        unused_bits,
        0, // otherflags
        black_level);
    libraw.unpack();

    libraw.dcraw_process();

    int width, height, colors, bpp;
    libraw.get_mem_image_format(&width, &height, &colors, &bpp);
    if (colors != 3 || bpp != 8 || width != raw_image.cols || height != raw_image.rows)
        throw std::runtime_error("Bad conversion");

    libraw.copy_mem_image(bgr8_image.data, bgr8_image.step, 1);
    libraw.recycle();
}


int main(int argc, char** argv) {
    CLI::App app("libraw_stripes");

    const std::map<std::string, spdlog::level::level_enum> map_log_level{
        {"trace", spdlog::level::trace}, {"debug", spdlog::level::debug}, {"info", spdlog::level::info},
        {"warn", spdlog::level::warn},   {"err", spdlog::level::err},     {"critical", spdlog::level::critical},
        {"off", spdlog::level::off}};
    spdlog::level::level_enum log_level{spdlog::level::info};
    app.add_option("--log", log_level, "Logging level")
        ->transform(CLI::CheckedTransformer(map_log_level, CLI::ignore_case));

    fs::path input;
    app.add_option("-i,--input", input,
                  "Path to the RAW image (must be a 3072x4096 uint16_t binary)")
        ->check(CLI::ExistingFile);

    fs::path output = "image.png";
    app.add_option("-o,--output", output, "Path to the output image")
        ->check(CLI::NonexistentPath);

    uint32_t depth = 10;
    app.add_option("-d,--depth", depth, "Depth of the raw");

    uint32_t black_level = 0;
    app.add_option("-b,--black_level", black_level, "Black level of the raw");

    const std::map<std::string, uint8_t> map_bayer_pattern{
        {"rggb", LIBRAW_OPENBAYER_RGGB}, {"bggr", LIBRAW_OPENBAYER_BGGR}, {"grbg", LIBRAW_OPENBAYER_GRBG},
        {"gbrg", LIBRAW_OPENBAYER_GBRG}};
    uint8_t bayer_pattern;
    app.add_option("-p,--bayer_pattern", bayer_pattern, "Bayer pattern of the raw")
        ->transform(CLI::CheckedTransformer(map_bayer_pattern, CLI::ignore_case));

    CLI11_PARSE(app, argc, argv);

    spdlog::set_level(log_level);
    // Making sure we save into png
    output.replace_extension(".png");

    spdlog::info("Reading RAW {} and saving it in {}", input.string(), output.string());

    cv::Mat input_raw(3072, 4096, CV_16UC1);
    auto input_raw_memsize = input_raw.total() * input_raw.elemSize();
    std::ifstream ifs(input, std::ifstream::binary);
    if (ifs) {
        ifs.seekg(0, ifs.end);
        int file_size = ifs.tellg();
        ifs.seekg(0, ifs.beg);

        if (file_size != input_raw_memsize) {
            auto msg = fmt::format("Error in reading {} (memsize is {}), file size is {}", input.string(), input_raw_memsize, file_size);
            spdlog::error(msg);
            throw std::invalid_argument(msg);
        }

        ifs.read(reinterpret_cast<char*>(input_raw.data), file_size);
        ifs.close();
    }

    static const uint32_t I = 5;
    static const uint32_t J = 5;
    spdlog::debug("Checking image values from (0, 0) to ({}, {})", I, J);
    for (uint32_t i = 0; i < I; ++i) {
        const auto& input_raw_i = input_raw.rowRange(i, i+1);
        const auto& input_raw_i_J = input_raw_i.colRange(0, J);
        spdlog::debug("{}", fmt::join(input_raw_i_J.begin<uint16_t>(), input_raw_i_J.end<uint16_t>(), "\t"));
    }

    cv::Mat output_raw;
    convert_raw_to_bgr(input_raw, output_raw, depth, bayer_pattern, black_level);

    cv::imwrite(output.string(), output_raw);
}
