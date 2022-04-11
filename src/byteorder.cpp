#include <iostream>

union endian {
    uint16_t n;
    uint8_t  nn[2];
};


int main() {

    endian data;
    data.n = 1042;

    std::cout <<"First byte: " << static_cast<unsigned short>(data.nn[1]) << std::endl;



    return 0;
}