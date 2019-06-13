#include <cstdint>
#include <string>
#include <memory>
#include <fstream>
#include <iostream>
#include <complex>
#include <algorithm>
#include <vector>
#include <array>
#include <thread>
#include <numeric>
#include <iterator>

#pragma pack(2)

struct BitmapFileHeader {
    char header[2] {'B', 'M'};

    std::int32_t fileSize;
    std::int32_t reserved {0};
    std::int32_t dataOffset;
};

#pragma pack(2)

struct BitmapInfoHeader {
    std::int32_t headerSize {40};
    std::int32_t width;
    std::int32_t height;

    std::int16_t planes {1};
    std::int16_t bitsPerPixel {24};

    std::int32_t compression {0};
    std::int32_t dataSize{0};
    std::int32_t horizontalResolution {2400};
    std::int32_t verticalResolution {2400};
    std::int32_t colors {0};
    std::int32_t importantColors {0};
};



class Bitmap {
private:
    int m_width {0};
    int m_height {0};

    std::unique_ptr<uint8_t[]> m_pPixels {nullptr};

public:
    Bitmap(int width, int height);
    virtual ~Bitmap();

    bool write(std::string filename);
    void setPixel(
        int x, int y,
        std::uint8_t red, uint8_t green, uint8_t blue);
};


Bitmap::Bitmap(int width, int height)
    : m_width{width},
      m_height{height},
      m_pPixels{new uint8_t[width * height * 3]{}}
{

}

Bitmap::~Bitmap()
{

}

bool Bitmap::write(std::string filename)
{
    BitmapFileHeader fileHeader;

    fileHeader.fileSize =
        sizeof(BitmapFileHeader) +
        sizeof(BitmapInfoHeader) +
        m_width * m_height * 3;
    fileHeader.dataOffset =
        sizeof(BitmapFileHeader) +
        sizeof(BitmapInfoHeader);

    BitmapInfoHeader infoHeader;

    infoHeader.width = m_width;
    infoHeader.height = m_height;

    std::ofstream file;

    file.open(filename, std::ios::out|std::ios::binary);

    if (!file) {
        return false;
    }

    file.write(
        reinterpret_cast<char*>(&fileHeader),
        sizeof(fileHeader));
    file.write(
        reinterpret_cast<char*>(&infoHeader),
        sizeof(infoHeader));
    file.write(
        reinterpret_cast<char*>(m_pPixels.get()),
        m_width * m_height * 3);

    file.close();

    if (!file) {
        return false;
    }

    return true;
}

void Bitmap::setPixel(
    int x, int y,
    std::uint8_t red,
    std::uint8_t green,
    std::uint8_t blue)
{
    std::uint8_t *pPixel = m_pPixels.get();

    pPixel += (y * 3) * m_width + (x * 3);

    pPixel[0] = blue;
    pPixel[1] = green;
    pPixel[2] = red;
}



class Mandelbrot {
public:
    static constexpr int MAX_ITERATIONS {100};

public:
    Mandelbrot();
    virtual ~Mandelbrot();

    static int getIterations(double x, double y);
};


Mandelbrot::Mandelbrot() {}

Mandelbrot::~Mandelbrot() {}

int Mandelbrot::getIterations(double x, double y)
{
    std::complex<double> z {0};
    std::complex<double> c {x, y};

    int iterations {0};

    while(iterations < MAX_ITERATIONS) {
        z = (z * z) + c;

        if (std::abs(z) > 2) break;

        ++iterations;
    }

    return iterations;
}


#pragma pack(2)

struct Pixel {
    int x;
    int y;
    std::uint8_t red   {0};
    std::uint8_t green {0};
    std::uint8_t blue  {0};
};



int main()
{
    int const WIDTH = 800;
    int const HEIGHT = 600;

    std::vector<Pixel> pixels;

    for(int y=0; y<HEIGHT; ++y)
        for(int x=0; x<WIDTH; ++x) {
            Pixel pixel;
            pixel.x = x;
            pixel.y = y;
            pixels.push_back(pixel);
        }

    double min {999999};
    double max {-999999};

    std::mutex LOCK;

    std::unique_ptr<int[]> histogram
        (new int[Mandelbrot::MAX_ITERATIONS + 1]{0});

    auto pixelEval = [&](int x, int y) -> std::uint8_t {
        double xFractal {(x - WIDTH / 2.0 - 0) * (4.5 / HEIGHT) };
        double yFractal {(y - HEIGHT / 2.0) * (4.5 / HEIGHT) };

        int iterations = Mandelbrot::getIterations(xFractal, yFractal);

        LOCK.lock();
        histogram[iterations] ++;
        LOCK.unlock();

        std::uint8_t color =
            static_cast<std::uint8_t>(
                256 *
                static_cast<double>(iterations) /
                Mandelbrot::MAX_ITERATIONS);

        color = color * color * color;

        return color;
    };

    Bitmap bitmap {WIDTH, HEIGHT};

    unsigned int threadCount = std::thread::hardware_concurrency();
    int splitSize = pixels.size() / threadCount;

    auto tFunc = [&](int begin, int end) {
        for (auto it=pixels.begin() + begin;
             it!=pixels.begin() + end;
             ++it)
        {
            std::uint8_t color = pixelEval((*it).x, (*it).y);
            bitmap.setPixel((*it).x, (*it).y, color, color, 0);
        }
    };

    std::vector<std::thread> threads;
    for (int i=0; i<threadCount; ++i) {
        int begin = i * splitSize;
        int end = begin + splitSize - 1;
        std::cout<<"Begin: "<<begin
                 <<"End: "<<end
                 <<"\n";

        threads.push_back(
            std::thread(tFunc, begin, end));
    }

    for (std::thread &t : threads) {
        if (t.joinable())
            t.join();
    }

    int totalPixels {0};
    for (int i=0; i<Mandelbrot::MAX_ITERATIONS; ++i) {
        std::cout<<histogram[i]<<" "<<std::flush;
        totalPixels += histogram[i];
    }
    std::cout<<"\n\nIterations: "<<totalPixels<<"\n";
    std::cout<<"Width * height: "<<WIDTH*HEIGHT<<"\n";

    bitmap.write("test.bmp");

    return 0;
}







